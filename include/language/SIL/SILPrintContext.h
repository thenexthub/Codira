//===--- SILPrintContext.h - Context for SIL print functions ----*- C++ -*-===//
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

#ifndef LANGUAGE_SIL_PRINTCONTEXT_H
#define LANGUAGE_SIL_PRINTCONTEXT_H

#include "language/AST/SILOptions.h"
#include "language/SIL/SILDebugScope.h"
#include "language/SIL/SILValue.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {

class SILDebugScope;
class SILInstruction;
class SILFunction;
class SILBasicBlock;

/// Used as context for the SIL print functions.
class SILPrintContext {
public:
  struct ID {
    enum ID_Kind { SILBasicBlock, SILUndef, SSAValue, Null } Kind;
    unsigned Number;

    // A stable ordering of ID objects.
    bool operator<(ID Other) const {
      if (unsigned(Kind) < unsigned(Other.Kind))
        return true;
      if (Number < Other.Number)
        return true;
      return false;
    }

    void print(raw_ostream &OS);
  };

protected:
  // Cache block and value identifiers for this function. This is useful in
  // general for identifying entities, not just emitting textual SIL.
  //
  const void *ContextFunctionOrBlock = nullptr;
  toolchain::DenseMap<const SILBasicBlock *, unsigned> BlocksToIDMap;
  toolchain::DenseMap<const SILNode *, unsigned> ValueToIDMap;

  toolchain::raw_ostream &OutStream;

  toolchain::DenseMap<const SILDebugScope *, unsigned> ScopeToIDMap;

  /// Dump more information in the SIL output.
  bool Verbose;
  
  /// Sort all kind of tables to ease diffing.
  bool SortedSIL;

  /// Print debug locations and scopes.
  bool DebugInfo;

  /// See \ref FrontendOptions.PrintFullConvention.
  bool PrintFullConvention;

public:
  /// Constructor with default values for options.
  ///
  /// DebugInfo will be set according to the -sil-print-debuginfo option.
  SILPrintContext(toolchain::raw_ostream &OS, bool Verbose = false,
                  bool SortedSIL = false, bool PrintFullConvention = false);

  /// Constructor based on SILOptions.
  ///
  /// DebugInfo will be set according to SILOptions::PrintDebugInfo or
  /// the -sil-print-debuginfo option.
  SILPrintContext(toolchain::raw_ostream &OS, const SILOptions &Opts);

  SILPrintContext(toolchain::raw_ostream &OS, bool Verbose, bool SortedSIL,
                  bool DebugInfo, bool PrintFullConvention);

  virtual ~SILPrintContext();

  void setContext(const void *FunctionOrBlock);

  // Initialized block IDs from the order provided in `blocks`.
  void initBlockIDs(ArrayRef<const SILBasicBlock *> Blocks);

  /// Returns the output stream for printing.
  toolchain::raw_ostream &OS() const { return OutStream; }

  /// Returns true if the SIL output should be sorted.
  bool sortSIL() const { return SortedSIL; }
  
  /// Returns true if verbose SIL should be printed.
  bool printVerbose() const { return Verbose; }

  /// Returns true if debug locations and scopes should be printed.
  bool printDebugInfo() const { return DebugInfo; }

  /// Returns true if the entire @convention(c, cType: ..) should be printed.
  bool printFullConvention() const { return PrintFullConvention; }

  SILPrintContext::ID getID(const SILBasicBlock *Block);

  SILPrintContext::ID getID(SILNodePointer node);

  /// Returns true if the \p Scope has and ID assigned.
  bool hasScopeID(const SILDebugScope *Scope) const {
    return ScopeToIDMap.count(Scope) != 0;
  }

  /// Returns the ID of \p Scope.
  unsigned getScopeID(const SILDebugScope *Scope) const {
    return ScopeToIDMap.lookup(Scope);
  }

  /// Assigns the next available ID to \p Scope.
  unsigned assignScopeID(const SILDebugScope *Scope) {
    assert(!hasScopeID(Scope));
    unsigned ID = ScopeToIDMap.size() + 1;
    ScopeToIDMap.insert({Scope, ID});
    return ID;
  }

  /// Callback which is invoked by the SILPrinter before the instruction \p I
  /// is written.
  virtual void printInstructionCallBack(const SILInstruction *I);
};

raw_ostream &operator<<(raw_ostream &OS, SILPrintContext::ID i);

} // end namespace language

#endif // LANGUAGE_SIL_PRINTCONTEXT_H
