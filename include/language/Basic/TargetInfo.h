//===--- TargetInfo.h - Target Info Output ---------------------*- C++ -*-===//
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
// This file provides a high-level API for emitting target info
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_TARGETINFO_H
#define SWIFT_TARGETINFO_H

#include "language/Basic/LLVM.h"

#include <optional>

namespace llvm {
class Triple;
class VersionTuple;
}

namespace language {
class CompilerInvocation;

namespace targetinfo {
void printTargetInfo(const CompilerInvocation &invocation,
                     llvm::raw_ostream &out);

void printTripleInfo(const CompilerInvocation &invocation,
                     const llvm::Triple &triple,
                     std::optional<llvm::VersionTuple> runtimeVersion,
                     llvm::raw_ostream &out);
} // namespace targetinfo
} // namespace language

#endif
