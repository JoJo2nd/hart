/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"

namespace hart {
namespace filesystem {

    enum FileMode
    {
        FILEMODE_READ,
        FILEMODE_WRITE,

        FILEMODE_MAX
    };

    enum class FileError {
        Ok = 0,
        Pending,
        Failed,
        EndOfFile,
    };

    enum class FileEntryType {
        Dir = 0x80,
        File = 0x40,
        SymLink = 0x20,
    };

    struct FileInfo
    {
        const char*    path_;
        const char*    name_;
        uint8_t        directory_;
    };


    typedef struct File*   FileHandle;
    typedef struct FileOp* FileOpHandle;

    struct FileInfo2
    {
        const char*    path_;
        const char*    name_;
        uint8_t        directory_;
    };

    struct hFileStat {
        uint64_t filesize;
        time_t   modifiedDate;
    };

    struct DirEntry {
        char filename[HART_MAX_PATH];
        uint32_t typeFlags; // of FileEntryType
    };

    bool initialise_filesystem();

    FileError fileOpComplete(FileOpHandle);
    FileError fileOpWait(FileOpHandle);
    void fileOpClose(FileOpHandle);

    FileOpHandle openFile(const char* filename, int mode, FileHandle* outhandle);
    void closeFile(FileHandle);
    FileOpHandle openDir(const char* path, FileHandle* outhandle);
    FileOpHandle readDir(FileHandle dir, DirEntry* out);
    void closeDir(FileHandle dir);

    FileOpHandle freadAsync(FileHandle file, void* buffer, size_t size, uint64_t offset);
    FileOpHandle fwriteAsync(FileHandle file, const void* buffer, size_t size, uint64_t offset);
    FileOpHandle fstatAsync(FileHandle file, hFileStat* out);

    void mountPoint(const char* path, const char* mount);
    void unmountPoint(const char* mount);
    bool isAbsolutePath(const char* path);
    void getCurrentWorkingDir(char* out, uint32_t bufsize);
    void getProcessDirectory(char* outdir, uint32_t size);
}
}

namespace hfs = hart::filesystem;