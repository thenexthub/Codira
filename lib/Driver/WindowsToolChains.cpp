//===---- WindowsToolChains.cpp - Job invocations (Windows-specific) ------===//
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

#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/Platform.h"
#include "language/Basic/Range.h"
#include "language/Basic/TaskQueue.h"
#include "language/Config.h"
#include "language/Driver/Compilation.h"
#include "language/Driver/Driver.h"
#include "language/Driver/Job.h"
#include "language/Option/Options.h"
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

std::string toolchains::Windows::sanitizerRuntimeLibName(StringRef Sanitizer,
                                                         bool shared) const {
  return (Twine("clang_rt.") + Sanitizer + "-" +
          this->getTriple().getArchName() + ".lib")
      .str();
}

ToolChain::InvocationInfo
toolchains::Windows::constructInvocation(const DynamicLinkJobAction &job,
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
    Arguments.push_back("-shared");
    break;
  case LinkKind::StaticLibrary:
    toolchain_unreachable("invalid link kind");
  }

  // Check to see whether we need to use lld as the linker.
  auto requiresLLD = [&]{
    if (const Arg *A = context.Args.getLastArg(options::OPT_use_ld)) {
      return toolchain::StringSwitch<bool>(A->getValue())
        .Cases("lld", "lld.exe", "lld-link", "lld-link.exe", true)
        .Default(false);
    }
    // Force to use lld for LTO on Windows because we don't support link LTO or
    // something else except for lld LTO at this time.
    if (context.OI.LTOVariant != OutputInfo::LTOKind::None) {
      return true;
    }
    // Profiling currently relies on the ability to emit duplicate weak
    // symbols across translation units and having the linker coalesce them.
    // Unfortunately link.exe does not support this, so require lld-link
    // for now, which supports the behavior via a flag.
    // TODO: Once we've changed coverage to no longer rely on emitting
    // duplicate weak symbols (rdar://131295678), we can remove this.
    if (context.Args.getLastArg(options::OPT_profile_generate)) {
      return true;
    }
    return false;
  }();

  // Select the linker to use.
  std::string Linker;
  if (const Arg *A = context.Args.getLastArg(options::OPT_use_ld)) {
    Linker = A->getValue();
  } else if (requiresLLD) {
    Linker = "lld";
  }

  switch (context.OI.LTOVariant) {
  case OutputInfo::LTOKind::LLVMThin:
    Arguments.push_back("-flto=thin");
    break;
  case OutputInfo::LTOKind::LLVMFull:
    Arguments.push_back("-flto=full");
    break;
  case OutputInfo::LTOKind::None:
    break;
  }

  if (!Linker.empty())
    Arguments.push_back(context.Args.MakeArgString("-fuse-ld=" + Linker));

  if (context.OI.DebugInfoFormat == IRGenDebugInfoFormat::CodeView) {
      Arguments.push_back("-Xlinker");
      Arguments.push_back("/DEBUG");
  }

  // Rely on `-libc` to correctly identify the MSVC Runtime Library.  We use
  // `-nostartfiles` as that limits the difference to just the
  // `-defaultlib:libcmt` which is passed unconditionally with the `clang`
  // driver rather than the `clang-cl` driver.
  Arguments.push_back("-nostartfiles");

  bool wantsStaticStdlib =
      context.Args.hasFlag(options::OPT_static_stdlib,
                           options::OPT_no_static_stdlib, false);

  SmallVector<std::string, 4> RuntimeLibPaths;
  getRuntimeLibraryPaths(RuntimeLibPaths, context.Args, context.OI.SDKPath,
                         /*Shared=*/!wantsStaticStdlib);

  for (auto path : RuntimeLibPaths) {
    Arguments.push_back("-L");
    // Since Windows has separate libraries per architecture, link against the
    // architecture specific version of the static library.
    Arguments.push_back(context.Args.MakeArgString(path + "/" +
                                                   getTriple().getArchName()));
  }

  if (!context.Args.hasArg(options::OPT_nostartfiles)) {
    SmallString<128> SharedResourceDirPath;
    getResourceDirPath(SharedResourceDirPath, context.Args, /*Shared=*/true);

    SmallString<128> languagertPath = SharedResourceDirPath;
    toolchain::sys::path::append(languagertPath,
                            language::getMajorArchitectureName(getTriple()));
    toolchain::sys::path::append(languagertPath, "languagert.obj");
    Arguments.push_back(context.Args.MakeArgString(languagertPath));
  }

  addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                         file_types::TY_Object);
  addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                         file_types::TY_TOOLCHAIN_BC);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_Object);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_TOOLCHAIN_BC);

  for (const Arg *arg :
       context.Args.filtered(options::OPT_F, options::OPT_Fsystem)) {
    if (arg->getOption().matches(options::OPT_Fsystem))
      Arguments.push_back("-iframework");
    else
      Arguments.push_back(context.Args.MakeArgString(arg->getSpelling()));
    Arguments.push_back(arg->getValue());
  }

  if (!context.OI.SDKPath.empty()) {
    Arguments.push_back("-I");
    Arguments.push_back(context.Args.MakeArgString(context.OI.SDKPath));
  }

  if (job.getKind() == LinkKind::Executable) {
    if (context.OI.SelectedSanitizers & SanitizerKind::Address)
      addLinkRuntimeLib(context.Args, Arguments,
                        sanitizerRuntimeLibName("asan"));

    if (context.OI.SelectedSanitizers & SanitizerKind::Undefined)
      addLinkRuntimeLib(context.Args, Arguments,
                        sanitizerRuntimeLibName("ubsan"));
  }

  if (context.Args.hasArg(options::OPT_profile_generate)) {
    Arguments.push_back(context.Args.MakeArgString("-Xlinker"));
    Arguments.push_back(context.Args.MakeArgString(
        Twine({"-include:", toolchain::getInstrProfRuntimeHookVarName()})));
    Arguments.push_back(context.Args.MakeArgString("-lclang_rt.profile"));

    // FIXME(rdar://131295678): Currently profiling requires the ability to
    // emit duplicate weak symbols. Assuming we're using lld, pass
    // -lld-allow-duplicate-weak to enable this behavior.
    if (requiresLLD) {
      Arguments.push_back("-Xlinker");
      Arguments.push_back("-lld-allow-duplicate-weak");
    }
  }

  context.Args.AddAllArgs(Arguments, options::OPT_Xlinker);
  context.Args.AddAllArgsExcept(Arguments, {options::OPT_linker_option_Group},
                                {options::OPT_l});
  ToolChain::addLinkedLibArgs(context.Args, Arguments);
  context.Args.AddAllArgValues(Arguments, options::OPT_Xclang_linker);

  // Run clang in verbose mode if "-v" is set
  if (context.Args.hasArg(options::OPT_v)) {
    Arguments.push_back("-v");
  }

  // This should be the last option, for convenience in checking output.
  Arguments.push_back("-o");
  Arguments.push_back(
      context.Args.MakeArgString(context.Output.getPrimaryOutputFilename()));

  InvocationInfo II{getClangLinkerDriver(context.Args), Arguments};
  II.allowsResponseFiles = true;

  return II;
}

ToolChain::InvocationInfo
toolchains::Windows::constructInvocation(const StaticLinkJobAction &job,
                                         const JobContext &context) const {
   assert(context.Output.getPrimaryOutputType() == file_types::TY_Image &&
         "Invalid linker output type.");

  ArgStringList Arguments;

  const char *Linker = "link";
  if (const Arg *A = context.Args.getLastArg(options::OPT_use_ld))
    Linker = context.Args.MakeArgString(A->getValue());

  Arguments.push_back("/lib");
  Arguments.push_back("-nologo");

  addPrimaryInputsOfType(Arguments, context.Inputs, context.Args,
                         file_types::TY_Object);
  addInputsOfType(Arguments, context.InputActions, file_types::TY_Object);

  StringRef OutputFile = context.Output.getPrimaryOutputFilename();
  Arguments.push_back(context.Args.MakeArgString(Twine("/OUT:") + OutputFile));

  InvocationInfo II{Linker, Arguments};
  II.allowsResponseFiles = true;

  return II;
}
