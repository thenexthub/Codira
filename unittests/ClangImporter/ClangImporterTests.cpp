#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/SearchPathOptions.h"
#include "language/Basic/CASOptions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/LangOptions.h"
#include "language/Basic/SourceManager.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Host.h"
#include "gtest/gtest.h"

using namespace language;

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

TEST(ClangImporterTest, emitPCHInMemory) {
  // Create a temporary cache on disk and clean it up at the end.
  ClangImporterOptions options;
  SmallString<256> temp;
  ASSERT_FALSE(toolchain::sys::fs::createUniqueDirectory(
      "ClangImporterTest.emitPCHInMemory", temp));
  LANGUAGE_DEFER { toolchain::sys::fs::remove_directories(temp); };

  // Create a cache subdirectory for the modules and PCH.
  std::string cache = createFilename(temp, "cache");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(cache));
  options.ModuleCachePath = cache;
  options.PrecompiledHeaderOutputDir = cache;

  // Create the includes.
  std::string include = createFilename(temp, "include");
  ASSERT_FALSE(toolchain::sys::fs::create_directory(include));
  options.ExtraArgs.emplace_back("-nosysteminc");
  options.ExtraArgs.emplace_back((toolchain::Twine("-I") + include).str());
  ASSERT_FALSE(emitFileWithContents(include, "module.modulemap",
                                    "module A {\n"
                                    "  header \"A.h\"\n"
                                    "}\n"));
  ASSERT_FALSE(emitFileWithContents(include, "A.h", "int foo(void);\n"));

  // Create a bridging header.
  ASSERT_FALSE(emitFileWithContents(temp, "bridging.h", "#import <A.h>\n",
                                    &options.BridgingHeader));

  // Set up the importer and emit a bridging PCH.
  language::LangOptions langOpts;
  langOpts.Target = toolchain::Triple("x86_64", "apple", "darwin");
  language::SILOptions silOpts;
  language::TypeCheckerOptions typecheckOpts;
  INITIALIZE_LLVM();
  language::SearchPathOptions searchPathOpts;
  language::symbolgraphgen::SymbolGraphOptions symbolGraphOpts;
  language::CASOptions casOpts;
  language::SerializationOptions serializationOpts;
  language::SourceManager sourceMgr;
  language::DiagnosticEngine diags(sourceMgr);
  std::unique_ptr<ASTContext> context(ASTContext::get(
      langOpts, typecheckOpts, silOpts, searchPathOpts, options,
      symbolGraphOpts, casOpts, serializationOpts, sourceMgr, diags));
  auto importer = ClangImporter::create(*context);

  std::string PCH = createFilename(cache, "bridging.h.pch");
  ASSERT_FALSE(importer->canReadPCH(PCH));
  ASSERT_FALSE(importer->emitBridgingPCH(options.BridgingHeader, PCH, true));
  ASSERT_TRUE(importer->canReadPCH(PCH));

  // Overwrite the PCH with garbage.  We should still be able to read it from
  // the in-memory cache.
  ASSERT_FALSE(emitFileWithContents(PCH, "garbage"));
  ASSERT_TRUE(importer->canReadPCH(PCH));
}

static StringRef getLibstdcxxModulemapContents() {
  static std::string value = ([]() -> std::string {
    auto executablePath = testing::internal::GetArgvs()[0];
    SmallString<256> libstdcxxModuleMapPath(executablePath);
    toolchain::sys::path::remove_filename(libstdcxxModuleMapPath);
    toolchain::sys::path::append(libstdcxxModuleMapPath, "libstdcxx.modulemap");

    auto fs = toolchain::vfs::getRealFileSystem();
    auto file = fs->openFileForRead(libstdcxxModuleMapPath);
    if (!file)
      return "";
    auto buf = (*file)->getBuffer("");
    if (!buf)
      return "";
    return (*buf)->getBuffer().str();
  })();
  return value;
}

struct LibStdCxxInjectionVFS {
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::InMemoryFileSystem> vfs;

  LibStdCxxInjectionVFS(const toolchain::Triple &triple = toolchain::Triple("x86_64",
                                                                  "redhat",
                                                                  "linux")) {
    vfs = new toolchain::vfs::InMemoryFileSystem;
    tripleString =
        toolchain::Twine(triple.getArchName() + "-" + triple.getVendorName() + "-" +
                    triple.getOSName())
            .str();
    osString = triple.getOSName().str();
    archString = triple.getArchName().str();
  }

  // Add devtoolset installation.
  LibStdCxxInjectionVFS &devtoolSet(StringRef version,
                                    StringRef dtName = "devtoolset") {
    // Two files needed for clang to detect the right paths / files.
    newFile(toolchain::Twine("/opt/rh/" + dtName + "-") + version + "/lib/gcc/" +
            tripleString + "/" + version + "/crtbegin.o");
    newFile(toolchain::Twine("/opt/rh/" + dtName + "-") + version +
            "/root/usr/lib/gcc/" + tripleString + "/" + version +
            "/crtbegin.o");

    // Libstdc++ headers needed to detect libstdc++.
    stdlibPath = (toolchain::Twine("/opt/rh/" + dtName + "-") + version +
                  "/root/usr/include/c++/" + version + "/")
                     .str();
    cxxStdlibHeader("string");
    cxxStdlibHeader("cstdlib");
    cxxStdlibHeader("vector");
    return *this;
  }

  LibStdCxxInjectionVFS &cxxStdlibHeader(StringRef name) {
    assert(!stdlibPath.empty());
    newFile(toolchain::Twine(stdlibPath) + name);
    return *this;
  }

  // Add a libstdc++ modulemap that's part of Codira's distribution.
  LibStdCxxInjectionVFS &libstdCxxModulemap(StringRef contents = "") {
    newFile("/usr/lib/language/" + osString + "/libstdcxx.modulemap",
            contents.empty() ? getLibstdcxxModulemapContents() : contents);
    return *this;
  }

private:
  std::string tripleString;
  std::string osString;
  std::string archString;
  std::string stdlibPath;

  void newFile(const toolchain::Twine &path, StringRef contents = "\n") {
    vfs->addFile(path, 0, toolchain::MemoryBuffer::getMemBuffer(contents));
  }
};

TEST(ClangImporterTest, libStdCxxInjectionTest) {
  // Ignore this test on Windows hosts.
  toolchain::Triple Host(toolchain::sys::getProcessTriple());
  if (Host.isOSWindows())
    GTEST_SKIP();

  language::LangOptions langOpts;
  langOpts.EnableCXXInterop = true;
  langOpts.Target = toolchain::Triple("x86_64", "unknown", "linux", "gnu");
  langOpts.CXXStdlib = CXXStdlibKind::Libstdcxx;
  langOpts.PlatformDefaultCXXStdlib = CXXStdlibKind::Libstdcxx;
  language::SILOptions silOpts;
  language::TypeCheckerOptions typecheckOpts;
  INITIALIZE_LLVM();
  language::SearchPathOptions searchPathOpts;
  searchPathOpts.RuntimeResourcePath = "/usr/lib/language";
  language::symbolgraphgen::SymbolGraphOptions symbolGraphOpts;
  language::SourceManager sourceMgr;
  language::DiagnosticEngine diags(sourceMgr);
  ClangImporterOptions options;
  CASOptions casOpts;
  SerializationOptions serializationOpts;
  options.clangPath = "/usr/bin/clang";
  options.ExtraArgs.push_back(
      (toolchain::Twine("--gcc-toolchain=") + "/opt/rh/devtoolset-9/root/usr")
          .str());
  options.ExtraArgs.push_back("--gcc-toolchain");
  std::unique_ptr<ASTContext> context(ASTContext::get(
      langOpts, typecheckOpts, silOpts, searchPathOpts, options,
      symbolGraphOpts, casOpts, serializationOpts, sourceMgr, diags));

  {
    LibStdCxxInjectionVFS vfs;
    vfs.devtoolSet("9").libstdCxxModulemap("\n");
    auto paths = language::getClangInvocationFileMapping(*context, vfs.vfs);
    ASSERT_TRUE(paths.redirectedFiles.size() == 2);
    ASSERT_TRUE(paths.overridenFiles.empty());
    EXPECT_EQ(paths.redirectedFiles[0].first,
              "/opt/rh/devtoolset-9/root/usr/include/c++/9/libstdcxx.h");
    EXPECT_EQ(paths.redirectedFiles[0].second,
              "/usr/lib/language/linux/libstdcxx.h");
    EXPECT_EQ(paths.redirectedFiles[1].first,
              "/opt/rh/devtoolset-9/root/usr/include/c++/9/module.modulemap");
    EXPECT_EQ(paths.redirectedFiles[1].second,
              "/usr/lib/language/linux/libstdcxx.modulemap");
  }

  {
    LibStdCxxInjectionVFS vfs;
    vfs.devtoolSet("9").cxxStdlibHeader("string_view").libstdCxxModulemap();
    auto paths = language::getClangInvocationFileMapping(*context, vfs.vfs);
    ASSERT_TRUE(paths.redirectedFiles.size() == 1);
    ASSERT_TRUE(paths.overridenFiles.size() == 1);
    EXPECT_EQ(paths.redirectedFiles[0].first,
              "/opt/rh/devtoolset-9/root/usr/include/c++/9/libstdcxx.h");
    EXPECT_EQ(paths.redirectedFiles[0].second,
              "/usr/lib/language/linux/libstdcxx.h");
    EXPECT_EQ(paths.overridenFiles[0].first,
              "/opt/rh/devtoolset-9/root/usr/include/c++/9/module.modulemap");
    EXPECT_NE(paths.overridenFiles[0].second.find(
                  "header \"string_view\"\n  /// additional headers."),
              StringRef::npos);
  }

  {
    LibStdCxxInjectionVFS vfs;
    vfs.devtoolSet("9")
        .cxxStdlibHeader("string_view")
        .cxxStdlibHeader("codecvt")
        .cxxStdlibHeader("variant")
        .cxxStdlibHeader("optional")
        .cxxStdlibHeader("memory_resource")
        .cxxStdlibHeader("filesystem")
        .cxxStdlibHeader("charconv")
        .cxxStdlibHeader("any")
        .libstdCxxModulemap();
    auto paths = language::getClangInvocationFileMapping(*context, vfs.vfs);
    ASSERT_TRUE(paths.redirectedFiles.size() == 1);
    ASSERT_TRUE(paths.overridenFiles.size() == 1);
    EXPECT_EQ(paths.redirectedFiles[0].first,
              "/opt/rh/devtoolset-9/root/usr/include/c++/9/libstdcxx.h");
    EXPECT_EQ(paths.redirectedFiles[0].second,
              "/usr/lib/language/linux/libstdcxx.h");
    EXPECT_EQ(paths.overridenFiles[0].first,
              "/opt/rh/devtoolset-9/root/usr/include/c++/9/module.modulemap");
    EXPECT_NE(
        paths.overridenFiles[0].second.find(
            "header \"codecvt\"\n  header \"any\"\n  header \"charconv\"\n  "
            "header \"filesystem\"\n  header \"memory_resource\"\n  header "
            "\"optional\"\n  header \"string_view\"\n  header \"variant\"\n  "
            "/// additional headers."),
        StringRef::npos);
  }
}
