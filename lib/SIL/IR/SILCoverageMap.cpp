//===--- SILCoverageMap.cpp - Defines the SILCoverageMap class ------------===//
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
// This file defines the SILCoverageMap class, which is used to relay coverage
// mapping information from the AST to lower layers of the compiler.
//
//===----------------------------------------------------------------------===//

#include "toolchain/ADT/STLExtras.h"
#include "language/SIL/SILCoverageMap.h"
#include "language/SIL/SILModule.h"
#include "language/Basic/Assertions.h"

using namespace language;

using toolchain::coverage::CounterExpression;
using toolchain::coverage::CounterMappingRegion;

SILCoverageMap *
SILCoverageMap::create(SILModule &M, SourceFile *ParentSourceFile,
                       StringRef Filename, StringRef Name,
                       StringRef PGOFuncName, uint64_t Hash,
                       ArrayRef<MappedRegion> MappedRegions,
                       ArrayRef<CounterExpression> Expressions) {
  auto *Buf = M.allocate<SILCoverageMap>(1);
  SILCoverageMap *CM = ::new (Buf) SILCoverageMap(ParentSourceFile, Hash);

  // Store a copy of the names so that we own the lifetime.
  CM->Filename = M.allocateCopy(Filename);
  CM->Name = M.allocateCopy(Name);
  CM->PGOFuncName = M.allocateCopy(PGOFuncName).str();

  // Since we have two arrays, we need to manually tail allocate each of them,
  // rather than relying on the flexible array trick.
  CM->MappedRegions = M.allocateCopy(MappedRegions);
  CM->Expressions = M.allocateCopy(Expressions);

  auto result = M.coverageMaps.insert({CM->PGOFuncName, CM});

  // Assert that this coverage map is unique.
  assert(result.second && "Duplicate coverage mapping for function");
  (void)result;

  return CM;
}

SILCoverageMap::SILCoverageMap(SourceFile *ParentSourceFile, uint64_t Hash)
  : ParentSourceFile(ParentSourceFile), Hash(Hash) {}

SILCoverageMap::~SILCoverageMap() {}

CounterMappingRegion
SILCoverageMap::MappedRegion::getLLVMRegion(unsigned int FileID) const {
  switch (RegionKind) {
  case MappedRegion::Kind::Code:
    return CounterMappingRegion::makeRegion(Counter, FileID, StartLine,
                                            StartCol, EndLine, EndCol);
  case MappedRegion::Kind::Skipped:
    return CounterMappingRegion::makeSkipped(FileID, StartLine, StartCol,
                                             EndLine, EndCol);
  }
  toolchain_unreachable("Unhandled case in switch!");
}

namespace {
struct Printer {
  const toolchain::coverage::Counter &C;
  ArrayRef<toolchain::coverage::CounterExpression> Exprs;
  Printer(const toolchain::coverage::Counter &C,
          ArrayRef<toolchain::coverage::CounterExpression> Exprs)
      : C(C), Exprs(Exprs) {}

  void print(raw_ostream &OS) const {
    // TODO: This format's nice and human readable, but does it fit well with
    // SIL's relatively simple structure?
    if (C.isZero())
      OS << "zero";
    else if (C.isExpression()) {
      assert(C.getExpressionID() < Exprs.size() && "expression out of range");
      const auto &E = Exprs[C.getExpressionID()];
      OS << '(' << Printer(E.LHS, Exprs)
         << (E.Kind == CounterExpression::Add ? " + " : " - ")
         << Printer(E.RHS, Exprs) << ')';
    } else
      OS << C.getCounterID();
  }

  friend raw_ostream &operator<<(raw_ostream &OS, const Printer &P) {
    P.print(OS);
    return OS;
  }
};
} // end anonymous namespace

void SILCoverageMap::printCounter(
    toolchain::raw_ostream &OS, toolchain::coverage::Counter C,
    ArrayRef<toolchain::coverage::CounterExpression> Expressions) {
  OS << Printer(C, Expressions);
}
