//===- CommonOptionsParser.h - common options for clang tools -*- C++ -*-=====//
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
//  This file implements the CommonOptionsParser class used to parse common
//  command-line options for clang tools, so that they can be run as separate
//  command-line applications with a consistent common interface for handling
//  compilation database and input files.
//
//  It provides a common subset of command-line options, common algorithm
//  for locating a compilation database and source files, and help messages
//  for the basic command-line interface.
//
//  It creates a CompilationDatabase and reads common command-line options.
//
//  This class uses the Clang Tooling infrastructure, see
//    http://clang.toolchain.org/docs/HowToSetupToolingForLLVM.html
//  for details on setting it up with LLVM source tree.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_COMMONOPTIONSPARSER_H
#define LANGUAGE_CORE_TOOLING_COMMONOPTIONSPARSER_H

#include "language/Core/Tooling/ArgumentsAdjusters.h"
#include "language/Core/Tooling/CompilationDatabase.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Error.h"

namespace language::Core {
namespace tooling {
/// A parser for options common to all command-line Clang tools.
///
/// Parses a common subset of command-line arguments, locates and loads a
/// compilation commands database and runs a tool with user-specified action. It
/// also contains a help message for the common command-line options.
///
/// An example of usage:
/// \code
/// #include "language/Core/Frontend/FrontendActions.h"
/// #include "language/Core/Tooling/CommonOptionsParser.h"
/// #include "language/Core/Tooling/Tooling.h"
/// #include "toolchain/Support/CommandLine.h"
///
/// using namespace language::Core::tooling;
/// using namespace toolchain;
///
/// static cl::OptionCategory MyToolCategory("my-tool options");
/// static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
/// static cl::extrahelp MoreHelp("\nMore help text...\n");
///
/// int main(int argc, const char **argv) {
///   auto ExpectedParser =
///       CommonOptionsParser::create(argc, argv, MyToolCategory);
///   if (!ExpectedParser) {
///     toolchain::errs() << ExpectedParser.takeError();
///     return 1;
///   }
///   CommonOptionsParser& OptionsParser = ExpectedParser.get();
///   ClangTool Tool(OptionsParser.getCompilations(),
///                  OptionsParser.getSourcePathList());
///   return Tool.run(
///       newFrontendActionFactory<language::Core::SyntaxOnlyAction>().get());
/// }
/// \endcode
class CommonOptionsParser {

protected:
  /// Parses command-line, initializes a compilation database.
  ///
  /// This constructor can change argc and argv contents, e.g. consume
  /// command-line options used for creating FixedCompilationDatabase.
  ///
  /// All options not belonging to \p Category become hidden.
  ///
  /// It also allows calls to set the required number of positional parameters.
  CommonOptionsParser(
      int &argc, const char **argv, toolchain::cl::OptionCategory &Category,
      toolchain::cl::NumOccurrencesFlag OccurrencesFlag = toolchain::cl::OneOrMore,
      const char *Overview = nullptr);

public:
  /// A factory method that is similar to the above constructor, except
  /// this returns an error instead exiting the program on error.
  static toolchain::Expected<CommonOptionsParser>
  create(int &argc, const char **argv, toolchain::cl::OptionCategory &Category,
         toolchain::cl::NumOccurrencesFlag OccurrencesFlag = toolchain::cl::OneOrMore,
         const char *Overview = nullptr);

  /// Returns a reference to the loaded compilations database.
  CompilationDatabase &getCompilations() {
    return *Compilations;
  }

  /// Returns a list of source file paths to process.
  const std::vector<std::string> &getSourcePathList() const {
    return SourcePathList;
  }

  /// Returns the argument adjuster calculated from "--extra-arg" and
  //"--extra-arg-before" options.
  ArgumentsAdjuster getArgumentsAdjuster() { return Adjuster; }

  static const char *const HelpMessage;

private:
  CommonOptionsParser() = default;

  toolchain::Error init(int &argc, const char **argv,
                   toolchain::cl::OptionCategory &Category,
                   toolchain::cl::NumOccurrencesFlag OccurrencesFlag,
                   const char *Overview);

  std::unique_ptr<CompilationDatabase> Compilations;
  std::vector<std::string> SourcePathList;
  ArgumentsAdjuster Adjuster;
};

class ArgumentsAdjustingCompilations : public CompilationDatabase {
public:
  ArgumentsAdjustingCompilations(
      std::unique_ptr<CompilationDatabase> Compilations)
      : Compilations(std::move(Compilations)) {}

  void appendArgumentsAdjuster(ArgumentsAdjuster Adjuster);

  std::vector<CompileCommand>
  getCompileCommands(StringRef FilePath) const override;

  std::vector<std::string> getAllFiles() const override;

  std::vector<CompileCommand> getAllCompileCommands() const override;

private:
  std::unique_ptr<CompilationDatabase> Compilations;
  std::vector<ArgumentsAdjuster> Adjusters;

  std::vector<CompileCommand>
  adjustCommands(std::vector<CompileCommand> Commands) const;
};

}  // namespace tooling
}  // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_COMMONOPTIONSPARSER_H
