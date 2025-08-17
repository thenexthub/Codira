//===-- CodeGen/ObjectFilePCHContainerWriter.h ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_CODEGEN_OBJECTFILEPCHCONTAINEROPERATIONS_H
#define LANGUAGE_CORE_CODEGEN_OBJECTFILEPCHCONTAINEROPERATIONS_H

#include "language/Core/Frontend/PCHContainerOperations.h"

namespace language::Core {

/// A PCHContainerWriter implementation that uses LLVM to
/// wraps Clang modules inside a COFF, ELF, or Mach-O container.
class ObjectFilePCHContainerWriter : public PCHContainerWriter {
  StringRef getFormat() const override { return "obj"; }

  /// Return an ASTConsumer that can be chained with a
  /// PCHGenerator that produces a wrapper file format
  /// that also contains full debug info for the module.
  std::unique_ptr<ASTConsumer>
  CreatePCHContainerGenerator(CompilerInstance &CI,
                              const std::string &MainFileName,
                              const std::string &OutputFileName,
                              std::unique_ptr<toolchain::raw_pwrite_stream> OS,
                              std::shared_ptr<PCHBuffer> Buffer) const override;
};

}

#endif
