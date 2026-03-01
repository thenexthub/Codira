//===-- MockTildeExpressionResolver.h ---------------------------*- C++ -*-===//
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

#ifndef LLDB_UNITTESTS_TESTINGSUPPORT_MOCKTILDEEXPRESSIONRESOLVER_H
#define LLDB_UNITTESTS_TESTINGSUPPORT_MOCKTILDEEXPRESSIONRESOLVER_H

#include "lldb/Utility/TildeExpressionResolver.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"

namespace lldb_private {
class MockTildeExpressionResolver : public TildeExpressionResolver {
  llvm::StringRef CurrentUser;
  llvm::StringMap<llvm::StringRef> UserDirectories;

public:
  MockTildeExpressionResolver(llvm::StringRef CurrentUser,
                              llvm::StringRef HomeDir);

  void AddKnownUser(llvm::StringRef User, llvm::StringRef HomeDir);
  void Clear();
  void SetCurrentUser(llvm::StringRef User);

  bool ResolveExact(llvm::StringRef Expr,
                    llvm::SmallVectorImpl<char> &Output) override;
  bool ResolvePartial(llvm::StringRef Expr, llvm::StringSet<> &Output) override;
};
} // namespace lldb_private

#endif
