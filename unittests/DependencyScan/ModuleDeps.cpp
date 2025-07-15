//===---------------------- ModuleDeps.cpp --------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "ScanFixture.h"
#include "language/Basic/Defer.h"
#include "language/Basic/Platform.h"
#include "language/DependencyScan/DependencyScanImpl.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Host.h"
#include "toolchain/TargetParser/Triple.h"
#include "gtest/gtest.h"
#include <string>

using namespace language;
using namespace language::unittest;
using namespace language::dependencies;

static std::string createFilename(StringRef base, StringRef name) {
  SmallString<256> path = base;
  toolchain::sys::path::append(path, name);
  return toolchain::Twine(path).str();
}

static bool emitFileWithContents(StringRef path, StringRef contents,
                                 std::string *pathOut = nullptr) {
  int FD;
  if (toolchain::sys::fs::openFileForWrite(path, FD))
    return true;
  if (pathOut)
    *pathOut = path.str();
  toolchain::raw_fd_ostream file(FD, /*shouldClose=*/true);
  file << contents;
  return false;
}

static bool emitFileWithContents(StringRef base, StringRef name,
                                 StringRef contents,
                                 std::string *pathOut = nullptr) {
  return emitFileWithContents(createFilename(base, name), contents, pathOut);
}

TEST_F(ScanTest, TestModuleDeps) {
  SmallString<256> tempDir;
  ASSERT_FALSE(toolchain::sys::fs::createUniqueDirectory("ScanTest.TestModuleDeps", tempDir));
  LANGUAGE_DEFER { toolchain::sys::fs::remove_directories(tempDir); };

  // Create test input file
  std::string TestPathStr = createFilename(tempDir, "foo.code");
  ASSERT_FALSE(emitFileWithContents(tempDir, "foo.code", "import A\n"));

  // Create includes
  std::string IncludeDirPath = createFilename(tempDir, "include");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(IncludeDirPath));
  std::string CHeadersDirPath = createFilename(IncludeDirPath, "CHeaders");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(CHeadersDirPath));
  std::string CodiraDirPath = createFilename(IncludeDirPath, "Codira");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(CodiraDirPath));

  // Create imported module Codira interface files
  ASSERT_FALSE(emitFileWithContents(CodiraDirPath, "A.codeinterface",
                                    "// language-interface-format-version: 1.0\n\
// language-module-flags: -module-name A\n\
import Codira\n\
@_exported import A\n\
public fn overlayFuncA() { }\n"));
  ASSERT_FALSE(emitFileWithContents(CodiraDirPath, "E.codeinterface",
                                    "// language-interface-format-version: 1.0\n\
// language-module-flags: -module-name E\n\
import Codira\n\
public fn funcE()\n"));
  ASSERT_FALSE(emitFileWithContents(CodiraDirPath, "F.codeinterface",
                                    "// language-interface-format-version: 1.0\n\
// language-module-flags: -module-name\n\
import Codira\n\
@_exported import F\n\
public fn funcF() { }"));
  ASSERT_FALSE(emitFileWithContents(CodiraDirPath, "G.codeinterface",
                                    "// language-interface-format-version: 1.0\n\
// language-module-flags: -module-name G -language-version 5 -target x86_64-apple-macosx10.9\n\
#if language(>=5.0)\n\
@_exported import G\n\
import Codira\n\
public fn overlayFuncG() { }\n\
let stringG : String = \"Build\"\n\
#endif"));

  // Create imported module C modulemap/headers
  ASSERT_FALSE(
      emitFileWithContents(CHeadersDirPath, "A.h", "void funcA(void);"));
  ASSERT_FALSE(emitFileWithContents(CHeadersDirPath, "B.h", "#include \"A.h\"\
void funcB(void);"));
  ASSERT_FALSE(emitFileWithContents(CHeadersDirPath, "C.h", "#include \"B.h\"\n\
void funcC(void);\
const char* stringC() { return \"Module\"; }"));
  ASSERT_FALSE(
      emitFileWithContents(CHeadersDirPath, "D.h", "void funcD(void);"));
  ASSERT_FALSE(
      emitFileWithContents(CHeadersDirPath, "F.h", "void funcF(void);"));
  ASSERT_FALSE(emitFileWithContents(
      CHeadersDirPath, "G.h",
      "#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 110000\n\
#include \"X.h\"\n\
#endif\n\
void funcG(void);"));
  ASSERT_FALSE(
      emitFileWithContents(CHeadersDirPath, "X.h", "void funcX(void);"));
  ASSERT_FALSE(emitFileWithContents(CHeadersDirPath, "Bridging.h",
                                    "#include \"BridgingOther.h\"\n\
int bridging_other(void);"));
  ASSERT_FALSE(emitFileWithContents(CHeadersDirPath, "BridgingOther.h",
                                    "#include \"F.h\"\n\
int bridging_other(void);"));

  ASSERT_FALSE(
      emitFileWithContents(CHeadersDirPath, "module.modulemap", "module A {\n\
header \"A.h\"\n\
export *\n\
}\n\
module B {\n\
header \"B.h\"\n\
export *\n\
}\n\
module C {\n\
header \"C.h\"\n\
export *\n\
}\n\
module D {\n\
header \"D.h\"\n\
export *\n\
}\n\
module F {\n\
header \"F.h\"\n\
export *\n\
}\n\
module G {\n\
header \"G.h\"\n\
export *\n\
}\n\
module X {\n\
header \"X.h\"\n\
export *\n\
}"));

  // Paths to shims and stdlib
  toolchain::SmallString<128> ShimsLibDir = StdLibDir;
  toolchain::sys::path::append(ShimsLibDir, "shims");
  auto Target = toolchain::Triple(toolchain::sys::getDefaultTargetTriple());
  toolchain::sys::path::append(StdLibDir, getPlatformNameForTriple(Target));

  std::vector<std::string> CommandStrArr = {
    TestPathStr,
    std::string("-I ") + CodiraDirPath,
    std::string("-I ") + CHeadersDirPath,
    std::string("-I ") + StdLibDir.str().str(),
    std::string("-I ") + ShimsLibDir.str().str(),
  };

  // On Windows we need to add an extra escape for path separator characters because otherwise
  // the command line tokenizer will treat them as escape characters.
  for (size_t i = 0; i < CommandStrArr.size(); ++i) {
    std::replace(CommandStrArr[i].begin(), CommandStrArr[i].end(), '\\', '/');
  }

  std::vector<const char*> Command;
  for (auto &command : CommandStrArr) {
    Command.push_back(command.c_str());
  }
  auto DependenciesOrErr = ScannerTool.getDependencies(Command, {});
  ASSERT_FALSE(DependenciesOrErr.getError());
  auto Dependencies = DependenciesOrErr.get();
  // TODO: Output/verify dependency graph correctness
  // toolchain::dbgs() << "Deps: " << Dependencies << "\n";

  languagescan_dependency_graph_dispose(Dependencies);
}

TEST_F(ScanTest, TestModuleDepsHash) {
  SmallString<256> tempDir;
  ASSERT_FALSE(toolchain::sys::fs::createUniqueDirectory("ScanTest.TestModuleDepsHash", tempDir));
  LANGUAGE_DEFER { toolchain::sys::fs::remove_directories(tempDir); };

  // Create test input file
  std::string TestPathStr = createFilename(tempDir, "foo.code");
  ASSERT_FALSE(emitFileWithContents(tempDir, "foo.code", "import A\n"));

  // Create includes
  std::string IncludeDirPath = createFilename(tempDir, "include");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(IncludeDirPath));
  std::string CodiraDirPath = createFilename(IncludeDirPath, "Codira");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(CodiraDirPath));

  // Create imported module Codira interface files
  ASSERT_FALSE(emitFileWithContents(CodiraDirPath, "A.codeinterface",
                                    "// language-interface-format-version: 1.0\n\
// language-module-flags: -module-name A\n\
import Codira\n\
public fn overlayFuncA() { }\n"));

  // Paths to shims and stdlib
  toolchain::SmallString<128> ShimsLibDir = StdLibDir;
  toolchain::sys::path::append(ShimsLibDir, "shims");
  auto Target = toolchain::Triple(toolchain::sys::getDefaultTargetTriple());
  toolchain::sys::path::append(StdLibDir, getPlatformNameForTriple(Target));

  std::vector<std::string> BaseCommandStrArr = {
    TestPathStr,
    std::string("-I ") + CodiraDirPath,
    std::string("-I ") + StdLibDir.str().str(),
    std::string("-I ") + ShimsLibDir.str().str(),
  };

  std::vector<std::string> CommandStrArrA = BaseCommandStrArr;
  CommandStrArrA.push_back("-module-name");
  CommandStrArrA.push_back("A");
  std::vector<std::string> CommandStrArrB = BaseCommandStrArr;
  CommandStrArrB.push_back("-module-name");
  CommandStrArrB.push_back("B");

  // On Windows we need to add an extra escape for path separator characters because otherwise
  // the command line tokenizer will treat them as escape characters.
  for (size_t i = 0; i < CommandStrArrA.size(); ++i) {
    std::replace(CommandStrArrA[i].begin(), CommandStrArrA[i].end(), '\\', '/');
  }
  std::vector<const char*> CommandA;
  for (auto &command : CommandStrArrA) {
    CommandA.push_back(command.c_str());
  }

  for (size_t i = 0; i < CommandStrArrB.size(); ++i) {
    std::replace(CommandStrArrB[i].begin(), CommandStrArrB[i].end(), '\\', '/');
  }
  std::vector<const char*> CommandB;
  for (auto &command : CommandStrArrB) {
    CommandB.push_back(command.c_str());
  }

  auto ScanDiagnosticConsumer = std::make_shared<DependencyScanDiagnosticCollector>();
  auto instanceA = ScannerTool.initCompilerInstanceForScan(CommandA, {}, ScanDiagnosticConsumer);
  auto instanceB = ScannerTool.initCompilerInstanceForScan(CommandB, {}, ScanDiagnosticConsumer);
  // Ensure that scans that only differ in module name have distinct scanning context hashes
  ASSERT_NE(instanceA->ScanInstance.get()->getInvocation().getModuleScanningHash(),
            instanceB->ScanInstance.get()->getInvocation().getModuleScanningHash());
}

TEST_F(ScanTest, TestModuleCycle) {
  SmallString<256> tempDir;
  ASSERT_FALSE(toolchain::sys::fs::createUniqueDirectory("ScanTest.TestModuleCycle", tempDir));
  LANGUAGE_DEFER { toolchain::sys::fs::remove_directories(tempDir); };

  // Create test input file
  std::string TestPathStr = createFilename(tempDir, "foo.code");
  ASSERT_FALSE(emitFileWithContents(tempDir, "foo.code", "import A\n"));

  // Create includes
  std::string IncludeDirPath = createFilename(tempDir, "include");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(IncludeDirPath));
  std::string CodiraDirPath = createFilename(IncludeDirPath, "Codira");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(CodiraDirPath));

  // Create imported module Codira interface files
  ASSERT_FALSE(emitFileWithContents(CodiraDirPath, "A.codeinterface",
                                    "// language-interface-format-version: 1.0\n\
// language-module-flags: -module-name A\n\
import Codira\n\
import B\n\
public fn funcA() { }\n"));
  ASSERT_FALSE(emitFileWithContents(CodiraDirPath, "B.codeinterface",
                                    "// language-interface-format-version: 1.0\n\
// language-module-flags: -module-name B\n\
import Codira\n\
import A\n\
public fn funcB() { }\n"));

  // Paths to shims and stdlib
  toolchain::SmallString<128> ShimsLibDir = StdLibDir;
  toolchain::sys::path::append(ShimsLibDir, "shims");
  auto Target = toolchain::Triple(toolchain::sys::getDefaultTargetTriple());
  toolchain::sys::path::append(StdLibDir, getPlatformNameForTriple(Target));

  std::vector<std::string> BaseCommandStrArr = {
    TestPathStr,
    std::string("-I ") + CodiraDirPath,
    std::string("-I ") + StdLibDir.str().str(),
    std::string("-I ") + ShimsLibDir.str().str(),
  };

  std::vector<std::string> CommandStr = BaseCommandStrArr;
  CommandStr.push_back("-module-name");
  CommandStr.push_back("test");

  // On Windows we need to add an extra escape for path separator characters because otherwise
  // the command line tokenizer will treat them as escape characters.
  for (size_t i = 0; i < CommandStr.size(); ++i) {
    std::replace(CommandStr[i].begin(), CommandStr[i].end(), '\\', '/');
  }
  std::vector<const char*> Command;
  for (auto &command : CommandStr)
    Command.push_back(command.c_str());

  auto ScanDiagnosticConsumer = std::make_shared<DependencyScanDiagnosticCollector>();

  auto DependenciesOrErr = ScannerTool.getDependencies(Command, {});

  // Ensure a hollow output with diagnostic info is produced
  ASSERT_FALSE(DependenciesOrErr.getError());
  auto Dependencies = DependenciesOrErr.get();
  auto Diagnostics = Dependencies->diagnostics;
  ASSERT_TRUE(Diagnostics->count == 1);
  auto Diagnostic = Diagnostics->diagnostics[0];
  ASSERT_TRUE(Diagnostic->severity == LANGUAGESCAN_DIAGNOSTIC_SEVERITY_ERROR);
  auto Message = std::string((const char*)Diagnostic->message.data,
                             Diagnostic->message.length);
  ASSERT_TRUE(Message == "module dependency cycle: 'A.codeinterface -> B.codeinterface -> A.codeinterface'\n");

  // Ensure hollow output is hollow
  ASSERT_TRUE(Dependencies->dependencies->count == 1);
  ASSERT_TRUE(Dependencies->dependencies->modules[0]->source_files->count == 0);
  ASSERT_TRUE(Dependencies->dependencies->modules[0]->direct_dependencies->count == 0);
  ASSERT_TRUE(Dependencies->dependencies->modules[0]->link_libraries->count == 0);
  languagescan_dependency_graph_dispose(Dependencies);
}
