//===--- ToolChains.h - Platform-specific ToolChain logic -------*- C++ -*-===//
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

#ifndef LANGUAGE_DRIVER_TOOLCHAINS_H
#define LANGUAGE_DRIVER_TOOLCHAINS_H

#include "language/Basic/Toolchain.h"
#include "language/Driver/ToolChain.h"
#include "clang/Basic/DarwinSDKInfo.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/Compiler.h"

namespace language {
class DiagnosticEngine;

namespace driver {
namespace toolchains {

class TOOLCHAIN_LIBRARY_VISIBILITY Darwin : public ToolChain {
protected:

  void addLinkerInputArgs(InvocationInfo &II,
                          const JobContext &context) const;

  void addSanitizerArgs(toolchain::opt::ArgStringList &Arguments,
                        const DynamicLinkJobAction &job,
                        const JobContext &context) const;

  void addArgsToLinkStdlib(toolchain::opt::ArgStringList &Arguments,
                           const DynamicLinkJobAction &job,
                           const JobContext &context) const;

  void addProfileGenerationArgs(toolchain::opt::ArgStringList &Arguments,
                                const JobContext &context) const;

  void addDeploymentTargetArgs(toolchain::opt::ArgStringList &Arguments,
                               const JobContext &context) const;

  void addLTOLibArgs(toolchain::opt::ArgStringList &Arguments,
                     const JobContext &context) const;

  void addCommonFrontendArgs(
      const OutputInfo &OI, const CommandOutput &output,
      const toolchain::opt::ArgList &inputArgs,
      toolchain::opt::ArgStringList &arguments) const override;

  void addPlatformSpecificPluginFrontendArgs(
      const OutputInfo &OI,
      const CommandOutput &output,
      const toolchain::opt::ArgList &inputArgs,
      toolchain::opt::ArgStringList &arguments) const override;

  InvocationInfo constructInvocation(const InterpretJobAction &job,
                                     const JobContext &context) const override;
  InvocationInfo constructInvocation(const DynamicLinkJobAction &job,
                                     const JobContext &context) const override;
  InvocationInfo constructInvocation(const StaticLinkJobAction &job,
                                     const JobContext &context) const override;

  void validateArguments(DiagnosticEngine &diags,
                         const toolchain::opt::ArgList &args,
                         StringRef defaultTarget) const override;

  void validateOutputInfo(DiagnosticEngine &diags,
                          const OutputInfo &outputInfo) const override;

  std::string findProgramRelativeToCodiraImpl(StringRef name) const override;

  bool shouldStoreInvocationInDebugInfo() const override;
  std::string getGlobalDebugPathRemapping() const override;
  
  /// Retrieve the target SDK version for the given target triple.
  std::optional<toolchain::VersionTuple>
  getTargetSDKVersion(const toolchain::Triple &triple) const;

  /// Information about the SDK that the application is being built against.
  /// This information is only used by the linker, so it is only populated
  /// when there will be a linker job.
  mutable std::optional<clang::DarwinSDKInfo> SDKInfo;

  const std::optional<toolchain::Triple> TargetVariant;

public:
  Darwin(const Driver &D, const toolchain::Triple &Triple,
         const std::optional<toolchain::Triple> &TargetVariant)
      : ToolChain(D, Triple), TargetVariant(TargetVariant) {}

  ~Darwin() = default;
  std::string sanitizerRuntimeLibName(StringRef Sanitizer,
                                      bool shared = true) const override;

  std::optional<toolchain::Triple> getTargetVariant() const { return TargetVariant; }
};

class TOOLCHAIN_LIBRARY_VISIBILITY Windows : public ToolChain {
protected:
  InvocationInfo constructInvocation(const DynamicLinkJobAction &job,
                                     const JobContext &context) const override;
  InvocationInfo constructInvocation(const StaticLinkJobAction &job,
                                     const JobContext &context) const override;

public:
  Windows(const Driver &D, const toolchain::Triple &Triple) : ToolChain(D, Triple) {}
  ~Windows() = default;

  std::string sanitizerRuntimeLibName(StringRef Sanitizer,
                                      bool shared = true) const override;
};

class TOOLCHAIN_LIBRARY_VISIBILITY WebAssembly : public ToolChain {
protected:
  InvocationInfo constructInvocation(const AutolinkExtractJobAction &job,
                                     const JobContext &context) const override;
  InvocationInfo constructInvocation(const DynamicLinkJobAction &job,
                                     const JobContext &context) const override;
  InvocationInfo constructInvocation(const StaticLinkJobAction &job,
                                     const JobContext &context) const override;
  void validateArguments(DiagnosticEngine &diags,
                         const toolchain::opt::ArgList &args,
                         StringRef defaultTarget) const override;

public:
  WebAssembly(const Driver &D, const toolchain::Triple &Triple) : ToolChain(D, Triple) {}
  ~WebAssembly() = default;
  std::string sanitizerRuntimeLibName(StringRef Sanitizer,
                                      bool shared = true) const override;
};


class TOOLCHAIN_LIBRARY_VISIBILITY GenericUnix : public ToolChain {
protected:
  InvocationInfo constructInvocation(const InterpretJobAction &job,
                                     const JobContext &context) const override;
  InvocationInfo constructInvocation(const AutolinkExtractJobAction &job,
                                     const JobContext &context) const override;

  /// If provided, and if the user has not already explicitly specified a
  /// linker to use via the "-fuse-ld=" option, this linker will be passed to
  /// the compiler invocation via "-fuse-ld=". Return an empty string to not
  /// specify any specific linker (the "-fuse-ld=" option will not be
  /// specified).
  ///
  /// The default behavior is to use the gold linker on ARM architectures,
  /// and to not provide a specific linker otherwise.
  virtual std::string getDefaultLinker() const;

  bool addRuntimeRPath(const toolchain::Triple &T,
                       const toolchain::opt::ArgList &Args) const;

  InvocationInfo constructInvocation(const DynamicLinkJobAction &job,
                                     const JobContext &context) const override;
  InvocationInfo constructInvocation(const StaticLinkJobAction &job,
                                     const JobContext &context) const override;

public:
  GenericUnix(const Driver &D, const toolchain::Triple &Triple)
      : ToolChain(D, Triple) {}
  ~GenericUnix() = default;
  std::string sanitizerRuntimeLibName(StringRef Sanitizer,
                                      bool shared = true) const override;
};

class TOOLCHAIN_LIBRARY_VISIBILITY Android : public GenericUnix {
public:
  Android(const Driver &D, const toolchain::Triple &Triple)
      : GenericUnix(D, Triple) {}
  ~Android() = default;
};

class TOOLCHAIN_LIBRARY_VISIBILITY Cygwin : public GenericUnix {
protected:
  std::string getDefaultLinker() const override;

public:
  Cygwin(const Driver &D, const toolchain::Triple &Triple)
      : GenericUnix(D, Triple) {}
  ~Cygwin() = default;
};

class TOOLCHAIN_LIBRARY_VISIBILITY OpenBSD : public GenericUnix {
protected:
  std::string getDefaultLinker() const override;

public:
  OpenBSD(const Driver &D, const toolchain::Triple &Triple)
      : GenericUnix(D, Triple) {}
  ~OpenBSD() = default;
};

class TOOLCHAIN_LIBRARY_VISIBILITY FreeBSD : public GenericUnix {
protected:
  std::string getDefaultLinker() const override;

public:
  FreeBSD(const Driver &D, const toolchain::Triple &Triple)
      : GenericUnix(D, Triple) {}
  ~FreeBSD() = default;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language

#endif
