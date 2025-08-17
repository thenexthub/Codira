//===- ASTSourceDescriptor.h -----------------------------*- C++ -*-===//
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
/// \file
/// Defines the language::Core::ASTSourceDescriptor class, which abstracts clang modules
/// and precompiled header files
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_ASTSOURCEDESCRIPTOR_H
#define LANGUAGE_CORE_BASIC_ASTSOURCEDESCRIPTOR_H

#include "language/Core/Basic/Module.h"
#include "toolchain/ADT/StringRef.h"
#include <string>
#include <utility>

namespace language::Core {

/// Abstracts clang modules and precompiled header files and holds
/// everything needed to generate debug info for an imported module
/// or PCH.
class ASTSourceDescriptor {
  StringRef PCHModuleName;
  StringRef Path;
  StringRef ASTFile;
  ASTFileSignature Signature;
  Module *ClangModule = nullptr;

public:
  ASTSourceDescriptor() = default;
  ASTSourceDescriptor(StringRef Name, StringRef Path, StringRef ASTFile,
                      ASTFileSignature Signature)
      : PCHModuleName(std::move(Name)), Path(std::move(Path)),
        ASTFile(std::move(ASTFile)), Signature(Signature) {}
  ASTSourceDescriptor(Module &M);

  std::string getModuleName() const;
  StringRef getPath() const { return Path; }
  StringRef getASTFile() const { return ASTFile; }
  ASTFileSignature getSignature() const { return Signature; }
  Module *getModuleOrNull() const { return ClangModule; }
};

} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_ASTSOURCEDESCRIPTOR_H
