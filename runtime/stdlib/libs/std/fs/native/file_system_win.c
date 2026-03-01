/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "file_system.h"

enum FILE_TIME_TYPE { CREATION, LAST_ACCESS, LAST_WRITE };

#define DOS_DEVICE_PATH_SPECIFIER_PERIOD "\\\\.\\"
#define DOS_DEVICE_PATH_SPECIFIER_QUESTION_MARK "\\\\?\\"
#define DOS_DEVICE_PATH_SPECIFIER_SIZE 4
#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL
#define TYPE_OFFSET 1
#define PATH_OFFSET 2
#define FT_DIR 4
#define FT_REG 8
#define FT_LNK 10
#define FT_UNKNOWN 0

const DWORD FILE_SHARE_MODE = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

static wchar_t* Char2Widechar(const char* cstr);
static char* Wchar2Char(const wchar_t* wstr);

static wchar_t* GetWPath(const char* cstr);
static wchar_t* GetWPathEndWithStar(const char* cstr);

static FsError* GetErrnoResult(void);
static FsError* GetLastErrorResult(void);

static char* CODE_FS_FormatMessage(int errnoValue);
static char* CODE_FS_ErrmesGet(int errnoValue);

static bool IsDosDevicePath(const char* path);
static BOOL GetFileAttr(const char* path, WIN32_FILE_ATTRIBUTE_DATA* wfad);
static int32_t GetDosDevicePathLocation(char* cPath, int64_t pathLen);
static int64_t FileTime(const char* path, enum FILE_TIME_TYPE ty);
static int64_t FileSize(const wchar_t* path);
static int64_t GetDirectorySize(const wchar_t* path);
static int64_t IsFileExecutable(const char* path);
static inline bool IsSlash(char c);

static int8_t IsDirectory(const wchar_t* path)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    BOOL result = GetFileAttributesExW(path, GetFileExInfoStandard, &wfad);
    return (int8_t)result != 0 ? ((wfad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0 &&
                                     (wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                               : -1;
}

static void RemoveTrailingSeparator(char* path)
{
    int end = strlen(path);
    while (end > 0) {
        if (!IsSlash(path[end - 1])) {
            if (path[end - 1] != ':') {
                path[end] = '\0';
            }
            break;
        }
        end--;
    }
}

static inline bool IsSlash(char c)
{
    return c == SLASH || c == '/';
}

static inline int SysWMkdir(const wchar_t* path)
{
    return _wmkdir(path);
}

static inline int SysWAccess(const wchar_t* path, int mode)
{
    return _waccess(path, mode);
}

static wchar_t* GetWPath(const char* cstr)
{
    if (cstr == NULL) {
        return NULL;
    }
    int length = MultiByteToWideChar(CP_UTF8, 0, cstr, -1, NULL, 0);
    if (length == 0) {
        return NULL;
    }

    length += 1;
    wchar_t* wstr = (wchar_t*)malloc(sizeof(wchar_t) * length);
    if (NULL == wstr) {
        return NULL;
    }
    (void)memset_s(wstr, length * sizeof(wchar_t), 0, length * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, cstr, -1, wstr, length);
    return wstr;
}

static wchar_t* GetWPathEndWithStar(const char* cstr)
{
    char namebuf[MAX_PATH_LEN];
    if (snprintf_s(namebuf, MAX_PATH_LEN, MAX_PATH_LEN - 1, "%s\\*", cstr) < 0) {
        return -1;
    }

    wchar_t* conv = (wchar_t*)Char2Widechar(namebuf);
    if (conv == NULL) {
        return -1;
    }
    return conv;
}

extern int64_t CODE_FS_PathSize(const char* path)
{
    wchar_t* wPath = GetWPath(path);
    if (wPath == NULL) {
        return -1;
    }
    int64_t len = FileSize(wPath);
    if (len < 0 && IsDirectory(wPath) > 0) {
        len = GetDirectorySize(wPath);
    }
    free(wPath);
    return len;
}

static int64_t GetDirectorySize(const wchar_t* wPath)
{
    wchar_t dirPath[MAX_PATH_LEN];
    if (swprintf_s(dirPath, MAX_PATH_LEN, L"%s\\*.*", wPath) < 0) {
        return -1;
    }

    WIN32_FIND_DATAW fileData;
    HANDLE hFind = FindFirstFileW(dirPath, &fileData);
    int64_t totalSize = 0;
    if (hFind == INVALID_HANDLE_VALUE) {
        return -1;
    }
    do {
        if (wcscmp(fileData.cFileName, L".") == 0 || wcscmp(fileData.cFileName, L"..") == 0) {
            continue;
        }
        DWORD fileType = fileData.dwFileAttributes;
        bool isFile = (fileType & FILE_ATTRIBUTE_REPARSE_POINT) == 0 && (fileType & FILE_ATTRIBUTE_DIRECTORY) == 0;
        bool isDir = (fileType & FILE_ATTRIBUTE_REPARSE_POINT) == 0 && (fileType & FILE_ATTRIBUTE_DIRECTORY) != 0;
        bool isLink = (fileType & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
        if (isFile) {
            totalSize += ((int64_t)fileData.nFileSizeHigh << 32) | fileData.nFileSizeLow; // Size is 32-bit
        } else if (isDir) {
            if (swprintf_s(dirPath, MAX_PATH_LEN, L"%s\\%s", wPath, fileData.cFileName) < 0) {
                return -1;
            }
            int subSize = GetDirectorySize(dirPath);
            if (subSize < 0) {
                totalSize = -1;
                break;
            } else {
                totalSize += subSize;
            }
        } else {
            continue;
        }
    } while (FindNextFileW(hFind, &fileData));

    int errCode = GetLastError();
    if (errCode != ERROR_NO_MORE_FILES && errCode != 0) {
        totalSize = -1;
    }
    (void)FindClose(hFind);
    return totalSize;
}

extern int64_t CODE_FS_GetCreationTime(const char* path)
{
    enum FILE_TIME_TYPE ty = CREATION;
    int64_t ret = FileTime(path, ty);
    return ret;
}

extern int64_t CODE_FS_GetLastAccessTime(const char* path)
{
    enum FILE_TIME_TYPE ty = LAST_ACCESS;
    int64_t ret = FileTime(path, ty);
    return ret;
}

extern int64_t CODE_FS_GetLastModificationTime(const char* path)
{
    enum FILE_TIME_TYPE ty = LAST_WRITE;
    int64_t ret = FileTime(path, ty);
    return ret;
}

static wchar_t* Char2Widechar(const char* cstr)
{
    if (cstr == NULL) {
        return NULL;
    }
    int length = MultiByteToWideChar(CP_UTF8, 0, cstr, -1, NULL, 0);
    if (length == 0) {
        return NULL;
    }
    wchar_t* wstr = (wchar_t*)malloc(sizeof(wchar_t) * length);
    if (NULL == wstr) {
        return NULL;
    }
    (void)memset_s(wstr, length * sizeof(wchar_t), 0, length * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, cstr, -1, wstr, length);
    return wstr;
}

static char* Wchar2Char(const wchar_t* wstr)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        return NULL;
    }
    len += 1;
    char* cstr = (char*)malloc(sizeof(char) * len);
    if (cstr == NULL) {
        return NULL;
    }
    (void)memset_s(cstr, len * sizeof(char), 0, len * sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, cstr, len, NULL, NULL);
    return cstr;
}

static bool IsDosDevicePath(const char* path)
{
    size_t pathLen = strlen(path);
    if (pathLen < DOS_DEVICE_PATH_SPECIFIER_SIZE) {
        return false;
    }
    bool isDosDevicePath = memcmp(DOS_DEVICE_PATH_SPECIFIER_PERIOD, path, DOS_DEVICE_PATH_SPECIFIER_SIZE) == 0 ||
        memcmp(DOS_DEVICE_PATH_SPECIFIER_QUESTION_MARK, path, DOS_DEVICE_PATH_SPECIFIER_SIZE) == 0;
    return isDosDevicePath;
}

static int32_t GetDosDevicePathLocation(char* cPath, int64_t pathLen)
{
    for (int i = DOS_DEVICE_PATH_SPECIFIER_SIZE; i < pathLen; i++) {
        if (IsSlash(cPath[i])) {
            return i + 1;
        }
    }
    return pathLen;
}

static BOOL GetFileAttr(const char* path, WIN32_FILE_ATTRIBUTE_DATA* wfad)
{
    BOOL result;
    wchar_t* conv = GetWPath(path);
    if (conv == NULL) {
        return -1;
    }
    result = GetFileAttributesExW(conv, GetFileExInfoStandard, wfad);
    free((void*)conv);
    return result;
}

static int64_t IsFileExecutable(const char* path)
{
    BOOL result;
    DWORD able;
    wchar_t* conv = (wchar_t*)GetWPath(path);
    if (conv == NULL) {
        return -1;
    }
    result = GetBinaryTypeW(conv, &able);
    free((void*)conv);
    if (result) {
        return able; // real value could be 0.
    } else {
        return -1;
    }
}

static int64_t FileTime(const char* path, enum FILE_TIME_TYPE ty)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    FILETIME systime;
    if (GetFileAttr(path, &wfad)) {
        if (ty == CREATION) {
            systime = wfad.ftCreationTime;
        } else if (ty == LAST_ACCESS) {
            systime = wfad.ftLastAccessTime;
        } else if (ty == LAST_WRITE) {
            systime = wfad.ftLastWriteTime;
        }
        return ((int64_t)((((uint64_t)systime.dwHighDateTime << 32) // 32 is the size of dwLowDateTime
                              | systime.dwLowDateTime) /
                    WINDOWS_TICK) -
            SEC_TO_UNIX_EPOCH);
    }
    return -1;
}

static int64_t FileSize(const wchar_t* path)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    if (GetFileAttributesExW(path, GetFileExInfoStandard, &wfad)) {
        // If this is a normal file, returns file's size
        if ((wfad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0 &&
            (wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            int64_t size = ((int64_t)wfad.nFileSizeHigh << 32) | wfad.nFileSizeLow;
            return size;
        }
    }
    return -1;
}

static uint8_t FileType(DWORD fileType)
{
    if ((fileType & FILE_ATTRIBUTE_REPARSE_POINT) == 0 && (fileType & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return FT_REG;
    }

    if ((fileType & FILE_ATTRIBUTE_REPARSE_POINT) == 0 && (fileType & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return FT_DIR;
    }

    if ((fileType & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
        return FT_LNK;
    }
    return FT_UNKNOWN;
}

extern int8_t CODE_FS_Exists(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    return GetFileAttr(path, &wfad) == 0 ? 1 : 0;
}

extern int8_t CODE_FS_IsDir(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    if (GetFileAttr(path, &wfad) == 0) {
        return -1;
    } else if ((wfad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        return 0; // is link
    } else {
        return (wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    }
}

extern int8_t CODE_FS_IsFile(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    if (GetFileAttr(path, &wfad) == 0) {
        return -1;
    } else if ((wfad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        return 0; // is link
    } else {
        return (wfad.dwFileAttributes & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) ? 1 : 0;
    }
}

extern int8_t CODE_FS_IsLink(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    if (GetFileAttr(path, &wfad) == 0) {
        return -1;
    } else {
        return (wfad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT ? 1 : 0;
    }
}

static HANDLE GetFileHandle(const char* path)
{
    wchar_t* conv = (wchar_t*)GetWPath(path);
    if (conv == NULL) {
        return NULL;
    }
    HANDLE hFile =
        CreateFileW(conv, FILE_READ_ATTRIBUTES, FILE_SHARE_MODE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    free((void*)conv);
    return hFile;
}

/**
 * If this function succeeds, the `rtnCode` is the length of `realPath` and the `msg` is a null pointer.
 * If this function fails, the `rtnCode` is negative and the `msg` is the error message.
 */
extern FsError* CODE_FS_NormalizePath(const char* path, char* realPath)
{
    HANDLE hFile = GetFileHandle(path);
    if (hFile == NULL) {
        return GetErrnoResult();
    }
    if (hFile == INVALID_HANDLE_VALUE) {
        return GetLastErrorResult();
    }
    wchar_t temp[MAX_PATH] = {0};

    DWORD dwRet = GetFinalPathNameByHandleW(hFile, temp, MAX_PATH, FILE_NAME_NORMALIZED);
    CloseHandle(hFile);

    FsError* result = GetDefaultResult();
    if (dwRet > MAX_PATH) {
        return result;
    }
    char* tempRealPath = Wchar2Char(temp);
    if (tempRealPath == NULL) {
        free(result);
        return NULL;
    }

    for (int i = 0; i < strlen(tempRealPath) + 1; i++) {
        realPath[i] = tempRealPath[i];
    }
    free((void*)tempRealPath);
    if (dwRet < MAX_PATH) {
        result->rtnCode = (int64_t)strlen(realPath);
    }
    return result;
}

static bool IsLinkToFDirExists(const char* path)
{
    HANDLE hFile = GetFileHandle(path);
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    CloseHandle(hFile);
    return true;
}

extern int8_t CODE_FS_IsFileORLinkToFile(const char* path)
{
    if (!IsLinkToFDirExists(path)) {
        return -1;
    }

    WIN32_FILE_ATTRIBUTE_DATA wfad;
    if (GetFileAttr(path, &wfad) == 0) {
        return -1;
    } else {
        return (wfad.dwFileAttributes & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) ? 1 : 0;
    }
}

extern int8_t CODE_FS_IsDirORLinkToFDir(const char* path)
{
    if (!IsLinkToFDirExists(path)) {
        return -1;
    }

    WIN32_FILE_ATTRIBUTE_DATA wfad;
    if (GetFileAttr(path, &wfad) == 0) {
        return -1;
    } else {
        return (wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    }
}

extern int8_t CODE_FS_CanRead(const char* path, int64_t pathLen)
{
    return 1;
}

static int64_t LocalStat(const char* path, struct stat* buf)
{
    // Remove the delimiter at the end of the path, except for the root directory like "C:\\".
    // If `_FILE_OFFSET_BITS` is defined, `stat` is replaced with `_stat64`.
    // In Windows, `_stat64` does not support a separator at the end of a path, except for
    // the root directory like "C:\\.
    RemoveTrailingSeparator(path);
    wchar_t* wPath = Char2Widechar(path);

    if (wPath == NULL) {
        return -1;
    }

    int64_t ret = _wstat(wPath, buf);
    free((void*)wPath);
    return ret;
}

extern int8_t CODE_FS_CanExecute(const char* path)
{
    struct stat buf;
    if (LocalStat(path, &buf) < 0) {
        return -1;
    }
    return (int8_t)((buf.st_mode | S_IXUSR) == buf.st_mode);
}

extern int8_t CODE_FS_IsReadOnly(const char* path)
{
    /*
     * In the Windows environment, the read-only permission of a user on a file is normal.
     * The user always has the permission to delete or modify a directory.
     */
    int8_t isDir = CODE_FS_IsDir(path);
    if (isDir < 0) {
        return -1;
    } else if (isDir == 1) {
        return 0;
    }
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    BOOL result = GetFileAttr(path, &wfad);
    return (int8_t)result != 0 ? ((wfad.dwFileAttributes == FILE_ATTRIBUTE_READONLY) ||
                                     (wfad.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
                               : 0;
}

extern int8_t CODE_FS_CanWrite(const char* path)
{
    int8_t isReadOnly = CODE_FS_IsReadOnly(path);
    if (isReadOnly < 0) {
        return -1;
    } else if (isReadOnly == 0) {
        return 1;
    } else if (isReadOnly == 1) {
        return 0;
    }
}

extern int8_t CODE_FS_SetExecutable(const char* path, bool executable)
{
    return 0;
}

extern int8_t CODE_FS_SetReadable(const char* path, int64_t pathLen, bool readable)
{
    return (int8_t)readable;
}

extern int8_t CODE_FS_SetWritable(const char* path, bool writable)
{
    /*
     * In the Windows environment, the write permission of a user on a file is normal.
     * The user always has the write permission on the directory and cannot be changed.
     * This function does not take effect.
     */
    int8_t isDir = CODE_FS_IsDir(path);
    if (isDir < 0) {
        return -1;
    } else if (isDir == 1) {
        return 0;
    }
    struct stat buf;

    if (LocalStat(path, &buf) < 0) {
        return -1;
    }
    mode_t m = buf.st_mode;
    bool isWritable = ((m | S_IWRITE) == m);
    if ((isWritable && !writable) || (!isWritable && writable)) {
        m = m ^ S_IWRITE;
    }
    return Chmod(path, m);
}


extern int8_t CODE_FS_ISDirEmpty(const char* path)
{
    wchar_t* conv = (wchar_t*)GetWPathEndWithStar(path);
    if (conv == NULL) {
        return -1;
    }
    WIN32_FIND_DATAW fileData;
    HANDLE hd = FindFirstFileW(conv, &fileData);
    free((void*)conv);
    if (hd == INVALID_HANDLE_VALUE) {
        return -1;
    }

    int8_t res = 0;
    do {
        if (wcscmp(fileData.cFileName, L".") == 0 || wcscmp(fileData.cFileName, L"..") == 0) {
            continue;
        }
        DWORD fileType = fileData.dwFileAttributes;
        if ((fileType & FILE_ATTRIBUTE_DIRECTORY) || (fileType & FILE_ATTRIBUTE_ARCHIVE) ||
            (fileType & FILE_ATTRIBUTE_NORMAL)) {
            res = 1;
            break;
        }
    } while (FindNextFileW(hd, &fileData));
    int errCode = GetLastError();
    if (errCode != ERROR_NO_MORE_FILES && errCode != 0) {
        res = -1;
    }
    (void)FindClose(hd);
    return res;
}

extern char* CODE_FS_GetDirHandleAndFirstFile(const char* path, uintptr_t* hd)
{
    wchar_t* conv = GetWPath(path);
    if (conv == NULL || hd == NULL) {
        return NULL;
    }
    WIN32_FIND_DATAW fileData;
    wchar_t dirPath[MAX_PATH_LEN];
    if (swprintf_s(dirPath, MAX_PATH_LEN, L"%s\\*.*", conv) < 0) {
        free((void*)conv);
        return NULL;
    }
    free((void*)conv);
    HANDLE hFind = FindFirstFileW(dirPath, &fileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    *hd = (uintptr_t)hFind;
    if (wcscmp(fileData.cFileName, L".") == 0 || wcscmp(fileData.cFileName, L"..") == 0) {
        return NULL;
    }
    return Wchar2Char(fileData.cFileName);
}

extern char* CODE_FS_ReadDirHandle(uintptr_t handle)
{
    if (handle == 0) {
        return NULL;
    }
    WIN32_FIND_DATAW fileData;
    if (FindNextFileW((HANDLE)handle, &fileData)) {
        if (wcscmp(fileData.cFileName, L".") == 0 || wcscmp(fileData.cFileName, L"..") == 0) {
            return CODE_FS_ReadDirHandle(handle);
        }
        return Wchar2Char(fileData.cFileName);
    }
    return NULL;
}

extern void CODE_FS_CloseDirHandle(uintptr_t handle)
{
    if (handle != 0) {
        (void)FindClose((HANDLE)handle);
    }
}

extern FsError* CODE_FS_DirCreate(const char* path)
{
    wchar_t* conv = (wchar_t*)GetWPath(path);
    if (conv == NULL) {
        return GetErrnoResult();
    }

    int ret = SysWMkdir(conv);
    free((void*)conv);
    if (ret != 0) {
        return GetErrnoResult();
    }
    FsError* result = GetDefaultResult();
    return result;
}

extern FsError* CODE_FS_DirCreateRecursive(char* path)
{
    int64_t pathLen = strlen(path);
    int dosDevicePathLocation = IsDosDevicePath(path) ? GetDosDevicePathLocation(path, pathLen) : 0;
    for (int i = dosDevicePathLocation == 0 ? 1 : dosDevicePathLocation; i < pathLen; i++) {
        if (IsSlash(path[i])) {
            path[i] = '\0';
            wchar_t* conv = (wchar_t*)Char2Widechar(path);
            if (conv == NULL) {
                return GetErrnoResult();
            }
            if (SysWAccess(conv, F_OK) != 0 && SysWMkdir(conv) != 0) {
                free((void*)conv);
                return GetErrnoResult();
            }
            free((void*)conv);
            path[i] = SLASH;
        }
    }
    if (pathLen > 0) {
        wchar_t* conv = (wchar_t*)Char2Widechar(path);
        if (conv == NULL) {
            return GetErrnoResult();
        }
        if (SysWAccess(conv, F_OK) != 0) {
            if (SysWMkdir(conv) != 0) {
                free((void*)conv);
                return GetErrnoResult();
            }
        }
        free((void*)conv);
    }
    FsError* result = GetDefaultResult();
    return result;
}

static FsError* DeleteTree(wchar_t* wPath)
{
    wchar_t szPath[MAX_PATH_LEN] = {0};
    WIN32_FIND_DATAW fd;

    if (swprintf_s(szPath, MAX_PATH_LEN, L"%ls\\*", wPath) == -1) {
        return GetErrnoResult();
    }

    HANDLE hFind = FindFirstFileW(szPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) {
                continue;
            }

            if (swprintf_s(szPath, MAX_PATH_LEN, L"%ls\\%ls", wPath, fd.cFileName) == -1) {
                return GetErrnoResult();
            }
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                FsError* ret = DeleteTree(szPath);
                if (ret != NULL && ret->rtnCode != 0) {
                    (void)FindClose(hFind);
                    return ret;
                }
            } else if (!DeleteFileW(szPath)) {
                (void)FindClose(hFind);
                return GetLastErrorResult();
            }
        } while (FindNextFileW(hFind, &fd));
    } else {
        return GetLastErrorResult();
    }
    (void)FindClose(hFind);
    if (!RemoveDirectoryW(wPath)) {
        return GetLastErrorResult();
    }
    return GetDefaultResult();
}

extern bool CODE_FS_CreateTempDir(char* path)
{
    // The input path is in a format of "<parent-path>/tmpDirXXXXXX"
    static const char dict[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // 62 chars in total
    const unsigned int dictLen = strlen(dict);
    // 238328 is 62^3, which is 1/1000 of the No. of all combination of 6 digits (aka. a collision rate of 1/1000)
    const int maxEntry = 238328;
    // 62 = 31*2, 1111 = 11*101, so 1111 is a number that is not too small and coprime with 62.
    const int increaseStep = 1111;
    int len = strlen(path);
    // 1. Generate 2 random numbers of 32 bits, each for the first and last 3 digits of the string.
    // 6 string digits have 62^6 possibilities, 2^35 < 62^6 < 2^36,
    // so at least 36 bits of random data are needed to generate the string.
    // Here we generate two 32-bit random numbers.
    unsigned int randomInt1 = 0;
    unsigned int randomInt2 = 0;
    rand_s(&randomInt1);
    rand_s(&randomInt2);
    // 2. Let randomStr points to the first char of "XXXXXX", which is 6 chars ahead of "\0".
    char* randomStr = path + len - 6;
    for (int i = 0; i < maxEntry; randomInt1 += increaseStep, randomInt2 += increaseStep, ++i) {
        unsigned int r1 = randomInt1;
        unsigned int r2 = randomInt2;
        int start1 = 0;
        int start2 = 3;
        int genLen = 3;
        // 3. Parse randomInt1 into the first 3 string digits, by regarding it as a 62-base number.
        for (int j = start1; j < start1 + genLen; ++j) {
            randomStr[j] = dict[r1 % dictLen];
            r1 /= dictLen;
        }
        // 4. Parse randomInt2 into the last 3 string digits.
        for (int j = start2; j < start2 + genLen; ++j) {
            randomStr[j] = dict[r2 % dictLen];
            r2 /= dictLen;
        }
        // 5. Try to create the directory with the generated name.
        // 6. If the directory is created successfully, return a success.
        wchar_t* conv = (wchar_t*)Char2Widechar(path);
        if (conv == NULL) {
            return false;
        }
        if (SysWMkdir(conv) == 0) {
            free((void*)conv);
            return true;
        }
        free((void*)conv);
        // 7. If a directory with the same name has already exists,
        // increase the two random number by INCREASE_STEP, and try another name again.
    }
    // 8. If the try count reaches MAX_RETRY, and there is no successful directory creation,
    // give up and return a failure;
    return false;
}

/* Copy symbolic link */
extern int CODE_FS_CopyLink(char* linkName1, char* linkName2)
{
    return -1;
}

/* Copy file */
extern int CODE_FS_CopyREF(char* dir1, char* dir2)
{
    wchar_t* source = (wchar_t*)Char2Widechar(dir1);
    if (source == NULL) {
        return -1;
    }
    wchar_t* destination = (wchar_t*)Char2Widechar(dir2);
    if (destination == NULL) {
        free((void*)source);
        return -1;
    }
    bool ret = CopyFileW(source, destination, false);

    free((void*)source);
    free((void*)destination);

    return ret ? 0 : -1;
}

/*
 * File
 */

extern int64_t CODE_FS_GetFileSize(HANDLE fd)
{
    LARGE_INTEGER li;
    li.LowPart = GetFileSize(fd, &li.HighPart);
    return (int64_t)li.QuadPart;
}

extern int64_t CODE_FS_Seek(HANDLE fd, int32_t whence, int64_t offset)
{
    DWORD dwMoveMethod;
    switch (whence) {
        case SEEK_SET:
            dwMoveMethod = FILE_BEGIN;
            break;
        case SEEK_CUR:
            dwMoveMethod = FILE_CURRENT;
            break;
        case SEEK_END:
            dwMoveMethod = FILE_END;
            break;
        default:
            return -1;
    }
    LARGE_INTEGER li;
    li.QuadPart = offset;
    li.LowPart = SetFilePointer(fd, li.LowPart, &li.HighPart, dwMoveMethod);

    int64_t errcode = GetLastError();
    if (li.LowPart == INVALID_SET_FILE_POINTER && errcode != NO_ERROR) {
        return -errcode;
    }

    return (int64_t)li.QuadPart;
}

static DWORD GetDesiredAccess(int32_t openMode)
{
    switch (openMode) {
        case CODE_RDONLY:
            return GENERIC_READ;
        case CODE_WRONLY:
            return GENERIC_WRITE;
        case CODE_APPEND:
            return GENERIC_WRITE;
        case CODE_RDWT:
            return GENERIC_READ | GENERIC_WRITE;
        default:
            return 0;
    }
}

static DWORD GetCreationDisposition(int32_t openMode)
{
    switch (openMode) {
        case CODE_RDONLY:
            return OPEN_EXISTING;
        case CODE_WRONLY:
            return CREATE_ALWAYS;
        case CODE_APPEND:
            return OPEN_ALWAYS;
        case CODE_RDWT:
            return OPEN_ALWAYS;
        default:
            return 0;
    }
}

static HANDLE OpenFile4Win32(const wchar_t* wPath, int32_t openMode)
{
    DWORD access = GetCreationDisposition(openMode);
    bool setToEnd = (openMode == CODE_APPEND);

    DWORD mode = GetDesiredAccess(openMode);
    HANDLE fd = CreateFileW(wPath, mode, FILE_SHARE_MODE, NULL, access, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }
    // If the file is opened with 'APPEND', set fd to end of the file.
    if (setToEnd) {
        SetFilePointer(fd, NULL, NULL, FILE_END);
    }
    return fd;
}

extern FsInfo* CODE_FS_OpenFile(const char* path, int32_t openMode)
{
    FsInfo* result = GetDefaultFsResult();
    if (result == NULL) {
        return NULL;
    }

    const wchar_t* wPath = GetWPath(path);

    if (wPath == NULL) {
        result->msg = CODE_FS_FormatMessage(GetLastError());
        return result;
    }

    HANDLE fd = OpenFile4Win32(wPath, openMode);
    free((void*)wPath);
    result->fd = fd;
    if (fd == INVALID_HANDLE_VALUE) {
        result->fd = NULL;
        result->msg = CODE_FS_FormatMessage(GetLastError());
    }
    return result;
}

/* If return < 0, close file failed. */
extern int64_t CODE_FS_CloseFile(HANDLE fd)
{
    return CloseHandle(fd) ? 0 : -1;
}

/*
 * If return == 0, read end.
 * If return < 0, read failed.
 */
extern int64_t CODE_FS_FileRead(HANDLE fd, char* buffer, size_t maxLen)
{
    DWORD numOfBytesToRead = (DWORD)maxLen;
    DWORD numOfBytesRead = 0;
    BOOL success = ReadFile(fd, buffer, numOfBytesToRead, &numOfBytesRead, NULL);
    if (!success) {
        return -1;
    }
    return (int64_t)numOfBytesRead;
}

extern bool CODE_FS_FileWrite(HANDLE fd, const char* buffer, size_t maxLen)
{
    DWORD numOfBytesToWrite = (DWORD)maxLen;
    DWORD numOfWritenBytes = 0;
    WriteFile(fd, buffer, numOfBytesToWrite, &numOfWritenBytes, NULL);
    return (int64_t)(numOfWritenBytes - numOfBytesToWrite) == 0;
}

extern FsError* CODE_FS_Rename(const char* sourcePath, const char* destinationPath)
{
    /* move the source file */
    wchar_t* source = (wchar_t*)GetWPath(sourcePath);
    if (source == NULL) {
        return GetErrnoResult();
    }

    wchar_t* destination = (wchar_t*)GetWPath(destinationPath);
    if (destination == NULL) {
        free((void*)source);
        return GetErrnoResult();
    }

    // if the destination exists, remove it first
    DWORD attributes = GetFileAttributesW(destination);
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        BOOL ret = FALSE;

        if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            ret = RemoveDirectoryW(destination);
        } else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
            FsError* result = DeleteTree(destination); // need free
            if (result == NULL || result->rtnCode != 0) {
                free(destination);
                free(source);
                return result; // freed by caller
            }
            free(result);
            ret = TRUE;
        } else {
            ret = DeleteFileW(destination);
        }

        if (ret == FALSE) {
            free(destination);
            free(source);
            return GetLastErrorResult(); // freed by caller
        }
    }

    BOOL ret = MoveFileW(source, destination);
    free((void*)source);
    free((void*)destination);
    if (ret == FALSE) {
        return GetLastErrorResult(); // freed by caller
    }
    return GetDefaultResult(); // freed by caller
}

extern HANDLE CODE_FS_CreateTempFile(char* path)
{
    wchar_t* wPath = Char2Widechar(path);
    if (NULL == wPath) {
        return false;
    }
    if (_wmktemp(wPath) == NULL) {
        free(wPath);
        return NULL;
    }
    HANDLE hFile = CreateFileW(
        wPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_MODE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile) {
        free(wPath);
        return NULL;
    }
    char* cPath = Wchar2Char(wPath);

    if (NULL == cPath) {
        DeleteFileW(wPath);
        free(wPath);
        return NULL;
    }
    if (memcpy_s(path, strlen(path), cPath, strlen(cPath)) != EOK) {
        free(cPath);
        DeleteFileW(wPath);
        free(wPath);
        return NULL;
    }
    free(cPath);
    free(wPath);
    return hFile;
}

/*
 * Util
 */
extern FsError* CODE_FS_Remove(const char* path, bool rescursive)
{
    wchar_t* wPath = (wchar_t*)GetWPath(path);
    if (wPath == NULL) {
        return GetErrnoResult();
    }

    DWORD attributes = GetFileAttributesW(wPath);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        free(wPath);
        return GetLastErrorResult();
    }

    BOOL ret = FALSE;

    if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        ret = RemoveDirectoryW(wPath);
    } else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (!rescursive) {
            ret = RemoveDirectoryW(wPath);
        } else {
            FsError* result = DeleteTree(wPath);
            free((void*)wPath);
            return result;
        }
    } else {
        ret = DeleteFileW(wPath);
    }

    free((void*)wPath);
    if (ret == 0) {
        return GetLastErrorResult();
    }
    FsError* result = GetDefaultResult();
    return result;
}

// libc err no
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

// win32
static FsError* GetLastErrorResult(void)
{
    int64_t errCode = GetLastError();
    FsError* result = (FsError*)malloc(sizeof(FsError));
    if (result == NULL) {
        return NULL;
    }
    result->rtnCode = -errCode;
    result->msg = CODE_FS_FormatMessage(errCode);
    return result;
}

static char* CODE_FS_FormatMessage(int errnoValue)
{
    LPVOID lpMsgBuf = NULL;
    char* buffer = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
        (DWORD)errnoValue, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    if (lpMsgBuf == NULL) {
        return NULL;
    }
    buffer = strdup((const char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
    return buffer;
}

static char* CODE_FS_ErrmesGet(int errnoValue)
{
    char* msg = (char*)malloc(BUF_SIZE * sizeof(char));
    if (msg == NULL) {
        return NULL;
    }

    if (strerror_s(msg, BUF_SIZE, errnoValue) != 0) {
        free(msg);
        return NULL;
    }
    return msg;
}

extern FsError* CODE_FS_Link(const char* linkPath, char* originPath)
{
    wchar_t* wLinkPath = Char2Widechar(linkPath);
    if (wLinkPath == NULL) {
        return NULL;
    }

    for (int i = 0; i < strlen(originPath); i++) {
        if (originPath[i] == '/') {
            originPath[i] = '\\';
        }
    }
    wchar_t* wOriginPath = Char2Widechar(originPath);
    if (wOriginPath == NULL) {
        free(wLinkPath);
        return NULL;
    }

    BOOL ret = CreateHardLinkW(wLinkPath, wOriginPath, NULL);
    free(wOriginPath);
    free(wLinkPath);
    if (!ret) {
        return GetLastErrorResult();
    }
    return GetDefaultResult();
}

extern FsError* CODE_FS_SymLink(const char* linkPath, char* originPath)
{
    wchar_t* wLinkPath = Char2Widechar(linkPath);
    if (wLinkPath == NULL) {
        return NULL;
    }

    for (int i = 0; i < strlen(originPath); i++) {
        if (originPath[i] == '/') {
            originPath[i] = '\\';
        }
    }
    wchar_t* wOriginPath = Char2Widechar(originPath);
    if (wOriginPath == NULL) {
        free(wLinkPath);
        return NULL;
    }

    DWORD dwFlags = 0;

    DWORD attributes = GetFileAttributesW(wOriginPath);
    if ((attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        dwFlags = SYMBOLIC_LINK_FLAG_DIRECTORY;
    }

    BOOL ret = CreateSymbolicLinkW(wLinkPath, wOriginPath, dwFlags);
    free(wOriginPath);
    free(wLinkPath);
    if (!ret) {
        return GetLastErrorResult();
    }
    return GetDefaultResult();
}

typedef struct _REPARSE_DATA_BUFFER {
    ULONG ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

/**
 * If this function succeeds, the `rtnCode` is zero and the `msg` is the target path.
 * If this function fails, the `rtnCode` is negative and the `msg` is the error message.
 */
extern FsError* CODE_FS_ReadLink(const char* path)
{
    wchar_t* wPath = Char2Widechar(path);
    if (wPath == NULL) {
        return NULL;
    }

    HANDLE hFile = CreateFileW(wPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, // necessary
        NULL);

    free(wPath);

    if (hFile == INVALID_HANDLE_VALUE) {
        return GetLastErrorResult();
    }

    DWORD dwBufferSize = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    REPARSE_DATA_BUFFER* pReparseData = (REPARSE_DATA_BUFFER*)malloc(dwBufferSize);
    if (pReparseData == NULL) {
        CloseHandle(hFile);
        return NULL;
    }

    DWORD dwBytesReturned;
    // use DeviceIoControl
    BOOL bResult =
        DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, pReparseData, dwBufferSize, &dwBytesReturned, NULL);

    CloseHandle(hFile);

    if (!bResult) {
        return GetLastErrorResult();
    }

    int wlen = pReparseData->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR);
    WCHAR* pszPrintName = pReparseData->SymbolicLinkReparseBuffer.PathBuffer +
        pReparseData->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR);

    int clen = WideCharToMultiByte(CP_UTF8, 0, pszPrintName, wlen, NULL, 0, NULL, NULL);
    if (clen == 0) {
        free(pReparseData);
        return NULL;
    }
    clen += 1;
    char* cstr = (char*)malloc(sizeof(char) * clen);
    if (cstr == NULL) {
        free(pReparseData);
        return NULL;
    }
    WideCharToMultiByte(CP_UTF8, 0, pszPrintName, wlen, cstr, clen, NULL, NULL);
    cstr[clen - 1] = 0;
    free(pReparseData);

    FsError* result = (FsError*)malloc(sizeof(FsError));
    if (result == NULL) {
        free(cstr);
        return NULL;
    }
    result->rtnCode = 0;
    result->msg = cstr;
    return result;
}

extern FsError* CODE_FS_Truncate(HANDLE hFile, int64_t length)
{
    LARGE_INTEGER liCurrentPos;
    LARGE_INTEGER liDistanceToMove;
    liDistanceToMove.QuadPart = 0;

    // record current position
    BOOL ret = SetFilePointerEx(hFile, liDistanceToMove, &liCurrentPos, FILE_CURRENT);
    if (!ret) {
        return GetLastErrorResult();
    }

    // set EOF to new postion
    liDistanceToMove.QuadPart = length;
    ret = SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN);
    if (!ret) {
        return GetLastErrorResult();
    }
    ret = SetEndOfFile(hFile);
    if (!ret) {
        return GetLastErrorResult();
    }

    // recover the current position
    ret = SetFilePointerEx(hFile, liCurrentPos, NULL, FILE_BEGIN);
    if (!ret) {
        return GetLastErrorResult();
    }

    return GetDefaultResult();
}
