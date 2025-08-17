//===--- TestAST.h - Build clang ASTs for testing -------------------------===//
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
// In normal operation of Clang, the FrontendAction's lifecycle both creates
// and destroys the AST, and code should operate on it during callbacks in
// between (e.g. via ASTConsumer).
//
// For tests it is often more convenient to parse an AST from code, and keep it
// alive as a normal local object, with assertions as straight-line code.
// TestAST provides such an interface.
// (ASTUnit can be used for this purpose, but is a production library with
// broad scope and complicated API).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TESTING_TESTAST_H
#define LANGUAGE_CORE_TESTING_TESTAST_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Testing/CommandLineArgs.h"
#include "toolchain/ADT/StringRef.h"
#include <string>
#include <vector>

namespace language::Core {

/// Specifies a virtual source file to be parsed as part of a test.
struct TestInputs {
  TestInputs() = default;
  TestInputs(StringRef Code) : Code(Code) {}

  /// The source code of the input file to be parsed.
  std::string Code;

  /// The language to parse as.
  /// This affects the -x and -std flags used, and the filename.
  TestLanguage Language = TestLanguage::Lang_OBJCXX;

  /// Extra argv to pass to clang -cc1.
  std::vector<std::string> ExtraArgs = {};

  /// Extra virtual files that are available to be #included.
  /// Keys are plain filenames ("foo.h"), values are file content.
  toolchain::StringMap<std::string> ExtraFiles = {};

  /// Root of execution, all relative paths in Args/Files are resolved against
  /// this.
  std::string WorkingDir;

  /// Filename to use for translation unit. A default will be used when empty.
  std::string FileName;

  /// By default, error diagnostics during parsing are reported as gtest errors.
  /// To suppress this, set ErrorOK or include "error-ok" in a comment in Code.
  /// In either case, all diagnostics appear in TestAST::diagnostics().
  bool ErrorOK = false;

  /// The action used to parse the code.
  /// By default, a SyntaxOnlyAction is used.
  std::function<std::unique_ptr<FrontendAction>()> MakeAction;
};

/// The result of parsing a file specified by TestInputs.
///
/// The ASTContext, Sema etc are valid as long as this object is alive.
class TestAST {
public:
  /// Constructing a TestAST parses the virtual file.
  ///
  /// To keep tests terse, critical errors (e.g. invalid flags) are reported as
  /// unit test failures with ADD_FAILURE() and produce an empty ASTContext,
  /// Sema etc. This frees the test code from handling these explicitly.
  TestAST(const TestInputs &);
  TestAST(StringRef Code) : TestAST(TestInputs(Code)) {}
  TestAST(TestAST &&M);
  TestAST &operator=(TestAST &&);
  ~TestAST();

  /// Provides access to the AST context and other parts of Clang.

  ASTContext &context() { return Clang->getASTContext(); }
  Sema &sema() { return Clang->getSema(); }
  SourceManager &sourceManager() { return Clang->getSourceManager(); }
  FileManager &fileManager() { return Clang->getFileManager(); }
  Preprocessor &preprocessor() { return Clang->getPreprocessor(); }
  FrontendAction &action() { return *Action; }

  /// Returns diagnostics emitted during parsing.
  /// (By default, errors cause test failures, see TestInputs::ErrorOK).
  toolchain::ArrayRef<StoredDiagnostic> diagnostics() { return Diagnostics; }

private:
  void clear();
  std::unique_ptr<FrontendAction> Action;
  std::unique_ptr<CompilerInstance> Clang;
  std::vector<StoredDiagnostic> Diagnostics;
};

} // end namespace language::Core

#endif
