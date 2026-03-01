//===-- TargetOptionsCommandFlags.h ----------------------------*- C++ -*-===//
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
//
// Helper to create TargetOptions from command line flags.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_COMMON_TARGETOPTIONSCOMMANDFLAGS_H
#define LLD_COMMON_TARGETOPTIONSCOMMANDFLAGS_H

#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetOptions.h"
#include <optional>

namespace lld {
llvm::TargetOptions initTargetOptionsFromCodeGenFlags();
std::optional<llvm::Reloc::Model> getRelocModelFromCMModel();
std::optional<llvm::CodeModel::Model> getCodeModelFromCMModel();
std::string getCPUStr();
std::vector<std::string> getMAttrs();
}

#endif
