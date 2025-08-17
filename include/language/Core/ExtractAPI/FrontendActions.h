//===- ExtractAPI/FrontendActions.h -----------------------------*- C++ -*-===//
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
/// This file defines the ExtractAPIAction and WrappingExtractAPIAction frontend
/// actions.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_EXTRACTAPI_FRONTEND_ACTIONS_H
#define LANGUAGE_CORE_EXTRACTAPI_FRONTEND_ACTIONS_H

#include "language/Core/ExtractAPI/ExtractAPIActionBase.h"
#include "language/Core/Frontend/FrontendAction.h"

namespace language::Core {

/// ExtractAPIAction sets up the output file and creates the ExtractAPIVisitor.
class ExtractAPIAction : public ASTFrontendAction,
                         private ExtractAPIActionBase {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;

private:

  /// The input file originally provided on the command line.
  ///
  /// This captures the spelling used to include the file and whether the
  /// include is quoted or not.
  SmallVector<std::pair<SmallString<32>, bool>> KnownInputFiles;

  /// Prepare to execute the action on the given CompilerInstance.
  ///
  /// This is called before executing the action on any inputs. This generates a
  /// single header that includes all of CI's inputs and replaces CI's input
  /// list with it before actually executing the action.
  bool PrepareToExecuteAction(CompilerInstance &CI) override;

  /// Called after executing the action on the synthesized input buffer.
  ///
  /// Note: Now that we have gathered all the API definitions to surface we can
  /// emit them in this callback.
  void EndSourceFileAction() override;

  static StringRef getInputBufferName() { return "<extract-api-includes>"; }
};

/// Wrap ExtractAPIAction on top of a pre-existing action
///
/// Used when the ExtractAPI action needs to be executed as a side effect of a
/// regular compilation job. Unlike ExtarctAPIAction, this is meant to be used
/// on regular source files ( .m , .c files) instead of header files
class WrappingExtractAPIAction : public WrapperFrontendAction,
                                 private ExtractAPIActionBase {
public:
  WrappingExtractAPIAction(std::unique_ptr<FrontendAction> WrappedAction)
      : WrapperFrontendAction(std::move(WrappedAction)) {}

protected:
  /// Create ExtractAPI consumer multiplexed on another consumer.
  ///
  /// This allows us to execute ExtractAPI action while on top of
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;

private:
  /// Flag to check if the wrapper front end action's consumer is
  /// craeted or not
  bool CreatedASTConsumer = false;

  void EndSourceFile() override { FrontendAction::EndSourceFile(); }

  /// Called after executing the action on the synthesized input buffer.
  ///
  /// Executes both Wrapper and ExtractAPIBase end source file
  /// actions. This is the place where all the gathered symbol graph
  /// information is emited.
  void EndSourceFileAction() override;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_EXTRACTAPI_FRONTEND_ACTIONS_H
