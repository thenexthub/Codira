/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <limits.h>
#include <sys/types.h>
#include "file_system.h"
#include <string.h>

static int64_t BuildSubPath(char* dirPath, const int64_t pathLen, const char* subName);
static FsError* GetErrnoResult(void);

struct FolderNode {
    char* path;
    struct FolderNode* next;
    struct FolderNode* prior;
};

static struct FolderNode* FolderNodeInit(const char* dirPath)
{
    struct FolderNode* newnode;
    if (dirPath == NULL) {
        return NULL;
    }
    size_t pathLen = strlen(dirPath);
    if (pathLen > MAX_PATH_LEN) {
        return NULL;
    }
    newnode = (struct FolderNode*)malloc(sizeof(struct FolderNode));
    if (newnode == NULL) {
        return NULL;
    }

    newnode->path = (char*)malloc(pathLen + 1);
    if (newnode->path == NULL) {
        free(newnode);
        return NULL;
    }
    (void)memset_s(newnode->path, pathLen + 1, 0, pathLen + 1);

    newnode->next = NULL;
    newnode->prior = NULL;

    int err = strncpy_s(newnode->path, pathLen + 1, dirPath, pathLen);
    if (err != 0) {
        free(newnode->path);
        newnode->path = NULL;
        free(newnode);
        newnode = NULL;
    }
    return newnode;
}

/*
 * FileInfo
 */
static inline bool IsSlash(char c)
{
    return c == SLASH;
}

static inline int SysMkdir(const char* path, mode_t mode)
{
    return mkdir(path, mode);
}

static inline int SysRmdir(const char* path)
{
    return remove(path);
}

/* If get get directory size faied, return 0. */
static int64_t GetDirectorySize(char* dirPath, int64_t pathLen)
{
    if (pathLen < 0 || pathLen > MAX_PATH_LEN) {
        return 0;
    }
    int64_t totalSize = 0;
    struct stat statbuf;
    if (lstat(dirPath, &statbuf) < 0) {
        return 0;
    }
    DIR* dirPtr = opendir(dirPath);
    if (dirPtr == NULL) {
        return 0;
    }
    totalSize += statbuf.st_size;
    struct dirent* dirInfo = NULL;
    dirPath[pathLen] = SLASH;
    while ((dirInfo = readdir(dirPtr)) != NULL) {
        int64_t newPathLen = BuildSubPath(dirPath, pathLen, dirInfo->d_name);
        if (newPathLen < 0) {
            continue;
        }
        if (lstat(dirPath, &statbuf) < 0) {
            break;
        }
        if (S_ISDIR(statbuf.st_mode)) {
            totalSize += GetDirectorySize(dirPath, newPathLen);
        } else {
            totalSize += statbuf.st_size;
        }
    }
    (void)closedir(dirPtr);
    return totalSize;
}

extern int64_t CODE_FS_PathSize(const char* path)
{
    struct stat buf;
    int64_t ret = lstat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    if (S_ISDIR(buf.st_mode)) {
        char cPath[MAX_PATH_LEN + 1] = {0};
        int64_t pathLen = (int64_t)strlen(path);
        errno_t rc = memcpy_s(cPath, MAX_PATH_LEN, path, (size_t)pathLen);
        if (rc != 0) {
            return -1;
        }
        int64_t res = GetDirectorySize(cPath, pathLen);
        return res;
    } else {
        return buf.st_size;
    }
}

extern int64_t CODE_FS_GetCreationTime(const char* path)
{
    struct stat buf;
    int64_t ret = stat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return buf.st_ctime;
}

extern int64_t CODE_FS_GetLastAccessTime(const char* path)
{
    struct stat buf;
    int64_t ret = stat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return buf.st_atime;
}

extern int64_t CODE_FS_GetLastModificationTime(const char* path)
{
    struct stat buf;
    int64_t ret = stat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return buf.st_mtime;
}

extern int8_t CODE_FS_Exists(const char* path)
{
    struct stat buf;
    int ret = lstat(path, &buf);
    return (int8_t)ret;
}

extern int8_t CODE_FS_IsLink(const char* path)
{
    struct stat buf;
    int64_t ret = lstat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return (S_ISLNK(buf.st_mode) ? 1 : 0);
}

extern int8_t CODE_FS_IsFile(const char* path)
{
    struct stat buf;
    int64_t ret = lstat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return (S_ISREG(buf.st_mode) ? 1 : 0);
}

extern int8_t CODE_FS_IsDir(const char* path)
{
    struct stat buf;
    int64_t ret = lstat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return (S_ISDIR(buf.st_mode) ? 1 : 0);
}

/*
 * Path
 */
extern FsError* CODE_FS_Link(const char* linkPath, const char* originPath)
{
    if (link(originPath, linkPath) != 0) {
        return GetErrnoResult();
    }
    return GetDefaultResult();
}

extern FsError* CODE_FS_SymLink(const char* linkPath, const char* originPath)
{
    if (symlink(originPath, linkPath) != 0) {
        return GetErrnoResult();
    }
    return GetDefaultResult();
}

/**
 * If this function succeeds, the `rtnCode` is zero and the `msg` is the target path.
 * If this function fails, the `rtnCode` is negative and the `msg` is the error message.
 */
extern FsError* CODE_FS_ReadLink(const char* path)
{
    char* target = (char*)malloc(MAX_PATH_LEN * sizeof(char));
    if (target == NULL) {
        return NULL;
    }
    (void)memset_s(target, MAX_PATH_LEN * sizeof(char), 0, MAX_PATH_LEN * sizeof(char));

    ssize_t len = readlink(path, target, MAX_PATH_LEN);
    if (len < 0) {
        free(target);
        return GetErrnoResult();
    }

    FsError* result = (FsError*)malloc(sizeof(FsError));
    if (result == NULL) {
        free(target);
        return NULL;
    }
    result->rtnCode = 0;
    // warning: errMsg should free on cangjie side
    result->msg = target;
    return result;
}

/**
 * If this function succeeds, the `rtnCode` is the length of `realPath` and the `msg` is a null pointer.
 * If this function fails, the `rtnCode` is negative and the `msg` is the error message.
 */
extern FsError* CODE_FS_NormalizePath(const char* path, char* realPath)
{
    // path normalization
    const char* res = realpath(path, realPath);
    if (res == NULL) {
        return GetErrnoResult();
    }
    int64_t len = (int64_t)strlen(realPath);
    FsError* result = (FsError*)malloc(sizeof(FsError));
    if (result == NULL) {
        return NULL;
    }
    result->rtnCode = len;
    result->msg = NULL;
    return result;
}

extern int8_t CODE_FS_IsFileORLinkToFile(const char* path)
{
    struct stat buf;
    int64_t ret = stat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return (S_ISREG(buf.st_mode) ? 1 : 0);
}

extern int8_t CODE_FS_IsDirORLinkToFDir(const char* path)
{
    struct stat buf;
    int64_t ret = stat(path, &buf);
    if (ret < 0) {
        return -1;
    }
    return (S_ISDIR(buf.st_mode) ? 1 : 0);
}

extern int8_t CODE_FS_CanRead(const char* path)
{
    struct stat buf;
    if (stat(path, &buf) < 0) {
        return -1;
    }
    return (int8_t)((buf.st_mode | S_IRUSR) == buf.st_mode);
}

extern int8_t CODE_FS_CanExecute(const char* path)
{
    struct stat buf;
    if (stat(path, &buf) < 0) {
        return -1;
    }
    return (int8_t)((buf.st_mode | S_IXUSR) == buf.st_mode);
}

extern int8_t CODE_FS_IsReadOnly(const char* path)
{
    struct stat buf;
    if (stat(path, &buf) < 0) {
        return -1;
    }
    mode_t m = buf.st_mode;
    return (int8_t)((m | S_IRUSR) == m && (m | S_IWUSR) != m && (m | S_IXUSR) != m);
}

extern int8_t CODE_FS_CanWrite(const char* path)
{
    struct stat buf;
    if (stat(path, &buf) < 0) {
        return -1;
    }
    return (int8_t)((buf.st_mode | S_IWUSR) == buf.st_mode);
}

extern int8_t CODE_FS_SetExecutable(const char* path, bool executable)
{
    struct stat buf;
    if (stat(path, &buf) < 0) {
        return -1;
    }
    mode_t m = buf.st_mode;
    bool isExecutable = ((m | S_IXUSR) == m);
    if ((isExecutable && !executable) || (!isExecutable && executable)) {
        m = m ^ S_IXUSR;
    }
    return Chmod(path, m);
}

extern int8_t CODE_FS_SetReadable(const char* path, bool readable)
{
    struct stat buf;
    if (stat(path, &buf) < 0) {
        return -1;
    }
    mode_t m = buf.st_mode;
    bool isReadable = ((m | S_IRUSR) == m);
    if ((isReadable && !readable) || (!isReadable && readable)) {
        m = m ^ S_IRUSR;
    }
    return Chmod(path, m);
}

extern int8_t CODE_FS_SetWritable(const char* path, bool writable)
{
    struct stat buf;
    if (stat(path, &buf) < 0) {
        return -1;
    }
    mode_t m = buf.st_mode;
    bool isWritable = ((m | S_IWUSR) == m);
    if ((isWritable && !writable) || (!isWritable && writable)) {
        m = m ^ S_IWUSR;
    }
    return Chmod(path, m);
}

extern int8_t CODE_FS_ISDirEmpty(const char* path)
{
    DIR* dirPtr = opendir(path);
    if (dirPtr == NULL) {
        return -1;
    }
    int8_t res = 0;
    struct dirent* ent = readdir(dirPtr);
    while (ent != NULL) {
        const char* entryName = ent->d_name;
        if ((strcmp(".", entryName) == 0) || (strcmp("..", entryName) == 0)) {
            ent = readdir(dirPtr);
            continue;
        }

        char fullName[MAX_PATH_LEN];
        if (sprintf_s(fullName, MAX_PATH_LEN, "%s/%s", path, entryName) == -1) {
            res = ERRNO_EINVAL;
            break;
        }

        struct stat buf;
        if (stat(fullName, &buf) == -1) {
            res = ERRNO_ESTAT;
            break;
        }
        if (S_ISDIR(buf.st_mode) || S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            res = 1;
            break;
        }
        ent = readdir(dirPtr);
    }
    (void)closedir(dirPtr);

    return res;
}

extern char* CODE_FS_GetDirHandleAndFirstFile(const char* path, uintptr_t* hd)
{
    if (path == NULL || hd == NULL) {
        return NULL;
    }
    DIR* dirPtr = opendir(path);
    if (dirPtr == NULL) {
        return NULL;
    }
    *hd = (uintptr_t)dirPtr;
    return NULL;
}

extern char* CODE_FS_ReadDirHandle(uintptr_t handle)
{
    if (handle == 0) {
        return NULL;
    }
    struct dirent* dirInfo = readdir((DIR*)handle);
    if (dirInfo == NULL) {
        return NULL;
    }
    if ((strcmp(".", dirInfo->d_name) == 0) || (strcmp("..", dirInfo->d_name) == 0)) {
        return CODE_FS_ReadDirHandle(handle);
    }

    return strdup(dirInfo->d_name);
}

extern void CODE_FS_CloseDirHandle(uintptr_t handle)
{
    if (handle != 0) {
        (void)closedir((DIR*)handle);
    }
}

extern FsError* CODE_FS_DirCreateRecursive(char* path)
{
    int pathLen = (int)strlen(path);
    for (int i = 1; i < pathLen; i++) {
        if (IsSlash(path[i])) {
            path[i] = '\0';
            if (access(path, F_OK) != 0 && SysMkdir(path, DEF_DIR_MODE) != 0) {
                return GetErrnoResult();
            }
            path[i] = SLASH;
        }
    }
    if (pathLen > 0 && access(path, F_OK) != 0) {
        if (SysMkdir(path, DEF_DIR_MODE) != 0) {
            return GetErrnoResult();
        }
    }
    FsError* result = GetDefaultResult();
    return result;
}

extern FsError* CODE_FS_DirCreate(const char* path)
{
    if (SysMkdir(path, DEF_DIR_MODE) != 0) {
        return GetErrnoResult();
    }
    FsError* result = GetDefaultResult();
    return result;
}

static int DeleteFileOrAppendDir(
    const struct FolderNode* folderfirst, struct FolderNode** folderlast, struct dirent* dirInfo)
{
    if (strncmp(dirInfo->d_name, ".", sizeof(".")) == 0 || strncmp(dirInfo->d_name, "..", sizeof("..")) == 0) {
        return 0;
    }

    char folderpath[MAX_PATH_LEN] = {0};
    int err =
        snprintf_s(folderpath, MAX_PATH_LEN, MAX_PATH_LEN - 1, "%s%c%s", folderfirst->path, SLASH, dirInfo->d_name);
    if (err < 0) {
        return -1;
    }

    struct stat statbuf;
    if (lstat(folderpath, &statbuf) != 0) {
        return -1;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        struct FolderNode* foldernew = NULL;
        foldernew = FolderNodeInit(folderpath);
        if (foldernew == NULL) {
            return -1;
        }
        foldernew->prior = *folderlast;
        (*folderlast)->next = foldernew;
        *folderlast = foldernew;
    } else if (remove(folderpath) != 0) {
        return -1;
    }
    return 0;
}

static FsError* DeleteTree(const char* dirPath)
{
    int ret = 0;
    DIR* dirPtr;
    struct dirent* dirInfo;

    struct FolderNode* folderstart;
    folderstart = FolderNodeInit(dirPath);
    if (folderstart == NULL) {
        return GetErrnoResult();
    }

    struct FolderNode* folderfirst = folderstart;
    struct FolderNode* folderlast = folderstart;
    struct FolderNode* oldfirst = NULL;

    while (folderfirst != NULL) {
        dirPtr = opendir(folderfirst->path);
        if (dirPtr == NULL) {
            ret = -1;
            break;
        }

        while ((dirInfo = readdir(dirPtr)) != NULL) {
            ret = DeleteFileOrAppendDir(folderfirst, &folderlast, dirInfo);
            if (ret == -1) {
                break;
            }
        }

        folderfirst = folderfirst->next;
        (void)closedir(dirPtr);
        if (ret == -1) {
            break;
        }
    }

    while (folderlast != NULL) {
        if (ret == 0 && SysRmdir(folderlast->path) != 0) {
            ret = -1;
        }
        oldfirst = folderlast;
        folderlast = folderlast->prior;
        if (oldfirst->path) {
            free(oldfirst->path);
            oldfirst->path = NULL;
        }
        free(oldfirst);
        oldfirst = NULL;
    }

    if (ret == -1) {
        return GetErrnoResult();
    }
    return GetDefaultResult();
}

extern bool CODE_FS_CreateTempDir(char* path)
{
    // The input path is in a format of "<parent-path>/tmpDirXXXXXX"
    return mkdtemp(path) != NULL;
}

/* Copy symbolic link */
extern int CODE_FS_CopyLink(char* linkName1, char* linkName2)
{
    char buf[MAX_PATH_LEN] = "";
    if (readlink(linkName1, buf, sizeof(buf)) == -1) {
        return -1;
    }
    if (symlink(buf, linkName2) != 0) {
        return -1;
    }
    return 0;
}

/* Copy file */
extern int CODE_FS_CopyREF(char* dir1, char* dir2)
{
    char buf[BUF_SIZE];

    int fd1 = open(dir1, O_RDONLY);
    if (fd1 < 0) {
        return -1;
    }
    int fd2 = open(dir2, O_WRONLY | O_CREAT | O_TRUNC, DEFFILEMODE);
    if (fd2 < 0) {
        (void)close(fd1);
        return -1;
    }

    int ret = (int)read(fd1, buf, sizeof(buf) - 1);
    int sts = 0;
    while (ret > 0) {
        sts = (int)write(fd2, buf, (size_t)ret);
        while (sts < ret) {
            sts += (int)write(fd2, buf + sts, (size_t)(ret - sts));
        }
        if (memset_s(buf, sizeof(buf), 0, sizeof(buf)) != EOK) {
            ret = -1;
            break;
        }
        ret = (int)read(fd1, buf, sizeof(buf) - 1);
    }

    if (ret == -1) {
        (void)close(fd1);
        (void)close(fd2);
        return -1;
    }

    struct stat info;
    if (fstat(fd1, &info) != 0) {
        (void)close(fd1);
        (void)close(fd2);
        return -1;
    }
    (void)close(fd1);

    if (fchmod(fd2, info.st_mode) != 0) {
        (void)close(fd2);
        return -1;
    }
    (void)close(fd2);
    return 0;
}

/*
 * File
 */

extern int64_t CODE_FS_GetFileSize(intptr_t fd)
{
    struct stat buf;

    if (fstat((int32_t)fd, &buf) < 0) {
        return -1;
    }
    // Check whether `path` refers to a readable file
    // To utilize "short-circuit evaluation", we list cases heuristically with their usage frequency
    bool readable = S_ISREG(buf.st_mode) || // Regular file
        S_ISBLK(buf.st_mode) ||             // Block device
        S_ISLNK(buf.st_mode) ||             // Symlink
        S_ISCHR(buf.st_mode) ||             // Character device
        S_ISFIFO(buf.st_mode);              // FIFO (named pipe)
    if (!readable) {
        return -1;
    }
    return buf.st_size;
}

extern int64_t CODE_FS_Seek(intptr_t fd, int32_t whence, int64_t offset)
{
    if (!(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END)) {
        return -1;
    }
    off_t res = lseek((int32_t)fd, offset, whence);
    if (res == -1) {
        return -errno;
    }
    return res;
}

static char* CODE_FS_ErrmesGet(int errnoValue)
{
    char* buf = (char*)malloc(BUF_SIZE * sizeof(char));
    if (buf == NULL) {
        return NULL;
    }

    if (strerror_r(errnoValue, buf, BUF_SIZE) != 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

extern FsInfo* CODE_FS_OpenFile(const char* path, int32_t openMode)
{
    FsInfo* result = GetDefaultFsResult();
    if (result == NULL) {
        return NULL;
    }

    char realPath[PATH_MAX + 1] = {0x00};
    const char* filePath = realPath;
    // path normalization
    if (realpath(path, realPath) == NULL) {
        /**
         * If there is no error, realpath() returns a pointer to the resolved_path.
         *
         * Otherwise, it returns NULL, the contents of the array resolved_path are
         * undefined, and errno is set to indicate the error.
         *
         * @see https://man7.org/linux/man-pages/man3/realpath.3.html
         */
        if (errno != ENOENT) { // No such file or directory
            result->fd = -1;
            result->msg = CODE_FS_ErrmesGet(errno);
            return result;
        }
        // create new file by origin path
        filePath = path;
    }

    size_t access = 0;
    switch (openMode) {
        case CODE_RDONLY:
            access = O_RDONLY;
            break;
        case CODE_WRONLY:
            access = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        case CODE_APPEND:
            access = O_WRONLY | O_CREAT | O_APPEND;
            break;
        case CODE_RDWT:
            access = O_RDWR | O_CREAT;
            break;
        default:;
    }
    /**
     * @see https://man7.org/linux/man-pages/man2/open.2.html
     *
     *      int open(const char *pathname, int flags, mode_t mode);
     *
     */
    int32_t fd = open(filePath, (int)(access), DEFFILEMODE);

    result->fd = (intptr_t)fd;
    if (fd == -1) {
        result->msg = CODE_FS_ErrmesGet(errno);
    }
    return result;
}

/* If return < 0, close file failed. */
extern int64_t CODE_FS_CloseFile(intptr_t fd)
{
    return (int64_t)close((int32_t)fd);
}

/*
 * If return == 0, read end.
 * If return < 0, read failed.
 */
extern int64_t CODE_FS_FileRead(intptr_t fd, char* buffer, size_t maxLen)
{
    return (int64_t)read((int32_t)fd, (char*)buffer, maxLen);
}

extern bool CODE_FS_FileWrite(intptr_t fd, const char* buffer, size_t maxLen)
{
    const char* ptr = buffer;
    size_t remainingLen = maxLen;
    while (remainingLen > 0) {
        ssize_t writeSize = write((int32_t)fd, (char*)ptr, remainingLen);
        if (writeSize <= 0) {
            break;
        }
        remainingLen -= (size_t)writeSize;
        ptr += writeSize;
    }
    return (int64_t)remainingLen == 0;
}

extern FsError* CODE_FS_Rename(const char* sourcePath, const char* destinationPath)
{
    // if destination is a directory, remove it first
    struct stat buf;

    if (lstat(destinationPath, &buf) == 0) {
        if (S_ISDIR(buf.st_mode)) {
            FsError* ret = DeleteTree(destinationPath); // need free
            if (ret == NULL) {
                return NULL;
            }

            if (ret->rtnCode != 0) {
                return ret; // freed by caller
            }
            free(ret);
        } else {
            if (remove(destinationPath) != 0) {
                return GetErrnoResult();
            }
        }
    }

    /* move the source file */
    if (rename(sourcePath, destinationPath) != 0) {
        return GetErrnoResult();
    }

    return GetDefaultResult();
}

extern intptr_t CODE_FS_CreateTempFile(char* path)
{
    return mkstemp(path);
}

/*
 * Util
 */
extern FsError* CODE_FS_Remove(const char* path, bool recursive)
{
    struct stat buf;
    int64_t ret = lstat(path, &buf);
    if (ret < 0) {
        return GetErrnoResult();
    }

    if (S_ISDIR(buf.st_mode)) {
        if (recursive) {
            return DeleteTree(path);
        }
        if (SysRmdir(path) != 0) {
            return GetErrnoResult();
        }
    } else {
        if (remove(path) != 0) {
            return GetErrnoResult();
        }
    }
    return GetDefaultResult();
}

static FsError* GetErrnoResult(void)
{
    char* errMsg = CODE_FS_ErrmesGet(errno);
    FsError* result = (FsError*)malloc(sizeof(FsError));
    if (result == NULL) {
        return NULL;
    }
    result->rtnCode = -errno;
    result->msg = errMsg;
    return result;
}

static int64_t BuildSubPath(char* dirPath, const int64_t pathLen, const char* subName)
{
    int64_t nameLen = (int64_t)strlen(subName);
    if ((nameLen == 1 && subName[0] == '.') ||
        (nameLen == 2 && subName[0] == '.' && subName[1] == '.')) { // 2 is the size of ".."
        return -1;
    }
    if ((MAX_PATH_LEN - (pathLen + 1)) <= 0 || (MAX_PATH_LEN - (pathLen + 1)) < (nameLen + 1)) {
        return -1;
    }
    errno_t rc = memcpy_s(dirPath + pathLen + 1, (size_t)(MAX_PATH_LEN - (pathLen + 1)), subName, (size_t)nameLen + 1);
    if (rc != 0) {
        return -1;
    }
    return pathLen + nameLen + 1;
}

extern FsError* CODE_FS_Truncate(int32_t fd, int64_t length)
{
    int ret = ftruncate(fd, length);
    if (ret != 0) {
        return GetErrnoResult();
    }
    return GetDefaultResult();
}
