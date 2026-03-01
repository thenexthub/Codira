//===--------------------- TildeExpressionResolver.h ------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_TILDEEXPRESSIONRESOLVER_H
#define LLDB_UTILITY_TILDEEXPRESSIONRESOLVER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"

namespace llvm {
template <typename T> class SmallVectorImpl;
}

namespace lldb_private {
class TildeExpressionResolver {
public:
  virtual ~TildeExpressionResolver();

  /// Resolve a Tilde Expression contained according to bash rules.
  ///
  /// \param Expr Contains the tilde expression to resolve.  A valid tilde
  ///             expression must begin with a tilde and contain only non
  ///             separator characters.
  ///
  /// \param Output Contains the resolved tilde expression, or the original
  ///               input if the tilde expression could not be resolved.
  ///
  /// \returns true if \p Expr was successfully resolved, false otherwise.
  virtual bool ResolveExact(llvm::StringRef Expr,
                            llvm::SmallVectorImpl<char> &Output) = 0;

  /// Auto-complete a tilde expression with all matching values.
  ///
  /// \param Expr Contains the tilde expression prefix to resolve.  See
  ///             ResolveExact() for validity rules.
  ///
  /// \param Output Contains all matching home directories, each one
  ///               itself unresolved (i.e. you need to call ResolveExact
  ///               on each item to turn it into a real path).
  ///
  /// \returns true if there were any matches, false otherwise.
  virtual bool ResolvePartial(llvm::StringRef Expr,
                              llvm::StringSet<> &Output) = 0;

  /// Resolve an entire path that begins with a tilde expression, replacing
  /// the username portion with the matched result.
  bool ResolveFullPath(llvm::StringRef Expr,
                       llvm::SmallVectorImpl<char> &Output);
};

class StandardTildeExpressionResolver : public TildeExpressionResolver {
public:
  bool ResolveExact(llvm::StringRef Expr,
                    llvm::SmallVectorImpl<char> &Output) override;
  bool ResolvePartial(llvm::StringRef Expr, llvm::StringSet<> &Output) override;
};
}

#endif // LLDB_UTILITY_TILDEEXPRESSIONRESOLVER_H
