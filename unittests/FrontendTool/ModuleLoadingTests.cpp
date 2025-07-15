//===--- ModuleLoadingTests.cpp -------------------------------------------===//
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

#include "gtest/gtest.h"
#include "language/AST/ASTContext.h"
#include "language/Basic/Defer.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/ModuleInterfaceLoader.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/Serialization/Validation.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/VirtualFileSystem.h"

using namespace language;

static std::string createFilename(StringRef base, StringRef name) {
  SmallString<256> path = base;
  toolchain::sys::path::append(path, name);
  return toolchain::Twine(path).str();
}

static bool emitFileWithContents(StringRef path, StringRef contents,
                                 std::string *pathOut = nullptr) {
  int fd;
  if (toolchain::sys::fs::openFileForWrite(path, fd))
    return true;
  if (pathOut)
    *pathOut = path.str();
  toolchain::raw_fd_ostream file(fd, /*shouldClose=*/true);
  file << contents;
  return false;
}

static bool emitFileWithContents(StringRef base, StringRef name,
                                 StringRef contents,
                                 std::string *pathOut = nullptr) {
  return emitFileWithContents(createFilename(base, name), contents, pathOut);
}

namespace unittest {

class OpenTrackingFileSystem : public toolchain::vfs::ProxyFileSystem {
  toolchain::StringMap<unsigned> numberOfOpensPerFile;
public:
  OpenTrackingFileSystem(toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fs)
    : toolchain::vfs::ProxyFileSystem(fs) {}

  toolchain::ErrorOr<std::unique_ptr<toolchain::vfs::File>>
  openFileForRead(const Twine &Path) override {
    numberOfOpensPerFile[Path.str()] += 1;
    return ProxyFileSystem::openFileForRead(Path);
  }

  unsigned numberOfOpens(StringRef path) {
    return numberOfOpensPerFile[path];
  }
};

class ModuleInterfaceLoaderTest : public testing::Test {
protected:
  void setupAndLoadModuleInterface() {
    SmallString<256> tempDir;
    ASSERT_FALSE(toolchain::sys::fs::createUniqueDirectory(
        "ModuleInterfaceBufferTests.emitModuleInMemory", tempDir));
    LANGUAGE_DEFER { toolchain::sys::fs::remove_directories(tempDir); };

    auto cacheDir = createFilename(tempDir, "ModuleCache");
    ASSERT_FALSE(toolchain::sys::fs::create_directory(cacheDir));

    auto prebuiltCacheDir = createFilename(tempDir, "PrebuiltModuleCache");
    ASSERT_FALSE(toolchain::sys::fs::create_directory(prebuiltCacheDir));

    // Emit an interface file that we can attempt to compile.
    ASSERT_FALSE(emitFileWithContents(tempDir, "Library.codeinterface",
        "// language-interface-format-version: 1.0\n"
        "// language-module-flags: -module-name TestModule -parse-stdlib\n"
        "public fn foo()\n"));

    SourceManager sourceMgr;

    // Create a file system that tracks how many times a file has been opened.
    toolchain::IntrusiveRefCntPtr<OpenTrackingFileSystem> fs(
      new OpenTrackingFileSystem(sourceMgr.getFileSystem()));

    sourceMgr.setFileSystem(fs);
    PrintingDiagnosticConsumer printingConsumer;
    DiagnosticEngine diags(sourceMgr);
    diags.addConsumer(printingConsumer);
    TypeCheckerOptions typecheckOpts;
    LangOptions langOpts;
    langOpts.Target = toolchain::Triple(toolchain::sys::getDefaultTargetTriple());
    SearchPathOptions searchPathOpts;
    ClangImporterOptions clangImpOpts;
    symbolgraphgen::SymbolGraphOptions symbolGraphOpts;
    SILOptions silOpts;
    CASOptions casOpts;
    SerializationOptions serializationOpts;
    auto ctx = ASTContext::get(langOpts, typecheckOpts, silOpts, searchPathOpts,
                               clangImpOpts, symbolGraphOpts, casOpts,
                               serializationOpts, sourceMgr, diags);

    ctx->addModuleInterfaceChecker(
      std::make_unique<ModuleInterfaceCheckerImpl>(*ctx, cacheDir,
        prebuiltCacheDir, ModuleInterfaceLoaderOptions(),
        language::RequireOSSAModules_t(silOpts)));

    auto loader = ModuleInterfaceLoader::create(
        *ctx, *static_cast<ModuleInterfaceCheckerImpl*>(
          ctx->getModuleInterfaceChecker()),
        /*dependencyTracker*/nullptr,
        ModuleLoadingMode::PreferSerialized);

    Identifier moduleName = ctx->getIdentifier("TestModule");

    std::unique_ptr<toolchain::MemoryBuffer> moduleBuffer;
    std::unique_ptr<toolchain::MemoryBuffer> moduleDocBuffer;
    std::unique_ptr<toolchain::MemoryBuffer> moduleSourceInfoBuffer;

    auto error =
      loader->findModuleFilesInDirectory({moduleName, SourceLoc()},
        SerializedModuleBaseName(tempDir, SerializedModuleBaseName("Library")),
        /*ModuleInterfacePath=*/nullptr, /*ModuleInterfaceSourcePath=*/nullptr,
        &moduleBuffer, &moduleDocBuffer, &moduleSourceInfoBuffer,
        /*skipBuildingInterface*/ false, /*IsFramework*/false);
    ASSERT_FALSE(error);
    ASSERT_FALSE(diags.hadAnyError());

    ASSERT_NE(nullptr, moduleBuffer);

    // We should not have written a module doc file.
    ASSERT_EQ(nullptr, moduleDocBuffer);

    // Make sure the buffer identifier points to the written module.
    StringRef cachedModulePath = moduleBuffer->getBufferIdentifier();
    ASSERT_TRUE(fs->exists(cachedModulePath));

    // Assert that we've only opened this file once, to write it.
    ASSERT_EQ((unsigned)1, fs->numberOfOpens(cachedModulePath));

    auto bufOrErr = fs->getBufferForFile(cachedModulePath);
    ASSERT_TRUE(bufOrErr);

    auto bufData = (*bufOrErr)->getBuffer();
    auto validationInfo = serialization::validateSerializedAST(
        bufData, silOpts.EnableOSSAModules,
        /*requiredSDK*/StringRef());
    ASSERT_EQ(serialization::Status::Valid, validationInfo.status);
    ASSERT_EQ(bufData, moduleBuffer->getBuffer());
  }
};

TEST_F(ModuleInterfaceLoaderTest, LoadModuleFromBuffer) {
  setupAndLoadModuleInterface();
}

} // end namespace unittest
