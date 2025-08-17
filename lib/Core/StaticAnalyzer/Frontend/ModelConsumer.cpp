//===--- ModelConsumer.cpp - ASTConsumer for consuming model files --------===//
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
///
/// \file
/// This file implements an ASTConsumer for consuming model files.
///
/// This ASTConsumer handles the AST of a parsed model file. All top level
/// function definitions will be collected from that model file for later
/// retrieval during the static analysis. The body of these functions will not
/// be injected into the ASTUnit of the analyzed translation unit. It will be
/// available through the BodyFarm which is utilized by the AnalysisDeclContext
/// class.
///
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Frontend/ModelConsumer.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclGroup.h"

using namespace language::Core;
using namespace ento;

ModelConsumer::ModelConsumer(toolchain::StringMap<Stmt *> &Bodies)
    : Bodies(Bodies) {}

bool ModelConsumer::HandleTopLevelDecl(DeclGroupRef DeclGroup) {
  for (const Decl *D : DeclGroup) {
    // Only interested in definitions.
    const auto *func = toolchain::dyn_cast<FunctionDecl>(D);
    if (func && func->hasBody()) {
      Bodies.insert(std::make_pair(func->getName(), func->getBody()));
    }
  }
  return true;
}
