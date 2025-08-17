//===--- Options.h - Option info & table ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_DRIVER_OPTIONS_H
#define LANGUAGE_CORE_DRIVER_OPTIONS_H

#include "toolchain/Option/OptTable.h"
#include "toolchain/Option/Option.h"

namespace language::Core {
namespace driver {

namespace options {
/// Flags specifically for clang options.  Must not overlap with
/// toolchain::opt::DriverFlag.
enum ClangFlags {
  NoXarchOption = (1 << 4),
  LinkerInput = (1 << 5),
  NoArgumentUnused = (1 << 6),
  Unsupported = (1 << 7),
  LinkOption = (1 << 8),
  Ignored = (1 << 9),
  TargetSpecific = (1 << 10),
};

// Flags specifically for clang option visibility. We alias DefaultVis to
// ClangOption, because "DefaultVis" is confusing in Options.td, which is used
// for multiple drivers (clang, cl, flang, etc).
enum ClangVisibility {
  ClangOption = toolchain::opt::DefaultVis,
  CLOption = (1 << 1),
  CC1Option = (1 << 2),
  CC1AsOption = (1 << 3),
  FlangOption = (1 << 4),
  FC1Option = (1 << 5),
  DXCOption = (1 << 6),
};

enum ID {
    OPT_INVALID = 0, // This is not an option ID.
#define OPTION(...) LLVM_MAKE_OPT_ID(__VA_ARGS__),
#include "language/Core/Driver/Options.inc"
    LastOption
#undef OPTION
  };
}

const toolchain::opt::OptTable &getDriverOptTable();
}
}

#endif
