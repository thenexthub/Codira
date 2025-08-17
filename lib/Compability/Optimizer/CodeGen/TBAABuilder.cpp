//===-- TBAABuilder.cpp -- TBAA builder definitions -----------------------===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/CodeGen/TBAABuilder.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "toolchain/ADT/TypeSwitch.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include <mlir/Dialect/LLVMIR/LLVMAttrs.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>
#include <mlir/Dialect/LLVMIR/LLVMTypes.h>

#define DEBUG_TYPE "flang-tbaa-builder"

using namespace mlir;
using namespace mlir::LLVM;

static toolchain::cl::opt<bool> disableTBAA(
    "disable-tbaa",
    toolchain::cl::desc("disable attaching TBAA tags to memory accessing operations "
                   "to override default Flang behavior"),
    toolchain::cl::init(false));

// disabling this will play badly with the FIR TBAA pass, leading to worse
// performance
static toolchain::cl::opt<bool> perFunctionTBAATrees(
    "per-function-tbaa-trees",
    toolchain::cl::desc("Give each function an independent TBAA tree (default)"),
    toolchain::cl::init(true), toolchain::cl::Hidden);

// tagAttachmentLimit is a debugging option that allows limiting
// the number of TBAA access tag attributes attached to operations.
// It is set to kTagAttachmentUnlimited by default denoting "no limit".
static constexpr unsigned kTagAttachmentUnlimited =
    std::numeric_limits<unsigned>::max();
static toolchain::cl::opt<unsigned>
    tagAttachmentLimit("tbaa-attach-tag-max", toolchain::cl::desc(""),
                       toolchain::cl::init(kTagAttachmentUnlimited));

namespace fir {

TBAABuilder::TBAABuilder(MLIRContext *context, bool applyTBAA,
                         bool forceUnifiedTree)
    : enableTBAA(applyTBAA && !disableTBAA),
      trees(/*separatePerFunction=*/perFunctionTBAATrees && !forceUnifiedTree) {
  // TODO: the TBAA tags created here are rooted in the root scope
  // of the enclosing function. This does not work best with MLIR inlining.
  // A better approach is to root them according to the scopes they belong to
  // and that were used by AddAliasTagsPass to create TBAA tags before
  // the CodeGen. For example:
  //   subroutine caller(a, b, ptr)
  //     real, target :: a(:), b(:)
  //     integer, pointer :: ptr(:)
  //     call callee(a, b, ptr)
  //   end
  //   subroutine callee(a, b, ptr)
  //     real :: a(:), b(:)
  //     integer, pointer :: ptr(:)
  //     do i=...
  //       a(ptr(i)) = b(ptr(i))
  //     end do
  //   end
  //
  // When callee is inlined, the dummy arguments 'a' and 'b' will
  // be rooted in TBAA tree corresponding to the `call callee` call site,
  // saying that the references to 'a' and 'b' cannot alias each other.
  // These tags will be created by AddAliasTagsPass, but it will not be able
  // to create any tags for 'ptr' references.
  // During the CodeGen, we create 'any data access' tags for the
  // 'ptr' acceses. If they are rooted within the root scope of `caller`,
  // they end up in a different TBAA tree with the 'a' and 'b' access
  // tags, so 'ptr', 'a' and 'b' references MayAlias. Moreover,
  // the box access of 'ptr' will also be in a different TBAA tree
  // with 'a' and 'b' tags, meaning they can also alias.
  // This will prevent LLVM vectorization even with memory conflict checks.
  // It seems that we'd better move all TBAA tags assignment to
  // AddAliasTagsPass, which can at least rely on the dummy arguments scopes.
  if (!enableTBAA)
    return;
}

TBAATagAttr TBAABuilder::getAccessTag(TBAATypeDescriptorAttr baseTypeDesc,
                                      TBAATypeDescriptorAttr accessTypeDesc,
                                      int64_t offset) {
  TBAATagAttr &tag = tagsMap[{baseTypeDesc, accessTypeDesc, offset}];
  if (tag)
    return tag;

  // Initialize new tag.
  tag = TBAATagAttr::get(baseTypeDesc, accessTypeDesc, offset);
  return tag;
}

TBAATagAttr TBAABuilder::getAnyBoxAccessTag(mlir::LLVM::LLVMFuncOp func) {
  TBAATypeDescriptorAttr boxMemberTypeDesc = trees[func].boxMemberTypeDesc;
  return getAccessTag(boxMemberTypeDesc, boxMemberTypeDesc, /*offset=*/0);
}

TBAATagAttr TBAABuilder::getBoxAccessTag(Type baseFIRType, Type accessFIRType,
                                         GEPOp gep,
                                         mlir::LLVM::LLVMFuncOp func) {
  return getAnyBoxAccessTag(func);
}

TBAATagAttr TBAABuilder::getAnyDataAccessTag(mlir::LLVM::LLVMFuncOp func) {
  TBAATypeDescriptorAttr anyDataAccessTypeDesc = trees[func].anyDataTypeDesc;
  return getAccessTag(anyDataAccessTypeDesc, anyDataAccessTypeDesc,
                      /*offset=*/0);
}

TBAATagAttr TBAABuilder::getDataAccessTag(Type baseFIRType, Type accessFIRType,
                                          GEPOp gep,
                                          mlir::LLVM::LLVMFuncOp func) {
  return getAnyDataAccessTag(func);
}

TBAATagAttr TBAABuilder::getAnyAccessTag(mlir::LLVM::LLVMFuncOp func) {
  TBAATypeDescriptorAttr anyAccessTypeDesc = trees[func].anyAccessDesc;
  return getAccessTag(anyAccessTypeDesc, anyAccessTypeDesc, /*offset=*/0);
}

void TBAABuilder::attachTBAATag(AliasAnalysisOpInterface op, Type baseFIRType,
                                Type accessFIRType, GEPOp gep) {
  if (!enableTBAA)
    return;

  mlir::LLVM::LLVMFuncOp func = op->getParentOfType<mlir::LLVM::LLVMFuncOp>();
  if (!func)
    return;

  ++tagAttachmentCounter;
  if (tagAttachmentLimit != kTagAttachmentUnlimited &&
      tagAttachmentCounter > tagAttachmentLimit)
    return;

  LLVM_DEBUG(toolchain::dbgs() << "Attaching TBAA tag #" << tagAttachmentCounter
                          << "\n");

  TBAATagAttr tbaaTagSym;
  if (fir::isRecordWithDescriptorMember(baseFIRType)) {
    // A memory access that addresses an aggregate that contains
    // a mix of data members and descriptor members may alias
    // with both data and descriptor accesses.
    // Conservatively set any-access tag if there is any descriptor member.
    tbaaTagSym = getAnyAccessTag(func);
  } else if (mlir::isa<fir::BaseBoxType>(baseFIRType)) {
    tbaaTagSym = getBoxAccessTag(baseFIRType, accessFIRType, gep, func);
  } else {
    tbaaTagSym = getDataAccessTag(baseFIRType, accessFIRType, gep, func);
  }

  if (!tbaaTagSym)
    return;

  op.setTBAATags(ArrayAttr::get(op->getContext(), tbaaTagSym));
}

} // namespace fir
