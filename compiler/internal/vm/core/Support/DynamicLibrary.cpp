//===-- DynamicLibrary.cpp - Runtime link/load libraries --------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the operating system DynamicLibrary concept.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/DynamicLibrary.h"
#include "vm/core-c/Support.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/Config/config.h"
#include "vm/core/Support/Mutex.h"
#include <vector>

using namespace vm::core;
using namespace vm::core::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
class DynamicLibrary::HandleSet {
  using HandleList = std::vector<void *>;
  HandleList Handles;
  void *Process = &Invalid;

public:
  static void *DLOpen(const char *Filename, std::string *Err);
  static void DLClose(void *Handle);
  static void *DLSym(void *Handle, const char *Symbol);

  HandleSet() = default;
  ~HandleSet();

  HandleList::iterator Find(void *Handle) { return find(Handles, Handle); }

  bool Contains(void *Handle) {
    return Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    if (LLVM_LIKELY(!IsProcess)) {
      if (!AllowDuplicates && Find(Handle) != Handles.end()) {
        if (CanClose)
          DLClose(Handle);
        return false;
      }
      Handles.push_back(Handle);
    } else {
#ifndef _WIN32
      if (Process != &Invalid) {
        if (CanClose)
          DLClose(Process);
        if (Process == Handle)
          return false;
      }
#endif
      Process = Handle;
    }
    return true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    if (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const char *Symbol, DynamicLibrary::SearchOrdering Order) {
    if (Order & SO_LoadOrder) {
      for (void *Handle : Handles) {
        if (void *Ptr = DLSym(Handle, Symbol))
          return Ptr;
      }
    } else {
      for (void *Handle : toolchain::reverse(Handles)) {
        if (void *Ptr = DLSym(Handle, Symbol))
          return Ptr;
      }
    }
    return nullptr;
  }

  void *Lookup(const char *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    if (Process == &Invalid || (Order & SO_LoadedFirst)) {
      if (void *Ptr = LibLookup(Symbol, Order))
        return Ptr;
    }
    if (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      if (void *Ptr = DLSym(Process, Symbol))
        return Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      if (Order & SO_LoadedLast) {
        if (void *Ptr = LibLookup(Symbol, Order))
          return Ptr;
      }
    }
    return nullptr;
  }
};

namespace {

struct Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  toolchain::StringMap<void *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  toolchain::sys::SmartMutex<true> SymbolsMutex;
};

Globals &getGlobals() {
  static Globals G;
  return G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#else

#include "Unix/DynamicLibrary.inc"

#endif

char DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace vm::core {
void *SearchForAddressOfSpecialSymbol(const char *SymbolName) {
  return DoSearch(SymbolName); // DynamicLibrary.inc
}
} // namespace vm::core

void DynamicLibrary::AddSymbol(StringRef SymbolName, void *SymbolValue) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[SymbolName] = SymbolValue;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(const char *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  void *Handle = HandleSet::DLOpen(FileName, Err);
  if (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  return DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  if (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  return DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const char *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  if (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  return DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  if (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const char *SymbolName) {
  if (!isValid())
    return nullptr;
  return HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const char *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    if (i != G.ExplicitSymbols.end())
      return i->second;

    // Now search the libraries.
    if (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      return Ptr;
    if (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      return Ptr;
  }

  return toolchain::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const char *Filename) {
  return toolchain::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const char *symbolName) {
  return toolchain::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const char *symbolName, void *symbolValue) {
  return toolchain::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
