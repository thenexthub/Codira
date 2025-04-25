//===--- SwiftInvocation.h - ------------------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKIT_LIB_SWIFTLANG_SWIFTINVOCATION_H
#define LLVM_SOURCEKIT_LIB_SWIFTLANG_SWIFTINVOCATION_H

#include "language/Basic/ThreadSafeRefCounted.h"
#include <string>
#include <vector>

namespace language {
  class CompilerInvocation;
}

namespace SourceKit {
  class SwiftASTManager;

/// Encompasses an invocation for getting an AST. This is used to control AST
/// sharing among different requests.
class SwiftInvocation : public llvm::ThreadSafeRefCountedBase<SwiftInvocation> {
public:
  ~SwiftInvocation();

  struct Implementation;
  Implementation &Impl;

  ArrayRef<std::string> getArgs() const;
  void applyTo(swift::CompilerInvocation &CompInvok) const;
  void raw(std::vector<std::string> &Args, std::string &PrimaryFile) const;

private:
  SwiftInvocation(Implementation &Impl) : Impl(Impl) { }
  friend class SwiftASTManager;
};

} // namespace SourceKit

#endif
