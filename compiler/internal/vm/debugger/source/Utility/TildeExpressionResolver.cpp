//===-- TildeExpressionResolver.cpp ---------------------------------------===//
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

#include "lldb/Utility/TildeExpressionResolver.h"

#include <cassert>
#include <system_error>

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#if !defined(_WIN32)
#include <pwd.h>
#endif

using namespace lldb_private;
using namespace llvm;

namespace fs = llvm::sys::fs;
namespace path = llvm::sys::path;

TildeExpressionResolver::~TildeExpressionResolver() = default;

bool StandardTildeExpressionResolver::ResolveExact(
    StringRef Expr, SmallVectorImpl<char> &Output) {
  // We expect the tilde expression to be ONLY the expression itself, and
  // contain no separators.
  assert(!llvm::any_of(Expr, [](char c) { return path::is_separator(c); }));
  assert(Expr.empty() || Expr[0] == '~');

  return !fs::real_path(Expr, Output, true);
}

bool StandardTildeExpressionResolver::ResolvePartial(StringRef Expr,
                                                     StringSet<> &Output) {
  // We expect the tilde expression to be ONLY the expression itself, and
  // contain no separators.
  assert(!llvm::any_of(Expr, [](char c) { return path::is_separator(c); }));
  assert(Expr.empty() || Expr[0] == '~');

  Output.clear();
#if defined(_WIN32) || defined(__ANDROID__)
  return false;
#else
  if (Expr.empty())
    return false;

  SmallString<32> Buffer("~");
  setpwent();
  struct passwd *user_entry;
  Expr = Expr.drop_front();

  while ((user_entry = getpwent()) != nullptr) {
    StringRef ThisName(user_entry->pw_name);
    if (!ThisName.starts_with(Expr))
      continue;

    Buffer.resize(1);
    Buffer.append(ThisName);
    Buffer.append(path::get_separator());
    Output.insert(Buffer);
  }

  return true;
#endif
}

bool TildeExpressionResolver::ResolveFullPath(
    StringRef Expr, llvm::SmallVectorImpl<char> &Output) {
  if (!Expr.starts_with("~")) {
    Output.assign(Expr.begin(), Expr.end());
    return false;
  }

  namespace path = llvm::sys::path;
  StringRef Left =
      Expr.take_until([](char c) { return path::is_separator(c); });

  if (!ResolveExact(Left, Output)) {
    Output.assign(Expr.begin(), Expr.end());
    return false;
  }

  Output.append(Expr.begin() + Left.size(), Expr.end());
  return true;
}
