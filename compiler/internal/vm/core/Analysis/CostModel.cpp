//===- CostModel.cpp ------ Cost Model Analysis ---------------------------===//
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
//
// This file defines the cost model analysis. It provides a very basic cost
// estimation for LLVM-IR. This analysis uses the services of the codegen
// to approximate the cost of any IR instruction when lowered to machine
// instructions. The cost results are unit-less and the cost number represents
// the throughput of the machine assuming that all loads hit the cache, all
// branches are predicted, etc. The cost numbers can be added in order to
// compare two or more transformation alternatives.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/CostModel.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

enum class OutputCostKind {
  RecipThroughput,
  Latency,
  CodeSize,
  SizeAndLatency,
  All,
};

static cl::opt<OutputCostKind> CostKind(
    "cost-kind", cl::desc("Target cost kind"),
    cl::init(OutputCostKind::RecipThroughput),
    cl::values(clEnumValN(OutputCostKind::RecipThroughput, "throughput",
                          "Reciprocal throughput"),
               clEnumValN(OutputCostKind::Latency, "latency",
                          "Instruction latency"),
               clEnumValN(OutputCostKind::CodeSize, "code-size", "Code size"),
               clEnumValN(OutputCostKind::SizeAndLatency, "size-latency",
                          "Code size and latency"),
               clEnumValN(OutputCostKind::All, "all", "Print all cost kinds")));

enum class IntrinsicCostStrategy {
  InstructionCost,
  IntrinsicCost,
  TypeBasedIntrinsicCost,
};

static cl::opt<IntrinsicCostStrategy> IntrinsicCost(
    "intrinsic-cost-strategy",
    cl::desc("Costing strategy for intrinsic instructions"),
    cl::init(IntrinsicCostStrategy::InstructionCost),
    cl::values(
        clEnumValN(IntrinsicCostStrategy::InstructionCost, "instruction-cost",
                   "Use TargetTransformInfo::getInstructionCost"),
        clEnumValN(IntrinsicCostStrategy::IntrinsicCost, "intrinsic-cost",
                   "Use TargetTransformInfo::getIntrinsicInstrCost"),
        clEnumValN(
            IntrinsicCostStrategy::TypeBasedIntrinsicCost,
            "type-based-intrinsic-cost",
            "Calculate the intrinsic cost based only on argument types")));

#define CM_NAME "cost-model"
#define DEBUG_TYPE CM_NAME

static InstructionCost getCost(Instruction &Inst, TTI::TargetCostKind CostKind,
                               TargetTransformInfo &TTI,
                               TargetLibraryInfo &TLI) {
  auto *II = dyn_cast<IntrinsicInst>(&Inst);
  if (II && IntrinsicCost != IntrinsicCostStrategy::InstructionCost) {
    IntrinsicCostAttributes ICA(
        II->getIntrinsicID(), *II, InstructionCost::getInvalid(),
        /*TypeBasedOnly=*/IntrinsicCost ==
            IntrinsicCostStrategy::TypeBasedIntrinsicCost,
        &TLI);
    return TTI.getIntrinsicInstrCost(ICA, CostKind);
  }

  return TTI.getInstructionCost(&Inst, CostKind);
}

static TTI::TargetCostKind
OutputCostKindToTargetCostKind(OutputCostKind CostKind) {
  switch (CostKind) {
  case OutputCostKind::RecipThroughput:
    return TTI::TCK_RecipThroughput;
  case OutputCostKind::Latency:
    return TTI::TCK_Latency;
  case OutputCostKind::CodeSize:
    return TTI::TCK_CodeSize;
  case OutputCostKind::SizeAndLatency:
    return TTI::TCK_SizeAndLatency;
  default:
    llvm_unreachable("Unexpected OutputCostKind!");
  };
}

PreservedAnalyses CostModelPrinterPass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  OS << "Printing analysis 'Cost Model Analysis' for function '" << F.getName() << "':\n";
  for (BasicBlock &B : F) {
    for (Instruction &Inst : B) {
      OS << "Cost Model: ";
      if (CostKind == OutputCostKind::All) {
        OS << "Found costs of ";
        InstructionCost RThru =
            getCost(Inst, TTI::TCK_RecipThroughput, TTI, TLI);
        InstructionCost CodeSize = getCost(Inst, TTI::TCK_CodeSize, TTI, TLI);
        InstructionCost Lat = getCost(Inst, TTI::TCK_Latency, TTI, TLI);
        InstructionCost SizeLat =
            getCost(Inst, TTI::TCK_SizeAndLatency, TTI, TLI);
        if (RThru == CodeSize && RThru == Lat && RThru == SizeLat)
          OS << RThru;
        else
          OS << "RThru:" << RThru << " CodeSize:" << CodeSize << " Lat:" << Lat
             << " SizeLat:" << SizeLat;
        OS << " for: " << Inst << "\n";
      } else {
        InstructionCost Cost =
            getCost(Inst, OutputCostKindToTargetCostKind(CostKind), TTI, TLI);
        if (Cost.isValid())
          OS << "Found an estimated cost of " << Cost.getValue();
        else
          OS << "Invalid cost";
        OS << " for instruction: " << Inst << "\n";
      }
    }
  }
  return PreservedAnalyses::all();
}
