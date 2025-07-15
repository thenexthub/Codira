//===--- Win32Extras.cpp - Windows support functions ------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  Defines some extra functions that aren't available in the OS or C library
//  on Windows.
//
//===----------------------------------------------------------------------===//

#ifdef _WIN32

#include <windows.h>

#include "modules/OS/Libc.h"

extern "C" ssize_t pread(int fd, void *buf, size_t nbyte, off_t offset) {
  HANDLE hFile = _get_osfhandle(fd);
  OVERLAPPED ovl = {0};
  DWORD dwBytesRead = 0;

  ovl.Offset = (DWORD)offset;
  ovl.OffsetHigh = (DWORD)(offset >> 32);

  if (!ReadFile(hFile, buf, (DWORD)count, &dwBytesRead, &ovl)) {
    errno = EIO;
    return -1;
  }

  return dwBytesRead;
}

extern "C" ssize_t pwrite(int fd, const void *buf, size_t nbyte, off_t offset) {
  HANDLE hFile = _get_osfhandle(fd);
  OVERLAPPED ovl = {0};
  DWORD dwBytesRead = 0;

  ovl.Offset = (DWORD)offset;
  ovl.OffsetHigh = (DWORD)(offset >> 32);

  if (!WriteFile(hFile, buf, (DWORD)count, &dwBytesRead, &ovl)) {
    errno = EIO;
    return -1;
  }

  return dwBytesRead;
}

#endif // _WIN32

