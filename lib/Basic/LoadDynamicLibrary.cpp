//===----------------------------------------------------------------------===//
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

#include "language/Basic/LoadDynamicLibrary.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Windows/WindowsSupport.h"
#include "language/Basic/LLVM.h"
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32)
void *swift::loadLibrary(const char *path, std::string *err) {
  SmallVector<wchar_t, MAX_PATH> pathUnicode;
  if (std::error_code ec = llvm::sys::windows::UTF8ToUTF16(path, pathUnicode)) {
    SetLastError(ec.value());
    llvm::MakeErrMsg(err, std::string(path) + ": Can't convert to UTF-16");
    return nullptr;
  }

  HMODULE handle = LoadLibraryW(pathUnicode.data());
  if (handle == NULL) {
    llvm::MakeErrMsg(err, std::string(path) + ": Can't open");
    return nullptr;
  }
  return (void *)handle;
}

void *swift::getAddressOfSymbol(void *handle, const char *symbol) {
  return (void *)uintptr_t(GetProcAddress((HMODULE)handle, symbol));
}

#else
void *swift::loadLibrary(const char *path, std::string *err) {
  void *handle = ::dlopen(path, RTLD_LAZY | RTLD_LOCAL);
  if (!handle)
    *err = ::dlerror();
  return handle;
}

void *swift::getAddressOfSymbol(void *handle, const char *symbol) {
  return ::dlsym(handle, symbol);
}
#endif