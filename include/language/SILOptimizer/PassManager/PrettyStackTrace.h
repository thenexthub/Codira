//===--- PrettyStackTrace.h - PrettyStackTrace for Transforms ---*- C++ -*-===//
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

#ifndef SWIFT_SILOPTIMIZER_PASSMANAGER_PRETTYSTACKTRACE_H
#define SWIFT_SILOPTIMIZER_PASSMANAGER_PRETTYSTACKTRACE_H

#include "language/SIL/PrettyStackTrace.h"
#include "llvm/Support/PrettyStackTrace.h"

namespace language {

class SILFunctionTransform;
class SILModuleTransform;

class PrettyStackTraceSILFunctionTransform
    : public PrettyStackTraceSILFunction {
  SILFunctionTransform *SFT;
  unsigned PassNumber;

public:
  PrettyStackTraceSILFunctionTransform(SILFunctionTransform *SFT,
                                       unsigned PassNumber);

  virtual void print(llvm::raw_ostream &OS) const override;
};

class PrettyStackTraceSILModuleTransform : public llvm::PrettyStackTraceEntry {
  SILModuleTransform *SMT;
  unsigned PassNumber;

public:
  PrettyStackTraceSILModuleTransform(SILModuleTransform *SMT,
                                     unsigned PassNumber)
      : SMT(SMT), PassNumber(PassNumber) {}
  virtual void print(llvm::raw_ostream &OS) const override;
};

} // end namespace language

#endif
