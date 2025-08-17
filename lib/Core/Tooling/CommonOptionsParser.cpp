//===--- CommonOptionsParser.cpp - common options for clang tools ---------===//
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

#include "language/Core/Tooling/CommonOptionsParser.h"
#include "language/Core/Tooling/Tooling.h"
#include "toolchain/Support/CommandLine.h"

using namespace language::Core::tooling;
using namespace toolchain;

const char *const CommonOptionsParser::HelpMessage =
    "\n"
    "-p <build-path> is used to read a compile command database.\n"
    "\n"
    "\tFor example, it can be a CMake build directory in which a file named\n"
    "\tcompile_commands.json exists (use -DCMAKE_EXPORT_COMPILE_COMMANDS=ON\n"
    "\tCMake option to get this output). When no build path is specified,\n"
    "\ta search for compile_commands.json will be attempted through all\n"
    "\tparent paths of the first input file . See:\n"
    "\thttps://clang.toolchain.org/docs/HowToSetupToolingForLLVM.html for an\n"
    "\texample of setting up Clang Tooling on a source tree.\n"
    "\n"
    "<source0> ... specify the paths of source files. These paths are\n"
    "\tlooked up in the compile command database. If the path of a file is\n"
    "\tabsolute, it needs to point into CMake's source tree. If the path is\n"
    "\trelative, the current working directory needs to be in the CMake\n"
    "\tsource tree and the file must be in a subdirectory of the current\n"
    "\tworking directory. \"./\" prefixes in the relative files will be\n"
    "\tautomatically removed, but the rest of a relative path must be a\n"
    "\tsuffix of a path in the compile command database.\n"
    "\n";

void ArgumentsAdjustingCompilations::appendArgumentsAdjuster(
    ArgumentsAdjuster Adjuster) {
  Adjusters.push_back(std::move(Adjuster));
}

std::vector<CompileCommand> ArgumentsAdjustingCompilations::getCompileCommands(
    StringRef FilePath) const {
  return adjustCommands(Compilations->getCompileCommands(FilePath));
}

std::vector<std::string>
ArgumentsAdjustingCompilations::getAllFiles() const {
  return Compilations->getAllFiles();
}

std::vector<CompileCommand>
ArgumentsAdjustingCompilations::getAllCompileCommands() const {
  return adjustCommands(Compilations->getAllCompileCommands());
}

std::vector<CompileCommand> ArgumentsAdjustingCompilations::adjustCommands(
    std::vector<CompileCommand> Commands) const {
  for (CompileCommand &Command : Commands)
    for (const auto &Adjuster : Adjusters)
      Command.CommandLine = Adjuster(Command.CommandLine, Command.Filename);
  return Commands;
}

toolchain::Error CommonOptionsParser::init(
    int &argc, const char **argv, cl::OptionCategory &Category,
    toolchain::cl::NumOccurrencesFlag OccurrencesFlag, const char *Overview) {

  static cl::opt<std::string> BuildPath("p", cl::desc("Build path"),
                                        cl::Optional, cl::cat(Category),
                                        cl::sub(cl::SubCommand::getAll()));

  static cl::list<std::string> SourcePaths(
      cl::Positional, cl::desc("<source0> [... <sourceN>]"), OccurrencesFlag,
      cl::cat(Category), cl::sub(cl::SubCommand::getAll()));

  static cl::list<std::string> ArgsAfter(
      "extra-arg",
      cl::desc("Additional argument to append to the compiler command line"),
      cl::cat(Category), cl::sub(cl::SubCommand::getAll()));

  static cl::list<std::string> ArgsBefore(
      "extra-arg-before",
      cl::desc("Additional argument to prepend to the compiler command line"),
      cl::cat(Category), cl::sub(cl::SubCommand::getAll()));

  cl::ResetAllOptionOccurrences();

  cl::HideUnrelatedOptions(Category);

  std::string ErrorMessage;
  Compilations =
      FixedCompilationDatabase::loadFromCommandLine(argc, argv, ErrorMessage);
  if (!ErrorMessage.empty())
    ErrorMessage.append("\n");
  toolchain::raw_string_ostream OS(ErrorMessage);
  // Stop initializing if command-line option parsing failed.
  if (!cl::ParseCommandLineOptions(argc, argv, Overview, &OS)) {
    return toolchain::make_error<toolchain::StringError>(ErrorMessage,
                                               toolchain::inconvertibleErrorCode());
  }

  cl::PrintOptionValues();

  SourcePathList = SourcePaths;
  if ((OccurrencesFlag == cl::ZeroOrMore || OccurrencesFlag == cl::Optional) &&
      SourcePathList.empty())
    return toolchain::Error::success();
  if (!Compilations) {
    if (!BuildPath.empty()) {
      Compilations =
          CompilationDatabase::autoDetectFromDirectory(BuildPath, ErrorMessage);
    } else {
      Compilations = CompilationDatabase::autoDetectFromSource(SourcePaths[0],
                                                               ErrorMessage);
    }
    if (!Compilations) {
      toolchain::errs() << "Error while trying to load a compilation database:\n"
                   << ErrorMessage << "Running without flags.\n";
      Compilations.reset(
          new FixedCompilationDatabase(".", std::vector<std::string>()));
    }
  }
  auto AdjustingCompilations =
      std::make_unique<ArgumentsAdjustingCompilations>(
          std::move(Compilations));
  Adjuster =
      getInsertArgumentAdjuster(ArgsBefore, ArgumentInsertPosition::BEGIN);
  Adjuster = combineAdjusters(
      std::move(Adjuster),
      getInsertArgumentAdjuster(ArgsAfter, ArgumentInsertPosition::END));
  AdjustingCompilations->appendArgumentsAdjuster(Adjuster);
  Compilations = std::move(AdjustingCompilations);
  return toolchain::Error::success();
}

toolchain::Expected<CommonOptionsParser> CommonOptionsParser::create(
    int &argc, const char **argv, toolchain::cl::OptionCategory &Category,
    toolchain::cl::NumOccurrencesFlag OccurrencesFlag, const char *Overview) {
  CommonOptionsParser Parser;
  toolchain::Error Err =
      Parser.init(argc, argv, Category, OccurrencesFlag, Overview);
  if (Err)
    return std::move(Err);
  return std::move(Parser);
}

CommonOptionsParser::CommonOptionsParser(
    int &argc, const char **argv, cl::OptionCategory &Category,
    toolchain::cl::NumOccurrencesFlag OccurrencesFlag, const char *Overview) {
  toolchain::Error Err = init(argc, argv, Category, OccurrencesFlag, Overview);
  if (Err) {
    toolchain::report_fatal_error(
        Twine("CommonOptionsParser: failed to parse command-line arguments. ") +
        toolchain::toString(std::move(Err)));
  }
}
