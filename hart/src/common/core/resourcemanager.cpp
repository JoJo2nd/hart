/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "hart/core/resourcemanager.h"
#include "hart/base/std.h"
#include "hart/base/filesystem.h"
#include "hart/base/atomic.h"
#include "hart/core/objectfactory.h"
#include "hart/fbs/resourcedb_generated.h"
#include "hart/base/mutex.h"
#include "hart/core/engine.h"

HART_OBJECT_TYPE_DECL(hart::resourcemanager::Collection);

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
#if HART_DEBUG_INFO
    Handle debugLoadHandle;
#endif
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
    engine::DebugMenuHandle dbmenuHdl;
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
        id.words[3] = (*asset_uuids)[i]->highword3();
        id.words[2] = (*asset_uuids)[i]->highword2();
        id.words[1] = (*asset_uuids)[i]->highword1();
        id.words[0] = (*asset_uuids)[i]->lowword();
        Resource& res = ctx.resources[id];
        res.uuid = id;
        res.info = (*asset_infos)[i];
    }

    hfs::closeFile(res_file);
#if HART_DEBUG_INFO
    ctx.dbmenuHdl = engine::addDebugMenu("Resource Manager", []() {
        hScopedMutex sentry(&ctx.access);
        if (ImGui::Begin("Resource Manager", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_MenuBar)) {
            static resid_t to_load;
            auto const* asset_uuids = ctx.resourceListings->assetUUIDs();
            bool loadResource = false;
            if (!huuid::isNull(to_load) && !ctx.resources[to_load].debugLoadHandle.isValid()) {
                if (ImGui::Button("Test Load Resource")) {
                    ctx.resources[to_load].debugLoadHandle = hresmgr::loadResource(to_load);
                }
            }
            if (!huuid::isNull(to_load) && ctx.resources[to_load].debugLoadHandle.isValid() && ctx.resources[to_load].debugLoadHandle.loaded()) {
                if (ImGui::Button("Test Unload Resource")) {
                    
                }
            }
            ImGui::Separator();
            ImGui::Columns(4, "resources");
            ImGui::Text("Friendly Name"); ImGui::NextColumn();
            ImGui::Text("UUID"); ImGui::NextColumn();
            ImGui::Text("TypeCC"); ImGui::NextColumn();
            ImGui::Text("RefCount"); ImGui::NextColumn();
            ImGui::Separator();
            static int32_t selected = -1;
            int32_t index = 0;
            for (const auto& i : ctx.resources) {
                Resource const& r = i.second;
                char txt_buf[256];
                if (ImGui::Selectable(r.info->friendlyName()->c_str(), selected == index, ImGuiSelectableFlags_SpanAllColumns)) {
                    selected = index;
                    to_load = r.uuid;
                }
                if (ImGui::IsItemHovered()) {
                    auto const* prerequisites = r.info->prerequisites();
                    if (prerequisites) {
                        ImGui::BeginTooltip();
                        ImGui::Text("Depends on asset(s):");
                        for (uint32_t i = 0, n = prerequisites->size(); i < n; ++i) {
                            uint32_t ridx = (*prerequisites)[i];
                            resid_t p = huuid::fromData(*(*asset_uuids)[ridx]);
                            ImGui::Text("%s", ctx.resources[p].info->friendlyName()->c_str());
                        }
                        ImGui::EndTooltip();
                    }
                }
                ImGui::NextColumn();
                huuid::toString(r.uuid, txt_buf, HART_ARRAYSIZE(txt_buf));
                ImGui::Text(txt_buf); ImGui::NextColumn();
                if (r.typecc) {
                    (*(uint32_t*)txt_buf)=r.typecc;
                    txt_buf[4]=0;
                    ImGui::Text(txt_buf); ImGui::NextColumn();
                } else {
                    ImGui::Text("Unknown until loaded once"); ImGui::NextColumn();
                }
                ImGui::Text("%d", hatomic::atomicGet(r.refCount)); ImGui::NextColumn();
                ++index;
            }
        }
        ImGui::End();
    });
#endif
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

        ResourceLoadData load_data;
        hobjfact::SerialiseParams ser_params;
        ser_params.resdata = &load_data;
        LoadRequest& lr = ctx.loadQueue.front();
        Resource& res = ctx.resources[lr.uuid];
        ctx.resState = ResourceLoadState::LoadResource;
        res.runtimeData = hobjfact::deserialiseObject(res.loadtimeData.get(), res.info->filesize(), &ser_params, &res.typecc);
        hatomic::increment(res.refCount);
        if (!load_data.persistFileData) {
            res.loadtimeData.reset();
        }
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
    engine::removeDebugMenu(ctx.dbmenuHdl);
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
        id.words[3] = (*asset_uuids)[ridx]->highword3();
        id.words[2] = (*asset_uuids)[ridx]->highword2();
        id.words[1] = (*asset_uuids)[ridx]->highword1();
        id.words[0] = (*asset_uuids)[ridx]->lowword();
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
    r.info = &ctx.resources[res_id];
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
        id.words[3] = (*asset_uuids)[ridx]->highword3();
        id.words[2] = (*asset_uuids)[ridx]->highword2();
        id.words[1] = (*asset_uuids)[ridx]->highword1();
        id.words[0] = (*asset_uuids)[ridx]->lowword();
        unloadResourceInternal(id); 
    }
}

void unloadResource(Handle res_hdl) {
    hScopedMutex sentry(&ctx.access);
    ++ctx.transactions;

    unloadResourceInternal(res_hdl.id);
}

bool checkResourceLoaded(resid_t res_id) {
    hScopedMutex sentry(&ctx.access);

    Resource& res = ctx.resources[res_id];
    return !!res.runtimeData;
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

void* weakGetResource(resid_t res_id, uint32_t o_typecc) {
    uint32_t r_typecc;
    void* data = getResourceDataPtr(res_id, &r_typecc);
    return r_typecc == o_typecc ? data : nullptr;
}

bool Handle::loaded() {
    if (data)
        return true;

    data = getResourceDataPtr(id, &typecc);
    return !!data;
}

bool Collection::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const&) {
    auto const* assets = in_data->assetUUIDs();
    for (uint32_t i = 0, n = assets->size(); i < n; ++i) {
        resid_t id;
        id.words[3] = (*assets)[i]->highword3();
        id.words[2] = (*assets)[i]->highword2();
        id.words[1] = (*assets)[i]->highword1();
        id.words[0] = (*assets)[i]->lowword();

        hdbassert(checkResourceLoaded(id), "Collection contained resource reference but it wasn't loaded.");
    }
    return true;
}

}
}
