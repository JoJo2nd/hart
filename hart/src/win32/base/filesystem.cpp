
#include "hart/config.h"
#include "hart/base/mutex.h"
#include "hart/core/utf8.h"
#include "hart/base/filesystem.h"
#include "hart/base/util.h"
#include "hart/base/crt.h"
#include "hart/base/debug.h"
#include <windows.h>
#include <vector>
#include <algorithm>

namespace hart {
namespace filesystem {

struct File {
    HANDLE fileHandle;
};

struct hDir : File {
    DirEntry   currentEntry;
};

struct FileOp {
    virtual ~FileOp() {}
};

struct FileOpRW : FileOp {
    FileOpRW() {
        hcrt::zeromem(&operation, sizeof(OVERLAPPED));
        operation.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    }
    ~FileOpRW() {
        CloseHandle(operation.hEvent);
    }
    HANDLE              fileHdl;
    OVERLAPPED          operation;
};

struct Mount {
    std::string mountName;
    std::string mountPoint;
};

// Dummy op to return if operation completes immediately 
FileOp g_syncOp;
FileOp g_syncOpEOF;
std::vector< Mount > g_mounts;
hMutex g_mountMtx;

static bool isAbsPath(const char* in_path) {
    return (in_path[0] != '0' && in_path[1] == ':' && in_path[2] == '\\');
}

static void getExpanedPath(const char* in_path, char* out_path, size_t max_len) {
    hScopedMutex sentry(&g_mountMtx);
    hcrt::strcpy(out_path, HART_MAX_PATH, in_path);
    do {
        size_t offset = 0;
        for (const auto& mnt : g_mounts) {
            if (hcrt::strncmp(mnt.mountName.c_str(), out_path, mnt.mountName.size()) == 0) {
                offset = mnt.mountName.size();
                auto plen = mnt.mountPoint.size();
                auto slen = hcrt::strlen(out_path);
                if ((plen + slen + 1) > HART_MAX_PATH) {
                    plen = HART_MAX_PATH-slen;
                }
                hcrt::memmove(out_path+plen, out_path+offset, (slen-offset)+1);
                hcrt::strncpy(out_path, plen, mnt.mountPoint.c_str());
                break;
            }
        }
    } while (!isAbsPath(out_path));
}

static size_t getExpanedPathUC2(const char* in_path, wchar_t* out_path, size_t max_len) {
    hScopedMutex sentry(&g_mountMtx);
    char expaned[HART_MAX_PATH] = { 0 };
    getExpanedPath(in_path, expaned, HART_MAX_PATH);
    return hutf8::utf8_to_uc2(expaned, (uint16_t*)out_path, max_len);
}

template < size_t t_array_size >
static size_t getExpanedPathUC2(const char* in_path, wchar_t (&out_path)[t_array_size]) {
    return getExpanedPathUC2(in_path, out_path, t_array_size);
}

void mountPoint(const char* path, const char* mount);
void getCurrentWorkingDir(char* out, uint32_t bufsize);

bool initialise_filesystem() {
    char pwd[HART_MAX_PATH];
    getCurrentWorkingDir(pwd, HART_MAX_PATH);
    mountPoint(pwd, "/");
    return true;
}

FileError fileOpComplete(FileOpHandle in_op) {
    if (&g_syncOp == in_op) {
        return FileError::Ok;
    }
    if (&g_syncOpEOF == in_op) {
        return FileError::EndOfFile;
    }

    auto* op = static_cast<FileOpRW*>(in_op);
    DWORD xferred;
    if (GetOverlappedResult(op->fileHdl, &op->operation, &xferred, FALSE) == FALSE) {
        auto LastErr = GetLastError();
        if (LastErr == ERROR_IO_INCOMPLETE) {
            return FileError::Pending;
        } if (LastErr == ERROR_HANDLE_EOF) {
            return FileError::EndOfFile;
        } else {
            return FileError::Failed;
        }
    }
    //completed
    return FileError::Ok;
}

FileError fileOpWait(FileOpHandle in_op) {
    if (in_op == nullptr) {
        return FileError::Failed;
    }
    if (&g_syncOp == in_op) {
        return FileError::Ok;
    }
    if (&g_syncOpEOF == in_op) {
        return FileError::EndOfFile;
    }

    auto* op = static_cast<FileOpRW*>(in_op);
    DWORD xferred;
    if (GetOverlappedResult(op->fileHdl, &op->operation, &xferred, TRUE) == FALSE) {
        return FileError::Failed;
    }
    //completed
    return FileError::Ok;
}

void fileOpClose(FileOpHandle in_op) {
    if (&g_syncOp == in_op || &g_syncOpEOF == in_op || !in_op) {
        return;
    }

    delete in_op;
}

FileOpHandle openFile(const char* filename, int mode, FileHandle* outhandle) {
    DWORD access = 0;
    DWORD share = 0;// < always ZERO, dont let things happen to file in use!
    LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
    DWORD creation = 0;
    DWORD flags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
    HANDLE fhandle;

    if (mode == FILEMODE_READ)
    {
        access = GENERIC_READ;
        creation = OPEN_EXISTING;
    }
    else if (mode == FILEMODE_WRITE)
    {
        access = GENERIC_WRITE;
        creation = CREATE_ALWAYS;
    }

    wchar_t filename_wide[HART_MAX_PATH];
    getExpanedPathUC2(filename, filename_wide);
    fhandle = CreateFileW(filename_wide, access, share, secatt, creation, flags, nullptr);

    if (fhandle == INVALID_HANDLE_VALUE)
    {
        (*outhandle) = nullptr;
        return nullptr;
    }

    (*outhandle) = new File();
    (*outhandle)->fileHandle = fhandle;

    return &g_syncOp;
}

void closeFile(FileHandle handle) {
    if (handle && handle->fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle->fileHandle);
    }

    delete handle;
}

FileOpHandle openDir(const char* path, FileHandle* outhandle) {
    WIN32_FIND_DATAW found;
    auto* dir = new hDir();
    wchar_t path_wide[HART_MAX_PATH];
    getExpanedPathUC2(path, path_wide);
    dir->fileHandle = FindFirstFileW(path_wide, &found);
    //Heart::hStrCopy(dir->currentEntry.filename, HART_MAX_PATH, found.cFileName);
    hutf8::uc2_to_utf8((uint16_t*)found.cFileName, dir->currentEntry.filename, HART_ARRAYSIZE(dir->currentEntry.filename));
    dir->currentEntry.typeFlags = 0;
    if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        dir->currentEntry.typeFlags |= (uint32_t)FileEntryType::Dir;
    } else {
        dir->currentEntry.typeFlags |= (uint32_t)FileEntryType::File;
    }
    if (found.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        dir->currentEntry.typeFlags |= (uint32_t)FileEntryType::SymLink;
    }
    *outhandle = dir;
    return &g_syncOp;
}

FileOpHandle readDir(FileHandle dirhandle, DirEntry* out) {
    auto* dir = static_cast<hDir*>(dirhandle);
    if (dir->fileHandle == INVALID_HANDLE_VALUE) {
        return &g_syncOpEOF;
    }

    *out = dir->currentEntry;
    WIN32_FIND_DATAW found;
    if (FindNextFileW(dir->fileHandle, &found)) {
        hutf8::uc2_to_utf8((uint16_t*)found.cFileName, dir->currentEntry.filename, HART_ARRAYSIZE(dir->currentEntry.filename));
        dir->currentEntry.typeFlags = 0;
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            dir->currentEntry.typeFlags |= (uint32_t)FileEntryType::Dir;
        }
        else {
            dir->currentEntry.typeFlags |= (uint32_t)FileEntryType::File;
        }
        if (found.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            dir->currentEntry.typeFlags |= (uint32_t)FileEntryType::SymLink;
        }
    } else {
        CloseHandle(dir->fileHandle);
        dir->fileHandle = INVALID_HANDLE_VALUE;
    }

    return &g_syncOp;
}

void closeDir(FileHandle dir) {
    delete dir;
}

FileOpHandle freadAsync(FileHandle file, void* buffer, size_t size, uint64_t offset) {
    auto* new_op = new FileOpRW();
    new_op->fileHdl = file->fileHandle;
    new_op->operation.Offset = offset & 0xFFFFFFFF;
    new_op->operation.OffsetHigh = (offset & ((uint64_t)0xFFFFFFFF << 32)) >> 32;
    auto Completed = ReadFile(file->fileHandle, buffer, (DWORD)size, nullptr, &new_op->operation);
    if (Completed) {
        delete new_op;
        return &g_syncOp;
    }
    return new_op;
}

FileOpHandle fwriteAsync(FileHandle file, const void* buffer, size_t size, uint64_t offset) {
    auto* new_op = new FileOpRW();
    new_op->fileHdl = file->fileHandle;
    new_op->operation.Offset = offset & 0xFFFFFFFF;
    new_op->operation.OffsetHigh = (offset & ((uint64_t)0xFFFFFFFF << 32)) >> 32;
    auto Completed = WriteFile(file->fileHandle, buffer, (DWORD)size, nullptr, &new_op->operation);
    if (Completed) {
        delete new_op;
        return &g_syncOp;
    }
    return new_op;
}

static time_t FILETIMETotime_t(FILETIME const& ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

FileOpHandle fstatAsync(FileHandle filename, hFileStat* out) {
    BY_HANDLE_FILE_INFORMATION fileinfo;
    GetFileInformationByHandle(filename->fileHandle, &fileinfo);
    out->filesize = ((uint64_t)fileinfo.nFileSizeHigh << 32)| fileinfo.nFileSizeLow;
    out->modifiedDate = FILETIMETotime_t(fileinfo.ftLastWriteTime);
    return &g_syncOp;
}

bool isAbsolutePath(const char* path) {
    if (!path) {
        return false;
    }
    return path[0] == '/';
}

void mountPoint(const char* path, const char* mount) {
    hScopedMutex sentry(&g_mountMtx);
    hdbassert(isAbsolutePath(mount), "Path is not absolute");
    char expath[HART_MAX_PATH];
    getExpanedPath(path, expath, HART_MAX_PATH);
    hdbassert(isAbsPath(expath), "Expanded path is not absolute");
    Mount mnt;
    mnt.mountName = mount;
    mnt.mountPoint = expath;
    g_mounts.push_back(mnt);
    std::stable_sort(g_mounts.begin(), g_mounts.end(), [](const Mount& lhs, const Mount& rhs) {
            return lhs.mountName.size() > rhs.mountName.size();
    });
}

void unmountPoint(const char* mount) {
    hScopedMutex sentry(&g_mountMtx);
    std::remove_if(g_mounts.begin(), g_mounts.end(), [=](const Mount& rhs) {
            return hcrt::strcmp(mount, rhs.mountName.c_str()) == 0;
        });
}

void getCurrentWorkingDir(char* out, uint32_t bufsize) {
    wchar_t wd[HART_MAX_PATH];
    auto len = GetCurrentDirectoryW(HART_MAX_PATH-1, wd);
    wd[len]='\\';
    wd[len+1]=0;
    hutf8::uc2_to_utf8((uint16_t*)wd, out, bufsize);
    hdbassert(isAbsPath(out), "Returned path is expected to be absolute");
}

void getProcessDirectory(char* outdir, uint32_t size) {
    wchar_t pd[HART_MAX_PATH];
    GetModuleFileNameW(0, pd, HART_MAX_PATH-1);
    auto* s = wcsrchr(pd, '\\');
    if (s) {
        *(s+1) = 0;
    }
    hutf8::uc2_to_utf8((uint16_t*)pd, outdir, size);
    hdbassert(isAbsPath(outdir), "Returned path is expected to be absolute");
}

}
}
