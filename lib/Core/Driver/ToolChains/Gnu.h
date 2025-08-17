//===--- Gnu.h - Gnu Tool and ToolChain Implementations ---------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_GNU_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_GNU_H

#include "language/Core/Driver/CudaInstallationDetector.h"
#include "language/Core/Driver/LazyDetector.h"
#include "language/Core/Driver/RocmInstallationDetector.h"
#include "language/Core/Driver/SyclInstallationDetector.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"
#include <set>

namespace language::Core {
namespace driver {

struct DetectedMultilibs {
  /// The set of multilibs that the detected installation supports.
  MultilibSet Multilibs;

  /// The multilibs appropriate for the given flags.
  toolchain::SmallVector<Multilib> SelectedMultilibs;

  /// On Biarch systems, this corresponds to the default multilib when
  /// targeting the non-default multilib. Otherwise, it is empty.
  std::optional<Multilib> BiarchSibling;
};

bool findMIPSMultilibs(const Driver &D, const toolchain::Triple &TargetTriple,
                       StringRef Path, const toolchain::opt::ArgList &Args,
                       DetectedMultilibs &Result);

namespace tools {

/// Directly call GNU Binutils' assembler and linker.
namespace gnutools {
class LLVM_LIBRARY_VISIBILITY Assembler : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("GNU::Assembler", "assembler", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("GNU::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY StaticLibTool : public Tool {
public:
  StaticLibTool(const ToolChain &TC)
      : Tool("GNU::StaticLibTool", "static-lib-linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace gnutools

/// gcc - Generic GCC tool implementations.
namespace gcc {
class LLVM_LIBRARY_VISIBILITY Common : public Tool {
public:
  Common(const char *Name, const char *ShortName, const ToolChain &TC)
      : Tool(Name, ShortName, TC) {}

  // A gcc tool has an "integrated" assembler that it will call to produce an
  // object. Let it use that assembler so that we don't have to deal with
  // assembly syntax incompatibilities.
  bool hasIntegratedAssembler() const override { return true; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;

  /// RenderExtraToolArgs - Render any arguments necessary to force
  /// the particular tool mode.
  virtual void RenderExtraToolArgs(const JobAction &JA,
                                   toolchain::opt::ArgStringList &CmdArgs) const = 0;
};

class LLVM_LIBRARY_VISIBILITY Preprocessor : public Common {
public:
  Preprocessor(const ToolChain &TC)
      : Common("gcc::Preprocessor", "gcc preprocessor", TC) {}

  bool hasGoodDiagnostics() const override { return true; }
  bool hasIntegratedCPP() const override { return false; }

  void RenderExtraToolArgs(const JobAction &JA,
                           toolchain::opt::ArgStringList &CmdArgs) const override;
};

class LLVM_LIBRARY_VISIBILITY Compiler : public Common {
public:
  Compiler(const ToolChain &TC) : Common("gcc::Compiler", "gcc frontend", TC) {}

  bool hasGoodDiagnostics() const override { return true; }
  bool hasIntegratedCPP() const override { return true; }

  void RenderExtraToolArgs(const JobAction &JA,
                           toolchain::opt::ArgStringList &CmdArgs) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker : public Common {
public:
  Linker(const ToolChain &TC) : Common("gcc::Linker", "linker (via gcc)", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void RenderExtraToolArgs(const JobAction &JA,
                           toolchain::opt::ArgStringList &CmdArgs) const override;
};
} // end namespace gcc
} // end namespace tools

namespace toolchains {

/// Generic_GCC - A tool chain using the 'gcc' command to perform
/// all subcommands; this relies on gcc translating the majority of
/// command line options.
class LLVM_LIBRARY_VISIBILITY Generic_GCC : public ToolChain {
public:
  /// Struct to store and manipulate GCC versions.
  ///
  /// We rely on assumptions about the form and structure of GCC version
  /// numbers: they consist of at most three '.'-separated components, and each
  /// component is a non-negative integer except for the last component. For
  /// the last component we are very flexible in order to tolerate release
  /// candidates or 'x' wildcards.
  ///
  /// Note that the ordering established among GCCVersions is based on the
  /// preferred version string to use. For example we prefer versions without
  /// a hard-coded patch number to those with a hard coded patch number.
  ///
  /// Currently this doesn't provide any logic for textual suffixes to patches
  /// in the way that (for example) Debian's version format does. If that ever
  /// becomes necessary, it can be added.
  struct GCCVersion {
    /// The unparsed text of the version.
    std::string Text;

    /// The parsed major, minor, and patch numbers.
    int Major, Minor, Patch;

    /// The text of the parsed major, and major+minor versions.
    std::string MajorStr, MinorStr;

    /// Any textual suffix on the patch number.
    std::string PatchSuffix;

    static GCCVersion Parse(StringRef VersionText);
    bool isOlderThan(int RHSMajor, int RHSMinor, int RHSPatch,
                     StringRef RHSPatchSuffix = StringRef()) const;
    bool operator<(const GCCVersion &RHS) const {
      return isOlderThan(RHS.Major, RHS.Minor, RHS.Patch, RHS.PatchSuffix);
    }
    bool operator>(const GCCVersion &RHS) const { return RHS < *this; }
    bool operator<=(const GCCVersion &RHS) const { return !(*this > RHS); }
    bool operator>=(const GCCVersion &RHS) const { return !(*this < RHS); }
  };

  /// This is a class to find a viable GCC installation for Clang to
  /// use.
  ///
  /// This class tries to find a GCC installation on the system, and report
  /// information about it. It starts from the host information provided to the
  /// Driver, and has logic for fuzzing that where appropriate.
  class GCCInstallationDetector {
    bool IsValid;
    toolchain::Triple GCCTriple;
    const Driver &D;

    // FIXME: These might be better as path objects.
    std::string GCCInstallPath;
    std::string GCCParentLibPath;

    /// The primary multilib appropriate for the given flags.
    Multilib SelectedMultilib;
    /// On Biarch systems, this corresponds to the default multilib when
    /// targeting the non-default multilib. Otherwise, it is empty.
    std::optional<Multilib> BiarchSibling;

    GCCVersion Version;

    // We retain the list of install paths that were considered and rejected in
    // order to print out detailed information in verbose mode.
    std::set<std::string> CandidateGCCInstallPaths;

    /// The set of multilibs that the detected installation supports.
    MultilibSet Multilibs;

    // Gentoo-specific toolchain configurations are stored here.
    const std::string GentooConfigDir = "/etc/env.d/gcc";

  public:
    explicit GCCInstallationDetector(const Driver &D) : IsValid(false), D(D) {}
    void init(const toolchain::Triple &TargetTriple, const toolchain::opt::ArgList &Args);

    /// Check whether we detected a valid GCC install.
    bool isValid() const { return IsValid; }

    /// Get the GCC triple for the detected install.
    const toolchain::Triple &getTriple() const { return GCCTriple; }

    /// Get the detected GCC installation path.
    StringRef getInstallPath() const { return GCCInstallPath; }

    /// Get the detected GCC parent lib path.
    StringRef getParentLibPath() const { return GCCParentLibPath; }

    /// Get the detected Multilib
    const Multilib &getMultilib() const { return SelectedMultilib; }

    /// Get the whole MultilibSet
    const MultilibSet &getMultilibs() const { return Multilibs; }

    /// Get the biarch sibling multilib (if it exists).
    /// \return true iff such a sibling exists
    bool getBiarchSibling(Multilib &M) const;

    /// Get the detected GCC version string.
    const GCCVersion &getVersion() const { return Version; }

    /// Print information about the detected GCC installation.
    void print(raw_ostream &OS) const;

  private:
    static void
    CollectLibDirsAndTriples(const toolchain::Triple &TargetTriple,
                             const toolchain::Triple &BiarchTriple,
                             SmallVectorImpl<StringRef> &LibDirs,
                             SmallVectorImpl<StringRef> &TripleAliases,
                             SmallVectorImpl<StringRef> &BiarchLibDirs,
                             SmallVectorImpl<StringRef> &BiarchTripleAliases);

    void AddDefaultGCCPrefixes(const toolchain::Triple &TargetTriple,
                               SmallVectorImpl<std::string> &Prefixes,
                               StringRef SysRoot);

    bool ScanGCCForMultilibs(const toolchain::Triple &TargetTriple,
                             const toolchain::opt::ArgList &Args,
                             StringRef Path,
                             bool NeedsBiarchSuffix = false);

    void ScanLibDirForGCCTriple(const toolchain::Triple &TargetArch,
                                const toolchain::opt::ArgList &Args,
                                const std::string &LibDir,
                                StringRef CandidateTriple,
                                bool NeedsBiarchSuffix, bool GCCDirExists,
                                bool GCCCrossDirExists);

    bool ScanGentooConfigs(const toolchain::Triple &TargetTriple,
                           const toolchain::opt::ArgList &Args,
                           const SmallVectorImpl<StringRef> &CandidateTriples,
                           const SmallVectorImpl<StringRef> &BiarchTriples);

    bool ScanGentooGccConfig(const toolchain::Triple &TargetTriple,
                             const toolchain::opt::ArgList &Args,
                             StringRef CandidateTriple,
                             bool NeedsBiarchSuffix = false);
  };

protected:
  GCCInstallationDetector GCCInstallation;
  LazyDetector<CudaInstallationDetector> CudaInstallation;
  LazyDetector<RocmInstallationDetector> RocmInstallation;
  LazyDetector<SYCLInstallationDetector> SYCLInstallation;

public:
  Generic_GCC(const Driver &D, const toolchain::Triple &Triple,
              const toolchain::opt::ArgList &Args);
  ~Generic_GCC() override;

  void printVerboseInfo(raw_ostream &OS) const override;

  UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefault() const override;
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefaultForced() const override;
  bool IsIntegratedAssemblerDefault() const override;
  toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const override;

protected:
  Tool *getTool(Action::ActionClass AC) const override;
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;

  /// \name ToolChain Implementation Helper Functions
  /// @{

  /// Check whether the target triple's architecture is 64-bits.
  bool isTarget64Bit() const { return getTriple().isArch64Bit(); }

  /// Check whether the target triple's architecture is 32-bits.
  bool isTarget32Bit() const { return getTriple().isArch32Bit(); }

  void PushPPaths(ToolChain::path_list &PPaths);
  void AddMultilibPaths(const Driver &D, const std::string &SysRoot,
                        const std::string &OSLibDir,
                        const std::string &MultiarchTriple,
                        path_list &Paths);
  void AddMultiarchPaths(const Driver &D, const std::string &SysRoot,
                         const std::string &OSLibDir, path_list &Paths);
  void AddMultilibIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                              toolchain::opt::ArgStringList &CC1Args) const;

  // FIXME: This should be final, but the CrossWindows toolchain does weird
  // things that can't be easily generalized.
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;

  void addSYCLIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const override;

  virtual void
  addLibCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args) const;
  virtual void
  addLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const;

  bool addGCCLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                                   toolchain::opt::ArgStringList &CC1Args,
                                   StringRef DebianMultiarch) const;

  bool addLibStdCXXIncludePaths(Twine IncludeDir, StringRef Triple,
                                Twine IncludeSuffix,
                                const toolchain::opt::ArgList &DriverArgs,
                                toolchain::opt::ArgStringList &CC1Args,
                                bool DetectDebian = false) const;

  /// @}

private:
  mutable std::unique_ptr<tools::gcc::Preprocessor> Preprocess;
  mutable std::unique_ptr<tools::gcc::Compiler> Compile;
};

class LLVM_LIBRARY_VISIBILITY Generic_ELF : public Generic_GCC {
  virtual void anchor();

public:
  Generic_ELF(const Driver &D, const toolchain::Triple &Triple,
              const toolchain::opt::ArgList &Args)
      : Generic_GCC(D, Triple, Args) {}

  void addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args,
                             Action::OffloadKind DeviceOffloadKind) const override;

  virtual std::string getDynamicLinker(const toolchain::opt::ArgList &Args) const {
    return {};
  }

  virtual void addExtraOpts(toolchain::opt::ArgStringList &CmdArgs) const {}
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_GNU_H
