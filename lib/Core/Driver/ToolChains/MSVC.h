//===--- MSVC.h - MSVC ToolChain Implementations ----------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MSVC_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MSVC_H

#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/CudaInstallationDetector.h"
#include "language/Core/Driver/LazyDetector.h"
#include "language/Core/Driver/RocmInstallationDetector.h"
#include "language/Core/Driver/SyclInstallationDetector.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"
#include "toolchain/Frontend/Debug/Options.h"
#include "toolchain/WindowsDriver/MSVCPaths.h"

namespace language::Core {
namespace driver {
namespace tools {

/// Visual studio tools.
namespace visualstudio {
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("visualstudio::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace visualstudio

} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY MSVCToolChain : public ToolChain {
public:
  MSVCToolChain(const Driver &D, const toolchain::Triple &Triple,
                const toolchain::opt::ArgList &Args);

  toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const override;

  UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefault() const override;
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefaultForced() const override;

  /// Set CodeView as the default debug info format for non-MachO binary
  /// formats, and to DWARF otherwise. Users can use -gcodeview and -gdwarf to
  /// override the default.
  toolchain::codegenoptions::DebugInfoFormat getDefaultDebugFormat() const override {
    return getTriple().isOSBinFormatCOFF() ? toolchain::codegenoptions::DIF_CodeView
                                           : toolchain::codegenoptions::DIF_DWARF;
  }

  /// Set the debugger tuning to "default", since we're definitely not tuning
  /// for GDB.
  toolchain::DebuggerKind getDefaultDebuggerTuning() const override {
    return toolchain::DebuggerKind::Default;
  }

  unsigned GetDefaultDwarfVersion() const override {
    return 4;
  }

  std::string getSubDirectoryPath(toolchain::SubDirectoryType Type,
                                  toolchain::StringRef SubdirParent = "") const;
  std::string getSubDirectoryPath(toolchain::SubDirectoryType Type,
                                  toolchain::Triple::ArchType TargetArch) const;

  bool getIsVS2017OrNewer() const {
    return VSLayout == toolchain::ToolsetLayout::VS2017OrNewer;
  }

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;

  void AddCudaIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const override;

  void AddHIPIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                         toolchain::opt::ArgStringList &CC1Args) const override;

  void AddHIPRuntimeLibArgs(const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs) const override;

  void addSYCLIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const override;

  bool getWindowsSDKLibraryPath(
      const toolchain::opt::ArgList &Args, std::string &path) const;
  bool getUniversalCRTLibraryPath(const toolchain::opt::ArgList &Args,
                                  std::string &path) const;
  bool useUniversalCRT() const;
  VersionTuple
  computeMSVCVersion(const Driver *D,
                     const toolchain::opt::ArgList &Args) const override;

  std::string ComputeEffectiveClangTriple(const toolchain::opt::ArgList &Args,
                                          types::ID InputType) const override;
  SanitizerMask getSupportedSanitizers() const override;

  void printVerboseInfo(raw_ostream &OS) const override;

  bool FoundMSVCInstall() const { return !VCToolChainPath.empty(); }

  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;

protected:
  void AddSystemIncludeWithSubfolder(const toolchain::opt::ArgList &DriverArgs,
                                     toolchain::opt::ArgStringList &CC1Args,
                                     const std::string &folder,
                                     const Twine &subfolder1,
                                     const Twine &subfolder2 = "",
                                     const Twine &subfolder3 = "") const;

  Tool *buildLinker() const override;
  Tool *buildAssembler() const override;
private:
  std::optional<toolchain::StringRef> WinSdkDir, WinSdkVersion, WinSysRoot;
  std::string VCToolChainPath;
  toolchain::ToolsetLayout VSLayout = toolchain::ToolsetLayout::OlderVS;
  LazyDetector<CudaInstallationDetector> CudaInstallation;
  LazyDetector<RocmInstallationDetector> RocmInstallation;
  LazyDetector<SYCLInstallationDetector> SYCLInstallation;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MSVC_H
