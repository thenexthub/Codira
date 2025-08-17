//===--- ASTConsumers.h - ASTConsumer implementations -----------*- C++ -*-===//
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
// AST Consumers.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_REWRITE_FRONTEND_ASTCONSUMERS_H
#define LANGUAGE_CORE_REWRITE_FRONTEND_ASTCONSUMERS_H

#include "language/Core/Basic/LLVM.h"
#include <memory>
#include <string>

namespace language::Core {

class ASTConsumer;
class DiagnosticsEngine;
class LangOptions;
class Preprocessor;

// ObjC rewriter: attempts to rewrite ObjC constructs into pure C code.
// This is considered experimental, and only works with Apple's ObjC runtime.
std::unique_ptr<ASTConsumer>
CreateObjCRewriter(const std::string &InFile, std::unique_ptr<raw_ostream> OS,
                   DiagnosticsEngine &Diags, const LangOptions &LOpts,
                   bool SilenceRewriteMacroWarning);
std::unique_ptr<ASTConsumer>
CreateModernObjCRewriter(const std::string &InFile,
                         std::unique_ptr<raw_ostream> OS,
                         DiagnosticsEngine &Diags, const LangOptions &LOpts,
                         bool SilenceRewriteMacroWarning, bool LineInfo);

/// CreateHTMLPrinter - Create an AST consumer which rewrites source code to
/// HTML with syntax highlighting suitable for viewing in a web-browser.
std::unique_ptr<ASTConsumer> CreateHTMLPrinter(std::unique_ptr<raw_ostream> OS,
                                               Preprocessor &PP,
                                               bool SyntaxHighlight = true,
                                               bool HighlightMacros = true);

} // end clang namespace

#endif
