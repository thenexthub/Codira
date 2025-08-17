//===--- IncrementalExecutor.h - Incremental Execution ----------*- C++ -*-===//
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
//
// This file implements the class which performs incremental code execution.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_INTERPRETER_INCREMENTALEXECUTOR_H
#define LANGUAGE_CORE_LIB_INTERPRETER_INCREMENTALEXECUTOR_H

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ExecutionEngine/Orc/ExecutionUtils.h"
#include "toolchain/ExecutionEngine/Orc/Shared/ExecutorAddress.h"

#include <memory>

namespace toolchain {
class Error;
namespace orc {
class JITTargetMachineBuilder;
class LLJIT;
class LLJITBuilder;
class ThreadSafeContext;
} // namespace orc
} // namespace toolchain

namespace language::Core {

struct PartialTranslationUnit;
class TargetInfo;

class IncrementalExecutor {
  using CtorDtorIterator = toolchain::orc::CtorDtorIterator;
  std::unique_ptr<toolchain::orc::LLJIT> Jit;
  toolchain::orc::ThreadSafeContext &TSCtx;

  toolchain::DenseMap<const PartialTranslationUnit *, toolchain::orc::ResourceTrackerSP>
      ResourceTrackers;

protected:
  IncrementalExecutor(toolchain::orc::ThreadSafeContext &TSC);

public:
  enum SymbolNameKind { IRName, LinkerName };

  IncrementalExecutor(toolchain::orc::ThreadSafeContext &TSC,
                      toolchain::orc::LLJITBuilder &JITBuilder, toolchain::Error &Err);
  virtual ~IncrementalExecutor();

  virtual toolchain::Error addModule(PartialTranslationUnit &PTU);
  virtual toolchain::Error removeModule(PartialTranslationUnit &PTU);
  virtual toolchain::Error runCtors() const;
  virtual toolchain::Error cleanUp();
  virtual toolchain::Expected<toolchain::orc::ExecutorAddr>
  getSymbolAddress(toolchain::StringRef Name, SymbolNameKind NameKind) const;

  toolchain::orc::LLJIT &GetExecutionEngine() { return *Jit; }

  static toolchain::Expected<std::unique_ptr<toolchain::orc::LLJITBuilder>>
  createDefaultJITBuilder(toolchain::orc::JITTargetMachineBuilder JTMB);
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_INTERPRETER_INCREMENTALEXECUTOR_H
