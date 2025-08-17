//===----------------------------------------------------------------------===//
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
// Emit OpenACC Stmt nodes as CIR code.
//
//===----------------------------------------------------------------------===//

#include "CIRGenBuilder.h"
#include "CIRGenFunction.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "language/Core/AST/OpenACCClause.h"
#include "language/Core/AST/StmtOpenACC.h"

using namespace language::Core;
using namespace language::Core::CIRGen;
using namespace cir;
using namespace mlir::acc;

template <typename Op, typename TermOp>
mlir::LogicalResult CIRGenFunction::emitOpenACCOpAssociatedStmt(
    mlir::Location start, mlir::Location end, OpenACCDirectiveKind dirKind,
    SourceLocation dirLoc, toolchain::ArrayRef<const OpenACCClause *> clauses,
    const Stmt *associatedStmt) {
  mlir::LogicalResult res = mlir::success();

  toolchain::SmallVector<mlir::Type> retTy;
  toolchain::SmallVector<mlir::Value> operands;
  auto op = builder.create<Op>(start, retTy, operands);

  emitOpenACCClauses(op, dirKind, dirLoc, clauses);

  {
    mlir::Block &block = op.getRegion().emplaceBlock();
    mlir::OpBuilder::InsertionGuard guardCase(builder);
    builder.setInsertionPointToEnd(&block);

    LexicalScope ls{*this, start, builder.getInsertionBlock()};
    res = emitStmt(associatedStmt, /*useCurrentScope=*/true);

    builder.create<TermOp>(end);
  }
  return res;
}

namespace {
template <typename Op> struct CombinedType;
template <> struct CombinedType<ParallelOp> {
  static constexpr mlir::acc::CombinedConstructsType value =
      mlir::acc::CombinedConstructsType::ParallelLoop;
};
template <> struct CombinedType<SerialOp> {
  static constexpr mlir::acc::CombinedConstructsType value =
      mlir::acc::CombinedConstructsType::SerialLoop;
};
template <> struct CombinedType<KernelsOp> {
  static constexpr mlir::acc::CombinedConstructsType value =
      mlir::acc::CombinedConstructsType::KernelsLoop;
};
} // namespace

template <typename Op, typename TermOp>
mlir::LogicalResult CIRGenFunction::emitOpenACCOpCombinedConstruct(
    mlir::Location start, mlir::Location end, OpenACCDirectiveKind dirKind,
    SourceLocation dirLoc, toolchain::ArrayRef<const OpenACCClause *> clauses,
    const Stmt *loopStmt) {
  mlir::LogicalResult res = mlir::success();

  toolchain::SmallVector<mlir::Type> retTy;
  toolchain::SmallVector<mlir::Value> operands;

  auto computeOp = builder.create<Op>(start, retTy, operands);
  computeOp.setCombinedAttr(builder.getUnitAttr());
  mlir::acc::LoopOp loopOp;

  // First, emit the bodies of both operations, with the loop inside the body of
  // the combined construct.
  {
    mlir::Block &block = computeOp.getRegion().emplaceBlock();
    mlir::OpBuilder::InsertionGuard guardCase(builder);
    builder.setInsertionPointToEnd(&block);

    LexicalScope ls{*this, start, builder.getInsertionBlock()};
    auto loopOp = builder.create<LoopOp>(start, retTy, operands);
    loopOp.setCombinedAttr(mlir::acc::CombinedConstructsTypeAttr::get(
        builder.getContext(), CombinedType<Op>::value));

    {
      mlir::Block &innerBlock = loopOp.getRegion().emplaceBlock();
      mlir::OpBuilder::InsertionGuard guardCase(builder);
      builder.setInsertionPointToEnd(&innerBlock);

      LexicalScope ls{*this, start, builder.getInsertionBlock()};
      ActiveOpenACCLoopRAII activeLoop{*this, &loopOp};

      res = emitStmt(loopStmt, /*useCurrentScope=*/true);

      builder.create<mlir::acc::YieldOp>(end);
    }

    emitOpenACCClauses(computeOp, loopOp, dirKind, dirLoc, clauses);

    updateLoopOpParallelism(loopOp, /*isOrphan=*/false, dirKind);

    builder.create<TermOp>(end);
  }

  return res;
}

template <typename Op>
Op CIRGenFunction::emitOpenACCOp(
    mlir::Location start, OpenACCDirectiveKind dirKind, SourceLocation dirLoc,
    toolchain::ArrayRef<const OpenACCClause *> clauses) {
  toolchain::SmallVector<mlir::Type> retTy;
  toolchain::SmallVector<mlir::Value> operands;
  auto op = builder.create<Op>(start, retTy, operands);

  emitOpenACCClauses(op, dirKind, dirLoc, clauses);
  return op;
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCComputeConstruct(const OpenACCComputeConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  mlir::Location end = getLoc(s.getSourceRange().getEnd());

  switch (s.getDirectiveKind()) {
  case OpenACCDirectiveKind::Parallel:
    return emitOpenACCOpAssociatedStmt<ParallelOp, mlir::acc::YieldOp>(
        start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
        s.getStructuredBlock());
  case OpenACCDirectiveKind::Serial:
    return emitOpenACCOpAssociatedStmt<SerialOp, mlir::acc::YieldOp>(
        start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
        s.getStructuredBlock());
  case OpenACCDirectiveKind::Kernels:
    return emitOpenACCOpAssociatedStmt<KernelsOp, mlir::acc::TerminatorOp>(
        start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
        s.getStructuredBlock());
  default:
    toolchain_unreachable("invalid compute construct kind");
  }
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCDataConstruct(const OpenACCDataConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  mlir::Location end = getLoc(s.getSourceRange().getEnd());

  return emitOpenACCOpAssociatedStmt<DataOp, mlir::acc::TerminatorOp>(
      start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
      s.getStructuredBlock());
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCInitConstruct(const OpenACCInitConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  emitOpenACCOp<InitOp>(start, s.getDirectiveKind(), s.getDirectiveLoc(),
                               s.clauses());
  return mlir::success();
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCSetConstruct(const OpenACCSetConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  emitOpenACCOp<SetOp>(start, s.getDirectiveKind(), s.getDirectiveLoc(),
                              s.clauses());
  return mlir::success();
}

mlir::LogicalResult CIRGenFunction::emitOpenACCShutdownConstruct(
    const OpenACCShutdownConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  emitOpenACCOp<ShutdownOp>(start, s.getDirectiveKind(),
                                   s.getDirectiveLoc(), s.clauses());
  return mlir::success();
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCWaitConstruct(const OpenACCWaitConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  auto waitOp = emitOpenACCOp<WaitOp>(start, s.getDirectiveKind(),
                                   s.getDirectiveLoc(), s.clauses());

  auto createIntExpr = [this](const Expr *intExpr) {
    mlir::Value expr = emitScalarExpr(intExpr);
    mlir::Location exprLoc = cgm.getLoc(intExpr->getBeginLoc());

    mlir::IntegerType targetType = mlir::IntegerType::get(
        &getMLIRContext(), getContext().getIntWidth(intExpr->getType()),
        intExpr->getType()->isSignedIntegerOrEnumerationType()
            ? mlir::IntegerType::SignednessSemantics::Signed
            : mlir::IntegerType::SignednessSemantics::Unsigned);

    auto conversionOp = builder.create<mlir::UnrealizedConversionCastOp>(
        exprLoc, targetType, expr);
    return conversionOp.getResult(0);
  };

  // Emit the correct 'wait' clauses.
  {
    mlir::OpBuilder::InsertionGuard guardCase(builder);
    builder.setInsertionPoint(waitOp);

    if (s.hasDevNumExpr())
      waitOp.getWaitDevnumMutable().append(createIntExpr(s.getDevNumExpr()));

    for (Expr *QueueExpr : s.getQueueIdExprs())
      waitOp.getWaitOperandsMutable().append(createIntExpr(QueueExpr));
  }

  return mlir::success();
}

mlir::LogicalResult CIRGenFunction::emitOpenACCCombinedConstruct(
    const OpenACCCombinedConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  mlir::Location end = getLoc(s.getSourceRange().getEnd());

  switch (s.getDirectiveKind()) {
  case OpenACCDirectiveKind::ParallelLoop:
    return emitOpenACCOpCombinedConstruct<ParallelOp, mlir::acc::YieldOp>(
        start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
        s.getLoop());
  case OpenACCDirectiveKind::SerialLoop:
    return emitOpenACCOpCombinedConstruct<SerialOp, mlir::acc::YieldOp>(
        start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
        s.getLoop());
  case OpenACCDirectiveKind::KernelsLoop:
    return emitOpenACCOpCombinedConstruct<KernelsOp, mlir::acc::TerminatorOp>(
        start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
        s.getLoop());
  default:
    toolchain_unreachable("invalid compute construct kind");
  }
}

mlir::LogicalResult CIRGenFunction::emitOpenACCHostDataConstruct(
    const OpenACCHostDataConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  mlir::Location end = getLoc(s.getSourceRange().getEnd());

  return emitOpenACCOpAssociatedStmt<HostDataOp, mlir::acc::TerminatorOp>(
      start, end, s.getDirectiveKind(), s.getDirectiveLoc(), s.clauses(),
      s.getStructuredBlock());
}

mlir::LogicalResult CIRGenFunction::emitOpenACCEnterDataConstruct(
    const OpenACCEnterDataConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  emitOpenACCOp<EnterDataOp>(start, s.getDirectiveKind(), s.getDirectiveLoc(),
                             s.clauses());
  return mlir::success();
}

mlir::LogicalResult CIRGenFunction::emitOpenACCExitDataConstruct(
    const OpenACCExitDataConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  emitOpenACCOp<ExitDataOp>(start, s.getDirectiveKind(), s.getDirectiveLoc(),
                            s.clauses());
  return mlir::success();
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCUpdateConstruct(const OpenACCUpdateConstruct &s) {
  mlir::Location start = getLoc(s.getSourceRange().getBegin());
  emitOpenACCOp<UpdateOp>(start, s.getDirectiveKind(), s.getDirectiveLoc(),
                          s.clauses());
  return mlir::success();
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCCacheConstruct(const OpenACCCacheConstruct &s) {
  // The 'cache' directive 'may' be at the top of a loop by standard, but
  // doesn't have to be. Additionally, there is nothing that requires this be a
  // loop affected by an OpenACC pragma. Sema doesn't do any level of
  // enforcement here, since it isn't particularly valuable to do so thanks to
  // that. Instead, we treat cache as a 'noop' if there is no acc.loop to apply
  // it to.
  if (!activeLoopOp)
    return mlir::success();

  mlir::acc::LoopOp loopOp = *activeLoopOp;

  mlir::OpBuilder::InsertionGuard guard(builder);
  builder.setInsertionPoint(loopOp);

  for (const Expr *var : s.getVarList()) {
    CIRGenFunction::OpenACCDataOperandInfo opInfo =
        getOpenACCDataOperandInfo(var);

    auto cacheOp = builder.create<CacheOp>(
        opInfo.beginLoc, opInfo.varValue,
        /*structured=*/false, /*implicit=*/false, opInfo.name, opInfo.bounds);

    loopOp.getCacheOperandsMutable().append(cacheOp.getResult());
  }

  return mlir::success();
}

mlir::LogicalResult
CIRGenFunction::emitOpenACCAtomicConstruct(const OpenACCAtomicConstruct &s) {
  cgm.errorNYI(s.getSourceRange(), "OpenACC Atomic Construct");
  return mlir::failure();
}
