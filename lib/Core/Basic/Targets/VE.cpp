//===--- VE.cpp - Implement VE target feature support ---------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
// This file implements VE TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "VE.h"
#include "language/Core/Basic/Builtins.h"
#include "language/Core/Basic/MacroBuilder.h"
#include "language/Core/Basic/TargetBuiltins.h"

using namespace language::Core;
using namespace language::Core::targets;

static constexpr int NumBuiltins =
    language::Core::VE::LastTSBuiltin - Builtin::FirstTSBuiltin;

static constexpr toolchain::StringTable BuiltinStrings =
    CLANG_BUILTIN_STR_TABLE_START
#define BUILTIN CLANG_BUILTIN_STR_TABLE
#include "language/Core/Basic/BuiltinsVE.def"
    ;

static constexpr auto BuiltinInfos = Builtin::MakeInfos<NumBuiltins>({
#define BUILTIN CLANG_BUILTIN_ENTRY
#include "language/Core/Basic/BuiltinsVE.def"
});

void VETargetInfo::getTargetDefines(const LangOptions &Opts,
                                    MacroBuilder &Builder) const {
  Builder.defineMacro("__ve", "1");
  Builder.defineMacro("__ve__", "1");
  Builder.defineMacro("__NEC__", "1");
  // FIXME: define __FAST_MATH__ 1 if -ffast-math is enabled
  // FIXME: define __OPTIMIZE__ n if -On is enabled
  // FIXME: define __VECTOR__ n 1 if automatic vectorization is enabled

  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8");
}

toolchain::SmallVector<Builtin::InfosShard> VETargetInfo::getTargetBuiltins() const {
  return {{&BuiltinStrings, BuiltinInfos}};
}
