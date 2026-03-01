//===----------- JITSymbol.cpp - JITSymbol class implementation -----------===//
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
// JITSymbol class implementation plus helper functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/JITSymbol.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalAlias.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/ModuleSummaryIndex.h"
#include "vm/core/Object/ObjectFile.h"

using namespace vm::core;

JITSymbolFlags toolchain::JITSymbolFlags::fromGlobalValue(const GlobalValue &GV) {
  assert(GV.hasName() && "Can't get flags for anonymous symbol");

  JITSymbolFlags Flags = JITSymbolFlags::None;
  if (GV.hasWeakLinkage() || GV.hasLinkOnceLinkage())
    Flags |= JITSymbolFlags::Weak;
  if (GV.hasCommonLinkage())
    Flags |= JITSymbolFlags::Common;
  if (!GV.hasLocalLinkage() && !GV.hasHiddenVisibility())
    Flags |= JITSymbolFlags::Exported;

  if (isa<Function>(GV))
    Flags |= JITSymbolFlags::Callable;
  else if (isa<GlobalAlias>(GV) &&
           isa<Function>(cast<GlobalAlias>(GV).getAliasee()))
    Flags |= JITSymbolFlags::Callable;

  // Check for a linker-private-global-prefix on the symbol name, in which
  // case it must be marked as non-exported.
  if (auto *M = GV.getParent()) {
    const auto &DL = M->getDataLayout();
    StringRef LPGP = DL.getLinkerPrivateGlobalPrefix();
    if (!LPGP.empty() && GV.getName().front() == '\01' &&
        GV.getName().substr(1).starts_with(LPGP))
      Flags &= ~JITSymbolFlags::Exported;
  }

  return Flags;
}

JITSymbolFlags toolchain::JITSymbolFlags::fromSummary(GlobalValueSummary *S) {
  JITSymbolFlags Flags = JITSymbolFlags::None;
  auto L = S->linkage();
  if (GlobalValue::isWeakLinkage(L) || GlobalValue::isLinkOnceLinkage(L))
    Flags |= JITSymbolFlags::Weak;
  if (GlobalValue::isCommonLinkage(L))
    Flags |= JITSymbolFlags::Common;
  if (GlobalValue::isExternalLinkage(L) || GlobalValue::isExternalWeakLinkage(L))
    Flags |= JITSymbolFlags::Exported;

  if (isa<FunctionSummary>(S))
    Flags |= JITSymbolFlags::Callable;

  return Flags;
}

Expected<JITSymbolFlags>
toolchain::JITSymbolFlags::fromObjectSymbol(const object::SymbolRef &Symbol) {
  Expected<uint32_t> SymbolFlagsOrErr = Symbol.getFlags();
  if (!SymbolFlagsOrErr)
    // TODO: Test this error.
    return SymbolFlagsOrErr.takeError();

  JITSymbolFlags Flags = JITSymbolFlags::None;
  if (*SymbolFlagsOrErr & object::BasicSymbolRef::SF_Weak)
    Flags |= JITSymbolFlags::Weak;
  if (*SymbolFlagsOrErr & object::BasicSymbolRef::SF_Common)
    Flags |= JITSymbolFlags::Common;
  if (*SymbolFlagsOrErr & object::BasicSymbolRef::SF_Exported)
    Flags |= JITSymbolFlags::Exported;

  auto SymbolType = Symbol.getType();
  if (!SymbolType)
    return SymbolType.takeError();

  if (*SymbolType == object::SymbolRef::ST_Function)
    Flags |= JITSymbolFlags::Callable;

  return Flags;
}

ARMJITSymbolFlags
toolchain::ARMJITSymbolFlags::fromObjectSymbol(const object::SymbolRef &Symbol) {
  Expected<uint32_t> SymbolFlagsOrErr = Symbol.getFlags();
  if (!SymbolFlagsOrErr)
    // TODO: Actually report errors helpfully.
    report_fatal_error(SymbolFlagsOrErr.takeError());
  ARMJITSymbolFlags Flags;
  if (*SymbolFlagsOrErr & object::BasicSymbolRef::SF_Thumb)
    Flags |= ARMJITSymbolFlags::Thumb;
  return Flags;
}

/// Performs lookup by, for each symbol, first calling
///        findSymbolInLogicalDylib and if that fails calling
///        findSymbol.
void LegacyJITSymbolResolver::lookup(const LookupSet &Symbols,
                                     OnResolvedFunction OnResolved) {
  JITSymbolResolver::LookupResult Result;
  for (auto &Symbol : Symbols) {
    std::string SymName = Symbol.str();
    if (auto Sym = findSymbolInLogicalDylib(SymName)) {
      if (auto AddrOrErr = Sym.getAddress())
        Result[Symbol] = JITEvaluatedSymbol(*AddrOrErr, Sym.getFlags());
      else {
        OnResolved(AddrOrErr.takeError());
        return;
      }
    } else if (auto Err = Sym.takeError()) {
      OnResolved(std::move(Err));
      return;
    } else {
      // findSymbolInLogicalDylib failed. Lets try findSymbol.
      if (auto Sym = findSymbol(SymName)) {
        if (auto AddrOrErr = Sym.getAddress())
          Result[Symbol] = JITEvaluatedSymbol(*AddrOrErr, Sym.getFlags());
        else {
          OnResolved(AddrOrErr.takeError());
          return;
        }
      } else if (auto Err = Sym.takeError()) {
        OnResolved(std::move(Err));
        return;
      } else {
        OnResolved(make_error<StringError>("Symbol not found: " + Symbol,
                                           inconvertibleErrorCode()));
        return;
      }
    }
  }

  OnResolved(std::move(Result));
}

/// Performs flags lookup by calling findSymbolInLogicalDylib and
///        returning the flags value for that symbol.
Expected<JITSymbolResolver::LookupSet>
LegacyJITSymbolResolver::getResponsibilitySet(const LookupSet &Symbols) {
  JITSymbolResolver::LookupSet Result;

  for (auto &Symbol : Symbols) {
    std::string SymName = Symbol.str();
    if (auto Sym = findSymbolInLogicalDylib(SymName)) {
      // If there's an existing def but it is not strong, then the caller is
      // responsible for it.
      if (!Sym.getFlags().isStrong())
        Result.insert(Symbol);
    } else if (auto Err = Sym.takeError())
      return std::move(Err);
    else {
      // If there is no existing definition then the caller is responsible for
      // it.
      Result.insert(Symbol);
    }
  }

  return std::move(Result);
}
