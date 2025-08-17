//===- JSONCompilationDatabase.h --------------------------------*- C++ -*-===//
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
//  The JSONCompilationDatabase finds compilation databases supplied as a file
//  'compile_commands.json'.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_JSONCOMPILATIONDATABASE_H
#define LANGUAGE_CORE_TOOLING_JSONCOMPILATIONDATABASE_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Tooling/CompilationDatabase.h"
#include "language/Core/Tooling/FileMatchTrie.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/SourceMgr.h"
#include "toolchain/Support/YAMLParser.h"
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace language::Core {
namespace tooling {

/// A JSON based compilation database.
///
/// JSON compilation database files must contain a list of JSON objects which
/// provide the command lines in the attributes 'directory', 'command',
/// 'arguments' and 'file':
/// [
///   { "directory": "<working directory of the compile>",
///     "command": "<compile command line>",
///     "file": "<path to source file>"
///   },
///   { "directory": "<working directory of the compile>",
///     "arguments": ["<raw>", "<command>" "<line>" "<parameters>"],
///     "file": "<path to source file>"
///   },
///   ...
/// ]
/// Each object entry defines one compile action. The specified file is
/// considered to be the main source file for the translation unit.
///
/// 'command' is a full command line that will be unescaped.
///
/// 'arguments' is a list of command line arguments that will not be unescaped.
///
/// JSON compilation databases can for example be generated in CMake projects
/// by setting the flag -DCMAKE_EXPORT_COMPILE_COMMANDS.
enum class JSONCommandLineSyntax { Windows, Gnu, AutoDetect };
class JSONCompilationDatabase : public CompilationDatabase {
public:
  /// Loads a JSON compilation database from the specified file.
  ///
  /// Returns NULL and sets ErrorMessage if the database could not be
  /// loaded from the given file.
  static std::unique_ptr<JSONCompilationDatabase>
  loadFromFile(StringRef FilePath, std::string &ErrorMessage,
               JSONCommandLineSyntax Syntax);

  /// Loads a JSON compilation database from a data buffer.
  ///
  /// Returns NULL and sets ErrorMessage if the database could not be loaded.
  static std::unique_ptr<JSONCompilationDatabase>
  loadFromBuffer(StringRef DatabaseString, std::string &ErrorMessage,
                 JSONCommandLineSyntax Syntax);

  /// Returns all compile commands in which the specified file was
  /// compiled.
  ///
  /// FIXME: Currently FilePath must be an absolute path inside the
  /// source directory which does not have symlinks resolved.
  std::vector<CompileCommand>
  getCompileCommands(StringRef FilePath) const override;

  /// Returns the list of all files available in the compilation database.
  ///
  /// These are the 'file' entries of the JSON objects.
  std::vector<std::string> getAllFiles() const override;

  /// Returns all compile commands for all the files in the compilation
  /// database.
  std::vector<CompileCommand> getAllCompileCommands() const override;

private:
  /// Constructs a JSON compilation database on a memory buffer.
  JSONCompilationDatabase(std::unique_ptr<toolchain::MemoryBuffer> Database,
                          JSONCommandLineSyntax Syntax)
      : Database(std::move(Database)), Syntax(Syntax),
        YAMLStream(this->Database->getBuffer(), SM) {}

  /// Parses the database file and creates the index.
  ///
  /// Returns whether parsing succeeded. Sets ErrorMessage if parsing
  /// failed.
  bool parse(std::string &ErrorMessage);

  // Tuple (directory, filename, commandline, output) where 'commandline'
  // points to the corresponding scalar nodes in the YAML stream.
  // If the command line contains a single argument, it is a shell-escaped
  // command line.
  // Otherwise, each entry in the command line vector is a literal
  // argument to the compiler.
  // The output field may be a nullptr.
  using CompileCommandRef =
      std::tuple<toolchain::yaml::ScalarNode *, toolchain::yaml::ScalarNode *,
                 std::vector<toolchain::yaml::ScalarNode *>,
                 toolchain::yaml::ScalarNode *>;

  /// Converts the given array of CompileCommandRefs to CompileCommands.
  void getCommands(ArrayRef<CompileCommandRef> CommandsRef,
                   std::vector<CompileCommand> &Commands) const;

  // Maps file paths to the compile command lines for that file.
  toolchain::StringMap<std::vector<CompileCommandRef>> IndexByFile;

  /// All the compile commands in the order that they were provided in the
  /// JSON stream.
  std::vector<CompileCommandRef> AllCommands;

  FileMatchTrie MatchTrie;

  std::unique_ptr<toolchain::MemoryBuffer> Database;
  JSONCommandLineSyntax Syntax;
  toolchain::SourceMgr SM;
  toolchain::yaml::Stream YAMLStream;
};

} // namespace tooling
} // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_JSONCOMPILATIONDATABASE_H
