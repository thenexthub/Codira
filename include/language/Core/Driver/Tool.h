//===--- Tool.h - Compilation Tools -----------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_DRIVER_TOOL_H
#define LANGUAGE_CORE_DRIVER_TOOL_H

#include "language/Core/Basic/LLVM.h"

namespace toolchain {
namespace opt {
  class ArgList;
}
}

namespace language::Core {
namespace driver {

  class Compilation;
  class InputInfo;
  class Job;
  class JobAction;
  class ToolChain;

  typedef SmallVector<InputInfo, 4> InputInfoList;

/// Tool - Information on a specific compilation tool.
class Tool {
  /// The tool name (for debugging).
  const char *Name;

  /// The human readable name for the tool, for use in diagnostics.
  const char *ShortName;

  /// The tool chain this tool is a part of.
  const ToolChain &TheToolChain;

public:
  Tool(const char *Name, const char *ShortName, const ToolChain &TC);

public:
  virtual ~Tool();

  const char *getName() const { return Name; }

  const char *getShortName() const { return ShortName; }

  const ToolChain &getToolChain() const { return TheToolChain; }

  virtual bool hasIntegratedAssembler() const { return false; }
  virtual bool hasIntegratedBackend() const { return true; }
  virtual bool canEmitIR() const { return false; }
  virtual bool hasIntegratedCPP() const = 0;
  virtual bool isLinkJob() const { return false; }
  virtual bool isDsymutilJob() const { return false; }

  /// Does this tool have "good" standardized diagnostics, or should the
  /// driver add an additional "command failed" diagnostic on failures.
  virtual bool hasGoodDiagnostics() const { return false; }

  /// ConstructJob - Construct jobs to perform the action \p JA,
  /// writing to \p Output and with \p Inputs, and add the jobs to
  /// \p C.
  ///
  /// \param TCArgs - The argument list for this toolchain, with any
  /// tool chain specific translations applied.
  /// \param LinkingOutput - If this output will eventually feed the
  /// linker, then this is the final output name of the linked image.
  virtual void ConstructJob(Compilation &C, const JobAction &JA,
                            const InputInfo &Output,
                            const InputInfoList &Inputs,
                            const toolchain::opt::ArgList &TCArgs,
                            const char *LinkingOutput) const = 0;
  /// Construct jobs to perform the action \p JA, writing to the \p Outputs and
  /// with \p Inputs, and add the jobs to \p C. The default implementation
  /// assumes a single output and is expected to be overloaded for the tools
  /// that support multiple inputs.
  ///
  /// \param TCArgs The argument list for this toolchain, with any
  /// tool chain specific translations applied.
  /// \param LinkingOutput If this output will eventually feed the
  /// linker, then this is the final output name of the linked image.
  virtual void ConstructJobMultipleOutputs(Compilation &C, const JobAction &JA,
                                           const InputInfoList &Outputs,
                                           const InputInfoList &Inputs,
                                           const toolchain::opt::ArgList &TCArgs,
                                           const char *LinkingOutput) const;
};

} // end namespace driver
} // end namespace language::Core

#endif
