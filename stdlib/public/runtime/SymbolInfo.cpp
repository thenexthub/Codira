//===--- ImageInspectionWin32.cpp - Win32 image inspection ----------------===//
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

#include "SymbolInfo.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <DbgHelp.h>
#include <psapi.h>

#pragma comment(lib, "DbgHelp.Lib")

#elif LANGUAGE_STDLIB_HAS_DLADDR
#include <dlfcn.h>
#endif

#include "language/Runtime/Win32.h"

#include "ImageInspection.h"

using namespace language;

const char *SymbolInfo::getFilename() const {
#if defined(_WIN32) && !defined(__CYGWIN__)
  return _moduleFileName;
#elif LANGUAGE_STDLIB_HAS_DLADDR
  return _info.dli_fname;
#else
  return nullptr;
#endif
}

const void *SymbolInfo::getBaseAddress() const {
#if defined(_WIN32) && !defined(__CYGWIN__)
  return _moduleBaseAddress;
#elif LANGUAGE_STDLIB_HAS_DLADDR
  return _info.dli_fbase;
#else
  return nullptr;
#endif
}

const char *SymbolInfo::getSymbolName() const {
#if defined(_WIN32) && !defined(__CYGWIN__)
  return _symbolName;
#elif LANGUAGE_STDLIB_HAS_DLADDR
  return _info.dli_sname;
#else
  return nullptr;
#endif
}

const void *SymbolInfo::getSymbolAddress() const {
#if defined(_WIN32) && !defined(__CYGWIN__)
  return _symbolAddress;
#elif LANGUAGE_STDLIB_HAS_DLADDR
  return _info.dli_saddr;
#else
  return nullptr;
#endif
}

#if defined(_WIN32) && !defined(__CYGWIN__)
struct Win32ModuleInfo {
  const char *name;
  const void *base;
};

// Get the filename and base of the module that contains the specified
// address.  N.B. This returns a `malloc`-ed copy of the filename in the
// Win32ModuleInfo struct; you are responsible for freeing that.
static Win32ModuleInfo moduleInfoFromAddress(const void *address) {
  HMODULE hModule;
  if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         (LPCTSTR)address,
                         &hModule)) {
    MODULEINFO mi;

    if (!GetModuleInformation(GetCurrentProcess(), hModule,
                              &mi, sizeof(mi))) {
      ZeroMemory(&mi, sizeof(mi));
    }

    WCHAR wszBuffer[256];
    LPWSTR pwszFileName = wszBuffer;
    DWORD dwCapacity = sizeof(wszBuffer) / sizeof(*wszBuffer);
    DWORD dwRet = GetModuleFileNameW(hModule, pwszFileName, dwCapacity);

    if (dwRet == dwCapacity) {
      dwCapacity = 512;

      pwszFileName = (LPWSTR)::malloc(sizeof(WCHAR) * dwCapacity);
      while (true) {
        dwRet = GetModuleFileNameW(hModule, pwszFileName, dwCapacity);
        if (dwRet != dwCapacity)
          break;

        dwCapacity *= 2;

        pwszFileName = (LPWSTR)::realloc(pwszFileName,
                                         sizeof(WCHAR) * dwCapacity);
      }
    }

    if (dwRet == 0) {
      if (pwszFileName != wszBuffer)
        ::free(pwszFileName);

      return { ::_strdup("<unknown>"), mi.lpBaseOfDll };
    }

    const char *result = _language_win32_copyUTF8FromWide(pwszFileName);

    if (pwszFileName != wszBuffer)
      ::free((void *)pwszFileName);

    return { result, mi.lpBaseOfDll };
  } else {
    return { ::_strdup("<unknown>"), nullptr };
  }
}
#endif

std::optional<SymbolInfo> SymbolInfo::lookup(const void *address) {
#if defined(__wasm__)
  // Currently, Wasm doesn't have a standard stable ABI for exporting address <->
  // symbol table, it's work in progress. Also, there is no API to access such
  // information from Wasm binary side. It's accessible only from host VM.
  // See https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
#elif defined(_WIN32) && !defined(__CYGWIN__)
  Win32ModuleInfo moduleInfo = moduleInfoFromAddress(address);
  SYMBOL_INFO_PACKAGE package;
  BOOL bRet;

  package.si.SizeOfStruct = sizeof(SYMBOL_INFO);
  package.si.MaxNameLen = MAX_SYM_NAME;

  _language_win32_withDbgHelpLibrary([&] (HANDLE hProcess) {
    if (!hProcess) {
      bRet = false;
      return;
    }

    bRet = SymFromAddr(hProcess, reinterpret_cast<const DWORD64>(address),
                       nullptr, &package.si);
  });

  if (bRet) {
    return SymbolInfo((const void *)package.si.Address,
                      ::_strdup(package.si.Name),
                      moduleInfo.name,
                      moduleInfo.base);
  } else {
    return SymbolInfo(address,
                      nullptr,
                      moduleInfo.name,
                      moduleInfo.base);
  }
#elif LANGUAGE_STDLIB_HAS_DLADDR
  SymbolInfo info;
  if (dladdr(address, &info._info)) {
    return info;
  }
#endif

  return {};
}
