//===-- ModelConsumer.h -----------------------------------------*- C++ -*-===//
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
/// This file implements language::Core::ento::ModelConsumer which is an
/// ASTConsumer for model files.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_FRONTEND_MODELCONSUMER_H
#define LANGUAGE_CORE_STATICANALYZER_FRONTEND_MODELCONSUMER_H

#include "language/Core/AST/ASTConsumer.h"
#include "toolchain/ADT/StringMap.h"

namespace language::Core {

class Stmt;

namespace ento {

/// ASTConsumer to consume model files' AST.
///
/// This consumer collects the bodies of function definitions into a StringMap
/// from a model file.
class ModelConsumer : public ASTConsumer {
public:
  ModelConsumer(toolchain::StringMap<Stmt *> &Bodies);

  bool HandleTopLevelDecl(DeclGroupRef D) override;

private:
  toolchain::StringMap<Stmt *> &Bodies;
};
}
}

#endif
