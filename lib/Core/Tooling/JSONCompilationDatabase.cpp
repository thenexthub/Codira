//===- JSONCompilationDatabase.cpp ----------------------------------------===//
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
//  This file contains the implementation of the JSONCompilationDatabase.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Tooling/JSONCompilationDatabase.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Tooling/CompilationDatabase.h"
#include "language/Core/Tooling/CompilationDatabasePluginRegistry.h"
#include "language/Core/Tooling/Tooling.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/ErrorOr.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/StringSaver.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/Support/YAMLParser.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Host.h"
#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace language::Core;
using namespace tooling;

namespace {

/// A parser for escaped strings of command line arguments.
///
/// Assumes \-escaping for quoted arguments (see the documentation of
/// unescapeCommandLine(...)).
class CommandLineArgumentParser {
 public:
  CommandLineArgumentParser(StringRef CommandLine)
      : Input(CommandLine), Position(Input.begin()-1) {}

  std::vector<std::string> parse() {
    bool HasMoreInput = true;
    while (HasMoreInput && nextNonWhitespace()) {
      std::string Argument;
      HasMoreInput = parseStringInto(Argument);
      CommandLine.push_back(Argument);
    }
    return CommandLine;
  }

 private:
  // All private methods return true if there is more input available.

  bool parseStringInto(std::string &String) {
    do {
      if (*Position == '"') {
        if (!parseDoubleQuotedStringInto(String)) return false;
      } else if (*Position == '\'') {
        if (!parseSingleQuotedStringInto(String)) return false;
      } else {
        if (!parseFreeStringInto(String)) return false;
      }
    } while (*Position != ' ');
    return true;
  }

  bool parseDoubleQuotedStringInto(std::string &String) {
    if (!next()) return false;
    while (*Position != '"') {
      if (!skipEscapeCharacter()) return false;
      String.push_back(*Position);
      if (!next()) return false;
    }
    return next();
  }

  bool parseSingleQuotedStringInto(std::string &String) {
    if (!next()) return false;
    while (*Position != '\'') {
      String.push_back(*Position);
      if (!next()) return false;
    }
    return next();
  }

  bool parseFreeStringInto(std::string &String) {
    do {
      if (!skipEscapeCharacter()) return false;
      String.push_back(*Position);
      if (!next()) return false;
    } while (*Position != ' ' && *Position != '"' && *Position != '\'');
    return true;
  }

  bool skipEscapeCharacter() {
    if (*Position == '\\') {
      return next();
    }
    return true;
  }

  bool nextNonWhitespace() {
    do {
      if (!next()) return false;
    } while (*Position == ' ');
    return true;
  }

  bool next() {
    ++Position;
    return Position != Input.end();
  }

  const StringRef Input;
  StringRef::iterator Position;
  std::vector<std::string> CommandLine;
};

std::vector<std::string> unescapeCommandLine(JSONCommandLineSyntax Syntax,
                                             StringRef EscapedCommandLine) {
  if (Syntax == JSONCommandLineSyntax::AutoDetect) {
#ifdef _WIN32
    // Assume Windows command line parsing on Win32
    Syntax = JSONCommandLineSyntax::Windows;
#else
    Syntax = JSONCommandLineSyntax::Gnu;
#endif
  }

  if (Syntax == JSONCommandLineSyntax::Windows) {
    toolchain::BumpPtrAllocator Alloc;
    toolchain::StringSaver Saver(Alloc);
    toolchain::SmallVector<const char *, 64> T;
    toolchain::cl::TokenizeWindowsCommandLine(EscapedCommandLine, Saver, T);
    std::vector<std::string> Result(T.begin(), T.end());
    return Result;
  }
  assert(Syntax == JSONCommandLineSyntax::Gnu);
  CommandLineArgumentParser parser(EscapedCommandLine);
  return parser.parse();
}

// This plugin locates a nearby compile_command.json file, and also infers
// compile commands for files not present in the database.
class JSONCompilationDatabasePlugin : public CompilationDatabasePlugin {
  std::unique_ptr<CompilationDatabase>
  loadFromDirectory(StringRef Directory, std::string &ErrorMessage) override {
    SmallString<1024> JSONDatabasePath(Directory);
    toolchain::sys::path::append(JSONDatabasePath, "compile_commands.json");
    auto Base = JSONCompilationDatabase::loadFromFile(
        JSONDatabasePath, ErrorMessage, JSONCommandLineSyntax::AutoDetect);
    return Base ? inferTargetAndDriverMode(
                      inferMissingCompileCommands(expandResponseFiles(
                          std::move(Base), toolchain::vfs::getRealFileSystem())))
                : nullptr;
  }
};

} // namespace

// Register the JSONCompilationDatabasePlugin with the
// CompilationDatabasePluginRegistry using this statically initialized variable.
static CompilationDatabasePluginRegistry::Add<JSONCompilationDatabasePlugin>
X("json-compilation-database", "Reads JSON formatted compilation databases");

namespace language::Core {
namespace tooling {

// This anchor is used to force the linker to link in the generated object file
// and thus register the JSONCompilationDatabasePlugin.
volatile int JSONAnchorSource = 0;

} // namespace tooling
} // namespace language::Core

std::unique_ptr<JSONCompilationDatabase>
JSONCompilationDatabase::loadFromFile(StringRef FilePath,
                                      std::string &ErrorMessage,
                                      JSONCommandLineSyntax Syntax) {
  // Don't mmap: if we're a long-lived process, the build system may overwrite.
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> DatabaseBuffer =
      toolchain::MemoryBuffer::getFile(FilePath, /*IsText=*/false,
                                  /*RequiresNullTerminator=*/true,
                                  /*IsVolatile=*/true);
  if (std::error_code Result = DatabaseBuffer.getError()) {
    ErrorMessage = "Error while opening JSON database: " + Result.message();
    return nullptr;
  }
  std::unique_ptr<JSONCompilationDatabase> Database(
      new JSONCompilationDatabase(std::move(*DatabaseBuffer), Syntax));
  if (!Database->parse(ErrorMessage))
    return nullptr;
  return Database;
}

std::unique_ptr<JSONCompilationDatabase>
JSONCompilationDatabase::loadFromBuffer(StringRef DatabaseString,
                                        std::string &ErrorMessage,
                                        JSONCommandLineSyntax Syntax) {
  std::unique_ptr<toolchain::MemoryBuffer> DatabaseBuffer(
      toolchain::MemoryBuffer::getMemBufferCopy(DatabaseString));
  std::unique_ptr<JSONCompilationDatabase> Database(
      new JSONCompilationDatabase(std::move(DatabaseBuffer), Syntax));
  if (!Database->parse(ErrorMessage))
    return nullptr;
  return Database;
}

std::vector<CompileCommand>
JSONCompilationDatabase::getCompileCommands(StringRef FilePath) const {
  SmallString<128> NativeFilePath;
  toolchain::sys::path::native(FilePath, NativeFilePath);

  std::string Error;
  toolchain::raw_string_ostream ES(Error);
  StringRef Match = MatchTrie.findEquivalent(NativeFilePath, ES);
  if (Match.empty())
    return {};
  const auto CommandsRefI = IndexByFile.find(Match);
  if (CommandsRefI == IndexByFile.end())
    return {};
  std::vector<CompileCommand> Commands;
  getCommands(CommandsRefI->getValue(), Commands);
  return Commands;
}

std::vector<std::string>
JSONCompilationDatabase::getAllFiles() const {
  std::vector<std::string> Result;
  for (const auto &CommandRef : IndexByFile)
    Result.push_back(CommandRef.first().str());
  return Result;
}

std::vector<CompileCommand>
JSONCompilationDatabase::getAllCompileCommands() const {
  std::vector<CompileCommand> Commands;
  getCommands(AllCommands, Commands);
  return Commands;
}

static toolchain::StringRef stripExecutableExtension(toolchain::StringRef Name) {
  Name.consume_back(".exe");
  return Name;
}

// There are compiler-wrappers (ccache, distcc) that take the "real"
// compiler as an argument, e.g. distcc gcc -O3 foo.c.
// These end up in compile_commands.json when people set CC="distcc gcc".
// Clang's driver doesn't understand this, so we need to unwrap.
static bool unwrapCommand(std::vector<std::string> &Args) {
  if (Args.size() < 2)
    return false;
  StringRef Wrapper =
      stripExecutableExtension(toolchain::sys::path::filename(Args.front()));
  if (Wrapper == "distcc" || Wrapper == "ccache" || Wrapper == "sccache") {
    // Most of these wrappers support being invoked 3 ways:
    // `distcc g++ file.c` This is the mode we're trying to match.
    //                     We need to drop `distcc`.
    // `distcc file.c`     This acts like compiler is cc or similar.
    //                     Clang's driver can handle this, no change needed.
    // `g++ file.c`        g++ is a symlink to distcc.
    //                     We don't even notice this case, and all is well.
    //
    // We need to distinguish between the first and second case.
    // The wrappers themselves don't take flags, so Args[1] is a compiler flag,
    // an input file, or a compiler. Inputs have extensions, compilers don't.
    bool HasCompiler =
        (Args[1][0] != '-') &&
        !toolchain::sys::path::has_extension(stripExecutableExtension(Args[1]));
    if (HasCompiler) {
      Args.erase(Args.begin());
      return true;
    }
    // If !HasCompiler, wrappers act like GCC. Fine: so do we.
  }
  return false;
}

static std::vector<std::string>
nodeToCommandLine(JSONCommandLineSyntax Syntax,
                  const std::vector<toolchain::yaml::ScalarNode *> &Nodes) {
  SmallString<1024> Storage;
  std::vector<std::string> Arguments;
  if (Nodes.size() == 1)
    Arguments = unescapeCommandLine(Syntax, Nodes[0]->getValue(Storage));
  else
    for (const auto *Node : Nodes)
      Arguments.push_back(std::string(Node->getValue(Storage)));
  // There may be multiple wrappers: using distcc and ccache together is common.
  while (unwrapCommand(Arguments))
    ;
  return Arguments;
}

void JSONCompilationDatabase::getCommands(
    ArrayRef<CompileCommandRef> CommandsRef,
    std::vector<CompileCommand> &Commands) const {
  for (const auto &CommandRef : CommandsRef) {
    SmallString<8> DirectoryStorage;
    SmallString<32> FilenameStorage;
    SmallString<32> OutputStorage;
    auto Output = std::get<3>(CommandRef);
    Commands.emplace_back(
        std::get<0>(CommandRef)->getValue(DirectoryStorage),
        std::get<1>(CommandRef)->getValue(FilenameStorage),
        nodeToCommandLine(Syntax, std::get<2>(CommandRef)),
        Output ? Output->getValue(OutputStorage) : "");
  }
}

bool JSONCompilationDatabase::parse(std::string &ErrorMessage) {
  toolchain::yaml::document_iterator I = YAMLStream.begin();
  if (I == YAMLStream.end()) {
    ErrorMessage = "Error while parsing YAML.";
    return false;
  }
  toolchain::yaml::Node *Root = I->getRoot();
  if (!Root) {
    ErrorMessage = "Error while parsing YAML.";
    return false;
  }
  auto *Array = dyn_cast<toolchain::yaml::SequenceNode>(Root);
  if (!Array) {
    ErrorMessage = "Expected array.";
    return false;
  }
  for (auto &NextObject : *Array) {
    auto *Object = dyn_cast<toolchain::yaml::MappingNode>(&NextObject);
    if (!Object) {
      ErrorMessage = "Expected object.";
      return false;
    }
    toolchain::yaml::ScalarNode *Directory = nullptr;
    std::optional<std::vector<toolchain::yaml::ScalarNode *>> Command;
    toolchain::yaml::ScalarNode *File = nullptr;
    toolchain::yaml::ScalarNode *Output = nullptr;
    for (auto& NextKeyValue : *Object) {
      auto *KeyString = dyn_cast<toolchain::yaml::ScalarNode>(NextKeyValue.getKey());
      if (!KeyString) {
        ErrorMessage = "Expected strings as key.";
        return false;
      }
      SmallString<10> KeyStorage;
      StringRef KeyValue = KeyString->getValue(KeyStorage);
      toolchain::yaml::Node *Value = NextKeyValue.getValue();
      if (!Value) {
        ErrorMessage = "Expected value.";
        return false;
      }
      auto *ValueString = dyn_cast<toolchain::yaml::ScalarNode>(Value);
      auto *SequenceString = dyn_cast<toolchain::yaml::SequenceNode>(Value);
      if (KeyValue == "arguments") {
        if (!SequenceString) {
          ErrorMessage = "Expected sequence as value.";
          return false;
        }
        Command = std::vector<toolchain::yaml::ScalarNode *>();
        for (auto &Argument : *SequenceString) {
          auto *Scalar = dyn_cast<toolchain::yaml::ScalarNode>(&Argument);
          if (!Scalar) {
            ErrorMessage = "Only strings are allowed in 'arguments'.";
            return false;
          }
          Command->push_back(Scalar);
        }
      } else {
        if (!ValueString) {
          ErrorMessage = "Expected string as value.";
          return false;
        }
        if (KeyValue == "directory") {
          Directory = ValueString;
        } else if (KeyValue == "command") {
          if (!Command)
            Command = std::vector<toolchain::yaml::ScalarNode *>(1, ValueString);
        } else if (KeyValue == "file") {
          File = ValueString;
        } else if (KeyValue == "output") {
          Output = ValueString;
        } else {
          ErrorMessage =
              ("Unknown key: \"" + KeyString->getRawValue() + "\"").str();
          return false;
        }
      }
    }
    if (!File) {
      ErrorMessage = "Missing key: \"file\".";
      return false;
    }
    if (!Command) {
      ErrorMessage = "Missing key: \"command\" or \"arguments\".";
      return false;
    }
    if (!Directory) {
      ErrorMessage = "Missing key: \"directory\".";
      return false;
    }
    SmallString<8> FileStorage;
    StringRef FileName = File->getValue(FileStorage);
    SmallString<128> NativeFilePath;
    if (toolchain::sys::path::is_relative(FileName)) {
      SmallString<8> DirectoryStorage;
      SmallString<128> AbsolutePath(Directory->getValue(DirectoryStorage));
      toolchain::sys::path::append(AbsolutePath, FileName);
      toolchain::sys::path::native(AbsolutePath, NativeFilePath);
    } else {
      toolchain::sys::path::native(FileName, NativeFilePath);
    }
    toolchain::sys::path::remove_dots(NativeFilePath, /*remove_dot_dot=*/true);
    auto Cmd = CompileCommandRef(Directory, File, *Command, Output);
    IndexByFile[NativeFilePath].push_back(Cmd);
    AllCommands.push_back(Cmd);
    MatchTrie.insert(NativeFilePath);
  }
  return true;
}
