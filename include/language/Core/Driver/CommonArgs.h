//===--- CommonArgs.h - Args handling for multiple toolchains ---*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_COMMONARGS_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_COMMONARGS_H

#include "language/Core/Basic/CodeGenOptions.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/InputInfo.h"
#include "language/Core/Driver/Multilib.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/CodeGen.h"

namespace language::Core {
namespace driver {
namespace tools {

void addPathIfExists(const Driver &D, const Twine &Path,
                     ToolChain::path_list &Paths);

void AddLinkerInputs(const ToolChain &TC, const InputInfoList &Inputs,
                     const toolchain::opt::ArgList &Args,
                     toolchain::opt::ArgStringList &CmdArgs, const JobAction &JA);

const char *getLDMOption(const toolchain::Triple &T, const toolchain::opt::ArgList &Args);

void addLinkerCompressDebugSectionsOption(const ToolChain &TC,
                                          const toolchain::opt::ArgList &Args,
                                          toolchain::opt::ArgStringList &CmdArgs);

void claimNoWarnArgs(const toolchain::opt::ArgList &Args);

bool addSanitizerRuntimes(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs);

void linkSanitizerRuntimeDeps(const ToolChain &TC,
                              const toolchain::opt::ArgList &Args,
                              toolchain::opt::ArgStringList &CmdArgs);

bool addXRayRuntime(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                    toolchain::opt::ArgStringList &CmdArgs);

void linkXRayRuntimeDeps(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &CmdArgs);

void AddRunTimeLibs(const ToolChain &TC, const Driver &D,
                    toolchain::opt::ArgStringList &CmdArgs,
                    const toolchain::opt::ArgList &Args);

void AddStaticDeviceLibsLinking(Compilation &C, const Tool &T,
                                const JobAction &JA,
                                const InputInfoList &Inputs,
                                const toolchain::opt::ArgList &DriverArgs,
                                toolchain::opt::ArgStringList &CmdArgs,
                                StringRef Arch, StringRef Target,
                                bool isBitCodeSDL);
void AddStaticDeviceLibs(Compilation *C, const Tool *T, const JobAction *JA,
                         const InputInfoList *Inputs, const Driver &D,
                         const toolchain::opt::ArgList &DriverArgs,
                         toolchain::opt::ArgStringList &CmdArgs, StringRef Arch,
                         StringRef Target, bool isBitCodeSDL);

const char *SplitDebugName(const JobAction &JA, const toolchain::opt::ArgList &Args,
                           const InputInfo &Input, const InputInfo &Output);

void SplitDebugInfo(const ToolChain &TC, Compilation &C, const Tool &T,
                    const JobAction &JA, const toolchain::opt::ArgList &Args,
                    const InputInfo &Output, const char *OutFile);

void addLTOOptions(const ToolChain &ToolChain, const toolchain::opt::ArgList &Args,
                   toolchain::opt::ArgStringList &CmdArgs, const InputInfo &Output,
                   const InputInfoList &Inputs, bool IsThinLTO);

const char *RelocationModelName(toolchain::Reloc::Model Model);

std::tuple<toolchain::Reloc::Model, unsigned, bool>
ParsePICArgs(const ToolChain &ToolChain, const toolchain::opt::ArgList &Args);

bool getStaticPIE(const toolchain::opt::ArgList &Args, const ToolChain &TC);

unsigned ParseFunctionAlignment(const ToolChain &TC,
                                const toolchain::opt::ArgList &Args);

void addDebugInfoKind(toolchain::opt::ArgStringList &CmdArgs,
                      toolchain::codegenoptions::DebugInfoKind DebugInfoKind);

toolchain::codegenoptions::DebugInfoKind
debugLevelToInfoKind(const toolchain::opt::Arg &A);

// Extract the integer N from a string spelled "-dwarf-N", returning 0
// on mismatch. The StringRef input (rather than an Arg) allows
// for use by the "-Xassembler" option parser.
unsigned DwarfVersionNum(StringRef ArgValue);
// Find a DWARF format version option.
// This function is a complementary for DwarfVersionNum().
const toolchain::opt::Arg *getDwarfNArg(const toolchain::opt::ArgList &Args);
unsigned getDwarfVersion(const ToolChain &TC, const toolchain::opt::ArgList &Args);

void AddAssemblerKPIC(const ToolChain &ToolChain,
                      const toolchain::opt::ArgList &Args,
                      toolchain::opt::ArgStringList &CmdArgs);

void addArchSpecificRPath(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs);
void addOpenMPRuntimeLibraryPath(const ToolChain &TC,
                                 const toolchain::opt::ArgList &Args,
                                 toolchain::opt::ArgStringList &CmdArgs);
/// Returns true, if an OpenMP runtime has been added.
bool addOpenMPRuntime(const Compilation &C, toolchain::opt::ArgStringList &CmdArgs,
                      const ToolChain &TC, const toolchain::opt::ArgList &Args,
                      bool ForceStaticHostRuntime = false,
                      bool IsOffloadingHost = false, bool GompNeedsRT = false);

/// Adds offloading options for OpenMP host compilation to \p CmdArgs.
void addOpenMPHostOffloadingArgs(const Compilation &C, const JobAction &JA,
                                 const toolchain::opt::ArgList &Args,
                                 toolchain::opt::ArgStringList &CmdArgs);

void addHIPRuntimeLibArgs(const ToolChain &TC, Compilation &C,
                          const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs);

void addAsNeededOption(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                       toolchain::opt::ArgStringList &CmdArgs, bool as_needed);

toolchain::opt::Arg *getLastCSProfileGenerateArg(const toolchain::opt::ArgList &Args);
toolchain::opt::Arg *getLastProfileUseArg(const toolchain::opt::ArgList &Args);
toolchain::opt::Arg *getLastProfileSampleUseArg(const toolchain::opt::ArgList &Args);

bool isObjCAutoRefCount(const toolchain::opt::ArgList &Args);

toolchain::StringRef getLTOParallelism(const toolchain::opt::ArgList &Args,
                                  const Driver &D);

bool areOptimizationsEnabled(const toolchain::opt::ArgList &Args);

bool isUseSeparateSections(const toolchain::Triple &Triple);
// Parse -mtls-dialect=. Return true if the target supports both general-dynamic
// and TLSDESC, and TLSDESC is requested.
bool isTLSDESCEnabled(const ToolChain &TC, const toolchain::opt::ArgList &Args);

/// \p EnvVar is split by system delimiter for environment variables.
/// If \p ArgName is "-I", "-L", or an empty string, each entry from \p EnvVar
/// is prefixed by \p ArgName then added to \p Args. Otherwise, for each
/// entry of \p EnvVar, \p ArgName is added to \p Args first, then the entry
/// itself is added.
void addDirectoryList(const toolchain::opt::ArgList &Args,
                      toolchain::opt::ArgStringList &CmdArgs, const char *ArgName,
                      const char *EnvVar);

void AddTargetFeature(const toolchain::opt::ArgList &Args,
                      std::vector<StringRef> &Features,
                      toolchain::opt::OptSpecifier OnOpt,
                      toolchain::opt::OptSpecifier OffOpt, StringRef FeatureName);

std::string getCPUName(const Driver &D, const toolchain::opt::ArgList &Args,
                       const toolchain::Triple &T, bool FromAs = false);

void getTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                       const toolchain::opt::ArgList &Args,
                       toolchain::opt::ArgStringList &CmdArgs, bool ForAS,
                       bool IsAux = false);

/// Iterate \p Args and convert -mxxx to +xxx and -mno-xxx to -xxx and
/// append it to \p Features.
///
/// Note: Since \p Features may contain default values before calling
/// this function, or may be appended with entries to override arguments,
/// entries in \p Features are not unique.
void handleTargetFeaturesGroup(const Driver &D, const toolchain::Triple &Triple,
                               const toolchain::opt::ArgList &Args,
                               std::vector<StringRef> &Features,
                               toolchain::opt::OptSpecifier Group);

/// If there are multiple +xxx or -xxx features, keep the last one.
SmallVector<StringRef> unifyTargetFeatures(ArrayRef<StringRef> Features);

/// Handles the -save-stats option and returns the filename to save statistics
/// to.
SmallString<128> getStatsFileName(const toolchain::opt::ArgList &Args,
                                  const InputInfo &Output,
                                  const InputInfo &Input, const Driver &D);

/// \p Flag must be a flag accepted by the driver.
void addMultilibFlag(bool Enabled, const StringRef Flag,
                     Multilib::flags_list &Flags);

void addX86AlignBranchArgs(const Driver &D, const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs, bool IsLTO,
                           const StringRef PluginOptPrefix = "");

void checkAMDGPUCodeObjectVersion(const Driver &D,
                                  const toolchain::opt::ArgList &Args);

unsigned getAMDGPUCodeObjectVersion(const Driver &D,
                                    const toolchain::opt::ArgList &Args);

bool haveAMDGPUCodeObjectVersionArgument(const Driver &D,
                                         const toolchain::opt::ArgList &Args);

void addMachineOutlinerArgs(const Driver &D, const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs,
                            const toolchain::Triple &Triple, bool IsLTO,
                            const StringRef PluginOptPrefix = "");

void addOpenMPDeviceRTL(const Driver &D, const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        StringRef BitcodeSuffix, const toolchain::Triple &Triple,
                        const ToolChain &HostTC);

void addOpenCLBuiltinsLib(const Driver &D, const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args);

void addOutlineAtomicsArgs(const Driver &D, const ToolChain &TC,
                           const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs,
                           const toolchain::Triple &Triple);
void addOffloadCompressArgs(const toolchain::opt::ArgList &TCArgs,
                            toolchain::opt::ArgStringList &CmdArgs);
void addMCModel(const Driver &D, const toolchain::opt::ArgList &Args,
                const toolchain::Triple &Triple,
                const toolchain::Reloc::Model &RelocationModel,
                toolchain::opt::ArgStringList &CmdArgs);

/// Handle the -f{no}-color-diagnostics and -f{no}-diagnostics-colors options.
void handleColorDiagnosticsArgs(const Driver &D, const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs);

/// Add backslashes to escape spaces and other backslashes.
/// This is used for the space-separated argument list specified with
/// the -dwarf-debug-flags option.
void escapeSpacesAndBackslashes(const char *Arg,
                                toolchain::SmallVectorImpl<char> &Res);

/// Join the args in the given ArgList, escape spaces and backslashes and
/// return the joined string. This is used when saving the command line as a
/// result of using either the -frecord-command-line or -grecord-command-line
/// options. The lifetime of the returned c-string will match that of the Args
/// argument.
const char *renderEscapedCommandLine(const ToolChain &TC,
                                     const toolchain::opt::ArgList &Args);

/// Check if the command line should be recorded in the object file. This is
/// done if either -frecord-command-line or -grecord-command-line options have
/// been passed. This also does some error checking since -frecord-command-line
/// is currently only supported on ELF platforms. The last two boolean
/// arguments are out parameters and will be set depending on the command
/// line options that were passed.
bool shouldRecordCommandLine(const ToolChain &TC,
                             const toolchain::opt::ArgList &Args,
                             bool &FRecordCommandLine,
                             bool &GRecordCommandLine);

void renderCommonIntegerOverflowOptions(const toolchain::opt::ArgList &Args,
                                        toolchain::opt::ArgStringList &CmdArgs);

bool shouldEnableVectorizerAtOLevel(const toolchain::opt::ArgList &Args,
                                    bool isSlpVec);

/// Enable -floop-interchange based on the optimization level selected.
void handleInterchangeLoopsArgs(const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs);

/// Enable -fvectorize based on the optimization level selected.
void handleVectorizeLoopsArgs(const toolchain::opt::ArgList &Args,
                              toolchain::opt::ArgStringList &CmdArgs);

/// Enable -fslp-vectorize based on the optimization level selected.
void handleVectorizeSLPArgs(const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs);

// Parse -mprefer-vector-width=. Return the Value string if well-formed.
// Otherwise, return an empty string and issue a diagnosic message if needed.
StringRef parseMPreferVectorWidthOption(language::Core::DiagnosticsEngine &Diags,
                                        const toolchain::opt::ArgList &Args);

// Parse -mrecip. Return the Value string if well-formed.
// Otherwise, return an empty string and issue a diagnosic message if needed.
StringRef parseMRecipOption(language::Core::DiagnosticsEngine &Diags,
                            const toolchain::opt::ArgList &Args);

// Convert ComplexRangeKind to a string that can be passed as a frontend option.
std::string complexRangeKindToStr(LangOptions::ComplexRangeKind Range);

// Render a frontend option corresponding to ComplexRangeKind.
std::string renderComplexRangeOption(LangOptions::ComplexRangeKind Range);

} // end namespace tools
} // end namespace driver
} // end namespace language::Core

language::Core::CodeGenOptions::FramePointerKind
getFramePointerKind(const toolchain::opt::ArgList &Args, const toolchain::Triple &Triple);

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_COMMONARGS_H
