//===-- Runtime.cpp -------------------------------------------------------===//
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

#include "language/Compability/Lower/Runtime.h"
#include "language/Compability/Lower/Bridge.h"
#include "language/Compability/Lower/OpenACC.h"
#include "language/Compability/Lower/OpenMP.h"
#include "language/Compability/Lower/StatementContext.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Runtime/misc-intrinsic.h"
#include "language/Compability/Runtime/pointer.h"
#include "language/Compability/Runtime/random.h"
#include "language/Compability/Runtime/stop.h"
#include "language/Compability/Runtime/time-intrinsic.h"
#include "language/Compability/Semantics/tools.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "toolchain/Support/Debug.h"
#include <optional>

#define DEBUG_TYPE "flang-lower-runtime"

using namespace language::Compability::runtime;

/// Runtime calls that do not return to the caller indicate this condition by
/// terminating the current basic block with an unreachable op.
static void genUnreachable(fir::FirOpBuilder &builder, mlir::Location loc) {
  mlir::Block *curBlock = builder.getBlock();
  mlir::Operation *parentOp = curBlock->getParentOp();
  if (parentOp->getDialect()->getNamespace() ==
      mlir::omp::OpenMPDialect::getDialectNamespace())
    language::Compability::lower::genOpenMPTerminator(builder, parentOp, loc);
  else if (parentOp->getDialect()->getNamespace() ==
           mlir::acc::OpenACCDialect::getDialectNamespace())
    language::Compability::lower::genOpenACCTerminator(builder, parentOp, loc);
  else
    fir::UnreachableOp::create(builder, loc);
  mlir::Block *newBlock = curBlock->splitBlock(builder.getInsertionPoint());
  builder.setInsertionPointToStart(newBlock);
}

//===----------------------------------------------------------------------===//
// Misc. Fortran statements that lower to runtime calls
//===----------------------------------------------------------------------===//

void language::Compability::lower::genStopStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::StopStmt &stmt) {
  const bool isError = std::get<language::Compability::parser::StopStmt::Kind>(stmt.t) ==
                       language::Compability::parser::StopStmt::Kind::ErrorStop;
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  mlir::Location loc = converter.getCurrentLocation();
  language::Compability::lower::StatementContext stmtCtx;
  toolchain::SmallVector<mlir::Value> operands;
  mlir::func::FuncOp callee;
  mlir::FunctionType calleeType;
  // First operand is stop code (zero if absent)
  if (const auto &code =
          std::get<std::optional<language::Compability::parser::StopCode>>(stmt.t)) {
    auto expr =
        converter.genExprValue(*language::Compability::semantics::GetExpr(*code), stmtCtx);
    LLVM_DEBUG(toolchain::dbgs() << "stop expression: "; expr.dump();
               toolchain::dbgs() << '\n');
    expr.match(
        [&](const fir::CharBoxValue &x) {
          callee = fir::runtime::getRuntimeFunc<mkRTKey(StopStatementText)>(
              loc, builder);
          calleeType = callee.getFunctionType();
          // Creates a pair of operands for the CHARACTER and its LEN.
          operands.push_back(
              builder.createConvert(loc, calleeType.getInput(0), x.getAddr()));
          operands.push_back(
              builder.createConvert(loc, calleeType.getInput(1), x.getLen()));
        },
        [&](fir::UnboxedValue x) {
          callee = fir::runtime::getRuntimeFunc<mkRTKey(StopStatement)>(
              loc, builder);
          calleeType = callee.getFunctionType();
          mlir::Value cast =
              builder.createConvert(loc, calleeType.getInput(0), x);
          operands.push_back(cast);
        },
        [&](auto) {
          mlir::emitError(loc, "unhandled expression in STOP");
          std::exit(1);
        });
  } else {
    callee = fir::runtime::getRuntimeFunc<mkRTKey(StopStatement)>(loc, builder);
    calleeType = callee.getFunctionType();
    // Default to values are advised in F'2023 11.4 p2.
    operands.push_back(builder.createIntegerConstant(
        loc, calleeType.getInput(0), isError ? 1 : 0));
  }

  // Second operand indicates ERROR STOP
  operands.push_back(builder.createIntegerConstant(
      loc, calleeType.getInput(operands.size()), isError));

  // Third operand indicates QUIET (default to false).
  if (const auto &quiet =
          std::get<std::optional<language::Compability::parser::ScalarLogicalExpr>>(stmt.t)) {
    const SomeExpr *expr = language::Compability::semantics::GetExpr(*quiet);
    assert(expr && "failed getting typed expression");
    mlir::Value q = fir::getBase(converter.genExprValue(*expr, stmtCtx));
    operands.push_back(
        builder.createConvert(loc, calleeType.getInput(operands.size()), q));
  } else {
    operands.push_back(builder.createIntegerConstant(
        loc, calleeType.getInput(operands.size()), 0));
  }

  fir::CallOp::create(builder, loc, callee, operands);
  auto blockIsUnterminated = [&builder]() {
    mlir::Block *currentBlock = builder.getBlock();
    return currentBlock->empty() ||
           !currentBlock->back().hasTrait<mlir::OpTrait::IsTerminator>();
  };
  if (blockIsUnterminated())
    genUnreachable(builder, loc);
}

void language::Compability::lower::genFailImageStatement(
    language::Compability::lower::AbstractConverter &converter) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  mlir::Location loc = converter.getCurrentLocation();
  mlir::func::FuncOp callee =
      fir::runtime::getRuntimeFunc<mkRTKey(FailImageStatement)>(loc, builder);
  fir::CallOp::create(builder, loc, callee, mlir::ValueRange{});
  genUnreachable(builder, loc);
}

void language::Compability::lower::genNotifyWaitStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::NotifyWaitStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: NOTIFY WAIT runtime");
}

void language::Compability::lower::genEventPostStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::EventPostStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: EVENT POST runtime");
}

void language::Compability::lower::genEventWaitStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::EventWaitStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: EVENT WAIT runtime");
}

void language::Compability::lower::genLockStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::LockStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: LOCK runtime");
}

void language::Compability::lower::genUnlockStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::UnlockStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: UNLOCK runtime");
}

void language::Compability::lower::genSyncAllStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::SyncAllStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: SYNC ALL runtime");
}

void language::Compability::lower::genSyncImagesStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::SyncImagesStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: SYNC IMAGES runtime");
}

void language::Compability::lower::genSyncMemoryStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::SyncMemoryStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: SYNC MEMORY runtime");
}

void language::Compability::lower::genSyncTeamStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::SyncTeamStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: SYNC TEAM runtime");
}

void language::Compability::lower::genPauseStatement(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::PauseStmt &) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  mlir::Location loc = converter.getCurrentLocation();
  mlir::func::FuncOp callee =
      fir::runtime::getRuntimeFunc<mkRTKey(PauseStatement)>(loc, builder);
  fir::CallOp::create(builder, loc, callee, mlir::ValueRange{});
}

void language::Compability::lower::genPointerAssociate(fir::FirOpBuilder &builder,
                                         mlir::Location loc,
                                         mlir::Value pointer,
                                         mlir::Value target) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(PointerAssociate)>(loc, builder);
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, func.getFunctionType(), pointer, target);
  fir::CallOp::create(builder, loc, func, args);
}

void language::Compability::lower::genPointerAssociateRemapping(
    fir::FirOpBuilder &builder, mlir::Location loc, mlir::Value pointer,
    mlir::Value target, mlir::Value bounds, bool isMonomorphic) {
  mlir::func::FuncOp func =
      isMonomorphic
          ? fir::runtime::getRuntimeFunc<mkRTKey(
                PointerAssociateRemappingMonomorphic)>(loc, builder)
          : fir::runtime::getRuntimeFunc<mkRTKey(PointerAssociateRemapping)>(
                loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(4));
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, func.getFunctionType(), pointer, target, bounds, sourceFile,
      sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void language::Compability::lower::genPointerAssociateLowerBounds(fir::FirOpBuilder &builder,
                                                    mlir::Location loc,
                                                    mlir::Value pointer,
                                                    mlir::Value target,
                                                    mlir::Value lbounds) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(PointerAssociateLowerBounds)>(
          loc, builder);
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, func.getFunctionType(), pointer, target, lbounds);
  fir::CallOp::create(builder, loc, func, args);
}
