//===-- FrontendActions.h - Useful Frontend Actions -------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_STATICANALYZER_FRONTEND_FRONTENDACTIONS_H
#define LANGUAGE_CORE_STATICANALYZER_FRONTEND_FRONTENDACTIONS_H

#include "language/Core/Frontend/FrontendAction.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {

class Stmt;

namespace ento {

//===----------------------------------------------------------------------===//
// AST Consumer Actions
//===----------------------------------------------------------------------===//

class AnalysisAction : public ASTFrontendAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;
};

/// Frontend action to parse model files.
///
/// This frontend action is responsible for parsing model files. Model files can
/// not be parsed on their own, they rely on type information that is available
/// in another translation unit. The parsing of model files is done by a
/// separate compiler instance that reuses the ASTContext and othen information
/// from the main translation unit that is being compiled. After a model file is
/// parsed, the function definitions will be collected into a StringMap.
class ParseModelFileAction : public ASTFrontendAction {
public:
  ParseModelFileAction(toolchain::StringMap<Stmt *> &Bodies);
  bool isModelParsingAction() const override { return true; }

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;

private:
  toolchain::StringMap<Stmt *> &Bodies;
};

} // namespace ento
} // end namespace language::Core

#endif
