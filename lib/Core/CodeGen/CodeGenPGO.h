//===--- CodeGenPGO.h - PGO Instrumentation for LLVM CodeGen ----*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// Instrumentation-based profile-guided optimization
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CODEGENPGO_H
#define LANGUAGE_CORE_LIB_CODEGEN_CODEGENPGO_H

#include "CGBuilder.h"
#include "CodeGenModule.h"
#include "CodeGenTypes.h"
#include "MCDCState.h"
#include "toolchain/ProfileData/InstrProfReader.h"
#include <array>
#include <memory>
#include <optional>

namespace language::Core {
namespace CodeGen {

/// Per-function PGO state.
class CodeGenPGO {
private:
  CodeGenModule &CGM;
  std::string FuncName;
  toolchain::GlobalVariable *FuncNameVar;

  std::array <unsigned, toolchain::IPVK_Last + 1> NumValueSites;
  unsigned NumRegionCounters;
  uint64_t FunctionHash;
  std::unique_ptr<toolchain::DenseMap<const Stmt *, CounterPair>> RegionCounterMap;
  std::unique_ptr<toolchain::DenseMap<const Stmt *, uint64_t>> StmtCountMap;
  std::unique_ptr<toolchain::InstrProfRecord> ProfRecord;
  std::unique_ptr<MCDC::State> RegionMCDCState;
  std::vector<uint64_t> RegionCounts;
  uint64_t CurrentRegionCount;

public:
  CodeGenPGO(CodeGenModule &CGModule)
      : CGM(CGModule), FuncNameVar(nullptr), NumValueSites({{0}}),
        NumRegionCounters(0), FunctionHash(0), CurrentRegionCount(0) {}

  /// Whether or not we have PGO region data for the current function. This is
  /// false both when we have no data at all and when our data has been
  /// discarded.
  bool haveRegionCounts() const { return !RegionCounts.empty(); }

  /// Return the counter value of the current region.
  uint64_t getCurrentRegionCount() const { return CurrentRegionCount; }

  /// Set the counter value for the current region. This is used to keep track
  /// of changes to the most recent counter from control flow and non-local
  /// exits.
  void setCurrentRegionCount(uint64_t Count) { CurrentRegionCount = Count; }

  /// Check if an execution count is known for a given statement. If so, return
  /// true and put the value in Count; else return false.
  std::optional<uint64_t> getStmtCount(const Stmt *S) const {
    if (!StmtCountMap)
      return std::nullopt;
    auto I = StmtCountMap->find(S);
    if (I == StmtCountMap->end())
      return std::nullopt;
    return I->second;
  }

  /// If the execution count for the current statement is known, record that
  /// as the current count.
  void setCurrentStmt(const Stmt *S) {
    if (auto Count = getStmtCount(S))
      setCurrentRegionCount(*Count);
  }

  /// Assign counters to regions and configure them for PGO of a given
  /// function. Does nothing if instrumentation is not enabled and either
  /// generates global variables or associates PGO data with each of the
  /// counters depending on whether we are generating or using instrumentation.
  void assignRegionCounters(GlobalDecl GD, toolchain::Function *Fn);
  /// Emit a coverage mapping range with a counter zero
  /// for an unused declaration.
  void emitEmptyCounterMapping(const Decl *D, StringRef FuncName,
                               toolchain::GlobalValue::LinkageTypes Linkage);
  // Insert instrumentation or attach profile metadata at value sites
  void valueProfile(CGBuilderTy &Builder, uint32_t ValueKind,
                    toolchain::Instruction *ValueSite, toolchain::Value *ValuePtr);

  // Set a module flag indicating if value profiling is enabled.
  void setValueProfilingFlag(toolchain::Module &M);

  void setProfileVersion(toolchain::Module &M);

private:
  void setFuncName(toolchain::Function *Fn);
  void setFuncName(StringRef Name, toolchain::GlobalValue::LinkageTypes Linkage);
  void mapRegionCounters(const Decl *D);
  void computeRegionCounts(const Decl *D);
  void applyFunctionAttributes(toolchain::IndexedInstrProfReader *PGOReader,
                               toolchain::Function *Fn);
  void loadRegionCounts(toolchain::IndexedInstrProfReader *PGOReader,
                        bool IsInMainFile);
  bool skipRegionMappingForDecl(const Decl *D);
  void emitCounterRegionMapping(const Decl *D);
  bool canEmitMCDCCoverage(const CGBuilderTy &Builder);

public:
  std::pair<bool, bool> getIsCounterPair(const Stmt *S) const;
  void emitCounterSetOrIncrement(CGBuilderTy &Builder, const Stmt *S,
                                 toolchain::Value *StepV);
  void emitMCDCTestVectorBitmapUpdate(CGBuilderTy &Builder, const Expr *S,
                                      Address MCDCCondBitmapAddr,
                                      CodeGenFunction &CGF);
  void emitMCDCParameters(CGBuilderTy &Builder);
  void emitMCDCCondBitmapReset(CGBuilderTy &Builder, const Expr *S,
                               Address MCDCCondBitmapAddr);
  void emitMCDCCondBitmapUpdate(CGBuilderTy &Builder, const Expr *S,
                                Address MCDCCondBitmapAddr, toolchain::Value *Val,
                                CodeGenFunction &CGF);

  void markStmtAsUsed(bool Skipped, const Stmt *S) {
    // Do nothing.
  }

  void markStmtMaybeUsed(const Stmt *S) {
    // Do nothing.
  }

  void verifyCounterMap() const {
    // Do nothing.
  }

  /// Return the region count for the counter at the given index.
  uint64_t getRegionCount(const Stmt *S) {
    if (!RegionCounterMap)
      return 0;
    if (!haveRegionCounts())
      return 0;
    // With profiles from a differing version of clang we can have mismatched
    // decl counts. Don't crash in such a case.
    auto Index = (*RegionCounterMap)[S].Executed;
    if (Index >= RegionCounts.size())
      return 0;
    return RegionCounts[Index];
  }
};

}  // end namespace CodeGen
}  // end namespace language::Core

#endif
