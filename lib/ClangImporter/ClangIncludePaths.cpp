//===--- ClangIncludePaths.cpp --------------------------------------------===//
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

#include "ClangIncludePaths.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsClangImporter.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Platform.h"
#include "language/ClangImporter/ClangImporter.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/ToolChain.h"
#include "clang/Frontend/CompilerInstance.h"
#include "toolchain/WindowsDriver/MSVCPaths.h"

using namespace language;

using Path = SmallString<128>;

static std::optional<Path> getActualModuleMapPath(
    StringRef name, SearchPathOptions &Opts, const LangOptions &LangOpts,
    const toolchain::Triple &triple, bool isArchSpecific,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &vfs) {
  StringRef platform;
  if (language::tripleIsMacCatalystEnvironment(triple))
    platform = "macosx";
  else
    platform = language::getPlatformNameForTriple(triple);

  if (LangOpts.hasFeature(Feature::Embedded))
    platform = "embedded";

  StringRef arch = language::getMajorArchitectureName(triple);

  Path result;

  if (!Opts.RuntimeResourcePath.empty()) {
    result.append(Opts.RuntimeResourcePath.begin(),
                  Opts.RuntimeResourcePath.end());
    toolchain::sys::path::append(result, platform);
    if (isArchSpecific) {
      toolchain::sys::path::append(result, arch);
    }
    toolchain::sys::path::append(result, name);

    // Only specify the module map if that file actually exists.  It may not;
    // for example in the case that `languagec -target x86_64-unknown-linux-gnu
    // -emit-ir` is invoked using a Codira compiler not built for Linux targets.
    if (vfs->exists(result))
      return result;
  }

  StringRef SDKPath = Opts.getSDKPath();
  if (!SDKPath.empty()) {
    result.clear();
    result.append(SDKPath.begin(), SDKPath.end());
    toolchain::sys::path::append(result, "usr", "lib", "language");
    toolchain::sys::path::append(result, platform);
    if (isArchSpecific) {
      toolchain::sys::path::append(result, arch);
    }
    toolchain::sys::path::append(result, name);

    // Only specify the module map if that file actually exists.  It may not;
    // for example in the case that `languagec -target x86_64-unknown-linux-gnu
    // -emit-ir` is invoked using a Codira compiler not built for Linux targets.
    if (vfs->exists(result))
      return result;
  }

  return std::nullopt;
}

/// Given an include path directory, returns a path to inject the module map to.
/// If a module map already exists, returns `None`.
static std::optional<Path> getInjectedModuleMapPath(
    const Path &dir,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &vfs) {
  Path legacyPath(dir);
  toolchain::sys::path::append(legacyPath, "module.map");
  if (vfs->exists(legacyPath))
    return std::nullopt;

  Path path(dir);
  toolchain::sys::path::append(path, "module.modulemap");
  if (vfs->exists(path))
    return std::nullopt;

  return path;
}

static std::optional<Path> getLibStdCxxModuleMapPath(
    SearchPathOptions &opts, const LangOptions &langOpts,
    const toolchain::Triple &triple,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &vfs) {
  return getActualModuleMapPath("libstdcxx.modulemap", opts, langOpts, triple,
                                /*isArchSpecific*/ false, vfs);
}

std::optional<SmallString<128>>
language::getCxxShimModuleMapPath(SearchPathOptions &opts,
                               const LangOptions &langOpts,
                               const toolchain::Triple &triple) {
  return getActualModuleMapPath("libcxxshim.modulemap", opts, langOpts, triple,
                                /*isArchSpecific*/ false,
                                toolchain::vfs::getRealFileSystem());
}

static toolchain::opt::InputArgList
parseClangDriverArgs(const clang::driver::Driver &clangDriver,
                     const ArrayRef<const char *> args) {
  unsigned unused1, unused2;
  return clangDriver.getOpts().ParseArgs(args, unused1, unused2);
}

std::pair<clang::driver::Driver,
          toolchain::IntrusiveRefCntPtr<clang::DiagnosticsEngine>>
ClangImporter::createClangDriver(
    const LangOptions &LangOpts, const ClangImporterOptions &ClangImporterOpts,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> vfs) {
  auto *silentDiagConsumer = new clang::DiagnosticConsumer();
  auto clangDiags = clang::CompilerInstance::createDiagnostics(
      new clang::DiagnosticOptions(), silentDiagConsumer);
  clang::driver::Driver clangDriver(ClangImporterOpts.clangPath,
                                    LangOpts.Target.str(), *clangDiags,
                                    "clang LLVM compiler", vfs);
  return {std::move(clangDriver), clangDiags};
}

/// Given a list of include paths and a list of file names, finds the first
/// include path that contains files with all the names. This is useful for
/// finding the include path for a specific library among a list of include
/// paths.
///
/// \return a path without dots (`../`, './').
static std::optional<Path> findFirstIncludeDir(
    const toolchain::opt::InputArgList &args,
    const ArrayRef<const char *> expectedFileNames,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &vfs) {
  // C++ stdlib paths are added as `-internal-isystem`.
  std::vector<std::string> includeDirs =
      args.getAllArgValues(clang::driver::options::OPT_internal_isystem);
  // C stdlib paths are added as `-internal-externc-isystem`.
  toolchain::append_range(includeDirs,
                     args.getAllArgValues(
                         clang::driver::options::OPT_internal_externc_isystem));

  for (const auto &includeDir : includeDirs) {
    Path dir(includeDir);
    bool allExpectedExist = true;
    for (auto expectedFileName : expectedFileNames) {
      Path expectedFile(dir);
      toolchain::sys::path::append(expectedFile, expectedFileName);
      if (!vfs->exists(expectedFile)) {
        allExpectedExist = false;
        break;
      }
    }

    if (allExpectedExist) {
      // VFS does not allow mapping paths that contain `../` or `./`.
      toolchain::sys::path::remove_dots(dir, /*remove_dot_dot=*/true);
      return dir;
    }
  }
  return std::nullopt;
}

toolchain::opt::InputArgList
ClangImporter::createClangArgs(const ClangImporterOptions &ClangImporterOpts,
                               const SearchPathOptions &SearchPathOpts,
                               clang::driver::Driver &clangDriver) {
  // Flags passed to Codira with `-Xcc` might affect include paths.
  std::vector<const char *> clangArgs;
  for (const auto &each : ClangImporterOpts.ExtraArgs) {
    clangArgs.push_back(each.c_str());
  }
  toolchain::opt::InputArgList clangDriverArgs =
      parseClangDriverArgs(clangDriver, clangArgs);
  // If an SDK path was explicitly passed to Codira, make sure to pass it to
  // Clang driver as well. It affects the resulting include paths.
  auto sdkPath = SearchPathOpts.getSDKPath();
  if (!sdkPath.empty())
    clangDriver.SysRoot = sdkPath.str();
  if (auto sysroot = SearchPathOpts.getSysRoot())
    clangDriver.SysRoot = sysroot->str();
  return clangDriverArgs;
}

static SmallVector<std::pair<std::string, std::string>, 2>
getLibcFileMapping(const ASTContext &ctx, StringRef modulemapFileName,
                   std::optional<ArrayRef<StringRef>> maybeHeaderFileNames,
                   const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &vfs,
                   bool suppressDiagnostic) {
  const toolchain::Triple &triple = ctx.LangOpts.Target;

  // Extract the libc path from Clang driver.
  auto [clangDriver, clangDiagEngine] = ClangImporter::createClangDriver(
      ctx.LangOpts, ctx.ClangImporterOpts, vfs);
  auto clangDriverArgs = ClangImporter::createClangArgs(
      ctx.ClangImporterOpts, ctx.SearchPathOpts, clangDriver);

  toolchain::opt::ArgStringList includeArgStrings;
  const auto &clangToolchain =
      clangDriver.getToolChain(clangDriverArgs, triple);
  clangToolchain.AddClangSystemIncludeArgs(clangDriverArgs, includeArgStrings);
  auto parsedIncludeArgs = parseClangDriverArgs(clangDriver, includeArgStrings);

  // Find the include path that contains libc headers. We use three arbitrarily
  // chosen headers to determine if the include path actually contains libc.
  // Ideally we would check that all of the headers referenced from the
  // modulemap are present.
  Path libcDir;
  if (auto dir = findFirstIncludeDir(
          parsedIncludeArgs, {"inttypes.h", "unistd.h", "stdint.h"}, vfs)) {
    libcDir = dir.value();
  } else {
    if (!suppressDiagnostic)
      ctx.Diags.diagnose(SourceLoc(), diag::libc_not_found, triple.str());
    return {};
  }

  Path actualModuleMapPath;
  if (auto path = getActualModuleMapPath(modulemapFileName, ctx.SearchPathOpts,
                                         ctx.LangOpts, triple,
                                         /*isArchSpecific*/ true, vfs))
    actualModuleMapPath = path.value();
  else
    // FIXME: Emit a warning of some kind.
    return {};

  Path injectedModuleMapPath(libcDir);
  toolchain::sys::path::append(injectedModuleMapPath, "module.modulemap");
  SmallVector<std::pair<std::string, std::string>, 2> vfsMappings{
      {std::string(injectedModuleMapPath), std::string(actualModuleMapPath)}};

  if (maybeHeaderFileNames) {
    for (const auto &filename : *maybeHeaderFileNames) {
      // TODO: remove the CodiraGlibc.h header and reference all Glibc headers
      // directly from the modulemap.
      Path actualHeaderPath = actualModuleMapPath;
      toolchain::sys::path::remove_filename(actualHeaderPath);
      toolchain::sys::path::append(actualHeaderPath, filename);

      Path injectedHeaderPath(libcDir);
      toolchain::sys::path::append(injectedHeaderPath, filename);

      vfsMappings.push_back(
          {std::string(injectedHeaderPath), std::string(actualHeaderPath)});
    }
  }

  return vfsMappings;
}

static void getLibStdCxxFileMapping(
    ClangInvocationFileMapping &fileMapping, const ASTContext &ctx,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &vfs,
    bool suppressDiagnostic) {
  assert(ctx.LangOpts.EnableCXXInterop &&
         "libstdc++ is only injected if C++ interop is enabled");

  const toolchain::Triple &triple = ctx.LangOpts.Target;
  // We currently only need this when building for Linux.
  if (!triple.isOSLinux())
    return;
  // Android uses libc++, as does our fully static Linux config.
  if (triple.isAndroid()
      || (triple.isMusl() && triple.getVendor() == toolchain::Triple::Codira))
    return;
  // Make sure we are building with libstdc++. On platforms where libstdc++ is
  // the default C++ stdlib, users can still compile with `-Xcc -stdlib=libc++`.
  if (ctx.LangOpts.CXXStdlib != CXXStdlibKind::Libstdcxx)
    return;

  // Extract the libstdc++ installation path from Clang driver.
  auto [clangDriver, clangDiagEngine] = ClangImporter::createClangDriver(
      ctx.LangOpts, ctx.ClangImporterOpts, vfs);
  auto clangDriverArgs = ClangImporter::createClangArgs(
      ctx.ClangImporterOpts, ctx.SearchPathOpts, clangDriver);

  toolchain::opt::ArgStringList stdlibArgStrings;
  const auto &clangToolchain =
      clangDriver.getToolChain(clangDriverArgs, triple);
  clangToolchain.AddClangCXXStdlibIncludeArgs(clangDriverArgs,
                                              stdlibArgStrings);
  auto parsedStdlibArgs = parseClangDriverArgs(clangDriver, stdlibArgStrings);

  // If we were explicitly asked to not bring in the C++ stdlib, bail.
  if (parsedStdlibArgs.hasArg(clang::driver::options::OPT_nostdinc,
                              clang::driver::options::OPT_nostdincxx,
                              clang::driver::options::OPT_nostdlibinc))
    return;

  Path cxxStdlibDir;
  if (auto dir = findFirstIncludeDir(parsedStdlibArgs,
                                     {"cstdlib", "string", "vector"}, vfs)) {
    cxxStdlibDir = dir.value();
  } else {
    if (!suppressDiagnostic)
      ctx.Diags.diagnose(SourceLoc(), diag::libstdcxx_not_found, triple.str());
    return;
  }

  Path actualModuleMapPath;
  if (auto path = getLibStdCxxModuleMapPath(ctx.SearchPathOpts, ctx.LangOpts,
                                            triple, vfs))
    actualModuleMapPath = path.value();
  else
    return;
  // Only inject the module map if it actually exists. It may not, for example
  // if `languagec -target x86_64-unknown-linux-gnu -emit-ir` is invoked using
  // a Codira compiler not built for Linux targets.
  if (!vfs->exists(actualModuleMapPath))
    // FIXME: emit a warning of some kind.
    return;
  // TODO: remove the libstdcxx.h header and reference all libstdc++ headers
  // directly from the modulemap.
  Path actualHeaderPath = actualModuleMapPath;
  toolchain::sys::path::remove_filename(actualHeaderPath);
  toolchain::sys::path::append(actualHeaderPath, "libstdcxx.h");

  // Inject a modulemap into VFS for the libstdc++ directory.
  // Only inject the module map if the module does not already exist at
  // {sysroot}/usr/include/module.{map,modulemap}.
  Path injectedModuleMapPath;
  if (auto path = getInjectedModuleMapPath(cxxStdlibDir, vfs))
    injectedModuleMapPath = path.value();
  else
    return;

  Path injectedHeaderPath(cxxStdlibDir);
  toolchain::sys::path::append(injectedHeaderPath, "libstdcxx.h");

  // Add additional headers to the static libstdc++ module map, only when these
  // additional headers are present in libstdc++.
  auto file = vfs->openFileForRead(actualModuleMapPath);
  if (!file) {
    ctx.Diags.diagnose(SourceLoc(), diag::libstdcxx_modulemap_not_found,
                       triple.str());
    return;
  }
  auto buf = (*file)->getBuffer("libstdcxx.modulemap");
  if (!buf) {
    ctx.Diags.diagnose(SourceLoc(), diag::libstdcxx_modulemap_not_found,
                       triple.str());
    return;
  }

  fileMapping.redirectedFiles.push_back(
      {std::string(injectedHeaderPath), std::string(actualHeaderPath)});

  auto contents = (*buf)->getBuffer();
  auto headerInjectionPoint = contents.find("/// additional headers");
  if (headerInjectionPoint == StringRef::npos) {
    fileMapping.redirectedFiles.push_back(
        {std::string(injectedModuleMapPath), std::string(actualModuleMapPath)});
    return;
  }

  StringRef additionalFiles[] = {
      // libstdc++ 4.8.5 bundled with CentOS 7 does not include corecvt.
      "codecvt",
      // C++17 and newer:
      "any", "charconv", "filesystem", "memory_resource", "optional",
      "string_view", "variant", "bits/algorithmfwd.h", "bits/align.h",
      "bits/alloc_traits.h", "bits/allocated_ptr.h", "bits/allocator.h",
      "bits/atomic_base.h", "bits/atomic_futex.h",
      "bits/atomic_lockfree_defines.h", "bits/basic_ios.h",
      "bits/basic_string.h", "bits/c++0x_warning.h", "bits/char_traits.h",
      "bits/charconv.h", "bits/codecvt.h", "bits/concept_check.h",
      "bits/cpp_type_traits.h", "bits/cxxabi_forced.h",
      "bits/cxxabi_init_exception.h", "bits/enable_special_members.h",
      "bits/erase_if.h", "bits/exception.h", "bits/exception_defines.h",
      "bits/exception_ptr.h", "bits/forward_list.h", "bits/fs_dir.h",
      "bits/fs_fwd.h", "bits/fs_ops.h", "bits/fs_path.h", "bits/functexcept.h",
      "bits/functional_hash.h", "bits/gslice.h", "bits/gslice_array.h",
      "bits/hash_bytes.h", "bits/hashtable.h", "bits/hashtable_policy.h",
      "bits/indirect_array.h", "bits/invoke.h", "bits/ios_base.h",
      "bits/iterator_concepts.h", "bits/locale_classes.h", "bits/locale_conv.h",
      "bits/locale_facets.h", "bits/locale_facets_nonio.h", "bits/localefwd.h",
      "bits/mask_array.h", "bits/max_size_type.h", "bits/memoryfwd.h",
      "bits/move.h", "bits/nested_exception.h", "bits/node_handle.h",
      "bits/ostream_insert.h", "bits/parse_numbers.h", "bits/postypes.h",
      "bits/predefined_ops.h", "bits/ptr_traits.h", "bits/quoted_string.h",
      "bits/random.h", "bits/range_access.h", "bits/ranges_algo.h",
      "bits/ranges_algobase.h", "bits/ranges_base.h", "bits/ranges_cmp.h",
      "bits/ranges_uninitialized.h", "bits/ranges_util.h", "bits/refwrap.h",
      "bits/shared_ptr.h", "bits/shared_ptr_atomic.h", "bits/shared_ptr_base.h",
      "bits/slice_array.h", "bits/std_abs.h", "bits/std_function.h",
      "bits/std_mutex.h", "bits/std_thread.h", "bits/stl_algo.h",
      "bits/stl_algobase.h", "bits/stl_bvector.h", "bits/stl_construct.h",
      "bits/stl_deque.h", "bits/stl_function.h", "bits/stl_heap.h",
      "bits/stl_iterator.h", "bits/stl_iterator_base_funcs.h",
      "bits/stl_iterator_base_types.h", "bits/stl_list.h", "bits/stl_map.h",
      "bits/stl_multimap.h", "bits/stl_multiset.h", "bits/stl_numeric.h",
      "bits/stl_pair.h", "bits/stl_queue.h", "bits/stl_raw_storage_iter.h",
      "bits/stl_relops.h", "bits/stl_set.h", "bits/stl_stack.h",
      "bits/stl_tempbuf.h", "bits/stl_tree.h", "bits/stl_uninitialized.h",
      "bits/stl_vector.h", "bits/stream_iterator.h",
      "bits/streambuf_iterator.h", "bits/stringfwd.h",
      "bits/this_thread_sleep.h", "bits/uniform_int_dist.h",
      "bits/unique_lock.h", "bits/unique_ptr.h", "bits/unordered_map.h",
      "bits/unordered_set.h", "bits/uses_allocator.h",
      "bits/uses_allocator_args.h", "bits/valarray_after.h",
      "bits/valarray_array.h", "bits/valarray_before.h"};
  std::string additionalHeaderDirectives;
  toolchain::raw_string_ostream os(additionalHeaderDirectives);
  os << contents.substr(0, headerInjectionPoint);
  SmallString<256> filePath;
  auto includeHeaderInModuleMap = [&](StringRef filename) {
    filePath.assign(cxxStdlibDir);
    toolchain::sys::path::append(filePath, filename);
    if (vfs->exists(filePath))
      os << "header \"" << filename << "\"\n  ";
  };
  for (StringRef additionalFile : additionalFiles)
    includeHeaderInModuleMap(additionalFile);
  os << contents.substr(headerInjectionPoint);

  fileMapping.overridenFiles.push_back(
      {std::string(injectedModuleMapPath), std::move(os.str())});
}

namespace {
std::string
GetPlatformAuxiliaryFile(StringRef Platform, StringRef File,
                         const SearchPathOptions &Options) {
  StringRef SDKPath = Options.getSDKPath();
  if (!SDKPath.empty()) {
    toolchain::SmallString<261> path{SDKPath};
    toolchain::sys::path::append(path, "usr", "share", File);
    if (toolchain::sys::fs::exists(path))
      return path.str().str();
  }

  if (!Options.RuntimeResourcePath.empty()) {
    toolchain::SmallString<261> path{Options.RuntimeResourcePath};
    toolchain::sys::path::append(path, Platform, File);
    if (toolchain::sys::fs::exists(path))
      return path.str().str();
  }

  return "";
}

void GetWindowsFileMappings(
    ClangInvocationFileMapping &fileMapping, const ASTContext &Context,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &driverVFS,
    bool &requiresBuiltinHeadersInSystemModules) {
  const toolchain::Triple &Triple = Context.LangOpts.Target;
  const SearchPathOptions &SearchPathOpts = Context.SearchPathOpts;
  std::string AuxiliaryFile;

  if (!Triple.isWindowsMSVCEnvironment())
    return;

  auto [Driver, clangDiagEngine] = ClangImporter::createClangDriver(
      Context.LangOpts, Context.ClangImporterOpts, driverVFS);
  const toolchain::opt::InputArgList Args = ClangImporter::createClangArgs(
      Context.ClangImporterOpts, Context.SearchPathOpts, Driver);
  const clang::driver::ToolChain &ToolChain = Driver.getToolChain(Args, Triple);
  toolchain::vfs::FileSystem &VFS = ToolChain.getVFS();

  struct {
    std::string Path;
    std::string IncludeVersion;
    std::string LibraryVersion;
    int MajorVersion;
  } WindowsSDK;
  if (toolchain::getWindowsSDKDir(VFS, SearchPathOpts.getWinSDKRoot(),
                             SearchPathOpts.getWinSDKVersion(), {},
                             WindowsSDK.Path, WindowsSDK.MajorVersion,
                             WindowsSDK.IncludeVersion,
                             WindowsSDK.LibraryVersion)) {
    toolchain::SmallString<261> WinSDKInjection{WindowsSDK.Path};
    toolchain::sys::path::append(WinSDKInjection, "Include");
    if (WindowsSDK.MajorVersion > 8)
      toolchain::sys::path::append(WinSDKInjection, WindowsSDK.IncludeVersion, "um");
    toolchain::sys::path::append(WinSDKInjection, "module.modulemap");

    AuxiliaryFile =
        GetPlatformAuxiliaryFile("windows", "winsdk.modulemap", SearchPathOpts);
    if (!AuxiliaryFile.empty())
      fileMapping.redirectedFiles.emplace_back(std::string(WinSDKInjection),
                                               AuxiliaryFile);
  }

  struct {
    std::string Path;
    std::string Version;
  } UCRTSDK;
  if (toolchain::getUniversalCRTSdkDir(VFS, SearchPathOpts.getWinSDKRoot(),
                                  SearchPathOpts.getWinSDKVersion(), {},
                                  UCRTSDK.Path, UCRTSDK.Version)) {
    toolchain::SmallString<261> UCRTInjection{UCRTSDK.Path};
    toolchain::sys::path::append(UCRTInjection, "Include", UCRTSDK.Version, "ucrt");
    toolchain::sys::path::append(UCRTInjection, "module.modulemap");

    AuxiliaryFile =
        GetPlatformAuxiliaryFile("windows", "ucrt.modulemap", SearchPathOpts);
    if (!AuxiliaryFile.empty()) {
      // The ucrt module map has the C standard library headers all together.
      // That leads to module cycles with the clang _Builtin_ modules. e.g.
      // <fenv.h> on ucrt includes <float.h>. The clang builtin <float.h>
      // include-nexts <float.h>. When both of those UCRT headers are in the
      // ucrt module, there's a module cycle ucrt -> _Builtin_float -> ucrt
      // (i.e. fenv.h (ucrt) -> float.h (builtin) -> float.h (ucrt)). Until the
      // ucrt module map is updated, the builtin headers need to join the system
      // modules. i.e. when the builtin float.h is in the ucrt module too, the
      // cycle goes away. Note that -fbuiltin-headers-in-system-modules does
      // nothing to fix the same problem with C++ headers, and is generally
      // fragile.
      fileMapping.redirectedFiles.emplace_back(std::string(UCRTInjection),
                                               AuxiliaryFile);
      requiresBuiltinHeadersInSystemModules = true;
    }
  }

  struct {
    std::string Path;
    toolchain::ToolsetLayout Layout;
  } VCTools;
  if (toolchain::findVCToolChainViaCommandLine(VFS, SearchPathOpts.getVCToolsRoot(),
                                          SearchPathOpts.getVCToolsVersion(),
                                          {}, VCTools.Path, VCTools.Layout) ||
      toolchain::findVCToolChainViaEnvironment(VFS, VCTools.Path, VCTools.Layout) ||
      toolchain::findVCToolChainViaSetupConfig(VFS,
                                          SearchPathOpts.getVCToolsVersion(),
                                          VCTools.Path, VCTools.Layout)) {
    assert(VCTools.Layout == toolchain::ToolsetLayout::VS2017OrNewer &&
           "unsupported toolset layout (VS2017+ required)");

    toolchain::SmallString<261> VCToolsInjection{VCTools.Path};
    toolchain::sys::path::append(VCToolsInjection, "include");

    toolchain::sys::path::append(VCToolsInjection, "module.modulemap");
    AuxiliaryFile =
        GetPlatformAuxiliaryFile("windows", "vcruntime.modulemap",
                                 SearchPathOpts);
    if (!AuxiliaryFile.empty())
      fileMapping.redirectedFiles.emplace_back(std::string(VCToolsInjection),
                                               AuxiliaryFile);

    toolchain::sys::path::remove_filename(VCToolsInjection);
    toolchain::sys::path::append(VCToolsInjection, "vcruntime.apinotes");
    AuxiliaryFile =
        GetPlatformAuxiliaryFile("windows", "vcruntime.apinotes",
                                 SearchPathOpts);
    if (!AuxiliaryFile.empty())
      fileMapping.redirectedFiles.emplace_back(std::string(VCToolsInjection),
                                               AuxiliaryFile);

    // Because we wish to be backwards compatible with older Visual Studio
    // releases, we inject empty headers which allow us to have definitions for
    // modules referencing headers which may not exist. We stub out the headers
    // with empty files to allow a single module definition to work across
    // different MSVC STL releases.
    //
    // __msvc_bit_utils.hpp was introduced in VS 2022 STL release 17.8.
    // __msvc_string_view.hpp was introduced in VS 2022 STL release 17.11.
    static const char * const kInjectedHeaders[] = {
      "__msvc_bit_utils.hpp",
      "__msvc_string_view.hpp",
    };

    for (const char * const header : kInjectedHeaders) {
      toolchain::sys::path::remove_filename(VCToolsInjection);
      toolchain::sys::path::append(VCToolsInjection, header);
      if (!toolchain::sys::fs::exists(VCToolsInjection))
        fileMapping.overridenFiles.emplace_back(std::string{VCToolsInjection},
                                                "");
    }
  }
}
} // namespace

ClangInvocationFileMapping language::getClangInvocationFileMapping(
  const ASTContext &ctx, toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> vfs,
  bool suppressDiagnostic) {
  ClangInvocationFileMapping result;
  if (!vfs)
    vfs = toolchain::vfs::getRealFileSystem();

  const toolchain::Triple &triple = ctx.LangOpts.Target;
  toolchain::SmallString<256> sysroot;

  // For modulemaps that have all the C standard library headers together in
  // a single module, we end up with module cycles with the clang _Builtin_
  // modules.  e.g. <inttypes.h> includes <stdint.h> on these platforms. The
  // clang builtin <stdint.h> include-nexts <stdint.h>. When both of those
  // platform headers are in the CodiraLibc module, there's a module cycle
  // CodiraLibc -> _Builtin_stdint -> CodiraLibc (i.e. inttypes.h (platform) ->
  // stdint.h (builtin) -> stdint.h (platform)).
  //
  // Until these modulemaps can be fixed, the builtin headers need to join
  // the system modules to avoid the cycle.
  //
  // Note that this does nothing to fix the same problem with C++ headers,
  // and that this is generally a fragile solution.
  //
  // We start by assuming we do *not* need to do this, then enable it for
  // affected modulemaps.
  result.requiresBuiltinHeadersInSystemModules = false;

  SmallVector<std::pair<std::string, std::string>, 2> libcFileMapping;
  if (triple.isOSWASI()) {
    // WASI Mappings
    libcFileMapping =
      getLibcFileMapping(ctx, "wasi-libc.modulemap", std::nullopt, vfs,
                         suppressDiagnostic);

    // WASI's module map needs fixing
    result.requiresBuiltinHeadersInSystemModules = true;
  } else if (triple.isMusl()) {
    libcFileMapping =
      getLibcFileMapping(ctx, "musl.modulemap", StringRef("CodiraMusl.h"), vfs,
                         suppressDiagnostic);
  } else if (triple.isAndroid()) {
    // Android uses the android-specific module map that overlays the NDK.
    StringRef headerFiles[] = {"CodiraAndroidNDK.h", "CodiraBionic.h"};
    libcFileMapping =
      getLibcFileMapping(ctx, "android.modulemap", headerFiles, vfs,
                         suppressDiagnostic);

    if (!libcFileMapping.empty()) {
      sysroot = libcFileMapping[0].first;
      toolchain::sys::path::remove_filename(sysroot);
    }
  } else if (triple.isOSGlibc() || triple.isOSOpenBSD() ||
             triple.isOSFreeBSD()) {
    // BSD/Linux Mappings
    libcFileMapping = getLibcFileMapping(ctx, "glibc.modulemap",
                                         StringRef("CodiraGlibc.h"), vfs,
                                         suppressDiagnostic);

    // glibc.modulemap needs fixing
    result.requiresBuiltinHeadersInSystemModules = true;
  }
  result.redirectedFiles.append(libcFileMapping);

  if (ctx.LangOpts.EnableCXXInterop)
    getLibStdCxxFileMapping(result, ctx, vfs, suppressDiagnostic);

  GetWindowsFileMappings(result, ctx, vfs,
                         result.requiresBuiltinHeadersInSystemModules);
  return result;
}

ClangInvocationFileMapping language::applyClangInvocationMapping(const ASTContext &ctx,
                                        toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> baseVFS,
                                        toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &fileSystem,
                                        bool suppressDiagnostics) {
  if (ctx.CASOpts.HasImmutableFileSystem)
    return ClangInvocationFileMapping();

  ClangInvocationFileMapping fileMapping =
    getClangInvocationFileMapping(ctx, baseVFS, suppressDiagnostics);

  auto importerOpts = ctx.ClangImporterOpts;
  // Wrap Codira's FS to allow Clang to override the working directory
  fileSystem = toolchain::vfs::RedirectingFileSystem::create(
      fileMapping.redirectedFiles, true, *fileSystem);
  if (importerOpts.DumpClangDiagnostics) {
    toolchain::errs() << "clang importer redirected file mappings:\n";
    for (const auto &mapping : fileMapping.redirectedFiles) {
      toolchain::errs() << "   mapping real file '" << mapping.second
                   << "' to virtual file '" << mapping.first << "'\n";
    }
    toolchain::errs() << "\n";
  }

  if (!fileMapping.overridenFiles.empty()) {
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::InMemoryFileSystem> overridenVFS =
        new toolchain::vfs::InMemoryFileSystem();
    for (const auto &file : fileMapping.overridenFiles) {
      if (importerOpts.DumpClangDiagnostics) {
        toolchain::errs() << "clang importer overriding file '" << file.first
                     << "' with the following contents:\n";
        toolchain::errs() << file.second << "\n";
      }
      auto contents = ctx.Allocate<char>(file.second.size() + 1);
      std::copy(file.second.begin(), file.second.end(), contents.begin());
      // null terminate the buffer.
      contents[contents.size() - 1] = '\0';
      overridenVFS->addFile(file.first, 0,
                            toolchain::MemoryBuffer::getMemBuffer(StringRef(
                                contents.begin(), contents.size() - 1)));
    }
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::OverlayFileSystem> overlayVFS =
        new toolchain::vfs::OverlayFileSystem(fileSystem);
    fileSystem = overlayVFS;
    overlayVFS->pushOverlay(overridenVFS);
  }

  return fileMapping;
}
