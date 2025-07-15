//===--- Win32.cpp - Win32 utility functions --------------------*- C++ -*-===//
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
// Utility functions that are specific to the Windows port.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Debug.h"
#include "language/Runtime/Win32.h"

#ifdef _WIN32

#include <windows.h>

char *
_language_win32_copyUTF8FromWide(const wchar_t *str) {
  char *result = nullptr;
  int len = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                  str, -1,
                                  nullptr, 0,
                                  nullptr, nullptr);
  if (len <= 0)
    return nullptr;

  result = reinterpret_cast<char *>(std::malloc(len));
  if (!result)
    return nullptr;

  len = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                              str, -1,
                              result, len,
                              nullptr, nullptr);

  if (len)
    return result;

  free(result);
  return nullptr;
}

wchar_t *
_language_win32_copyWideFromUTF8(const char *str) {
  wchar_t *result = nullptr;
  int len = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                  str, -1,
                                  nullptr, 0);
  if (len <= 0)
    return nullptr;

  result = reinterpret_cast<wchar_t *>(std::malloc(len * sizeof(wchar_t)));
  if (!result)
    return nullptr;

  len = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                              str, -1,
                              result, len);

  if (len)
    return result;

  free(result);
  return nullptr;
}

#endif // defined(_WIN32)
