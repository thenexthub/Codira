//===-- CustomIntrinsicCall.cpp -------------------------------------------===//
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

#include "language/Compability/Lower/CustomIntrinsicCall.h"
#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Evaluate/fold.h"
#include "language/Compability/Evaluate/tools.h"
#include "language/Compability/Lower/StatementContext.h"
#include "language/Compability/Optimizer/Builder/IntrinsicCall.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Semantics/tools.h"
#include <optional>

/// Is this a call to MIN or MAX intrinsic with arguments that may be absent at
/// runtime? This is a special case because MIN and MAX can have any number of
/// arguments.
static bool isMinOrMaxWithDynamicallyOptionalArg(
    toolchain::StringRef name, const language::Compability::evaluate::ProcedureRef &procRef) {
  if (name != "min" && name != "max")
    return false;
  const auto &args = procRef.arguments();
  std::size_t argSize = args.size();
  if (argSize <= 2)
    return false;
  for (std::size_t i = 2; i < argSize; ++i) {
    if (auto *expr =
            language::Compability::evaluate::UnwrapExpr<language::Compability::lower::SomeExpr>(args[i]))
      if (language::Compability::evaluate::MayBePassedAsAbsentOptional(*expr))
        return true;
  }
  return false;
}

/// Is this a call to ISHFTC intrinsic with a SIZE argument that may be absent
/// at runtime? This is a special case because the SIZE value to be applied
/// when absent is not zero.
static bool isIshftcWithDynamicallyOptionalArg(
    toolchain::StringRef name, const language::Compability::evaluate::ProcedureRef &procRef) {
  if (name != "ishftc" || procRef.arguments().size() < 3)
    return false;
  auto *expr = language::Compability::evaluate::UnwrapExpr<language::Compability::lower::SomeExpr>(
      procRef.arguments()[2]);
  return expr && language::Compability::evaluate::MayBePassedAsAbsentOptional(*expr);
}

/// Is this a call to ASSOCIATED where the TARGET is an OPTIONAL (but not a
/// deallocated allocatable or disassociated pointer)?
/// Subtle: contrary to other intrinsic optional arguments, disassociated
/// POINTER and unallocated ALLOCATABLE actual argument are not considered
/// absent here. This is because ASSOCIATED has special requirements for TARGET
/// actual arguments that are POINTERs. There is no precise requirements for
/// ALLOCATABLEs, but all existing Fortran compilers treat them similarly to
/// POINTERs. That is: unallocated TARGETs cause ASSOCIATED to rerun false.  The
/// runtime deals with the disassociated/unallocated case. Simply ensures that
/// TARGET that are OPTIONAL get conditionally emboxed here to convey the
/// optional aspect to the runtime.
static bool isAssociatedWithDynamicallyOptionalArg(
    toolchain::StringRef name, const language::Compability::evaluate::ProcedureRef &procRef) {
  if (name != "associated" || procRef.arguments().size() < 2)
    return false;
  auto *expr = language::Compability::evaluate::UnwrapExpr<language::Compability::lower::SomeExpr>(
      procRef.arguments()[1]);
  const language::Compability::semantics::Symbol *sym{
      expr ? language::Compability::evaluate::UnwrapWholeSymbolOrComponentDataRef(expr)
           : nullptr};
  return (sym && language::Compability::semantics::IsOptional(*sym));
}

bool language::Compability::lower::intrinsicRequiresCustomOptionalHandling(
    const language::Compability::evaluate::ProcedureRef &procRef,
    const language::Compability::evaluate::SpecificIntrinsic &intrinsic,
    AbstractConverter &converter) {
  toolchain::StringRef name = intrinsic.name;
  return isMinOrMaxWithDynamicallyOptionalArg(name, procRef) ||
         isIshftcWithDynamicallyOptionalArg(name, procRef) ||
         isAssociatedWithDynamicallyOptionalArg(name, procRef);
}

/// Generate the FIR+MLIR operations for the generic intrinsic \p name
/// with arguments \p args and the expected result type \p resultType.
/// Returned fir::ExtendedValue is the returned Fortran intrinsic value.
fir::ExtendedValue
language::Compability::lower::genIntrinsicCall(fir::FirOpBuilder &builder, mlir::Location loc,
                                 toolchain::StringRef name,
                                 std::optional<mlir::Type> resultType,
                                 toolchain::ArrayRef<fir::ExtendedValue> args,
                                 language::Compability::lower::StatementContext &stmtCtx,
                                 language::Compability::lower::AbstractConverter *converter) {
  auto [result, mustBeFreed] =
      fir::genIntrinsicCall(builder, loc, name, resultType, args, converter);
  if (mustBeFreed) {
    mlir::Value addr = fir::getBase(result);
    if (auto *box = result.getBoxOf<fir::BoxValue>())
      addr =
          fir::BoxAddrOp::create(builder, loc, box->getMemTy(), box->getAddr());
    fir::FirOpBuilder *bldr = &builder;
    stmtCtx.attachCleanup([=]() { fir::FreeMemOp::create(*bldr, loc, addr); });
  }
  return result;
}

static void prepareMinOrMaxArguments(
    const language::Compability::evaluate::ProcedureRef &procRef,
    const language::Compability::evaluate::SpecificIntrinsic &intrinsic,
    std::optional<mlir::Type> retTy,
    const language::Compability::lower::OperandPrepare &prepareOptionalArgument,
    const language::Compability::lower::OperandPrepareAs &prepareOtherArgument,
    language::Compability::lower::AbstractConverter &converter) {
  assert(retTy && "MIN and MAX must have a return type");
  mlir::Type resultType = *retTy;
  mlir::Location loc = converter.getCurrentLocation();
  if (fir::isa_char(resultType))
    TODO(loc, "CHARACTER MIN and MAX with dynamically optional arguments");
  for (auto arg : toolchain::enumerate(procRef.arguments())) {
    const auto *expr =
        language::Compability::evaluate::UnwrapExpr<language::Compability::lower::SomeExpr>(arg.value());
    if (!expr)
      continue;
    if (arg.index() <= 1 ||
        !language::Compability::evaluate::MayBePassedAsAbsentOptional(*expr)) {
      // Non optional arguments.
      prepareOtherArgument(*expr, fir::LowerIntrinsicArgAs::Value);
    } else {
      // Dynamically optional arguments.
      // Subtle: even for scalar the if-then-else will be generated in the loop
      // nest because the then part will require the current extremum value that
      // may depend on previous array element argument and cannot be outlined.
      prepareOptionalArgument(*expr);
    }
  }
}

static fir::ExtendedValue
lowerMinOrMax(fir::FirOpBuilder &builder, mlir::Location loc,
              toolchain::StringRef name, std::optional<mlir::Type> retTy,
              const language::Compability::lower::OperandPresent &isPresentCheck,
              const language::Compability::lower::OperandGetter &getOperand,
              std::size_t numOperands,
              language::Compability::lower::StatementContext &stmtCtx) {
  assert(numOperands >= 2 && !isPresentCheck(0) && !isPresentCheck(1) &&
         "min/max must have at least two non-optional args");
  assert(retTy && "MIN and MAX must have a return type");
  mlir::Type resultType = *retTy;
  toolchain::SmallVector<fir::ExtendedValue> args;
  const bool loadOperand = true;
  args.push_back(getOperand(0, loadOperand));
  args.push_back(getOperand(1, loadOperand));
  mlir::Value extremum = fir::getBase(
      genIntrinsicCall(builder, loc, name, resultType, args, stmtCtx));

  for (std::size_t opIndex = 2; opIndex < numOperands; ++opIndex) {
    if (std::optional<mlir::Value> isPresentRuntimeCheck =
            isPresentCheck(opIndex)) {
      // Argument is dynamically optional.
      extremum =
          builder
              .genIfOp(loc, {resultType}, *isPresentRuntimeCheck,
                       /*withElseRegion=*/true)
              .genThen([&]() {
                toolchain::SmallVector<fir::ExtendedValue> args;
                args.emplace_back(extremum);
                args.emplace_back(getOperand(opIndex, loadOperand));
                fir::ExtendedValue newExtremum = genIntrinsicCall(
                    builder, loc, name, resultType, args, stmtCtx);
                fir::ResultOp::create(builder, loc, fir::getBase(newExtremum));
              })
              .genElse([&]() { fir::ResultOp::create(builder, loc, extremum); })
              .getResults()[0];
    } else {
      // Argument is know to be present at compile time.
      toolchain::SmallVector<fir::ExtendedValue> args;
      args.emplace_back(extremum);
      args.emplace_back(getOperand(opIndex, loadOperand));
      extremum = fir::getBase(
          genIntrinsicCall(builder, loc, name, resultType, args, stmtCtx));
    }
  }
  return extremum;
}

static void prepareIshftcArguments(
    const language::Compability::evaluate::ProcedureRef &procRef,
    const language::Compability::evaluate::SpecificIntrinsic &intrinsic,
    std::optional<mlir::Type> retTy,
    const language::Compability::lower::OperandPrepare &prepareOptionalArgument,
    const language::Compability::lower::OperandPrepareAs &prepareOtherArgument,
    language::Compability::lower::AbstractConverter &converter) {
  for (auto arg : toolchain::enumerate(procRef.arguments())) {
    const auto *expr =
        language::Compability::evaluate::UnwrapExpr<language::Compability::lower::SomeExpr>(arg.value());
    assert(expr && "expected all ISHFTC argument to be textually present here");
    if (arg.index() == 2) {
      assert(language::Compability::evaluate::MayBePassedAsAbsentOptional(*expr) &&
             "expected ISHFTC SIZE arg to be dynamically optional");
      prepareOptionalArgument(*expr);
    } else {
      // Non optional arguments.
      prepareOtherArgument(*expr, fir::LowerIntrinsicArgAs::Value);
    }
  }
}

static fir::ExtendedValue
lowerIshftc(fir::FirOpBuilder &builder, mlir::Location loc,
            toolchain::StringRef name, std::optional<mlir::Type> retTy,
            const language::Compability::lower::OperandPresent &isPresentCheck,
            const language::Compability::lower::OperandGetter &getOperand,
            std::size_t numOperands,
            language::Compability::lower::StatementContext &stmtCtx) {
  assert(numOperands == 3 && !isPresentCheck(0) && !isPresentCheck(1) &&
         isPresentCheck(2) &&
         "only ISHFTC SIZE arg is expected to be dynamically optional here");
  assert(retTy && "ISFHTC must have a return type");
  mlir::Type resultType = *retTy;
  toolchain::SmallVector<fir::ExtendedValue> args;
  const bool loadOperand = true;
  args.push_back(getOperand(0, loadOperand));
  args.push_back(getOperand(1, loadOperand));
  auto iPC = isPresentCheck(2);
  assert(iPC.has_value());
  args.push_back(
      builder
          .genIfOp(loc, {resultType}, *iPC,
                   /*withElseRegion=*/true)
          .genThen([&]() {
            fir::ExtendedValue sizeExv = getOperand(2, loadOperand);
            mlir::Value size =
                builder.createConvert(loc, resultType, fir::getBase(sizeExv));
            fir::ResultOp::create(builder, loc, size);
          })
          .genElse([&]() {
            mlir::Value bitSize = builder.createIntegerConstant(
                loc, resultType,
                mlir::cast<mlir::IntegerType>(resultType).getWidth());
            fir::ResultOp::create(builder, loc, bitSize);
          })
          .getResults()[0]);
  return genIntrinsicCall(builder, loc, name, resultType, args, stmtCtx);
}

static void prepareAssociatedArguments(
    const language::Compability::evaluate::ProcedureRef &procRef,
    const language::Compability::evaluate::SpecificIntrinsic &intrinsic,
    std::optional<mlir::Type> retTy,
    const language::Compability::lower::OperandPrepare &prepareOptionalArgument,
    const language::Compability::lower::OperandPrepareAs &prepareOtherArgument,
    language::Compability::lower::AbstractConverter &converter) {
  const auto *pointer = procRef.UnwrapArgExpr(0);
  const auto *optionalTarget = procRef.UnwrapArgExpr(1);
  assert(pointer && optionalTarget &&
         "expected call to associated with a target");
  prepareOtherArgument(*pointer, fir::LowerIntrinsicArgAs::Inquired);
  prepareOptionalArgument(*optionalTarget);
}

static fir::ExtendedValue
lowerAssociated(fir::FirOpBuilder &builder, mlir::Location loc,
                toolchain::StringRef name, std::optional<mlir::Type> resultType,
                const language::Compability::lower::OperandPresent &isPresentCheck,
                const language::Compability::lower::OperandGetter &getOperand,
                std::size_t numOperands,
                language::Compability::lower::StatementContext &stmtCtx) {
  assert(numOperands == 2 && "expect two arguments when TARGET is OPTIONAL");
  toolchain::SmallVector<fir::ExtendedValue> args;
  args.push_back(getOperand(0, /*loadOperand=*/false));
  // Ensure a null descriptor is passed to the code lowering Associated if
  // TARGET is absent.
  fir::ExtendedValue targetExv = getOperand(1, /*loadOperand=*/false);
  mlir::Value targetBase = fir::getBase(targetExv);
  // subtle: isPresentCheck would test for an unallocated/disassociated target,
  // while the optionality of the target pointer/allocatable is what must be
  // checked here.
  mlir::Value isPresent =
      fir::IsPresentOp::create(builder, loc, builder.getI1Type(), targetBase);
  mlir::Type targetType = fir::unwrapRefType(targetBase.getType());
  mlir::Type targetValueType = fir::unwrapPassByRefType(targetType);
  mlir::Type boxType = mlir::isa<fir::BaseBoxType>(targetType)
                           ? targetType
                           : fir::BoxType::get(targetValueType);
  fir::BoxValue targetBox =
      builder
          .genIfOp(loc, {boxType}, isPresent,
                   /*withElseRegion=*/true)
          .genThen([&]() {
            mlir::Value box = builder.createBox(loc, targetExv);
            mlir::Value cast = builder.createConvert(loc, boxType, box);
            fir::ResultOp::create(builder, loc, cast);
          })
          .genElse([&]() {
            mlir::Value absentBox =
                fir::AbsentOp::create(builder, loc, boxType);
            fir::ResultOp::create(builder, loc, absentBox);
          })
          .getResults()[0];
  args.emplace_back(std::move(targetBox));
  return genIntrinsicCall(builder, loc, name, resultType, args, stmtCtx);
}

void language::Compability::lower::prepareCustomIntrinsicArgument(
    const language::Compability::evaluate::ProcedureRef &procRef,
    const language::Compability::evaluate::SpecificIntrinsic &intrinsic,
    std::optional<mlir::Type> retTy,
    const OperandPrepare &prepareOptionalArgument,
    const OperandPrepareAs &prepareOtherArgument,
    AbstractConverter &converter) {
  toolchain::StringRef name = intrinsic.name;
  if (name == "min" || name == "max")
    return prepareMinOrMaxArguments(procRef, intrinsic, retTy,
                                    prepareOptionalArgument,
                                    prepareOtherArgument, converter);
  if (name == "associated")
    return prepareAssociatedArguments(procRef, intrinsic, retTy,
                                      prepareOptionalArgument,
                                      prepareOtherArgument, converter);
  assert(name == "ishftc" && "unexpected custom intrinsic argument call");
  return prepareIshftcArguments(procRef, intrinsic, retTy,
                                prepareOptionalArgument, prepareOtherArgument,
                                converter);
}

fir::ExtendedValue language::Compability::lower::lowerCustomIntrinsic(
    fir::FirOpBuilder &builder, mlir::Location loc, toolchain::StringRef name,
    std::optional<mlir::Type> retTy, const OperandPresent &isPresentCheck,
    const OperandGetter &getOperand, std::size_t numOperands,
    language::Compability::lower::StatementContext &stmtCtx) {
  if (name == "min" || name == "max")
    return lowerMinOrMax(builder, loc, name, retTy, isPresentCheck, getOperand,
                         numOperands, stmtCtx);
  if (name == "associated")
    return lowerAssociated(builder, loc, name, retTy, isPresentCheck,
                           getOperand, numOperands, stmtCtx);
  assert(name == "ishftc" && "unexpected custom intrinsic call");
  return lowerIshftc(builder, loc, name, retTy, isPresentCheck, getOperand,
                     numOperands, stmtCtx);
}
