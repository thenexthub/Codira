//===--- CodiraInvocation.h - ------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_LANGUAGEINVOCATION_H
#define TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_LANGUAGEINVOCATION_H

#include "language/Basic/ThreadSafeRefCounted.h"
#include <string>
#include <vector>

namespace language {
  class CompilerInvocation;
}

namespace SourceKit {
  class CodiraASTManager;

/// Encompasses an invocation for getting an AST. This is used to control AST
/// sharing among different requests.
class CodiraInvocation : public toolchain::ThreadSafeRefCountedBase<CodiraInvocation> {
public:
  ~CodiraInvocation();

  struct Implementation;
  Implementation &Impl;

  ArrayRef<std::string> getArgs() const;
  void applyTo(language::CompilerInvocation &CompInvok) const;
  void raw(std::vector<std::string> &Args, std::string &PrimaryFile) const;

private:
  CodiraInvocation(Implementation &Impl) : Impl(Impl) { }
  friend class CodiraASTManager;
};

} // namespace SourceKit

#endif
