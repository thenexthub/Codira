//===- HexagonMCChecker.h - Instruction bundle checking ---------*- C++ -*-===//
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
// This implements the checking of insns inside a bundle according to the
// packet constraint rules of the Hexagon ISA.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCCHECKER_H
#define LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCCHECKER_H

#include "MCTargetDesc/HexagonMCInstrInfo.h"
#include "MCTargetDesc/HexagonMCTargetDesc.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/Support/SMLoc.h"
#include <set>
#include <utility>

namespace vm::core {

class MCContext;
class MCInst;
class MCInstrInfo;
class MCRegisterInfo;
class MCSubtargetInfo;

/// Check for a valid bundle.
class HexagonMCChecker {
  MCContext &Context;
  MCInst &MCB;
  const MCRegisterInfo &RI;
  MCInstrInfo const &MCII;
  MCSubtargetInfo const &STI;
  bool ReportErrors;

  /// Set of definitions: register #, if predicated, if predicated true.
  using PredSense = std::pair<MCRegister, bool>;
  static const PredSense Unconditional;
  using PredSet = std::multiset<PredSense>;
  using PredSetIterator = std::multiset<PredSense>::iterator;

  using DefsIterator = DenseMap<MCRegister, PredSet>::iterator;
  DenseMap<MCRegister, PredSet> Defs;

  /// Set of weak definitions whose clashes should be enforced selectively.
  using SoftDefsIterator = std::set<MCRegister>::iterator;
  std::set<MCRegister> SoftDefs;

  /// Set of temporary definitions not committed to the register file.
  using TmpDefsIterator = std::set<MCRegister>::iterator;
  std::set<MCRegister> TmpDefs;

  /// Set of new predicates used.
  using NewPredsIterator = std::set<MCRegister>::iterator;
  std::set<MCRegister> NewPreds;

  /// Set of predicates defined late.
  using LatePredsIterator = std::multiset<MCRegister>::iterator;
  std::multiset<MCRegister> LatePreds;

  /// Set of uses.
  using UsesIterator = std::set<MCRegister>::iterator;
  std::set<MCRegister> Uses;

  /// Pre-defined set of read-only registers.
  using ReadOnlyIterator = std::set<MCRegister>::iterator;
  std::set<MCRegister> ReadOnly;

  // Contains the vector-pair-registers with the even number
  // first ("v0:1", e.g.) used/def'd in this packet.
  std::set<MCRegister> ReversePairs;

  void init();
  void init(MCInst const &);
  void initReg(MCInst const &, MCRegister, MCRegister &PredReg, bool &isTrue);

  bool registerUsed(MCRegister Register);

  /// \return a tuple of: pointer to the producer instruction or nullptr if
  /// none was found, the operand index, and the PredicateInfo for the
  /// producer.
  std::tuple<MCInst const *, unsigned, HexagonMCInstrInfo::PredicateInfo>
  registerProducer(MCRegister Register,
                   HexagonMCInstrInfo::PredicateInfo Predicated);

  // Checks performed.
  bool checkBranches();
  bool checkPredicates();
  bool checkNewValues();
  bool checkRegisters();
  bool checkRegistersReadOnly();
  void checkRegisterCurDefs();
  bool checkSolo();
  bool checkShuffle();
  bool checkSlots();
  bool checkAXOK();
  bool checkHWLoop();
  bool checkCOFMax1();
  bool checkLegalVecRegPair();
  bool checkValidTmpDst();
  bool checkHVXAccum();

  static void compoundRegisterMap(unsigned &);

  bool isLoopRegister(MCRegister R) const {
    return (Hexagon::SA0 == R || Hexagon::LC0 == R || Hexagon::SA1 == R ||
            Hexagon::LC1 == R);
  }

public:
  explicit HexagonMCChecker(MCContext &Context, MCInstrInfo const &MCII,
                            MCSubtargetInfo const &STI, MCInst &mcb,
                            const MCRegisterInfo &ri, bool ReportErrors = true);
  explicit HexagonMCChecker(HexagonMCChecker const &Check,
                            MCSubtargetInfo const &STI, bool CopyReportErrors);

  bool check(bool FullCheck = true);
  void reportErrorRegisters(MCRegister Register);
  void reportErrorNewValue(MCRegister Register);
  void reportError(SMLoc Loc, Twine const &Msg);
  void reportNote(SMLoc Loc, Twine const &Msg);
  void reportError(Twine const &Msg);
  void reportWarning(Twine const &Msg);
  void reportBranchErrors();
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCCHECKER_H
