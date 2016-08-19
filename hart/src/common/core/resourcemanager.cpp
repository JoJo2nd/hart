/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/core/resourcemanager.h"
#include "hart/base/std.h"
#include "hart/base/filesystem.h"
#include "hart/base/atomic.h"
#include "hart/fbs/resourcedb_generated.h"
#include "hart/base/mutex.h"

namespace hart {
namespace resourcemanager {
namespace hfb = hart::fb;

struct Resource {
    resid_t uuid;
    uint32_t typecc = 0;          // The four CC code
    hfb::ResourceInfo const* info = nullptr;
    hstd::unique_ptr<uint8_t> loadtimeData;
    void* runtimeData = nullptr;     //
    hatomic::aint32_t refCount = 0; // Only valid when runtimeData is !nullptr (or resource system is loading runtime data. Need extra flag?)
};

struct LoadRequest {
    LoadRequest() = default;
    LoadRequest(resid_t a, uint64_t c) : uuid(a), transaction(c) {}
    resid_t uuid;
    uint64_t transaction = 0;
};

enum class ResourceLoadState {
    OpenFile, OpenFileWait,
    ReadFile, ReadFileWait,
    LoadResource, LoadNext,
    Waiting,
};

static struct {
    hMutex access;
    hstd::unique_ptr<uint8_t> resourcedb;
    hfb::ResourceList const* resourceListings;
    hstd::unordered_map<resid_t, Resource> resources;
    hstd::vector<LoadRequest> loadQueue;
    hstd::vector<LoadRequest> unloadQueue;
    uint64_t transactions = 0;
    ResourceLoadState resState = ResourceLoadState::Waiting;
    hfs::FileHandle fileHdl;
    hfs::FileOpHandle fileOp;
} ctx;

bool initialise() {
    hfs::FileHandle res_file;
    hfs::FileOpHandle op_hdl = hfs::openFile("/resourcedb.bin", hfs::Mode::Read, &res_file);
    if (hfs::fileOpWait(op_hdl) != hfs::Error::Ok)
        return false;

    hfs::FileStat stat;
    op_hdl = hfs::fstatAsync(res_file, &stat);
    if (hfs::fileOpWait(op_hdl) != hfs::Error::Ok)
        return false;

    ctx.resourcedb.reset(new uint8_t[stat.filesize]);
    op_hdl = hfs::freadAsync(res_file, ctx.resourcedb.get(), stat.filesize, 0);
    if (hfs::fileOpWait(op_hdl) != hfs::Error::Ok)
        return false;

    ctx.resourceListings = hfb::GetResourceList(ctx.resourcedb.get());

    // Alloc space for all handles upfront 
    auto const* asset_uuids = ctx.resourceListings->assetUUIDs();
    auto const* asset_infos = ctx.resourceListings->assetInfos();
    for (uint32_t i = 0, n = asset_uuids->size(); i < n; ++i) {
        resid_t id;
        id.words[0] = (*asset_uuids)[i]->highword3();
        id.words[1] = (*asset_uuids)[i]->highword2();
        id.words[2] = (*asset_uuids)[i]->highword1();
        id.words[3] = (*asset_uuids)[i]->lowword();
        Resource& res = ctx.resources[id];
        res.uuid = id;
        res.info = (*asset_infos)[i];
        hdbprintf("uuid: %x-%x-%x-%x ", id.words[0], id.words[1], id.words[2], id.words[3]);
        hdbprintf("friendly name: %s filepath: %s\n", (*asset_infos)[i]->friendlyName()->c_str(), (*asset_infos)[i]->filepath()->c_str());
    }

    hfs::closeFile(res_file);

    return true;    
}

void update() {
    hScopedMutex sentry(&ctx.access);

    // Process load queue
    if ((ctx.resState == ResourceLoadState::Waiting && ctx.loadQueue.size() > 0) || ctx.resState == ResourceLoadState::OpenFile) {
        LoadRequest& lr = ctx.loadQueue.front();
        Resource& res = ctx.resources[lr.uuid];
        if (hatomic::atomicGet(res.refCount) == 0) {
            // Need to load
            ctx.fileOp = hfs::openFile(res.info->filepath()->c_str(), hfs::Mode::Read, &ctx.fileHdl);
            ctx.resState = ResourceLoadState::OpenFileWait;
        } else {
            // just ++ the ref count
            hatomic::increment(res.refCount);
            ctx.resState = ResourceLoadState::LoadNext;
        }
    } else if (ctx.resState == ResourceLoadState::OpenFileWait) {
        hfs::Error er = hfs::fileOpComplete(ctx.fileOp);
        if (er == hfs::Error::Pending)
            return;
        if (er != hfs::Error::Ok) {
            hfs::closeFile(ctx.fileHdl);
            ctx.resState = ResourceLoadState::OpenFile; // Try again!
            return;
        }

        LoadRequest& lr = ctx.loadQueue.front();
        Resource& res = ctx.resources[lr.uuid];
        if (!res.loadtimeData)
            res.loadtimeData.reset(new uint8_t[res.info->filesize()]);
        ctx.fileOp = hfs::freadAsync(ctx.fileHdl, res.loadtimeData.get(), res.info->filesize(), 0);
        ctx.resState = ResourceLoadState::ReadFileWait;
    } else if (ctx.resState == ResourceLoadState::ReadFileWait) {
        hfs::Error er =  hfs::fileOpComplete(ctx.fileOp);
        if (er == hfs::Error::Pending)
            return;
        if (er != hfs::Error::Ok) {
            hfs::closeFile(ctx.fileHdl);
            ctx.resState = ResourceLoadState::OpenFile; // Try again!
            return;
        }

        LoadRequest& lr = ctx.loadQueue.front();
        Resource& res = ctx.resources[lr.uuid];
        ctx.resState = ResourceLoadState::LoadResource;
        //TODO: actually load data
        hatomic::increment(res.refCount);
        res.loadtimeData.reset();
        ctx.resState = ResourceLoadState::LoadNext;
    }

    // finished loading a resource, remove and carry on to whatevers next.
    if (ctx.resState == ResourceLoadState::LoadNext) {
        ctx.loadQueue.erase(ctx.loadQueue.begin());
        ctx.resState = ResourceLoadState::Waiting;
    }

    //TODO: if load queue is done, process unload queue.
    if (ctx.loadQueue.size() == 0 && ctx.unloadQueue.size() > 0) {
        //TODO:
    }
}

void shutdown() {

}

static void loadResourceInternal(resid_t res_id) {
    auto const* asset_uuids = ctx.resourceListings->assetUUIDs();
    auto const* asset_infos = ctx.resourceListings->assetInfos();
    
    // Push the prerequisites first
    Resource const& res = ctx.resources[res_id];
    auto const* prerequisites = res.info->prerequisites();
    for (uint32_t i = 0, n = prerequisites->size(); i < n; ++i) {
        uint32_t ridx = (*prerequisites)[i];
        resid_t id;
        id.words[0] = (*asset_uuids)[ridx]->highword3();
        id.words[1] = (*asset_uuids)[ridx]->highword2();
        id.words[2] = (*asset_uuids)[ridx]->highword1();
        id.words[3] = (*asset_uuids)[ridx]->lowword();
        loadResourceInternal(id); 
    }

    // Loads are handled in order so push this request after the prerequisites
    ctx.loadQueue.emplace_back(res_id, ctx.transactions);
}

Handle loadResource(resid_t res_id) {
    hScopedMutex sentry(&ctx.access);
    ++ctx.transactions;

    loadResourceInternal(res_id);

    Handle r;
    r.id = res_id;
    r.data = (void const*)&ctx.resources[res_id];
    return r;
}

static void unloadResourceInternal(resid_t res_id) {
    auto const* asset_uuids = ctx.resourceListings->assetUUIDs();
    auto const* asset_infos = ctx.resourceListings->assetInfos();
    
    // Unloads are handled in order so push this request before its prerequisites
    ctx.unloadQueue.emplace_back(res_id, ctx.transactions);

    // Now the resource dependent on the prerequisites is gone, unload the prerequisites
    Resource const& res = ctx.resources[res_id];
    auto const* prerequisites = res.info->prerequisites();
    for (uint32_t i = 0, n = prerequisites->size(); i < n; ++i) {
        uint32_t ridx = (*prerequisites)[i];
        resid_t id;
        id.words[0] = (*asset_uuids)[ridx]->highword3();
        id.words[1] = (*asset_uuids)[ridx]->highword2();
        id.words[2] = (*asset_uuids)[ridx]->highword1();
        id.words[3] = (*asset_uuids)[ridx]->lowword();
        unloadResourceInternal(id); 
    }
}

void unloadResource(Handle res_hdl) {
    hScopedMutex sentry(&ctx.access);
    ++ctx.transactions;

    unloadResourceInternal(res_hdl.id);
}

static void* getResourceDataPtr(resid_t res_id, uint32_t* o_typecc) {
    hdbassert(o_typecc, "o_typecc must not be null");
    hScopedMutex sentry(&ctx.access);

    Resource& res = ctx.resources[res_id];
    if (!res.runtimeData)
        return nullptr;

    *o_typecc = res.typecc;
    return res.runtimeData;
}

bool Handle::loaded() {
    if (data)
        return true;

    data = getResourceDataPtr(id, &typecc);
    return !!data;
}

}
}
