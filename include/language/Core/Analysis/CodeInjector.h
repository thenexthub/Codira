//===-- CodeInjector.h ------------------------------------------*- C++ -*-===//
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
/// Defines the language::Core::CodeInjector interface which is responsible for
/// injecting AST of function definitions that may not be available in the
/// original source.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_CODEINJECTOR_H
#define LANGUAGE_CORE_ANALYSIS_CODEINJECTOR_H

namespace language::Core {

class Stmt;
class FunctionDecl;
class ObjCMethodDecl;

/// CodeInjector is an interface which is responsible for injecting AST
/// of function definitions that may not be available in the original source.
///
/// The getBody function will be called each time the static analyzer examines a
/// function call that has no definition available in the current translation
/// unit. If the returned statement is not a null pointer, it is assumed to be
/// the body of a function which will be used for the analysis. The source of
/// the body can be arbitrary, but it is advised to use memoization to avoid
/// unnecessary reparsing of the external source that provides the body of the
/// functions.
class CodeInjector {
public:
  CodeInjector();
  virtual ~CodeInjector();

  virtual Stmt *getBody(const FunctionDecl *D) = 0;
  virtual Stmt *getBody(const ObjCMethodDecl *D) = 0;
};
}

#endif
