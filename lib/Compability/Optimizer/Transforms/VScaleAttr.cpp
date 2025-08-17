//===- VScaleAttr.cpp -------------------------------------------------===//
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

//===----------------------------------------------------------------------===//
/// \file
/// This pass adds a `vscale_range` attribute to function definitions.
/// The attribute is used for scalable vector operations on Arm processors
/// and should only be run on processors that support this feature. [It is
/// likely harmless to run it on something else, but it is also not valuable].
//===----------------------------------------------------------------------===//

#include "language/Compability/Common/ISO_Fortran_binding_wrapper.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/Inquiry.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Transforms/RegionUtils.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/raw_ostream.h"

#include <algorithm>

namespace fir {
#define GEN_PASS_DEF_VSCALEATTR
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "vscale-attr"

namespace {

class VScaleAttrPass : public fir::impl::VScaleAttrBase<VScaleAttrPass> {
public:
  VScaleAttrPass(const fir::VScaleAttrOptions &options) {
    vscaleRange = options.vscaleRange;
  }
  VScaleAttrPass() {}
  void runOnOperation() override;
};

} // namespace

void VScaleAttrPass::runOnOperation() {
  LLVM_DEBUG(toolchain::dbgs() << "=== Begin " DEBUG_TYPE " ===\n");
  mlir::func::FuncOp func = getOperation();

  LLVM_DEBUG(toolchain::dbgs() << "Func-name:" << func.getSymName() << "\n");

  auto context = &getContext();

  auto intTy = mlir::IntegerType::get(context, 32);

  assert(vscaleRange.first && "VScaleRange minimum should be non-zero");

  func->setAttr("vscale_range",
                mlir::LLVM::VScaleRangeAttr::get(
                    context, mlir::IntegerAttr::get(intTy, vscaleRange.first),
                    mlir::IntegerAttr::get(intTy, vscaleRange.second)));

  LLVM_DEBUG(toolchain::dbgs() << "=== End " DEBUG_TYPE " ===\n");
}
