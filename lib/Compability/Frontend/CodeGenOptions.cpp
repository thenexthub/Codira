//===--- CodeGenOptions.cpp -----------------------------------------------===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/CodeGenOptions.h"
#include <optional>
#include <string.h>

namespace language::Compability::frontend {

CodeGenOptions::CodeGenOptions() {
#define CODEGENOPT(Name, Bits, Default) Name = Default;
#define ENUM_CODEGENOPT(Name, Type, Bits, Default) set##Name(Default);
#include "language/Compability/Frontend/CodeGenOptions.def"
}

std::optional<toolchain::CodeModel::Model> getCodeModel(toolchain::StringRef string) {
  return toolchain::StringSwitch<std::optional<toolchain::CodeModel::Model>>(string)
      .Case("tiny", toolchain::CodeModel::Model::Tiny)
      .Case("small", toolchain::CodeModel::Model::Small)
      .Case("kernel", toolchain::CodeModel::Model::Kernel)
      .Case("medium", toolchain::CodeModel::Model::Medium)
      .Case("large", toolchain::CodeModel::Model::Large)
      .Default(std::nullopt);
}

} // end namespace language::Compability::frontend
