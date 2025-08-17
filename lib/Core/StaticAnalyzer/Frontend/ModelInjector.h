//===-- ModelInjector.h -----------------------------------------*- C++ -*-===//
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
/// This file defines the language::Core::ento::ModelInjector class which implements the
/// language::Core::CodeInjector interface. This class is responsible for injecting
/// function definitions that were synthesized from model files.
///
/// Model files allow definitions of functions to be lazily constituted for functions
/// which lack bodies in the original source code.  This allows the analyzer
/// to more precisely analyze code that calls such functions, analyzing the
/// artificial definitions (which typically approximate the semantics of the
/// called function) when called by client code.  These definitions are
/// reconstituted lazily, on-demand, by the static analyzer engine.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SA_FRONTEND_MODELINJECTOR_H
#define LANGUAGE_CORE_SA_FRONTEND_MODELINJECTOR_H

#include "language/Core/Analysis/CodeInjector.h"
#include "toolchain/ADT/StringMap.h"

namespace language::Core {

class CompilerInstance;
class NamedDecl;

namespace ento {
class ModelInjector : public CodeInjector {
public:
  ModelInjector(CompilerInstance &CI);
  Stmt *getBody(const FunctionDecl *D) override;
  Stmt *getBody(const ObjCMethodDecl *D) override;

private:
  /// Synthesize a body for a declaration
  ///
  /// This method first looks up the appropriate model file based on the
  /// model-path configuration option and the name of the declaration that is
  /// looked up. If no model were synthesized yet for a function with that name
  /// it will create a new compiler instance to parse the model file using the
  /// ASTContext, Preprocessor, SourceManager of the original compiler instance.
  /// The former resources are shared between the two compiler instance, so the
  /// newly created instance have to "leak" these objects, since they are owned
  /// by the original instance.
  ///
  /// The model-path should be either an absolute path or relative to the
  /// working directory of the compiler.
  void onBodySynthesis(const NamedDecl *D);

  CompilerInstance &CI;

  // FIXME: double memoization is redundant, with memoization both here and in
  // BodyFarm.
  toolchain::StringMap<Stmt *> Bodies;
};
}
}

#endif
