//===-- RuntimeDyldCheckerImpl.h -- RuntimeDyld test framework --*- C++ -*-===//
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

#ifndef LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_RUNTIMEDYLDCHECKERIMPL_H
#define LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_RUNTIMEDYLDCHECKERIMPL_H

#include "RuntimeDyldImpl.h"

namespace vm::core {

/// Holds target-specific properties for a symbol.
using TargetFlagsType = uint8_t;

class RuntimeDyldCheckerImpl {
  friend class RuntimeDyldChecker;
  friend class RuntimeDyldCheckerExprEval;

  using IsSymbolValidFunction =
    RuntimeDyldChecker::IsSymbolValidFunction;
  using GetSymbolInfoFunction = RuntimeDyldChecker::GetSymbolInfoFunction;
  using GetSectionInfoFunction = RuntimeDyldChecker::GetSectionInfoFunction;
  using GetStubInfoFunction = RuntimeDyldChecker::GetStubInfoFunction;
  using GetGOTInfoFunction = RuntimeDyldChecker::GetGOTInfoFunction;

public:
  RuntimeDyldCheckerImpl(IsSymbolValidFunction IsSymbolValid,
                         GetSymbolInfoFunction GetSymbolInfo,
                         GetSectionInfoFunction GetSectionInfo,
                         GetStubInfoFunction GetStubInfo,
                         GetGOTInfoFunction GetGOTInfo,
                         toolchain::endianness Endianness, Triple TT, StringRef CPU,
                         SubtargetFeatures TF, toolchain::raw_ostream &ErrStream);

  bool check(StringRef CheckExpr) const;
  bool checkAllRulesInBuffer(StringRef RulePrefix, MemoryBuffer *MemBuf) const;

private:

  // StubMap typedefs.

  Expected<JITSymbolResolver::LookupResult>
  lookup(const JITSymbolResolver::LookupSet &Symbols) const;

  bool isSymbolValid(StringRef Symbol) const;
  uint64_t getSymbolLocalAddr(StringRef Symbol) const;
  uint64_t getSymbolRemoteAddr(StringRef Symbol) const;
  uint64_t readMemoryAtAddr(uint64_t Addr, unsigned Size) const;

  StringRef getSymbolContent(StringRef Symbol) const;

  TargetFlagsType getTargetFlag(StringRef Symbol) const;
  Triple getTripleForSymbol(TargetFlagsType Flag) const;
  StringRef getCPU() const { return CPU; }
  SubtargetFeatures getFeatures() const { return TF; }

  std::pair<uint64_t, std::string> getSectionAddr(StringRef FileName,
                                                  StringRef SectionName,
                                                  bool IsInsideLoad) const;

  std::pair<uint64_t, std::string>
  getStubOrGOTAddrFor(StringRef StubContainerName, StringRef Symbol,
                      StringRef StubKindFilter, bool IsInsideLoad,
                      bool IsStubAddr) const;

  std::optional<uint64_t> getSectionLoadAddress(void *LocalAddr) const;

  IsSymbolValidFunction IsSymbolValid;
  GetSymbolInfoFunction GetSymbolInfo;
  GetSectionInfoFunction GetSectionInfo;
  GetStubInfoFunction GetStubInfo;
  GetGOTInfoFunction GetGOTInfo;
  toolchain::endianness Endianness;
  Triple TT;
  std::string CPU;
  SubtargetFeatures TF;
  toolchain::raw_ostream &ErrStream;
};
}

#endif
