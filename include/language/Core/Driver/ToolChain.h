//===- ToolChain.h - Collections of tools for one platform ------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_DRIVER_TOOLCHAIN_H
#define LANGUAGE_CORE_DRIVER_TOOLCHAIN_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/Sanitizers.h"
#include "language/Core/Driver/Action.h"
#include "language/Core/Driver/Multilib.h"
#include "language/Core/Driver/Types.h"
#include "toolchain/ADT/APFloat.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/FloatingPointMode.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Frontend/Debug/Options.h"
#include "toolchain/MC/MCTargetOptions.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/Target/TargetOptions.h"
#include "toolchain/TargetParser/Triple.h"
#include <cassert>
#include <climits>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace toolchain {
namespace opt {

class Arg;
class ArgList;
class DerivedArgList;

} // namespace opt
namespace vfs {

class FileSystem;

} // namespace vfs
} // namespace toolchain

namespace language::Core {

class ObjCRuntime;

namespace driver {

class Driver;
class InputInfo;
class SanitizerArgs;
class Tool;
class XRayArgs;

/// Helper structure used to pass information extracted from clang executable
/// name such as `i686-linux-android-g++`.
struct ParsedClangName {
  /// Target part of the executable name, as `i686-linux-android`.
  std::string TargetPrefix;

  /// Driver mode part of the executable name, as `g++`.
  std::string ModeSuffix;

  /// Corresponding driver mode argument, as '--driver-mode=g++'
  const char *DriverMode = nullptr;

  /// True if TargetPrefix is recognized as a registered target name.
  bool TargetIsValid = false;

  ParsedClangName() = default;
  ParsedClangName(std::string Suffix, const char *Mode)
      : ModeSuffix(Suffix), DriverMode(Mode) {}
  ParsedClangName(std::string Target, std::string Suffix, const char *Mode,
                  bool IsRegistered)
      : TargetPrefix(Target), ModeSuffix(Suffix), DriverMode(Mode),
        TargetIsValid(IsRegistered) {}

  bool isEmpty() const {
    return TargetPrefix.empty() && ModeSuffix.empty() && DriverMode == nullptr;
  }
};

/// ToolChain - Access to tools for a single platform.
class ToolChain {
public:
  using path_list = SmallVector<std::string, 16>;

  enum CXXStdlibType {
    CST_Libcxx,
    CST_Libstdcxx
  };

  enum RuntimeLibType {
    RLT_CompilerRT,
    RLT_Libgcc
  };

  enum UnwindLibType {
    UNW_None,
    UNW_CompilerRT,
    UNW_Libgcc
  };

  enum class UnwindTableLevel {
    None,
    Synchronous,
    Asynchronous,
  };

  enum RTTIMode {
    RM_Enabled,
    RM_Disabled,
  };

  enum ExceptionsMode {
    EM_Enabled,
    EM_Disabled,
  };

  struct BitCodeLibraryInfo {
    std::string Path;
    bool ShouldInternalize;
    BitCodeLibraryInfo(StringRef Path, bool ShouldInternalize = true)
        : Path(Path), ShouldInternalize(ShouldInternalize) {}
  };

  enum FileType { FT_Object, FT_Static, FT_Shared };

private:
  friend class RegisterEffectiveTriple;

  const Driver &D;
  toolchain::Triple Triple;
  const toolchain::opt::ArgList &Args;

  // We need to initialize CachedRTTIArg before CachedRTTIMode
  const toolchain::opt::Arg *const CachedRTTIArg;

  const RTTIMode CachedRTTIMode;

  const ExceptionsMode CachedExceptionsMode;

  /// The list of toolchain specific path prefixes to search for libraries.
  path_list LibraryPaths;

  /// The list of toolchain specific path prefixes to search for files.
  path_list FilePaths;

  /// The list of toolchain specific path prefixes to search for programs.
  path_list ProgramPaths;

  mutable std::unique_ptr<Tool> Clang;
  mutable std::unique_ptr<Tool> Flang;
  mutable std::unique_ptr<Tool> Assemble;
  mutable std::unique_ptr<Tool> Link;
  mutable std::unique_ptr<Tool> StaticLibTool;
  mutable std::unique_ptr<Tool> IfsMerge;
  mutable std::unique_ptr<Tool> OffloadBundler;
  mutable std::unique_ptr<Tool> OffloadPackager;
  mutable std::unique_ptr<Tool> LinkerWrapper;

  Tool *getClang() const;
  Tool *getFlang() const;
  Tool *getAssemble() const;
  Tool *getLink() const;
  Tool *getStaticLibTool() const;
  Tool *getIfsMerge() const;
  Tool *getClangAs() const;
  Tool *getOffloadBundler() const;
  Tool *getOffloadPackager() const;
  Tool *getLinkerWrapper() const;

  mutable bool SanitizerArgsChecked = false;

  /// The effective clang triple for the current Job.
  mutable toolchain::Triple EffectiveTriple;

  /// Set the toolchain's effective clang triple.
  void setEffectiveTriple(toolchain::Triple ET) const {
    EffectiveTriple = std::move(ET);
  }

  std::optional<std::string>
  getFallbackAndroidTargetPath(StringRef BaseDir) const;

  mutable std::optional<CXXStdlibType> cxxStdlibType;
  mutable std::optional<RuntimeLibType> runtimeLibType;
  mutable std::optional<UnwindLibType> unwindLibType;

protected:
  MultilibSet Multilibs;
  toolchain::SmallVector<Multilib> SelectedMultilibs;

  ToolChain(const Driver &D, const toolchain::Triple &T,
            const toolchain::opt::ArgList &Args);

  void setTripleEnvironment(toolchain::Triple::EnvironmentType Env);

  virtual Tool *buildAssembler() const;
  virtual Tool *buildLinker() const;
  virtual Tool *buildStaticLibTool() const;
  virtual Tool *getTool(Action::ActionClass AC) const;

  virtual std::string buildCompilerRTBasename(const toolchain::opt::ArgList &Args,
                                              StringRef Component,
                                              FileType Type, bool AddArch,
                                              bool IsFortran = false) const;

  /// Find the target-specific subdirectory for the current target triple under
  /// \p BaseDir, doing fallback triple searches as necessary.
  /// \return The subdirectory path if it exists.
  std::optional<std::string> getTargetSubDirPath(StringRef BaseDir) const;

  /// \name Utilities for implementing subclasses.
  ///@{
  static void addSystemFrameworkInclude(const toolchain::opt::ArgList &DriverArgs,
                                        toolchain::opt::ArgStringList &CC1Args,
                                        const Twine &Path);
  static void addSystemInclude(const toolchain::opt::ArgList &DriverArgs,
                               toolchain::opt::ArgStringList &CC1Args,
                               const Twine &Path);
  static void addExternCSystemInclude(const toolchain::opt::ArgList &DriverArgs,
                                      toolchain::opt::ArgStringList &CC1Args,
                                      const Twine &Path);
  static void
      addExternCSystemIncludeIfExists(const toolchain::opt::ArgList &DriverArgs,
                                      toolchain::opt::ArgStringList &CC1Args,
                                      const Twine &Path);
  static void addSystemFrameworkIncludes(const toolchain::opt::ArgList &DriverArgs,
                                         toolchain::opt::ArgStringList &CC1Args,
                                         ArrayRef<StringRef> Paths);
  static void addSystemIncludes(const toolchain::opt::ArgList &DriverArgs,
                                toolchain::opt::ArgStringList &CC1Args,
                                ArrayRef<StringRef> Paths);

  static std::string concat(StringRef Path, const Twine &A, const Twine &B = "",
                            const Twine &C = "", const Twine &D = "");
  ///@}

public:
  virtual ~ToolChain();

  // Accessors

  const Driver &getDriver() const { return D; }
  toolchain::vfs::FileSystem &getVFS() const;
  const toolchain::Triple &getTriple() const { return Triple; }

  /// Get the toolchain's aux triple, if it has one.
  ///
  /// Exactly what the aux triple represents depends on the toolchain, but for
  /// example when compiling CUDA code for the GPU, the triple might be NVPTX,
  /// while the aux triple is the host (CPU) toolchain, e.g. x86-linux-gnu.
  virtual const toolchain::Triple *getAuxTriple() const { return nullptr; }

  /// Some toolchains need to modify the file name, for example to replace the
  /// extension for object files with .cubin for OpenMP offloading to Nvidia
  /// GPUs.
  virtual std::string getInputFilename(const InputInfo &Input) const;

  toolchain::Triple::ArchType getArch() const { return Triple.getArch(); }
  StringRef getArchName() const { return Triple.getArchName(); }
  StringRef getPlatform() const { return Triple.getVendorName(); }
  StringRef getOS() const { return Triple.getOSName(); }

  /// Provide the default architecture name (as expected by -arch) for
  /// this toolchain.
  StringRef getDefaultUniversalArchName() const;

  std::string getTripleString() const {
    return Triple.getTriple();
  }

  /// Get the toolchain's effective clang triple.
  const toolchain::Triple &getEffectiveTriple() const {
    assert(!EffectiveTriple.getTriple().empty() && "No effective triple");
    return EffectiveTriple;
  }

  bool hasEffectiveTriple() const {
    return !EffectiveTriple.getTriple().empty();
  }

  path_list &getLibraryPaths() { return LibraryPaths; }
  const path_list &getLibraryPaths() const { return LibraryPaths; }

  path_list &getFilePaths() { return FilePaths; }
  const path_list &getFilePaths() const { return FilePaths; }

  path_list &getProgramPaths() { return ProgramPaths; }
  const path_list &getProgramPaths() const { return ProgramPaths; }

  const MultilibSet &getMultilibs() const { return Multilibs; }

  const toolchain::SmallVector<Multilib> &getSelectedMultilibs() const {
    return SelectedMultilibs;
  }

  /// Get flags suitable for multilib selection, based on the provided clang
  /// command line arguments. The command line arguments aren't suitable to be
  /// used directly for multilib selection because they are not normalized and
  /// normalization is a complex process. The result of this function is similar
  /// to clang command line arguments except that the list of arguments is
  /// incomplete. Only certain command line arguments are processed. If more
  /// command line arguments are needed for multilib selection then this
  /// function should be extended.
  /// To allow users to find out what flags are returned, clang accepts a
  /// -print-multi-flags-experimental argument.
  Multilib::flags_list getMultilibFlags(const toolchain::opt::ArgList &) const;

  SanitizerArgs getSanitizerArgs(const toolchain::opt::ArgList &JobArgs) const;

  const XRayArgs getXRayArgs(const toolchain::opt::ArgList &) const;

  // Returns the Arg * that explicitly turned on/off rtti, or nullptr.
  const toolchain::opt::Arg *getRTTIArg() const { return CachedRTTIArg; }

  // Returns the RTTIMode for the toolchain with the current arguments.
  RTTIMode getRTTIMode() const { return CachedRTTIMode; }

  // Returns the ExceptionsMode for the toolchain with the current arguments.
  ExceptionsMode getExceptionsMode() const { return CachedExceptionsMode; }

  /// Return any implicit target and/or mode flag for an invocation of
  /// the compiler driver as `ProgName`.
  ///
  /// For example, when called with i686-linux-android-g++, the first element
  /// of the return value will be set to `"i686-linux-android"` and the second
  /// will be set to "--driver-mode=g++"`.
  /// It is OK if the target name is not registered. In this case the return
  /// value contains false in the field TargetIsValid.
  ///
  /// \pre `toolchain::InitializeAllTargets()` has been called.
  /// \param ProgName The name the Clang driver was invoked with (from,
  /// e.g., argv[0]).
  /// \return A structure of type ParsedClangName that contains the executable
  /// name parts.
  static ParsedClangName getTargetAndModeFromProgramName(StringRef ProgName);

  // Tool access.

  /// TranslateArgs - Create a new derived argument list for any argument
  /// translations this ToolChain may wish to perform, or 0 if no tool chain
  /// specific translations are needed. If \p DeviceOffloadKind is specified
  /// the translation specific for that offload kind is performed.
  ///
  /// \param BoundArch - The bound architecture name, or 0.
  /// \param DeviceOffloadKind - The device offload kind used for the
  /// translation.
  virtual toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const {
    return nullptr;
  }

  /// TranslateOpenMPTargetArgs - Create a new derived argument list for
  /// that contains the OpenMP target specific flags passed via
  /// -Xopenmp-target -opt=val OR -Xopenmp-target=<triple> -opt=val
  virtual toolchain::opt::DerivedArgList *TranslateOpenMPTargetArgs(
      const toolchain::opt::DerivedArgList &Args, bool SameTripleAsHost,
      SmallVectorImpl<toolchain::opt::Arg *> &AllocatedArgs) const;

  /// Append the argument following \p A to \p DAL assuming \p A is an Xarch
  /// argument. If \p AllocatedArgs is null pointer, synthesized arguments are
  /// added to \p DAL, otherwise they are appended to \p AllocatedArgs.
  virtual void TranslateXarchArgs(
      const toolchain::opt::DerivedArgList &Args, toolchain::opt::Arg *&A,
      toolchain::opt::DerivedArgList *DAL,
      SmallVectorImpl<toolchain::opt::Arg *> *AllocatedArgs = nullptr) const;

  /// Translate -Xarch_ arguments. If there are no such arguments, return
  /// a null pointer, otherwise return a DerivedArgList containing the
  /// translated arguments.
  virtual toolchain::opt::DerivedArgList *
  TranslateXarchArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                     Action::OffloadKind DeviceOffloadKind,
                     SmallVectorImpl<toolchain::opt::Arg *> *AllocatedArgs) const;

  /// Choose a tool to use to handle the action \p JA.
  ///
  /// This can be overridden when a particular ToolChain needs to use
  /// a compiler other than Clang.
  virtual Tool *SelectTool(const JobAction &JA) const;

  // Helper methods

  std::string GetFilePath(const char *Name) const;
  std::string GetProgramPath(const char *Name) const;

  /// Returns the linker path, respecting the -fuse-ld= argument to determine
  /// the linker suffix or name.
  /// If LinkerIsLLD is non-nullptr, it is set to true if the returned linker
  /// is LLD. If it's set, it can be assumed that the linker is LLD built
  /// at the same revision as clang, and clang can make assumptions about
  /// LLD's supported flags, error output, etc.
  std::string GetLinkerPath(bool *LinkerIsLLD = nullptr) const;

  /// Returns the linker path for emitting a static library.
  std::string GetStaticLibToolPath() const;

  /// Dispatch to the specific toolchain for verbose printing.
  ///
  /// This is used when handling the verbose option to print detailed,
  /// toolchain-specific information useful for understanding the behavior of
  /// the driver on a specific platform.
  virtual void printVerboseInfo(raw_ostream &OS) const {}

  // Platform defaults information

  /// Returns true if the toolchain is targeting a non-native
  /// architecture.
  virtual bool isCrossCompiling() const;

  /// HasNativeLTOLinker - Check whether the linker and related tools have
  /// native LLVM support.
  virtual bool HasNativeLLVMSupport() const;

  /// LookupTypeForExtension - Return the default language type to use for the
  /// given extension.
  virtual types::ID LookupTypeForExtension(StringRef Ext) const;

  /// IsBlocksDefault - Does this tool chain enable -fblocks by default.
  virtual bool IsBlocksDefault() const { return false; }

  /// IsIntegratedAssemblerDefault - Does this tool chain enable -integrated-as
  /// by default.
  virtual bool IsIntegratedAssemblerDefault() const { return true; }

  /// IsIntegratedBackendDefault - Does this tool chain enable
  /// -fintegrated-objemitter by default.
  virtual bool IsIntegratedBackendDefault() const { return true; }

  /// IsIntegratedBackendSupported - Does this tool chain support
  /// -fintegrated-objemitter.
  virtual bool IsIntegratedBackendSupported() const { return true; }

  /// IsNonIntegratedBackendSupported - Does this tool chain support
  /// -fno-integrated-objemitter.
  virtual bool IsNonIntegratedBackendSupported() const { return false; }

  /// Check if the toolchain should use the integrated assembler.
  virtual bool useIntegratedAs() const;

  /// Check if the toolchain should use the integrated backend.
  virtual bool useIntegratedBackend() const;

  /// Check if the toolchain should use AsmParser to parse inlineAsm when
  /// integrated assembler is not default.
  virtual bool parseInlineAsmUsingAsmParser() const { return false; }

  /// IsMathErrnoDefault - Does this tool chain use -fmath-errno by default.
  virtual bool IsMathErrnoDefault() const { return true; }

  /// IsEncodeExtendedBlockSignatureDefault - Does this tool chain enable
  /// -fencode-extended-block-signature by default.
  virtual bool IsEncodeExtendedBlockSignatureDefault() const { return false; }

  /// IsObjCNonFragileABIDefault - Does this tool chain set
  /// -fobjc-nonfragile-abi by default.
  virtual bool IsObjCNonFragileABIDefault() const { return false; }

  /// UseObjCMixedDispatchDefault - When using non-legacy dispatch, should the
  /// mixed dispatch method be used?
  virtual bool UseObjCMixedDispatch() const { return false; }

  /// Check whether to enable x86 relax relocations by default.
  virtual bool useRelaxRelocations() const;

  /// Check whether use IEEE binary128 as long double format by default.
  bool defaultToIEEELongDouble() const;

  /// GetDefaultStackProtectorLevel - Get the default stack protector level for
  /// this tool chain.
  virtual LangOptions::StackProtectorMode
  GetDefaultStackProtectorLevel(bool KernelOrKext) const {
    return LangOptions::SSPOff;
  }

  /// Get the default trivial automatic variable initialization.
  virtual LangOptions::TrivialAutoVarInitKind
  GetDefaultTrivialAutoVarInit() const {
    return LangOptions::TrivialAutoVarInitKind::Uninitialized;
  }

  /// GetDefaultLinker - Get the default linker to use.
  virtual const char *getDefaultLinker() const { return "ld"; }

  /// GetDefaultRuntimeLibType - Get the default runtime library variant to use.
  virtual RuntimeLibType GetDefaultRuntimeLibType() const {
    return ToolChain::RLT_Libgcc;
  }

  virtual CXXStdlibType GetDefaultCXXStdlibType() const {
    return ToolChain::CST_Libstdcxx;
  }

  virtual UnwindLibType GetDefaultUnwindLibType() const {
    return ToolChain::UNW_None;
  }

  virtual std::string getCompilerRTPath() const;

  virtual std::string getCompilerRT(const toolchain::opt::ArgList &Args,
                                    StringRef Component,
                                    FileType Type = ToolChain::FT_Static,
                                    bool IsFortran = false) const;

  /// Adds Fortran runtime libraries to \p CmdArgs.
  virtual void addFortranRuntimeLibs(const toolchain::opt::ArgList &Args,
                                     toolchain::opt::ArgStringList &CmdArgs) const;

  /// Adds the path for the Fortran runtime libraries to \p CmdArgs.
  virtual void
  addFortranRuntimeLibraryPath(const toolchain::opt::ArgList &Args,
                               toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add the path for libflang_rt.runtime.a
  void addFlangRTLibPath(const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &CmdArgs) const;

  const char *getCompilerRTArgString(const toolchain::opt::ArgList &Args,
                                     StringRef Component,
                                     FileType Type = ToolChain::FT_Static,
                                     bool IsFortran = false) const;

  std::string getCompilerRTBasename(const toolchain::opt::ArgList &Args,
                                    StringRef Component,
                                    FileType Type = ToolChain::FT_Static) const;

  // Returns Triple without the OSs version.
  toolchain::Triple getTripleWithoutOSVersion() const;

  // Returns the target specific runtime path if it exists.
  std::optional<std::string> getRuntimePath() const;

  // Returns target specific standard library path if it exists.
  std::optional<std::string> getStdlibPath() const;

  // Returns target specific standard library include path if it exists.
  std::optional<std::string> getStdlibIncludePath() const;

  // Returns <ResourceDir>/lib/<OSName>/<arch> or <ResourceDir>/lib/<triple>.
  // This is used by runtimes (such as OpenMP) to find arch-specific libraries.
  virtual path_list getArchSpecificLibPaths() const;

  // Returns <OSname> part of above.
  virtual StringRef getOSLibName() const;

  /// needsProfileRT - returns true if instrumentation profile is on.
  static bool needsProfileRT(const toolchain::opt::ArgList &Args);

  /// Returns true if gcov instrumentation (-fprofile-arcs or --coverage) is on.
  static bool needsGCovInstrumentation(const toolchain::opt::ArgList &Args);

  /// How detailed should the unwind tables be by default.
  virtual UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const;

  /// Test whether this toolchain supports outline atomics by default.
  virtual bool
  IsAArch64OutlineAtomicsDefault(const toolchain::opt::ArgList &Args) const {
    return false;
  }

  /// Test whether this toolchain defaults to PIC.
  virtual bool isPICDefault() const = 0;

  /// Test whether this toolchain defaults to PIE.
  virtual bool isPIEDefault(const toolchain::opt::ArgList &Args) const = 0;

  /// Tests whether this toolchain forces its default for PIC, PIE or
  /// non-PIC.  If this returns true, any PIC related flags should be ignored
  /// and instead the results of \c isPICDefault() and \c isPIEDefault(const
  /// toolchain::opt::ArgList &Args) are used exclusively.
  virtual bool isPICDefaultForced() const = 0;

  /// SupportsProfiling - Does this tool chain support -pg.
  virtual bool SupportsProfiling() const { return true; }

  /// Complain if this tool chain doesn't support Objective-C ARC.
  virtual void CheckObjCARC() const {}

  /// Get the default debug info format. Typically, this is DWARF.
  virtual toolchain::codegenoptions::DebugInfoFormat getDefaultDebugFormat() const {
    return toolchain::codegenoptions::DIF_DWARF;
  }

  /// UseDwarfDebugFlags - Embed the compile options to clang into the Dwarf
  /// compile unit information.
  virtual bool UseDwarfDebugFlags() const { return false; }

  /// Add an additional -fdebug-prefix-map entry.
  virtual std::string GetGlobalDebugPathRemapping() const { return {}; }

  // Return the DWARF version to emit, in the absence of arguments
  // to the contrary.
  virtual unsigned GetDefaultDwarfVersion() const { return 5; }

  // Some toolchains may have different restrictions on the DWARF version and
  // may need to adjust it. E.g. NVPTX may need to enforce DWARF2 even when host
  // compilation uses DWARF5.
  virtual unsigned getMaxDwarfVersion() const { return UINT_MAX; }

  // True if the driver should assume "-fstandalone-debug"
  // in the absence of an option specifying otherwise,
  // provided that debugging was requested in the first place.
  // i.e. a value of 'true' does not imply that debugging is wanted.
  virtual bool GetDefaultStandaloneDebug() const { return false; }

  // Return the default debugger "tuning."
  virtual toolchain::DebuggerKind getDefaultDebuggerTuning() const {
    return toolchain::DebuggerKind::GDB;
  }

  /// Does this toolchain supports given debug info option or not.
  virtual bool supportsDebugInfoOption(const toolchain::opt::Arg *) const {
    return true;
  }

  /// Adjust debug information kind considering all passed options.
  virtual void
  adjustDebugInfoKind(toolchain::codegenoptions::DebugInfoKind &DebugInfoKind,
                      const toolchain::opt::ArgList &Args) const {}

  /// GetExceptionModel - Return the tool chain exception model.
  virtual toolchain::ExceptionHandling
  GetExceptionModel(const toolchain::opt::ArgList &Args) const;

  /// SupportsEmbeddedBitcode - Does this tool chain support embedded bitcode.
  virtual bool SupportsEmbeddedBitcode() const { return false; }

  /// getThreadModel() - Which thread model does this target use?
  virtual std::string getThreadModel() const { return "posix"; }

  /// isThreadModelSupported() - Does this target support a thread model?
  virtual bool isThreadModelSupported(const StringRef Model) const;

  /// isBareMetal - Is this a bare metal target.
  virtual bool isBareMetal() const { return false; }

  virtual std::string getMultiarchTriple(const Driver &D,
                                         const toolchain::Triple &TargetTriple,
                                         StringRef SysRoot) const {
    return TargetTriple.str();
  }

  /// ComputeLLVMTriple - Return the LLVM target triple to use, after taking
  /// command line arguments into account.
  virtual std::string
  ComputeLLVMTriple(const toolchain::opt::ArgList &Args,
                    types::ID InputType = types::TY_INVALID) const;

  /// ComputeEffectiveClangTriple - Return the Clang triple to use for this
  /// target, which may take into account the command line arguments. For
  /// example, on Darwin the -mmacos-version-min= command line argument (which
  /// sets the deployment target) determines the version in the triple passed to
  /// Clang.
  virtual std::string ComputeEffectiveClangTriple(
      const toolchain::opt::ArgList &Args,
      types::ID InputType = types::TY_INVALID) const;

  /// getDefaultObjCRuntime - Return the default Objective-C runtime
  /// for this platform.
  ///
  /// FIXME: this really belongs on some sort of DeploymentTarget abstraction
  virtual ObjCRuntime getDefaultObjCRuntime(bool isNonFragile) const;

  /// hasBlocksRuntime - Given that the user is compiling with
  /// -fblocks, does this tool chain guarantee the existence of a
  /// blocks runtime?
  ///
  /// FIXME: this really belongs on some sort of DeploymentTarget abstraction
  virtual bool hasBlocksRuntime() const { return true; }

  /// Return the sysroot, possibly searching for a default sysroot using
  /// target-specific logic.
  virtual std::string computeSysRoot() const;

  /// Add the clang cc1 arguments for system include paths.
  ///
  /// This routine is responsible for adding the necessary cc1 arguments to
  /// include headers from standard system header directories.
  virtual void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const;

  /// Add options that need to be passed to cc1 for this target.
  virtual void addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                                     toolchain::opt::ArgStringList &CC1Args,
                                     Action::OffloadKind DeviceOffloadKind) const;

  /// Add options that need to be passed to cc1as for this target.
  virtual void
  addClangCC1ASTargetOptions(const toolchain::opt::ArgList &Args,
                             toolchain::opt::ArgStringList &CC1ASArgs) const;

  /// Add warning options that need to be passed to cc1 for this target.
  virtual void addClangWarningOptions(toolchain::opt::ArgStringList &CC1Args) const;

  // Get the list of extra macro defines requested by the multilib
  // configuration.
  virtual SmallVector<std::string>
  getMultilibMacroDefinesStr(toolchain::opt::ArgList &Args) const {
    return {};
  };

  // GetRuntimeLibType - Determine the runtime library type to use with the
  // given compilation arguments.
  virtual RuntimeLibType
  GetRuntimeLibType(const toolchain::opt::ArgList &Args) const;

  // GetCXXStdlibType - Determine the C++ standard library type to use with the
  // given compilation arguments.
  virtual CXXStdlibType GetCXXStdlibType(const toolchain::opt::ArgList &Args) const;

  // GetUnwindLibType - Determine the unwind library type to use with the
  // given compilation arguments.
  virtual UnwindLibType GetUnwindLibType(const toolchain::opt::ArgList &Args) const;

  // Detect the highest available version of libc++ in include path.
  virtual std::string detectLibcxxVersion(StringRef IncludePath) const;

  /// AddClangCXXStdlibIncludeArgs - Add the clang -cc1 level arguments to set
  /// the include paths to use for the given C++ standard library type.
  virtual void
  AddClangCXXStdlibIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                               toolchain::opt::ArgStringList &CC1Args) const;

  /// AddClangCXXStdlibIsystemArgs - Add the clang -cc1 level arguments to set
  /// the specified include paths for the C++ standard library.
  void AddClangCXXStdlibIsystemArgs(const toolchain::opt::ArgList &DriverArgs,
                                    toolchain::opt::ArgStringList &CC1Args) const;

  /// Returns if the C++ standard library should be linked in.
  /// Note that e.g. -lm should still be linked even if this returns false.
  bool ShouldLinkCXXStdlib(const toolchain::opt::ArgList &Args) const;

  /// AddCXXStdlibLibArgs - Add the system specific linker arguments to use
  /// for the given C++ standard library type.
  virtual void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                                   toolchain::opt::ArgStringList &CmdArgs) const;

  /// AddFilePathLibArgs - Add each thing in getFilePaths() as a "-L" option.
  void AddFilePathLibArgs(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs) const;

  /// AddCCKextLibArgs - Add the system specific linker arguments to use
  /// for kernel extensions (Darwin-specific).
  virtual void AddCCKextLibArgs(const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs) const;

  /// If a runtime library exists that sets global flags for unsafe floating
  /// point math, return true.
  ///
  /// This checks for presence of the -Ofast, -ffast-math or -funsafe-math flags.
  virtual bool isFastMathRuntimeAvailable(
    const toolchain::opt::ArgList &Args, std::string &Path) const;

  /// AddFastMathRuntimeIfAvailable - If a runtime library exists that sets
  /// global flags for unsafe floating point math, add it and return true.
  ///
  /// This checks for presence of the -Ofast, -ffast-math or -funsafe-math flags.
  bool addFastMathRuntimeIfAvailable(
    const toolchain::opt::ArgList &Args, toolchain::opt::ArgStringList &CmdArgs) const;

  /// getSystemGPUArchs - Use a tool to detect the user's availible GPUs.
  virtual Expected<SmallVector<std::string>>
  getSystemGPUArchs(const toolchain::opt::ArgList &Args) const;

  /// addProfileRTLibs - When -fprofile-instr-profile is specified, try to pass
  /// a suitable profile runtime library to the linker.
  virtual void addProfileRTLibs(const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add arguments to use system-specific CUDA includes.
  virtual void AddCudaIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                                  toolchain::opt::ArgStringList &CC1Args) const;

  /// Add arguments to use system-specific HIP includes.
  virtual void AddHIPIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                                 toolchain::opt::ArgStringList &CC1Args) const;

  /// Add arguments to use system-specific SYCL includes.
  virtual void addSYCLIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                                  toolchain::opt::ArgStringList &CC1Args) const;

  /// Add arguments to use MCU GCC toolchain includes.
  virtual void AddIAMCUIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                                   toolchain::opt::ArgStringList &CC1Args) const;

  /// On Windows, returns the MSVC compatibility version.
  virtual VersionTuple computeMSVCVersion(const Driver *D,
                                          const toolchain::opt::ArgList &Args) const;

  /// Get paths for device libraries.
  virtual toolchain::SmallVector<BitCodeLibraryInfo, 12>
  getDeviceLibs(const toolchain::opt::ArgList &Args,
                const Action::OffloadKind DeviceOffloadingKind) const;

  /// Add the system specific linker arguments to use
  /// for the given HIP runtime library type.
  virtual void AddHIPRuntimeLibArgs(const toolchain::opt::ArgList &Args,
                                    toolchain::opt::ArgStringList &CmdArgs) const {}

  /// Return sanitizers which are available in this toolchain.
  virtual SanitizerMask getSupportedSanitizers() const;

  /// Return sanitizers which are enabled by default.
  virtual SanitizerMask getDefaultSanitizers() const {
    return SanitizerMask();
  }

  /// Returns true when it's possible to split LTO unit to use whole
  /// program devirtualization and CFI santiizers.
  virtual bool canSplitThinLTOUnit() const { return true; }

  /// Returns the output denormal handling type in the default floating point
  /// environment for the given \p FPType if given. Otherwise, the default
  /// assumed mode for any floating point type.
  virtual toolchain::DenormalMode getDefaultDenormalModeForType(
      const toolchain::opt::ArgList &DriverArgs, const JobAction &JA,
      const toolchain::fltSemantics *FPType = nullptr) const {
    return toolchain::DenormalMode::getIEEE();
  }

  // We want to expand the shortened versions of the triples passed in to
  // the values used for the bitcode libraries.
  static toolchain::Triple getOpenMPTriple(StringRef TripleStr) {
    toolchain::Triple TT(TripleStr);
    if (TT.getVendor() == toolchain::Triple::UnknownVendor ||
        TT.getOS() == toolchain::Triple::UnknownOS) {
      if (TT.getArch() == toolchain::Triple::nvptx)
        return toolchain::Triple("nvptx-nvidia-cuda");
      if (TT.getArch() == toolchain::Triple::nvptx64)
        return toolchain::Triple("nvptx64-nvidia-cuda");
      if (TT.isAMDGCN())
        return toolchain::Triple("amdgcn-amd-amdhsa");
    }
    return TT;
  }
};

/// Set a ToolChain's effective triple. Reset it when the registration object
/// is destroyed.
class RegisterEffectiveTriple {
  const ToolChain &TC;

public:
  RegisterEffectiveTriple(const ToolChain &TC, toolchain::Triple T) : TC(TC) {
    TC.setEffectiveTriple(std::move(T));
  }

  ~RegisterEffectiveTriple() { TC.setEffectiveTriple(toolchain::Triple()); }
};

} // namespace driver

} // namespace language::Core

#endif // LANGUAGE_CORE_DRIVER_TOOLCHAIN_H
