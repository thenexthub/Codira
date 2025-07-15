//===------ DarwinToolChains.cpp - Job invocations (Darwin-specific) ------===//
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

#include "ToolChains.h"

#include "language/AST/DiagnosticsDriver.h"
#include "language/AST/PlatformKindUtils.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/Platform.h"
#include "language/Basic/Range.h"
#include "language/Basic/STLExtras.h"
#include "language/Basic/TaskQueue.h"
#include "language/Config.h"
#include "language/Driver/Compilation.h"
#include "language/Driver/Driver.h"
#include "language/Driver/Job.h"
#include "language/IDETool/CompilerInvocation.h"
#include "language/Option/Options.h"
#include "clang/Basic/DarwinSDKInfo.h"
#include "clang/Basic/Version.h"
#include "clang/Driver/Util.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/ProfileData/InstrProf.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Process.h"
#include "toolchain/Support/Program.h"
#include "toolchain/Support/VersionTuple.h"

using namespace language;
using namespace language::driver;
using namespace toolchain::opt;

std::string
toolchains::Darwin::findProgramRelativeToCodiraImpl(StringRef name) const {
  StringRef languagePath = getDriver().getCodiraProgramPath();
  StringRef languageBinDir = toolchain::sys::path::parent_path(languagePath);

  // See if we're in an Xcode toolchain.
  bool hasToolchain = false;
  toolchain::SmallString<128> path{languageBinDir};
  toolchain::sys::path::remove_filename(path); // bin
  toolchain::sys::path::remove_filename(path); // usr
  if (toolchain::sys::path::extension(path) == ".xctoolchain") {
    hasToolchain = true;
    toolchain::sys::path::remove_filename(path); // *.xctoolchain
    toolchain::sys::path::remove_filename(path); // Toolchains
    toolchain::sys::path::append(path, "usr", "bin");
  }

  StringRef paths[] = {languageBinDir, path};
  auto pathsRef = toolchain::ArrayRef(paths);
  if (!hasToolchain)
    pathsRef = pathsRef.drop_back();

  auto result = toolchain::sys::findProgramByName(name, pathsRef);
  if (result)
    return result.get();
  return {};
}

ToolChain::InvocationInfo
toolchains::Darwin::constructInvocation(const InterpretJobAction &job,
                                        const JobContext &context) const {
  InvocationInfo II = ToolChain::constructInvocation(job, context);

  SmallVector<std::string, 4> runtimeLibraryPaths;
  getRuntimeLibraryPaths(runtimeLibraryPaths, context.Args, context.OI.SDKPath,
                         /*Shared=*/true);

  addPathEnvironmentVariableIfNeeded(II.ExtraEnvironment, "DYLD_LIBRARY_PATH",
                                     ":", options::OPT_L, context.Args,
                                     runtimeLibraryPaths);
  addPathEnvironmentVariableIfNeeded(II.ExtraEnvironment, "DYLD_FRAMEWORK_PATH",
                                     ":", options::OPT_F, context.Args,
                                     {"/System/Library/Frameworks"});
  // FIXME: Add options::OPT_Fsystem paths to DYLD_FRAMEWORK_PATH as well.
  return II;
}

static StringRef
getDarwinLibraryNameSuffixForTriple(const toolchain::Triple &triple) {
  const DarwinPlatformKind kind = getDarwinPlatformKind(triple);
  switch (kind) {
  case DarwinPlatformKind::MacOS:
    return "osx";
  case DarwinPlatformKind::IPhoneOS:
    // Here we return "osx" under the assumption that all the
    // darwin runtime libraries are zippered and so the "osx" variants
    // should be used for macCatalyst targets.
    if (tripleIsMacCatalystEnvironment(triple))
        return "osx";
    return "ios";
  case DarwinPlatformKind::IPhoneOSSimulator:
    return "iossim";
  case DarwinPlatformKind::TvOS:
    return "tvos";
  case DarwinPlatformKind::TvOSSimulator:
    return "tvossim";
  case DarwinPlatformKind::WatchOS:
    return "watchos";
  case DarwinPlatformKind::WatchOSSimulator:
    return "watchossim";
  case DarwinPlatformKind::VisionOS:
    return "xros";
  case DarwinPlatformKind::VisionOSSimulator:
    return "xrossim";
  }
  toolchain_unreachable("Unsupported Darwin platform");
}

std::string toolchains::Darwin::sanitizerRuntimeLibName(StringRef Sanitizer,
                                                        bool shared) const {
  return (Twine("libclang_rt.") + Sanitizer + "_" +
          getDarwinLibraryNameSuffixForTriple(this->getTriple()) +
          (shared ? "_dynamic.dylib" : ".a"))
      .str();
}

static void addLinkRuntimeLibRPath(const ArgList &Args,
                                   ArgStringList &Arguments,
                                   StringRef DarwinLibName,
                                   const ToolChain &TC) {
  // Adding the rpaths might negatively interact when other rpaths are involved,
  // so we should make sure we add the rpaths last, after all user-specified
  // rpaths. This is currently true from this place, but we need to be
  // careful if this function is ever called before user's rpaths are emitted.
  assert(DarwinLibName.ends_with(".dylib") && "must be a dynamic library");

  // Add @executable_path to rpath to support having the dylib copied with
  // the executable.
  Arguments.push_back("-rpath");
  Arguments.push_back("@executable_path");

  // Add the path to the resource dir to rpath to support using the dylib
  // from the default location without copying.

  SmallString<128> ClangLibraryPath;
  TC.getClangLibraryPath(Args, ClangLibraryPath);

  Arguments.push_back("-rpath");
  Arguments.push_back(Args.MakeArgString(ClangLibraryPath));
}

static void addLinkSanitizerLibArgsForDarwin(const ArgList &Args,
                                             ArgStringList &Arguments,
                                             StringRef Sanitizer,
                                             const ToolChain &TC,
                                             bool shared = true) {
  // Sanitizer runtime libraries requires C++.
  Arguments.push_back("-lc++");
  // Add explicit dependency on -lc++abi, as -lc++ doesn't re-export
  // all RTTI-related symbols that are used.
  Arguments.push_back("-lc++abi");

  auto LibName = TC.sanitizerRuntimeLibName(Sanitizer, shared);
  TC.addLinkRuntimeLib(Args, Arguments, LibName);

  if (shared)
    addLinkRuntimeLibRPath(Args, Arguments, LibName, TC);
}

/// Runs <code>xcrun -f clang</code> in order to find the location of Clang for
/// the currently active Xcode.
///
/// We get the "currently active" part by passing through the DEVELOPER_DIR
/// environment variable (along with the rest of the environment).
static bool findXcodeClangPath(toolchain::SmallVectorImpl<char> &path) {
  assert(path.empty());

  auto xcrunPath = toolchain::sys::findProgramByName("xcrun");
  if (!xcrunPath.getError()) {
    // Explicitly ask for the default toolchain so that we don't find a Clang
    // included with an open-source toolchain.
    const char *args[] = {"-toolchain", "default", "-f", "clang", nullptr};
    sys::TaskQueue queue;
    queue.addTask(xcrunPath->c_str(), args, /*Env=*/std::nullopt,
                  /*Context=*/nullptr,
                  /*SeparateErrors=*/true);
    queue.execute(nullptr,
                  [&path](sys::ProcessId PID, int returnCode, StringRef output,
                          StringRef errors,
                          sys::TaskProcessInformation ProcInfo,
                          void *unused) -> sys::TaskFinishedResponse {
                    if (returnCode == 0) {
                      output = output.rtrim();
                      path.append(output.begin(), output.end());
                    }
                    return sys::TaskFinishedResponse::ContinueExecution;
                  });
  }

  return !path.empty();
}

static bool findXcodeClangLibPath(const Twine &libName,
                                  toolchain::SmallVectorImpl<char> &path) {
  assert(path.empty());

  if (!findXcodeClangPath(path)) {
    return false;
  }
  toolchain::sys::path::remove_filename(path); // 'clang'
  toolchain::sys::path::remove_filename(path); // 'bin'
  toolchain::sys::path::append(path, "lib", libName);
  return true;
}

static void addVersionString(const ArgList &inputArgs, ArgStringList &arguments,
                             toolchain::VersionTuple version) {
  toolchain::SmallString<8> buf;
  toolchain::raw_svector_ostream os{buf};
  os << version.getMajor() << '.' << version.getMinor().value_or(0) << '.'
     << version.getSubminor().value_or(0);
  arguments.push_back(inputArgs.MakeArgString(os.str()));
}

void
toolchains::Darwin::addLinkerInputArgs(InvocationInfo &II,
                                       const JobContext &context) const {
  ArgStringList &Arguments = II.Arguments;
  if (context.shouldUseInputFileList()) {
    Arguments.push_back("-filelist");
    Arguments.push_back(context.getTemporaryFilePath("inputs", "LinkFileList"));
    II.FilelistInfos.push_back(
        {Arguments.back(), context.OI.CompilerOutputType,
         FilelistInfo::WhichFiles::InputJobsAndSourceInputActions});
  } else {
    addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                           file_types::TY_Object);
    addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                           file_types::TY_TBD);
    addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                           file_types::TY_TOOLCHAIN_BC);
    addInputsOfType(Arguments, context.InputActions, file_types::TY_Object);
    addInputsOfType(Arguments, context.InputActions, file_types::TY_TBD);
    addInputsOfType(Arguments, context.InputActions, file_types::TY_TOOLCHAIN_BC);
  }


  if (context.OI.CompilerMode == OutputInfo::Mode::SingleCompile)
    addInputsOfType(Arguments, context.Inputs, context.Args,
                    file_types::TY_CodiraModuleFile, "-add_ast_path");
  else
    addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                           file_types::TY_CodiraModuleFile, "-add_ast_path");

  // Add all .codemodule file inputs as arguments, preceded by the
  // "-add_ast_path" linker option.
  addInputsOfType(Arguments, context.InputActions,
                  file_types::TY_CodiraModuleFile, "-add_ast_path");
}

void toolchains::Darwin::addLTOLibArgs(ArgStringList &Arguments,
                                       const JobContext &context) const {
  if (!context.OI.LibLTOPath.empty()) {
    // Check for user-specified LTO library.
    Arguments.push_back("-lto_library");
    Arguments.push_back(context.Args.MakeArgString(context.OI.LibLTOPath));
  } else {
    // Check for relative libLTO.dylib. This would be the expected behavior in an
    // Xcode toolchain.
    StringRef P = toolchain::sys::path::parent_path(getDriver().getCodiraProgramPath());
    toolchain::SmallString<128> LibLTOPath(P);
    toolchain::sys::path::remove_filename(LibLTOPath); // Remove '/bin'
    toolchain::sys::path::append(LibLTOPath, "lib");
    toolchain::sys::path::append(LibLTOPath, "libLTO.dylib");
    if (toolchain::sys::fs::exists(LibLTOPath)) {
      Arguments.push_back("-lto_library");
      Arguments.push_back(context.Args.MakeArgString(LibLTOPath));
    } else {
      // Use libLTO.dylib from the default toolchain if a relative one does not exist.
      toolchain::SmallString<128> LibLTOPath;
      if (findXcodeClangLibPath("libLTO.dylib", LibLTOPath)) {
        Arguments.push_back("-lto_library");
        Arguments.push_back(context.Args.MakeArgString(LibLTOPath));
      }
    }
  }
}

void
toolchains::Darwin::addSanitizerArgs(ArgStringList &Arguments,
                                     const DynamicLinkJobAction &job,
                                     const JobContext &context) const {
  // Linking sanitizers will add rpaths, which might negatively interact when
  // other rpaths are involved, so we should make sure we add the rpaths after
  // all user-specified rpaths.
  if (context.OI.SelectedSanitizers & SanitizerKind::Address) {
    if (context.OI.SanitizerUseStableABI)
      addLinkSanitizerLibArgsForDarwin(context.Args, Arguments, "asan_abi",
                                       *this, false);
    else
      addLinkSanitizerLibArgsForDarwin(context.Args, Arguments, "asan", *this);
  }

  if (context.OI.SelectedSanitizers & SanitizerKind::Thread)
    addLinkSanitizerLibArgsForDarwin(context.Args, Arguments, "tsan", *this);

  if (context.OI.SelectedSanitizers & SanitizerKind::Undefined)
    addLinkSanitizerLibArgsForDarwin(context.Args, Arguments, "ubsan", *this);

  // Only link in libFuzzer for executables.
  if (job.getKind() == LinkKind::Executable &&
      (context.OI.SelectedSanitizers & SanitizerKind::Fuzzer))
    addLinkSanitizerLibArgsForDarwin(context.Args, Arguments, "fuzzer", *this,
                                     /*shared=*/false);
}

namespace {

enum class BackDeployLibFilter {
  executable,
  all
};

// Whether the given job matches the backward-deployment library filter.
bool jobMatchesFilter(LinkKind jobKind, BackDeployLibFilter filter) {
  switch (filter) {
  case BackDeployLibFilter::executable:
    return jobKind == LinkKind::Executable;
    
  case BackDeployLibFilter::all:
    return true;
  }
  toolchain_unreachable("unhandled back deploy lib filter!");
}

}

void
toolchains::Darwin::addArgsToLinkStdlib(ArgStringList &Arguments,
                                        const DynamicLinkJobAction &job,
                                        const JobContext &context) const {

  // Link compatibility libraries, if we're deploying back to OSes that
  // have an older Codira runtime.
  SmallString<128> SharedResourceDirPath;
  getResourceDirPath(SharedResourceDirPath, context.Args, /*Shared=*/true);
  std::optional<toolchain::VersionTuple> runtimeCompatibilityVersion;

  if (context.Args.hasArg(options::OPT_runtime_compatibility_version)) {
    auto value = context.Args.getLastArgValue(
                                    options::OPT_runtime_compatibility_version);
    if (value == "5.0") {
      runtimeCompatibilityVersion = toolchain::VersionTuple(5, 0);
    } else if (value == "5.1") {
      runtimeCompatibilityVersion = toolchain::VersionTuple(5, 1);
    } else if (value == "5.5") {
      runtimeCompatibilityVersion = toolchain::VersionTuple(5, 5);
    } else if (value == "5.6") {
      runtimeCompatibilityVersion = toolchain::VersionTuple(5, 6);
    } else if (value == "5.8") {
      runtimeCompatibilityVersion = toolchain::VersionTuple(5, 8);
    } else if (value == "6.0") {
      runtimeCompatibilityVersion = toolchain::VersionTuple(6, 0);
    } else if (value == "none") {
      runtimeCompatibilityVersion = std::nullopt;
    } else {
      // TODO: diagnose unknown runtime compatibility version?
    }
  } else if (job.getKind() == LinkKind::Executable) {
    runtimeCompatibilityVersion
                   = getCodiraRuntimeCompatibilityVersionForTarget(getTriple());
  }
  
  if (runtimeCompatibilityVersion) {
    auto addBackDeployLib = [&](toolchain::VersionTuple version,
                                BackDeployLibFilter filter,
                                StringRef libraryName,
                                bool forceLoad) {
      if (*runtimeCompatibilityVersion > version)
        return;

      if (!jobMatchesFilter(job.getKind(), filter))
        return;
      
      SmallString<128> BackDeployLib;
      BackDeployLib.append(SharedResourceDirPath);
      toolchain::sys::path::append(BackDeployLib, "lib" + libraryName + ".a");
      
      if (toolchain::sys::fs::exists(BackDeployLib)) {
        if (forceLoad)
          Arguments.push_back("-force_load");
        Arguments.push_back(context.Args.MakeArgString(BackDeployLib));
      }
    };

    #define BACK_DEPLOYMENT_LIB(Version, Filter, LibraryName, ForceLoad) \
      addBackDeployLib(                                                  \
          toolchain::VersionTuple Version, BackDeployLibFilter::Filter,       \
          LibraryName, ForceLoad);
    #include "language/Frontend/BackDeploymentLibs.def"
  }
    
  // Add the runtime library link path, which is platform-specific and found
  // relative to the compiler.
  SmallVector<std::string, 4> RuntimeLibPaths;
  getRuntimeLibraryPaths(RuntimeLibPaths, context.Args,
                         context.OI.SDKPath, /*Shared=*/true);

  for (auto path : RuntimeLibPaths) {
    Arguments.push_back("-L");
    Arguments.push_back(context.Args.MakeArgString(path));
  }

  if (context.Args.hasFlag(options::OPT_toolchain_stdlib_rpath,
                           options::OPT_no_toolchain_stdlib_rpath, false)) {
    // If the user has explicitly asked for a toolchain stdlib, we should
    // provide one using -rpath. This used to be the default behaviour but it
    // was considered annoying in at least the CodiraPM scenario (see
    // https://github.com/apple/language/issues/44576) and is obsolete in all
    // scenarios of deploying for Codira-in-the-OS. We keep it here as an
    // optional behaviour so that people downloading snapshot toolchains for
    // testing new stdlibs will be able to link to the stdlib bundled in
    // that toolchain.
    for (auto path : RuntimeLibPaths) {
      Arguments.push_back("-rpath");
      Arguments.push_back(context.Args.MakeArgString(path));
    }
  } else if (!tripleRequiresRPathForCodiraLibrariesInOS(getTriple()) ||
             context.Args.hasArg(options::OPT_no_stdlib_rpath)) {
    // If targeting an OS with Codira in /usr/lib/language, the LC_ID_DYLIB
    // install_name the stdlib will be an absolute path like
    // /usr/lib/language/liblanguageCore.dylib, and we do not need to provide an rpath
    // at all.
    //
    // Also, if the user explicitly asks for no rpath entry, we assume they know
    // what they're doing and do not add one here.
  } else {
    // The remaining cases are back-deploying (to OSs predating
    // Codira-in-the-OS). In these cases, the stdlib will be giving us (via
    // stdlib/linker-support/magic-symbols-for-install-name.c) an LC_ID_DYLIB
    // install_name that is rpath-relative, like @rpath/liblanguageCore.dylib.
    //
    // If we're linking an app bundle, it's possible there's an embedded stdlib
    // in there, in which case we'd want to put @executable_path/../Frameworks
    // in the rpath to find and prefer it, but (a) we don't know when we're
    // linking an app bundle and (b) we probably _never_ will be because Xcode
    // links using clang, not the language driver.
    //
    // So that leaves us with the case of linking a command-line app. These are
    // only supported by installing a secondary package that puts some frozen
    // Codira-in-OS libraries in the /usr/lib/language location. That's the best we
    // can give for rpath, though it might fail at runtime if the support
    // package isn't installed.
    Arguments.push_back("-rpath");
    Arguments.push_back(context.Args.MakeArgString("/usr/lib/language"));
    // We don’t need an rpath for /System/iOSSupport/usr/lib/language because:
    // 1. The standard library and overlays were part of the OS before
    //    Catalyst was introduced, so they are always available for Catalyst.
    // 2. The _Concurrency back-deployment library is zippered, whereas only
    //    unzippered frameworks need an unzippered twin in /System/iOSSupport.
  }
}

void
toolchains::Darwin::addProfileGenerationArgs(ArgStringList &Arguments,
                                             const JobContext &context) const {
  const toolchain::Triple &Triple = getTriple();
  if (context.Args.hasArg(options::OPT_profile_generate)) {
    SmallString<128> LibProfile;
    getClangLibraryPath(context.Args, LibProfile);

    StringRef RT;
    if (Triple.isiOS()) {
      if (Triple.isTvOS())
        RT = "tvos";
      else
        RT = "ios";
    } else if (Triple.isWatchOS()) {
      RT = "watchos";
    } else if (Triple.isXROS()) {
      RT = "xros";
    } else {
      assert(Triple.isMacOSX());
      RT = "osx";
    }

    StringRef Sim;
    if (Triple.isSimulatorEnvironment()) {
      Sim = "sim";
    }

    toolchain::sys::path::append(LibProfile,
                            "libclang_rt.profile_" + RT + Sim + ".a");

    // FIXME: Continue accepting the old path for simulator libraries for now.
    if (!Sim.empty() && !toolchain::sys::fs::exists(LibProfile)) {
      toolchain::sys::path::remove_filename(LibProfile);
      toolchain::sys::path::append(LibProfile, "libclang_rt.profile_" + RT + ".a");
    }

    Arguments.push_back(context.Args.MakeArgString(LibProfile));
  }
}

std::optional<toolchain::VersionTuple>
toolchains::Darwin::getTargetSDKVersion(const toolchain::Triple &triple) const {
  if (!SDKInfo)
    return std::nullopt;
  return language::getTargetSDKVersion(*SDKInfo, triple);
}

void
toolchains::Darwin::addDeploymentTargetArgs(ArgStringList &Arguments,
                                            const JobContext &context) const {
  auto addPlatformVersionArg = [&](const toolchain::Triple &triple) {
    // Compute the name of the platform for the linker.
    const char *platformName;
    if (tripleIsMacCatalystEnvironment(triple)) {
      platformName = "mac-catalyst";
    } else {
      switch (getDarwinPlatformKind(triple)) {
      case DarwinPlatformKind::MacOS:
        platformName = "macos";
        break;
      case DarwinPlatformKind::IPhoneOS:
        platformName = "ios";
        break;
      case DarwinPlatformKind::IPhoneOSSimulator:
        platformName = "ios-simulator";
        break;
      case DarwinPlatformKind::TvOS:
        platformName = "tvos";
        break;
      case DarwinPlatformKind::TvOSSimulator:
        platformName = "tvos-simulator";
        break;
      case DarwinPlatformKind::WatchOS:
        platformName = "watchos";
        break;
      case DarwinPlatformKind::WatchOSSimulator:
        platformName = "watchos-simulator";
        break;
      case DarwinPlatformKind::VisionOS:
        platformName = "xros";
        break;
      case DarwinPlatformKind::VisionOSSimulator:
        platformName = "xros-simulator";
        break;
      }
    }

    // Compute the platform version.
    toolchain::VersionTuple osVersion;
    if (tripleIsMacCatalystEnvironment(triple)) {
      osVersion = triple.getiOSVersion();

      if (osVersion.getMajor() < 14 && triple.isAArch64()) {
        // Mac Catalyst on arm was introduced with an iOS deployment target of
        // 14.0; the linker doesn't want to see a deployment target before that.
        osVersion = toolchain::VersionTuple(/*Major=*/14, /*Minor=*/0);
      } else if (osVersion.getMajor() < 13) {
        // Mac Catalyst was introduced with an iOS deployment target of 13.1;
        // the linker doesn't want to see a deployment target before that.
        osVersion = toolchain::VersionTuple(/*Major=*/13, /*Minor=*/1);
      }
    } else {
      switch (getDarwinPlatformKind((triple))) {
      case DarwinPlatformKind::MacOS:
        triple.getMacOSXVersion(osVersion);

        // The first deployment of arm64 for macOS is version 10.16;
        if (triple.isAArch64() && osVersion.getMajor() <= 10 &&
            osVersion.getMinor().value_or(0) < 16) {
          osVersion = toolchain::VersionTuple(/*Major=*/10, /*Minor=*/16);
          osVersion = canonicalizePlatformVersion(PlatformKind::macOS,
                                                  osVersion);
        }

        break;
      case DarwinPlatformKind::IPhoneOS:
      case DarwinPlatformKind::IPhoneOSSimulator:
      case DarwinPlatformKind::TvOS:
      case DarwinPlatformKind::TvOSSimulator:
        osVersion = triple.getiOSVersion();

        // The first deployment of arm64 simulators is iOS/tvOS 14.0;
        // the linker doesn't want to see a deployment target before that.
        if (triple.isSimulatorEnvironment() && triple.isAArch64() &&
            osVersion.getMajor() < 14) {
          osVersion = toolchain::VersionTuple(/*Major=*/14, /*Minor=*/0);
        }

        break;
      case DarwinPlatformKind::WatchOS:
      case DarwinPlatformKind::WatchOSSimulator:
        osVersion = triple.getOSVersion();
        break;
      case DarwinPlatformKind::VisionOS:
      case DarwinPlatformKind::VisionOSSimulator:
        osVersion = triple.getOSVersion();

        // The first deployment of 64-bit xrOS simulator is version 1.0.
        if (triple.isArch64Bit() && triple.isSimulatorEnvironment() &&
            osVersion.getMajor() < 1) {
          osVersion = toolchain::VersionTuple(/*Major=*/1, /*Minor=*/0);
        }

        break;
      }
    }

    // Compute the SDK version.
    auto sdkVersion = getTargetSDKVersion(triple)
        .value_or(toolchain::VersionTuple());

    Arguments.push_back("-platform_version");
    Arguments.push_back(platformName);
    addVersionString(context.Args, Arguments, osVersion);
    addVersionString(context.Args, Arguments, sdkVersion);
  };

  addPlatformVersionArg(getTriple());

  if (auto targetVariant = getTargetVariant()) {
    assert(triplesAreValidForZippering(getTriple(), *targetVariant));
    addPlatformVersionArg(*targetVariant);
  }
}

static unsigned getDWARFVersionForTriple(const toolchain::Triple &triple) {
  toolchain::VersionTuple osVersion;
  const DarwinPlatformKind kind = getDarwinPlatformKind(triple);
  // Default to DWARF 2 on OS X 10.10 / iOS 8 and lower.
  // Default to DWARF 4 on OS X 10.11 - macOS 14 / iOS - iOS 17.
  switch (kind) {
  case DarwinPlatformKind::MacOS:
    triple.getMacOSXVersion(osVersion);
    if (osVersion < toolchain::VersionTuple(10, 11))
      return 2;
    if (osVersion < toolchain::VersionTuple(15))
      return 4;
    return 5;
  case DarwinPlatformKind::IPhoneOSSimulator:
  case DarwinPlatformKind::IPhoneOS:
  case DarwinPlatformKind::TvOS:
  case DarwinPlatformKind::TvOSSimulator:
    osVersion = triple.getiOSVersion();
   if (osVersion < toolchain::VersionTuple(9))
     return 2;
    if (osVersion < toolchain::VersionTuple(18))
      return 4;
    return 5;
  case DarwinPlatformKind::WatchOS:
  case DarwinPlatformKind::WatchOSSimulator:
    osVersion = triple.getWatchOSVersion();
    if (osVersion < toolchain::VersionTuple(11))
      return 4;
    return 5;
  case DarwinPlatformKind::VisionOS:
  case DarwinPlatformKind::VisionOSSimulator:
    osVersion = triple.getOSVersion();
    if (osVersion < toolchain::VersionTuple(2))
      return 4;
    return 5;
  }
  toolchain_unreachable("unsupported platform kind");
}

void toolchains::Darwin::addCommonFrontendArgs(
    const OutputInfo &OI, const CommandOutput &output,
    const toolchain::opt::ArgList &inputArgs,
    toolchain::opt::ArgStringList &arguments) const {
  ToolChain::addCommonFrontendArgs(OI, output, inputArgs, arguments);

  if (auto sdkVersion = getTargetSDKVersion(getTriple())) {
    arguments.push_back("-target-sdk-version");
    arguments.push_back(inputArgs.MakeArgString(sdkVersion->getAsString()));
  }

  if (auto targetVariant = getTargetVariant()) {
    if (auto variantSDKVersion = getTargetSDKVersion(*targetVariant)) {
      arguments.push_back("-target-variant-sdk-version");
      arguments.push_back(
          inputArgs.MakeArgString(variantSDKVersion->getAsString()));
    }
  }
  std::string dwarfVersion;
  {
    toolchain::raw_string_ostream os(dwarfVersion);
    os << "-dwarf-version=";
    if (OI.DWARFVersion)
      os << std::to_string(*OI.DWARFVersion);
    else
      os << getDWARFVersionForTriple(getTriple());
  }
  arguments.push_back(inputArgs.MakeArgString(dwarfVersion));
}

/// Add the frontend arguments needed to find external plugins in standard
/// locations based on the base path.
static void addExternalPluginFrontendArgs(
    StringRef basePath, const toolchain::opt::ArgList &inputArgs,
    toolchain::opt::ArgStringList &arguments) {
  // Plugin server: $BASE/usr/bin/language-plugin-server
  SmallString<128> pluginServer;
  toolchain::sys::path::append(
      pluginServer, basePath, "usr", "bin", "language-plugin-server");

  SmallString<128> pluginDir;
  toolchain::sys::path::append(pluginDir, basePath, "usr", "lib");
  toolchain::sys::path::append(pluginDir, "language", "host", "plugins");
  arguments.push_back("-external-plugin-path");
  arguments.push_back(inputArgs.MakeArgString(pluginDir + "#" + pluginServer));

  pluginDir.clear();
  toolchain::sys::path::append(pluginDir, basePath, "usr", "local", "lib");
  toolchain::sys::path::append(pluginDir, "language", "host", "plugins");
  arguments.push_back("-external-plugin-path");
  arguments.push_back(inputArgs.MakeArgString(pluginDir + "#" + pluginServer));
}

void toolchains::Darwin::addPlatformSpecificPluginFrontendArgs(
    const OutputInfo &OI,
    const CommandOutput &output,
    const toolchain::opt::ArgList &inputArgs,
    toolchain::opt::ArgStringList &arguments) const {
  // Add SDK-relative directories for plugins.
  if (!OI.SDKPath.empty()) {
    addExternalPluginFrontendArgs(OI.SDKPath, inputArgs, arguments);
  }

  // Add platform-relative directories for plugins.
  if (!OI.SDKPath.empty()) {
    SmallString<128> platformPath;
    toolchain::sys::path::append(platformPath, OI.SDKPath);
    toolchain::sys::path::remove_filename(platformPath); // specific SDK
    toolchain::sys::path::remove_filename(platformPath); // SDKs
    toolchain::sys::path::remove_filename(platformPath); // Developer

    StringRef platformName = toolchain::sys::path::filename(platformPath);
    if (platformName.ends_with("Simulator.platform")){
      StringRef devicePlatformName =
          platformName.drop_back(strlen("Simulator.platform"));
      toolchain::sys::path::remove_filename(platformPath); // Platform
      toolchain::sys::path::append(platformPath, devicePlatformName + "OS.platform");
    }

    toolchain::sys::path::append(platformPath, "Developer");
    addExternalPluginFrontendArgs(platformPath, inputArgs, arguments);
  }
}

ToolChain::InvocationInfo
toolchains::Darwin::constructInvocation(const DynamicLinkJobAction &job,
                                        const JobContext &context) const {
  assert(context.Output.getPrimaryOutputType() == file_types::TY_Image &&
         "Invalid linker output type.");

  if (context.Args.hasFlag(options::OPT_static_executable,
                           options::OPT_no_static_executable, false)) {
    toolchain::report_fatal_error("-static-executable is not supported on Darwin");
  }

  const toolchain::Triple &Triple = getTriple();

  // Configure the toolchain.
  // By default, use the system `ld` to link.
  const char *LD = "ld";
  if (const Arg *A = context.Args.getLastArg(options::OPT_tools_directory)) {
    StringRef toolchainPath(A->getValue());

    // If there is a 'ld' in the toolchain folder, use that instead.
    if (auto toolchainLD =
            toolchain::sys::findProgramByName("ld", {toolchainPath})) {
      LD = context.Args.MakeArgString(toolchainLD.get());
    }
  }

  InvocationInfo II = {LD};
  ArgStringList &Arguments = II.Arguments;

  addLinkerInputArgs(II, context);

  switch (job.getKind()) {
  case LinkKind::None:
    toolchain_unreachable("invalid link kind");
  case LinkKind::Executable:
    // The default for ld; no extra flags necessary.
    break;
  case LinkKind::DynamicLibrary:
    Arguments.push_back("-dylib");
    break;
  case LinkKind::StaticLibrary:
    toolchain_unreachable("the dynamic linker cannot build static libraries");
  }

  assert(Triple.isOSDarwin());

  // FIXME: If we used Clang as a linker instead of going straight to ld,
  // we wouldn't have to replicate a bunch of Clang's logic here.

  // Always link the regular compiler_rt if it's present.
  //
  // Note: Normally we'd just add this unconditionally, but it's valid to build
  // Codira and use it as a linker without building compiler_rt.
  SmallString<128> CompilerRTPath;
  getClangLibraryPath(context.Args, CompilerRTPath);
  toolchain::sys::path::append(
      CompilerRTPath,
      Twine("libclang_rt.") +
        getDarwinLibraryNameSuffixForTriple(Triple) +
        ".a");
  if (toolchain::sys::fs::exists(CompilerRTPath))
    Arguments.push_back(context.Args.MakeArgString(CompilerRTPath));

  if (job.shouldPerformLTO()) {
    addLTOLibArgs(Arguments, context);
  }

  for (const Arg *arg :
       context.Args.filtered(options::OPT_F, options::OPT_Fsystem)) {
    Arguments.push_back("-F");
    Arguments.push_back(arg->getValue());
  }

  if (context.Args.hasArg(options::OPT_enable_app_extension)) {
    // Keep this string fixed in case the option used by the
    // compiler itself changes.
    Arguments.push_back("-application_extension");
  }

  addSanitizerArgs(Arguments, job, context);

  if (context.Args.hasArg(options::OPT_embed_bitcode,
                          options::OPT_embed_bitcode_marker)) {
    Arguments.push_back("-bitcode_bundle");
  }

  if (!context.OI.SDKPath.empty()) {
    Arguments.push_back("-syslibroot");
    Arguments.push_back(context.Args.MakeArgString(context.OI.SDKPath));
  }

  Arguments.push_back("-lobjc");
  Arguments.push_back("-lSystem");

  Arguments.push_back("-arch");
  Arguments.push_back(context.Args.MakeArgString(getTriple().getArchName()));

  // On Darwin, we only support libc++.
  if (context.Args.hasArg(options::OPT_enable_experimental_cxx_interop)) {
    Arguments.push_back("-lc++");
  }

  addArgsToLinkStdlib(Arguments, job, context);

  addProfileGenerationArgs(Arguments, context);
  addDeploymentTargetArgs(Arguments, context);

  Arguments.push_back("-no_objc_category_merging");

  // These custom arguments should be right before the object file at the end.
  context.Args.AddAllArgsExcept(Arguments, {options::OPT_linker_option_Group},
                                {options::OPT_l});
  ToolChain::addLinkedLibArgs(context.Args, Arguments);
  context.Args.AddAllArgValues(Arguments, options::OPT_Xlinker);

  // This should be the last option, for convenience in checking output.
  Arguments.push_back("-o");
  Arguments.push_back(
      context.Args.MakeArgString(context.Output.getPrimaryOutputFilename()));

  return II;
}


ToolChain::InvocationInfo
toolchains::Darwin::constructInvocation(const StaticLinkJobAction &job,
                                        const JobContext &context) const {
   assert(context.Output.getPrimaryOutputType() == file_types::TY_Image &&
         "Invalid linker output type.");

  // Configure the toolchain.
  const char *LibTool = "libtool";

  InvocationInfo II = {LibTool};
  ArgStringList &Arguments = II.Arguments;

  Arguments.push_back("-static");

  if (context.shouldUseInputFileList()) {
    Arguments.push_back("-filelist");
    Arguments.push_back(context.getTemporaryFilePath("inputs", "LinkFileList"));
    II.FilelistInfos.push_back({Arguments.back(), context.OI.CompilerOutputType,
                                FilelistInfo::WhichFiles::InputJobs});
  } else {
    addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                           file_types::TY_Object);
    addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                           file_types::TY_TOOLCHAIN_BC);
  }

  addInputsOfType(Arguments, context.InputActions, file_types::TY_Object);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_TOOLCHAIN_BC);

  Arguments.push_back("-o");

  Arguments.push_back(
      context.Args.MakeArgString(context.Output.getPrimaryOutputFilename()));

  return II;
}

bool toolchains::Darwin::shouldStoreInvocationInDebugInfo() const {
  // This matches the behavior in Clang (see
  // clang/lib/driver/ToolChains/Darwin.cpp).
  if (const char *S = ::getenv("RC_DEBUG_OPTIONS"))
    return S[0] != '\0';
  return false;
}

std::string toolchains::Darwin::getGlobalDebugPathRemapping() const {
  // This matches the behavior in Clang (see
  // clang/lib/driver/ToolChains/Darwin.cpp).
  if (const char *S = ::getenv("RC_DEBUG_PREFIX_MAP"))
    return S;
  return {};
}

static void validateDeploymentTarget(const toolchains::Darwin &TC,
                                     DiagnosticEngine &diags,
                                     const toolchain::opt::ArgList &args) {
  // Check minimum supported OS versions.
  auto triple = TC.getTriple();
  if (triple.isMacOSX()) {
    if (triple.isMacOSXVersionLT(10, 9))
      diags.diagnose(SourceLoc(), diag::error_os_minimum_deployment,
                     "OS X 10.9");
  } else if (triple.isiOS()) {
    if (triple.isTvOS()) {
      if (triple.isOSVersionLT(9, 0)) {
        diags.diagnose(SourceLoc(), diag::error_os_minimum_deployment,
                       "tvOS 9.0");
        return;
      }
    }
    if (triple.isOSVersionLT(7))
      diags.diagnose(SourceLoc(), diag::error_os_minimum_deployment,
                     "iOS 7");
    if (triple.isArch32Bit() && !triple.isOSVersionLT(11)) {
      diags.diagnose(SourceLoc(), diag::error_ios_maximum_deployment_32,
                     triple.getOSMajorVersion());
    }
  } else if (triple.isWatchOS()) {
    if (triple.isOSVersionLT(2, 0)) {
      diags.diagnose(SourceLoc(), diag::error_os_minimum_deployment,
                     "watchOS 2.0");
      return;
    }
  }
}

static void validateTargetVariant(const toolchains::Darwin &TC,
                                  DiagnosticEngine &diags,
                                  const toolchain::opt::ArgList &args,
                                  StringRef defaultTarget) {
  if (TC.getTargetVariant().has_value()) {
    auto target = TC.getTriple();
    auto variant = *TC.getTargetVariant();

    if (!triplesAreValidForZippering(target, variant)) {
      diags.diagnose(SourceLoc(), diag::error_unsupported_target_variant,
                    variant.str(),
                    variant.isiOS());
    }
  }
}

void 
toolchains::Darwin::validateArguments(DiagnosticEngine &diags,
                                      const toolchain::opt::ArgList &args,
                                      StringRef defaultTarget) const {
  // Validating apple platforms deployment targets.
  validateDeploymentTarget(*this, diags, args);
  validateTargetVariant(*this, diags, args, defaultTarget);

  // Validating darwin unsupported -static-stdlib argument.
  if (args.hasArg(options::OPT_static_stdlib)) {
    diags.diagnose(SourceLoc(), diag::error_darwin_static_stdlib_not_supported);
  }

  // Validating darwin deprecated -link-objc-runtime.
  if (args.hasArg(options::OPT_link_objc_runtime,
		  options::OPT_no_link_objc_runtime)) {
    diags.diagnose(SourceLoc(), diag::warn_darwin_link_objc_deprecated);
  }
}

void
toolchains::Darwin::validateOutputInfo(DiagnosticEngine &diags,
                                       const OutputInfo &outputInfo) const {
  // If we have been provided with an SDK, go read the SDK information.
  if (!outputInfo.SDKPath.empty()) {
    auto SDKInfoOrErr = clang::parseDarwinSDKInfo(
        *toolchain::vfs::getRealFileSystem(), outputInfo.SDKPath);
    if (SDKInfoOrErr) {
      SDKInfo = *SDKInfoOrErr;
    } else {
      toolchain::consumeError(SDKInfoOrErr.takeError());
      diags.diagnose(SourceLoc(), diag::warn_drv_darwin_sdk_invalid_settings);
    }
  }
}
