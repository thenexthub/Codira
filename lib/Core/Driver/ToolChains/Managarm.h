//===--- Managarm.h - Managarm ToolChain Implementations --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MANAGARM_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MANAGARM_H

#include "Gnu.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY Managarm : public Generic_ELF {
public:
  Managarm(const Driver &D, const toolchain::Triple &Triple,
           const toolchain::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override;

  std::string getMultiarchTriple(const Driver &D,
                                 const toolchain::Triple &TargetTriple,
                                 StringRef SysRoot) const override;

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void
  addLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override;
  SanitizerMask getSupportedSanitizers() const override;
  std::string computeSysRoot() const override;

  std::string getDynamicLinker(const toolchain::opt::ArgList &Args) const override;

  void addExtraOpts(toolchain::opt::ArgStringList &CmdArgs) const override;

  std::vector<std::string> ExtraOpts;

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MANAGARM_H
