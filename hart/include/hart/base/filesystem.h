/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"

namespace hart {
namespace filesystem {

enum class Mode {
  Read,
  Write,
};

enum class Error {
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

struct FileInfo {
  const char* path_;
  const char* name_;
  uint8_t     directory_;
};

typedef struct File*   FileHandle;
typedef struct FileOp* FileOpHandle;

struct FileInfo2 {
  const char* path_;
  const char* name_;
  uint8_t     directory_;
};

struct FileStat {
  uint64_t filesize;
  time_t   modifiedDate;
};

struct DirEntry {
  char     filename[HART_MAX_PATH];
  uint32_t typeFlags; // of FileEntryType
};

bool initialise_filesystem();

Error fileOpComplete(FileOpHandle);
Error fileOpWait(FileOpHandle);

FileOpHandle openFile(const char* filename, Mode mode, FileHandle* outhandle);
void         closeFile(FileHandle);
FileOpHandle openDir(const char* path, FileHandle* outhandle);
FileOpHandle readDir(FileHandle dir, DirEntry* out);
void closeDir(FileHandle dir);

FileOpHandle freadAsync(FileHandle file, void* buffer, size_t size, uint64_t offset);
FileOpHandle fwriteAsync(FileHandle file, const void* buffer, size_t size, uint64_t offset);
FileOpHandle fstatAsync(FileHandle file, FileStat* out);

void mountPoint(const char* path, const char* mount);
void unmountPoint(const char* mount);
bool isAbsolutePath(const char* path);
void getCurrentWorkingDir(char* out, uint32_t bufsize);
void getProcessDirectory(char* outdir, uint32_t size);
}
}

namespace hfs = hart::filesystem;