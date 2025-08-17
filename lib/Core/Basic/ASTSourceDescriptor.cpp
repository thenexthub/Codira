//===- ASTSourceDescriptor.cpp -------------------------------------===//
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
/// Defines the language::Core::ASTSourceDescriptor class, which abstracts clang modules
/// and precompiled header files
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/ASTSourceDescriptor.h"

namespace language::Core {

ASTSourceDescriptor::ASTSourceDescriptor(Module &M)
    : Signature(M.Signature), ClangModule(&M) {
  if (M.Directory)
    Path = M.Directory->getName();
  if (auto File = M.getASTFile())
    ASTFile = File->getName();
}

std::string ASTSourceDescriptor::getModuleName() const {
  if (ClangModule)
    return ClangModule->Name;
  else
    return std::string(PCHModuleName);
}

} // namespace language::Core
