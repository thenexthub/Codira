//===-- MockTildeExpressionResolver.cpp -----------------------------------===//
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

#include "MockTildeExpressionResolver.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Path.h"

using namespace lldb_private;
using namespace llvm;

MockTildeExpressionResolver::MockTildeExpressionResolver(StringRef CurrentUser,
                                                         StringRef HomeDir)
    : CurrentUser(CurrentUser) {
  UserDirectories.insert(std::make_pair(CurrentUser, HomeDir));
}

void MockTildeExpressionResolver::AddKnownUser(StringRef User,
                                               StringRef HomeDir) {
  assert(!UserDirectories.contains(User));
  UserDirectories.insert(std::make_pair(User, HomeDir));
}

void MockTildeExpressionResolver::Clear() {
  CurrentUser = StringRef();
  UserDirectories.clear();
}

void MockTildeExpressionResolver::SetCurrentUser(StringRef User) {
  assert(UserDirectories.contains(User));
  CurrentUser = User;
}

bool MockTildeExpressionResolver::ResolveExact(StringRef Expr,
                                               SmallVectorImpl<char> &Output) {
  Output.clear();

  assert(!llvm::any_of(
      Expr, [](char c) { return llvm::sys::path::is_separator(c); }));
  assert(Expr.empty() || Expr[0] == '~');
  Expr = Expr.drop_front();
  if (Expr.empty()) {
    auto Dir = UserDirectories[CurrentUser];
    Output.append(Dir.begin(), Dir.end());
    return true;
  }

  for (const auto &User : UserDirectories) {
    if (User.getKey() != Expr)
      continue;
    Output.append(User.getValue().begin(), User.getValue().end());
    return true;
  }
  return false;
}

bool MockTildeExpressionResolver::ResolvePartial(StringRef Expr,
                                                 StringSet<> &Output) {
  Output.clear();

  assert(!llvm::any_of(
      Expr, [](char c) { return llvm::sys::path::is_separator(c); }));
  assert(Expr.empty() || Expr[0] == '~');
  Expr = Expr.drop_front();

  SmallString<16> QualifiedName("~");
  for (const auto &User : UserDirectories) {
    if (!User.getKey().starts_with(Expr))
      continue;
    QualifiedName.resize(1);
    QualifiedName.append(User.getKey().begin(), User.getKey().end());
    Output.insert(QualifiedName);
  }

  return !Output.empty();
}
