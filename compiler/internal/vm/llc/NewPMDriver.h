//===- NewPMDriver.h - Function to drive llc with the new PM ----*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
/// \file
///
/// A single function which is called to drive the llc behavior for the new
/// PassManager.
///
/// This is only in a separate TU with a header to avoid including all of the
/// old pass manager headers and the new pass manager headers into the same
/// file. Eventually all of the routines here will get folded back into
/// llc.cpp.
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_TOOLS_LLC_NEWPMDRIVER_H
#define LLVM_TOOLS_LLC_NEWPMDRIVER_H

#include "vm/core/IR/DiagnosticHandler.h"
#include "vm/core/Support/CodeGen.h"
#include <memory>

namespace vm::core {
    class Module;
    class TargetLibraryInfoImpl;
    class TargetMachine;
    class ToolOutputFile;
    class LLVMContext;
    class MIRParser;

    enum class VerifierKind { None, InputOutput, EachPass };

    struct LLCDiagnosticHandler : public DiagnosticHandler {
        bool handleDiagnostics(const DiagnosticInfo& DI) override;
    };

    int compileModuleWithNewPM(StringRef Arg0, std::unique_ptr<Module> M,
        std::unique_ptr<MIRParser> MIR,
        std::unique_ptr<TargetMachine> Target,
        std::unique_ptr<ToolOutputFile> Out,
        std::unique_ptr<ToolOutputFile> DwoOut,
        LLVMContext& Context,
        const TargetLibraryInfoImpl& TLII, VerifierKind VK,
        StringRef PassPipeline, CodeGenFileType FileType);
} // namespace llvm

#endif