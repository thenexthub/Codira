//===---- WebAssemblyToolChains.cpp - Job invocations (WebAssembly-specific) ------===//
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

#include "language/ABI/System.h"
#include "language/AST/DiagnosticsDriver.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/Platform.h"
#include "language/Basic/Range.h"
#include "language/Config.h"
#include "language/Driver/Compilation.h"
#include "language/Driver/Driver.h"
#include "language/Driver/Job.h"
#include "language/Option/Options.h"
#include "language/Option/SanitizerOptions.h"
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

using namespace language;
using namespace language::driver;
using namespace toolchain::opt;

std::string
toolchains::WebAssembly::sanitizerRuntimeLibName(StringRef Sanitizer,
                                                 bool shared) const {
  return (Twine("clang_rt.") + Sanitizer + "-" +
          this->getTriple().getArchName() + ".lib")
      .str();
}

ToolChain::InvocationInfo toolchains::WebAssembly::constructInvocation(
    const AutolinkExtractJobAction &job, const JobContext &context) const {
  assert(context.Output.getPrimaryOutputType() == file_types::TY_AutolinkFile);

  InvocationInfo II{"language-autolink-extract"};
  ArgStringList &Arguments = II.Arguments;
  II.allowsResponseFiles = true;

  addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                         file_types::TY_Object);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_Object);

  Arguments.push_back("-o");
  Arguments.push_back(
      context.Args.MakeArgString(context.Output.getPrimaryOutputFilename()));

  return II;
}

ToolChain::InvocationInfo
toolchains::WebAssembly::constructInvocation(const DynamicLinkJobAction &job,
                                             const JobContext &context) const {
  assert(context.Output.getPrimaryOutputType() == file_types::TY_Image &&
         "Invalid linker output type.");

  ArgStringList Arguments;

  std::string Target = getTriple().str();
  if (!Target.empty()) {
    Arguments.push_back("-target");
    Arguments.push_back(context.Args.MakeArgString(Target));
  }

  switch (job.getKind()) {
  case LinkKind::None:
    toolchain_unreachable("invalid link kind");
  case LinkKind::Executable:
    // Default case, nothing extra needed.
    break;
  case LinkKind::DynamicLibrary:
    toolchain_unreachable("WebAssembly doesn't support dynamic library yet");
  case LinkKind::StaticLibrary:
    toolchain_unreachable("invalid link kind");
  }

  // Select the linker to use.
  std::string Linker;
  if (const Arg *A = context.Args.getLastArg(options::OPT_use_ld)) {
    Linker = A->getValue();
  }
  if (!Linker.empty())
    Arguments.push_back(context.Args.MakeArgString("-fuse-ld=" + Linker));

  const char *Clang = "clang";
  if (const Arg *A = context.Args.getLastArg(options::OPT_tools_directory)) {
    StringRef toolchainPath(A->getValue());

    // If there is a clang in the toolchain folder, use that instead.
    if (auto tool = toolchain::sys::findProgramByName("clang", {toolchainPath}))
      Clang = context.Args.MakeArgString(tool.get());
  }

  SmallVector<std::string, 4> RuntimeLibPaths;
  getRuntimeLibraryPaths(RuntimeLibPaths, context.Args, context.OI.SDKPath,
                         /*Shared=*/false);

  SmallString<128> SharedResourceDirPath;
  getResourceDirPath(SharedResourceDirPath, context.Args, /*Shared=*/false);

  if (!context.Args.hasArg(options::OPT_nostartfiles)) {
    SmallString<128> languagertPath = SharedResourceDirPath;
    toolchain::sys::path::append(languagertPath,
                            language::getMajorArchitectureName(getTriple()));
    toolchain::sys::path::append(languagertPath, "languagert.o");
    Arguments.push_back(context.Args.MakeArgString(languagertPath));
  }

  addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                         file_types::TY_Object);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_Object);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_TOOLCHAIN_BC);

  if (!context.OI.SDKPath.empty()) {
    Arguments.push_back("--sysroot");
    Arguments.push_back(context.Args.MakeArgString(context.OI.SDKPath));
  }

  // Add any autolinking scripts to the arguments
  for (const Job *Cmd : context.Inputs) {
    auto &OutputInfo = Cmd->getOutput();
    if (OutputInfo.getPrimaryOutputType() == file_types::TY_AutolinkFile)
      Arguments.push_back(context.Args.MakeArgString(
          Twine("@") + OutputInfo.getPrimaryOutputFilename()));
  }

  // Add the runtime library link paths.
  for (auto path : RuntimeLibPaths) {
    Arguments.push_back("-L");
    Arguments.push_back(context.Args.MakeArgString(path));
  }
  // Link the standard library. In two paths, we do this using a .lnk file;
  // if we're going that route, we'll set `linkFilePath` to the path to that
  // file.
  SmallString<128> linkFilePath;
  getResourceDirPath(linkFilePath, context.Args, /*Shared=*/false);
  toolchain::sys::path::append(linkFilePath, "static-executable-args.lnk");

  auto linkFile = linkFilePath.str();
  if (toolchain::sys::fs::is_regular_file(linkFile)) {
    Arguments.push_back(context.Args.MakeArgString(Twine("@") + linkFile));
  } else {
    toolchain::report_fatal_error(linkFile + " not found");
  }

  // Delegate to Clang for sanitizers. It will figure out the correct linker
  // options.
  if (job.getKind() == LinkKind::Executable && context.OI.SelectedSanitizers) {
    Arguments.push_back(context.Args.MakeArgString(
        "-fsanitize=" + getSanitizerList(context.OI.SelectedSanitizers)));
  }

  if (context.Args.hasArg(options::OPT_profile_generate)) {
    SmallString<128> LibProfile(SharedResourceDirPath);
    toolchain::sys::path::remove_filename(LibProfile); // remove platform name
    toolchain::sys::path::append(LibProfile, "clang", "lib");

    toolchain::sys::path::append(LibProfile, getTriple().getOSName(),
                            Twine("libclang_rt.profile-") +
                                getTriple().getArchName() + ".a");
    Arguments.push_back(context.Args.MakeArgString(LibProfile));
    Arguments.push_back(context.Args.MakeArgString(
        Twine("-u", toolchain::getInstrProfRuntimeHookVarName())));
  }

  // Run clang++ in verbose mode if "-v" is set
  if (context.Args.hasArg(options::OPT_v)) {
    Arguments.push_back("-v");
  }

  // WebAssembly doesn't reserve low addresses But without "extra inhabitants"
  // of the pointer representation, runtime performance and memory footprint are
  // worse. So assume that compiler driver uses wasm-ld and --global-base=1024
  // to reserve low 1KB.
  Arguments.push_back("-Xlinker");
  Arguments.push_back(context.Args.MakeArgString(
      Twine("--global-base=") +
      std::to_string(LANGUAGE_ABI_WASM32_LEAST_VALID_POINTER)));

  // These custom arguments should be right before the object file at the end.
  context.Args.AddAllArgs(Arguments, options::OPT_linker_option_Group);
  context.Args.AddAllArgs(Arguments, options::OPT_Xlinker);
  context.Args.AddAllArgValues(Arguments, options::OPT_Xclang_linker);

  // This should be the last option, for convenience in checking output.
  Arguments.push_back("-o");
  Arguments.push_back(
      context.Args.MakeArgString(context.Output.getPrimaryOutputFilename()));

  InvocationInfo II{Clang, Arguments};
  II.allowsResponseFiles = true;

  return II;
}

void validateLinkerArguments(DiagnosticEngine &diags,
                             ArgStringList linkerArgs) {
  for (auto arg : linkerArgs) {
    if (StringRef(arg).starts_with("--global-base=")) {
      diags.diagnose(SourceLoc(), diag::error_wasm_doesnt_support_global_base);
    }
  }
}
void toolchains::WebAssembly::validateArguments(DiagnosticEngine &diags,
                                                const toolchain::opt::ArgList &args,
                                                StringRef defaultTarget) const {
  ArgStringList linkerArgs;
  args.AddAllArgValues(linkerArgs, options::OPT_Xlinker);
  validateLinkerArguments(diags, linkerArgs);
}

ToolChain::InvocationInfo
toolchains::WebAssembly::constructInvocation(const StaticLinkJobAction &job,
                                             const JobContext &context) const {
  assert(context.Output.getPrimaryOutputType() == file_types::TY_Image &&
         "Invalid linker output type.");

  ArgStringList Arguments;

  const char *AR = "toolchain-ar";
  Arguments.push_back("crs");

  Arguments.push_back(
      context.Args.MakeArgString(context.Output.getPrimaryOutputFilename()));

  addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                         file_types::TY_Object);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_Object);

  InvocationInfo II{AR, Arguments};

  return II;
}
