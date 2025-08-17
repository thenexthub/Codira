//===-- CommandLineOpts.cpp -- shared command line options ------*- C++ -*-===//
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

/// This file defines some shared command-line options that can be used when
/// debugging the test tools.

#include "language/Compability/Optimizer/Passes/CommandLineOpts.h"

using namespace toolchain;

#define DisableOption(DOName, DOOption, DODescription)                         \
  cl::opt<bool> disable##DOName("disable-" DOOption,                           \
                                cl::desc("disable " DODescription " pass"),    \
                                cl::init(false), cl::Hidden)
#define EnableOption(EOName, EOOption, EODescription)                          \
  cl::opt<bool> enable##EOName("enable-" EOOption,                             \
                               cl::desc("enable " EODescription " pass"),      \
                               cl::init(false), cl::Hidden)

cl::opt<bool> dynamicArrayStackToHeapAllocation(
    "fdynamic-heap-array",
    cl::desc("place all array allocations of dynamic size on the heap"),
    cl::init(false), cl::Hidden);

cl::opt<std::size_t> arrayStackAllocationThreshold(
    "fstack-array-size",
    cl::desc(
        "place all array allocations more than <size> elements on the heap"),
    cl::init(~static_cast<std::size_t>(0)), cl::Hidden);

cl::opt<bool> ignoreMissingTypeDescriptors(
    "ignore-missing-type-desc",
    cl::desc("ignore failures to find derived type descriptors when "
             "translating FIR to LLVM"),
    cl::init(false), cl::Hidden);

cl::opt<bool> skipExternalRttiDefinition(
    "skip-external-rtti-definition", toolchain::cl::init(false),
    toolchain::cl::desc("do not define rtti static objects for types belonging to "
                   "other compilation units"),
    cl::Hidden);

OptimizationLevel defaultOptLevel{OptimizationLevel::O0};

codegenoptions::DebugInfoKind noDebugInfo{codegenoptions::NoDebugInfo};

/// Optimizer Passes
DisableOption(CfgConversion, "cfg-conversion", "disable FIR to CFG pass");
DisableOption(FirAvc, "avc", "array value copy analysis and transformation");
DisableOption(FirMao, "memory-allocation-opt",
              "memory allocation optimization");

DisableOption(FirAliasTags, "fir-alias-tags", "fir alias analysis");
cl::opt<bool> useOldAliasTags(
    "use-old-alias-tags",
    cl::desc("Use a single TBAA tree for all functions and do not use "
             "the FIR alias tags pass"),
    cl::init(false), cl::Hidden);

/// CodeGen Passes
DisableOption(CodeGenRewrite, "codegen-rewrite", "rewrite FIR for codegen");
DisableOption(TargetRewrite, "target-rewrite", "rewrite FIR for target");
DisableOption(DebugInfo, "debug-info", "Add debug info");
DisableOption(FirToLlvmIr, "fir-to-toolchainir", "FIR to LLVM-IR dialect");
DisableOption(LlvmIrToLlvm, "toolchain", "conversion to LLVM");
DisableOption(BoxedProcedureRewrite, "boxed-procedure-rewrite",
              "rewrite boxed procedures");

DisableOption(ExternalNameConversion, "external-name-interop",
              "convert names with external convention");
EnableOption(ConstantArgumentGlobalisation, "constant-argument-globalisation",
             "the local constant argument to global constant conversion");
DisableOption(CompilerGeneratedNamesConversion, "compiler-generated-names",
              "replace special symbols in compiler generated names");
