//===-- Allocatable.cpp -- Allocatable statements lowering ----------------===//
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

#include "language/Compability/Lower/Allocatable.h"
#include "language/Compability/Evaluate/tools.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/CUDA.h"
#include "language/Compability/Lower/ConvertType.h"
#include "language/Compability/Lower/ConvertVariable.h"
#include "language/Compability/Lower/IterationSpace.h"
#include "language/Compability/Lower/Mangler.h"
#include "language/Compability/Lower/OpenACC.h"
#include "language/Compability/Lower/PFTBuilder.h"
#include "language/Compability/Lower/Runtime.h"
#include "language/Compability/Lower/StatementContext.h"
#include "language/Compability/Optimizer/Builder/CUFCommon.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "language/Compability/Optimizer/Support/FatalError.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Runtime/allocatable.h"
#include "language/Compability/Runtime/pointer.h"
#include "language/Compability/Semantics/tools.h"
#include "language/Compability/Semantics/type.h"
#include "toolchain/Support/CommandLine.h"

/// By default fir memory operation fir::AllocMemOp/fir::FreeMemOp are used.
/// This switch allow forcing the use of runtime and descriptors for everything.
/// This is mainly intended as a debug switch.
static toolchain::cl::opt<bool> useAllocateRuntime(
    "use-alloc-runtime",
    toolchain::cl::desc("Lower allocations to fortran runtime calls"),
    toolchain::cl::init(false));
/// Switch to force lowering of allocatable and pointers to descriptors in all
/// cases. This is now turned on by default since that is what will happen with
/// HLFIR lowering, so this allows getting early feedback of the impact.
/// If this turns out to cause performance regressions, a dedicated fir.box
/// "discretization pass" would make more sense to cover all the fir.box usage
/// (taking advantage of any future inlining for instance).
static toolchain::cl::opt<bool> useDescForMutableBox(
    "use-desc-for-alloc",
    toolchain::cl::desc("Always use descriptors for POINTER and ALLOCATABLE"),
    toolchain::cl::init(true));

//===----------------------------------------------------------------------===//
// Error management
//===----------------------------------------------------------------------===//

namespace {
// Manage STAT and ERRMSG specifier information across a sequence of runtime
// calls for an ALLOCATE/DEALLOCATE stmt.
struct ErrorManager {
  void init(language::Compability::lower::AbstractConverter &converter, mlir::Location loc,
            const language::Compability::lower::SomeExpr *statExpr,
            const language::Compability::lower::SomeExpr *errMsgExpr) {
    language::Compability::lower::StatementContext stmtCtx;
    fir::FirOpBuilder &builder = converter.getFirOpBuilder();
    hasStat = builder.createBool(loc, statExpr != nullptr);
    statAddr = statExpr
                   ? fir::getBase(converter.genExprAddr(loc, statExpr, stmtCtx))
                   : mlir::Value{};
    errMsgAddr =
        statExpr && errMsgExpr
            ? builder.createBox(loc,
                                converter.genExprAddr(loc, errMsgExpr, stmtCtx))
            : fir::AbsentOp::create(
                  builder, loc,
                  fir::BoxType::get(mlir::NoneType::get(builder.getContext())));
    sourceFile = fir::factory::locationToFilename(builder, loc);
    sourceLine = fir::factory::locationToLineNo(builder, loc,
                                                builder.getIntegerType(32));
  }

  bool hasStatSpec() const { return static_cast<bool>(statAddr); }

  void genStatCheck(fir::FirOpBuilder &builder, mlir::Location loc) {
    if (statValue) {
      mlir::Value zero =
          builder.createIntegerConstant(loc, statValue.getType(), 0);
      auto cmp = mlir::arith::CmpIOp::create(
          builder, loc, mlir::arith::CmpIPredicate::eq, statValue, zero);
      auto ifOp = fir::IfOp::create(builder, loc, cmp,
                                    /*withElseRegion=*/false);
      builder.setInsertionPointToStart(&ifOp.getThenRegion().front());
    }
  }

  void assignStat(fir::FirOpBuilder &builder, mlir::Location loc,
                  mlir::Value stat) {
    if (hasStatSpec()) {
      assert(stat && "missing stat value");
      mlir::Value castStat = builder.createConvert(
          loc, fir::dyn_cast_ptrEleTy(statAddr.getType()), stat);
      fir::StoreOp::create(builder, loc, castStat, statAddr);
      statValue = stat;
    }
  }

  mlir::Value hasStat;
  mlir::Value errMsgAddr;
  mlir::Value sourceFile;
  mlir::Value sourceLine;

private:
  mlir::Value statAddr;  // STAT variable address
  mlir::Value statValue; // current runtime STAT value
};

//===----------------------------------------------------------------------===//
// Allocatables runtime call generators
//===----------------------------------------------------------------------===//

using namespace language::Compability::runtime;
/// Generate a runtime call to set the bounds of an allocatable or pointer
/// descriptor.
static void genRuntimeSetBounds(fir::FirOpBuilder &builder, mlir::Location loc,
                                const fir::MutableBoxValue &box,
                                mlir::Value dimIndex, mlir::Value lowerBound,
                                mlir::Value upperBound) {
  mlir::func::FuncOp callee =
      box.isPointer()
          ? fir::runtime::getRuntimeFunc<mkRTKey(PointerSetBounds)>(loc,
                                                                    builder)
          : fir::runtime::getRuntimeFunc<mkRTKey(AllocatableSetBounds)>(
                loc, builder);
  const auto args = fir::runtime::createArguments(
      builder, loc, callee.getFunctionType(), box.getAddr(), dimIndex,
      lowerBound, upperBound);
  fir::CallOp::create(builder, loc, callee, args);
}

/// Generate runtime call to set the lengths of a character allocatable or
/// pointer descriptor.
static void genRuntimeInitCharacter(fir::FirOpBuilder &builder,
                                    mlir::Location loc,
                                    const fir::MutableBoxValue &box,
                                    mlir::Value len, int64_t kind = 0) {
  mlir::func::FuncOp callee =
      box.isPointer()
          ? fir::runtime::getRuntimeFunc<mkRTKey(PointerNullifyCharacter)>(
                loc, builder)
          : fir::runtime::getRuntimeFunc<mkRTKey(
                AllocatableInitCharacterForAllocate)>(loc, builder);
  toolchain::ArrayRef<mlir::Type> inputTypes = callee.getFunctionType().getInputs();
  if (inputTypes.size() != 5)
    fir::emitFatalError(
        loc, "AllocatableInitCharacter runtime interface not as expected");
  toolchain::SmallVector<mlir::Value> args = {box.getAddr(), len};
  if (kind == 0)
    kind = mlir::cast<fir::CharacterType>(box.getEleTy()).getFKind();
  args.push_back(builder.createIntegerConstant(loc, inputTypes[2], kind));
  int rank = box.rank();
  args.push_back(builder.createIntegerConstant(loc, inputTypes[3], rank));
  // TODO: coarrays
  int corank = 0;
  args.push_back(builder.createIntegerConstant(loc, inputTypes[4], corank));
  const auto convertedArgs = fir::runtime::createArguments(
      builder, loc, callee.getFunctionType(), args);
  fir::CallOp::create(builder, loc, callee, convertedArgs);
}

/// Generate a sequence of runtime calls to allocate memory.
static mlir::Value genRuntimeAllocate(fir::FirOpBuilder &builder,
                                      mlir::Location loc,
                                      const fir::MutableBoxValue &box,
                                      ErrorManager &errorManager) {
  mlir::func::FuncOp callee =
      box.isPointer()
          ? fir::runtime::getRuntimeFunc<mkRTKey(PointerAllocate)>(loc, builder)
          : fir::runtime::getRuntimeFunc<mkRTKey(AllocatableAllocate)>(loc,
                                                                       builder);
  toolchain::SmallVector<mlir::Value> args{box.getAddr()};
  if (!box.isPointer())
    args.push_back(
        builder.createIntegerConstant(loc, builder.getI64Type(), -1));
  args.push_back(errorManager.hasStat);
  args.push_back(errorManager.errMsgAddr);
  args.push_back(errorManager.sourceFile);
  args.push_back(errorManager.sourceLine);
  const auto convertedArgs = fir::runtime::createArguments(
      builder, loc, callee.getFunctionType(), args);
  return fir::CallOp::create(builder, loc, callee, convertedArgs).getResult(0);
}

/// Generate a sequence of runtime calls to allocate memory and assign with the
/// \p source.
static mlir::Value genRuntimeAllocateSource(fir::FirOpBuilder &builder,
                                            mlir::Location loc,
                                            const fir::MutableBoxValue &box,
                                            fir::ExtendedValue source,
                                            ErrorManager &errorManager) {
  mlir::func::FuncOp callee =
      box.isPointer()
          ? fir::runtime::getRuntimeFunc<mkRTKey(PointerAllocateSource)>(
                loc, builder)
          : fir::runtime::getRuntimeFunc<mkRTKey(AllocatableAllocateSource)>(
                loc, builder);
  const auto args = fir::runtime::createArguments(
      builder, loc, callee.getFunctionType(), box.getAddr(),
      fir::getBase(source), errorManager.hasStat, errorManager.errMsgAddr,
      errorManager.sourceFile, errorManager.sourceLine);
  return fir::CallOp::create(builder, loc, callee, args).getResult(0);
}

/// Generate runtime call to apply mold to the descriptor.
static void genRuntimeAllocateApplyMold(fir::FirOpBuilder &builder,
                                        mlir::Location loc,
                                        const fir::MutableBoxValue &box,
                                        fir::ExtendedValue mold, int rank) {
  mlir::func::FuncOp callee =
      box.isPointer()
          ? fir::runtime::getRuntimeFunc<mkRTKey(PointerApplyMold)>(loc,
                                                                    builder)
          : fir::runtime::getRuntimeFunc<mkRTKey(AllocatableApplyMold)>(
                loc, builder);
  const auto args = fir::runtime::createArguments(
      builder, loc, callee.getFunctionType(),
      fir::factory::getMutableIRBox(builder, loc, box), fir::getBase(mold),
      builder.createIntegerConstant(
          loc, callee.getFunctionType().getInputs()[2], rank));
  fir::CallOp::create(builder, loc, callee, args);
}

/// Generate a runtime call to deallocate memory.
static mlir::Value genRuntimeDeallocate(fir::FirOpBuilder &builder,
                                        mlir::Location loc,
                                        const fir::MutableBoxValue &box,
                                        ErrorManager &errorManager,
                                        mlir::Value declaredTypeDesc = {}) {
  // Ensure fir.box is up-to-date before passing it to deallocate runtime.
  mlir::Value boxAddress = fir::factory::getMutableIRBox(builder, loc, box);
  mlir::func::FuncOp callee;
  toolchain::SmallVector<mlir::Value> args;
  toolchain::SmallVector<mlir::Value> operands;
  if (box.isPolymorphic() || box.isUnlimitedPolymorphic()) {
    callee = box.isPointer()
                 ? fir::runtime::getRuntimeFunc<mkRTKey(
                       PointerDeallocatePolymorphic)>(loc, builder)
                 : fir::runtime::getRuntimeFunc<mkRTKey(
                       AllocatableDeallocatePolymorphic)>(loc, builder);
    if (!declaredTypeDesc)
      declaredTypeDesc = builder.createNullConstant(loc);
    operands = fir::runtime::createArguments(
        builder, loc, callee.getFunctionType(), boxAddress, declaredTypeDesc,
        errorManager.hasStat, errorManager.errMsgAddr, errorManager.sourceFile,
        errorManager.sourceLine);
  } else {
    callee = box.isPointer()
                 ? fir::runtime::getRuntimeFunc<mkRTKey(PointerDeallocate)>(
                       loc, builder)
                 : fir::runtime::getRuntimeFunc<mkRTKey(AllocatableDeallocate)>(
                       loc, builder);
    operands = fir::runtime::createArguments(
        builder, loc, callee.getFunctionType(), boxAddress,
        errorManager.hasStat, errorManager.errMsgAddr, errorManager.sourceFile,
        errorManager.sourceLine);
  }
  return fir::CallOp::create(builder, loc, callee, operands).getResult(0);
}

//===----------------------------------------------------------------------===//
// Allocate statement implementation
//===----------------------------------------------------------------------===//

/// Helper to get symbol from AllocateObject.
static const language::Compability::semantics::Symbol &
unwrapSymbol(const language::Compability::parser::AllocateObject &allocObj) {
  const language::Compability::parser::Name &lastName =
      language::Compability::parser::GetLastName(allocObj);
  assert(lastName.symbol);
  return *lastName.symbol;
}

static fir::MutableBoxValue
genMutableBoxValue(language::Compability::lower::AbstractConverter &converter,
                   mlir::Location loc,
                   const language::Compability::parser::AllocateObject &allocObj) {
  const language::Compability::lower::SomeExpr *expr = language::Compability::semantics::GetExpr(allocObj);
  assert(expr && "semantic analysis failure");
  return converter.genExprMutableBox(loc, *expr);
}

/// Implement Allocate statement lowering.
class AllocateStmtHelper {
public:
  AllocateStmtHelper(language::Compability::lower::AbstractConverter &converter,
                     const language::Compability::parser::AllocateStmt &stmt,
                     mlir::Location loc)
      : converter{converter}, builder{converter.getFirOpBuilder()}, stmt{stmt},
        loc{loc} {}

  void lower() {
    visitAllocateOptions();
    lowerAllocateLengthParameters();
    errorManager.init(converter, loc, statExpr, errMsgExpr);
    language::Compability::lower::StatementContext stmtCtx;
    if (sourceExpr)
      sourceExv = converter.genExprBox(loc, *sourceExpr, stmtCtx);
    if (moldExpr)
      moldExv = converter.genExprBox(loc, *moldExpr, stmtCtx);
    mlir::OpBuilder::InsertPoint insertPt = builder.saveInsertionPoint();
    for (const auto &allocation :
         std::get<std::list<language::Compability::parser::Allocation>>(stmt.t))
      lowerAllocation(unwrapAllocation(allocation));
    builder.restoreInsertionPoint(insertPt);
  }

private:
  struct Allocation {
    const language::Compability::parser::Allocation &alloc;
    const language::Compability::semantics::DeclTypeSpec &type;
    bool hasCoarraySpec() const {
      return std::get<std::optional<language::Compability::parser::AllocateCoarraySpec>>(
                 alloc.t)
          .has_value();
    }
    const language::Compability::parser::AllocateObject &getAllocObj() const {
      return std::get<language::Compability::parser::AllocateObject>(alloc.t);
    }
    const language::Compability::semantics::Symbol &getSymbol() const {
      return unwrapSymbol(getAllocObj());
    }
    const std::list<language::Compability::parser::AllocateShapeSpec> &getShapeSpecs() const {
      return std::get<std::list<language::Compability::parser::AllocateShapeSpec>>(alloc.t);
    }
  };

  Allocation unwrapAllocation(const language::Compability::parser::Allocation &alloc) {
    const auto &allocObj = std::get<language::Compability::parser::AllocateObject>(alloc.t);
    const language::Compability::semantics::Symbol &symbol = unwrapSymbol(allocObj);
    assert(symbol.GetType());
    return Allocation{alloc, *symbol.GetType()};
  }

  void visitAllocateOptions() {
    for (const auto &allocOption :
         std::get<std::list<language::Compability::parser::AllocOpt>>(stmt.t))
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](const language::Compability::parser::StatOrErrmsg &statOrErr) {
                language::Compability::common::visit(
                    language::Compability::common::visitors{
                        [&](const language::Compability::parser::StatVariable &statVar) {
                          statExpr = language::Compability::semantics::GetExpr(statVar);
                        },
                        [&](const language::Compability::parser::MsgVariable &errMsgVar) {
                          errMsgExpr = language::Compability::semantics::GetExpr(errMsgVar);
                        },
                    },
                    statOrErr.u);
              },
              [&](const language::Compability::parser::AllocOpt::Source &source) {
                sourceExpr = language::Compability::semantics::GetExpr(source.v.value());
              },
              [&](const language::Compability::parser::AllocOpt::Mold &mold) {
                moldExpr = language::Compability::semantics::GetExpr(mold.v.value());
              },
              [&](const language::Compability::parser::AllocOpt::Stream &stream) {
                streamExpr = language::Compability::semantics::GetExpr(stream.v.value());
              },
              [&](const language::Compability::parser::AllocOpt::Pinned &pinned) {
                pinnedExpr = language::Compability::semantics::GetExpr(pinned.v.value());
              },
          },
          allocOption.u);
  }

  void lowerAllocation(const Allocation &alloc) {
    fir::MutableBoxValue boxAddr =
        genMutableBoxValue(converter, loc, alloc.getAllocObj());

    if (sourceExpr)
      genSourceMoldAllocation(alloc, boxAddr, /*isSource=*/true);
    else if (moldExpr)
      genSourceMoldAllocation(alloc, boxAddr, /*isSource=*/false);
    else
      genSimpleAllocation(alloc, boxAddr);
  }

  static bool lowerBoundsAreOnes(const Allocation &alloc) {
    for (const language::Compability::parser::AllocateShapeSpec &shapeSpec :
         alloc.getShapeSpecs())
      if (std::get<0>(shapeSpec.t))
        return false;
    return true;
  }

  /// Build name for the fir::allocmem generated for alloc.
  std::string mangleAlloc(const Allocation &alloc) {
    return converter.mangleName(alloc.getSymbol()) + ".alloc";
  }

  /// Generate allocation without runtime calls.
  /// Only for intrinsic types. No coarrays, no polymorphism. No error recovery.
  void genInlinedAllocation(const Allocation &alloc,
                            const fir::MutableBoxValue &box) {
    toolchain::SmallVector<mlir::Value> lbounds;
    toolchain::SmallVector<mlir::Value> extents;
    language::Compability::lower::StatementContext stmtCtx;
    mlir::Type idxTy = builder.getIndexType();
    bool lBoundsAreOnes = lowerBoundsAreOnes(alloc);
    mlir::Value one = builder.createIntegerConstant(loc, idxTy, 1);
    for (const language::Compability::parser::AllocateShapeSpec &shapeSpec :
         alloc.getShapeSpecs()) {
      mlir::Value lb;
      if (!lBoundsAreOnes) {
        if (const std::optional<language::Compability::parser::BoundExpr> &lbExpr =
                std::get<0>(shapeSpec.t)) {
          lb = fir::getBase(converter.genExprValue(
              loc, language::Compability::semantics::GetExpr(*lbExpr), stmtCtx));
          lb = builder.createConvert(loc, idxTy, lb);
        } else {
          lb = one;
        }
        lbounds.emplace_back(lb);
      }
      mlir::Value ub = fir::getBase(converter.genExprValue(
          loc, language::Compability::semantics::GetExpr(std::get<1>(shapeSpec.t)), stmtCtx));
      ub = builder.createConvert(loc, idxTy, ub);
      if (lb) {
        mlir::Value diff = mlir::arith::SubIOp::create(builder, loc, ub, lb);
        extents.emplace_back(
            mlir::arith::AddIOp::create(builder, loc, diff, one));
      } else {
        extents.emplace_back(ub);
      }
    }
    fir::factory::genInlinedAllocation(builder, loc, box, lbounds, extents,
                                       lenParams, mangleAlloc(alloc),
                                       /*mustBeHeap=*/true);
  }

  void postAllocationAction(const Allocation &alloc,
                            const fir::MutableBoxValue &box) {
    if (alloc.getSymbol().test(language::Compability::semantics::Symbol::Flag::AccDeclare))
      language::Compability::lower::attachDeclarePostAllocAction(converter, builder,
                                                   alloc.getSymbol());
    if (language::Compability::semantics::HasCUDAComponent(alloc.getSymbol()))
      language::Compability::lower::initializeDeviceComponentAllocator(
          converter, alloc.getSymbol(), box);
  }

  void setPinnedToFalse() {
    if (!pinnedExpr)
      return;
    language::Compability::lower::StatementContext stmtCtx;
    mlir::Value pinned =
        fir::getBase(converter.genExprAddr(loc, *pinnedExpr, stmtCtx));
    mlir::Location loc = pinned.getLoc();
    mlir::Value falseValue = builder.createBool(loc, false);
    mlir::Value falseConv = builder.createConvert(
        loc, fir::unwrapRefType(pinned.getType()), falseValue);
    fir::StoreOp::create(builder, loc, falseConv, pinned);
  }

  void genSimpleAllocation(const Allocation &alloc,
                           const fir::MutableBoxValue &box) {
    bool isCudaAllocate =
        language::Compability::semantics::HasCUDAAttr(alloc.getSymbol()) ||
        language::Compability::semantics::HasCUDAComponent(alloc.getSymbol());
    bool isCudaDeviceContext = cuf::isCUDADeviceContext(builder.getRegion());
    bool inlineAllocation = !box.isDerived() && !errorManager.hasStatSpec() &&
                            !alloc.type.IsPolymorphic() &&
                            !alloc.hasCoarraySpec() && !useAllocateRuntime &&
                            !box.isPointer();
    unsigned allocatorIdx = language::Compability::lower::getAllocatorIdx(alloc.getSymbol());

    if (inlineAllocation &&
        ((isCudaAllocate && isCudaDeviceContext) || !isCudaAllocate)) {
      // Pointers must use PointerAllocate so that their deallocations
      // can be validated.
      genInlinedAllocation(alloc, box);
      postAllocationAction(alloc, box);
      setPinnedToFalse();
      return;
    }

    // Generate a sequence of runtime calls.
    errorManager.genStatCheck(builder, loc);
    genAllocateObjectInit(box, allocatorIdx);
    if (alloc.hasCoarraySpec())
      TODO(loc, "coarray: allocation of a coarray object");
    if (alloc.type.IsPolymorphic())
      genSetType(alloc, box, loc);
    genSetDeferredLengthParameters(alloc, box);
    genAllocateObjectBounds(alloc, box);
    mlir::Value stat;
    if (!isCudaAllocate) {
      stat = genRuntimeAllocate(builder, loc, box, errorManager);
      setPinnedToFalse();
    } else {
      stat =
          genCudaAllocate(builder, loc, box, errorManager, alloc.getSymbol());
    }
    fir::factory::syncMutableBoxFromIRBox(builder, loc, box);
    postAllocationAction(alloc, box);
    errorManager.assignStat(builder, loc, stat);
  }

  /// Lower the length parameters that may be specified in the optional
  /// type specification.
  void lowerAllocateLengthParameters() {
    const language::Compability::semantics::DeclTypeSpec *typeSpec =
        getIfAllocateStmtTypeSpec();
    if (!typeSpec)
      return;
    if (const language::Compability::semantics::DerivedTypeSpec *derived =
            typeSpec->AsDerived())
      if (language::Compability::semantics::CountLenParameters(*derived) > 0)
        TODO(loc, "setting derived type params in allocation");
    if (typeSpec->category() ==
        language::Compability::semantics::DeclTypeSpec::Category::Character) {
      language::Compability::semantics::ParamValue lenParam =
          typeSpec->characterTypeSpec().length();
      if (language::Compability::semantics::MaybeIntExpr intExpr = lenParam.GetExplicit()) {
        language::Compability::lower::StatementContext stmtCtx;
        language::Compability::lower::SomeExpr lenExpr{*intExpr};
        lenParams.push_back(
            fir::getBase(converter.genExprValue(loc, lenExpr, stmtCtx)));
      }
    }
  }

  // Set length parameters in the box stored in boxAddr.
  // This must be called before setting the bounds because it may use
  // Init runtime calls that may set the bounds to zero.
  void genSetDeferredLengthParameters(const Allocation &alloc,
                                      const fir::MutableBoxValue &box) {
    if (lenParams.empty())
      return;
    // TODO: in case a length parameter was not deferred, insert a runtime check
    // that the length is the same (AllocatableCheckLengthParameter runtime
    // call).
    if (box.isCharacter())
      genRuntimeInitCharacter(builder, loc, box, lenParams[0]);

    if (box.isDerived())
      TODO(loc, "derived type length parameters in allocate");
  }

  void genAllocateObjectInit(const fir::MutableBoxValue &box,
                             unsigned allocatorIdx) {
    if (box.isPointer()) {
      // For pointers, the descriptor may still be uninitialized (see Fortran
      // 2018 19.5.2.2). The allocation runtime needs to be given a descriptor
      // with initialized rank, types and attributes. Initialize the descriptor
      // here to ensure these constraints are fulfilled.
      mlir::Value nullPointer = fir::factory::createUnallocatedBox(
          builder, loc, box.getBoxTy(), box.nonDeferredLenParams(),
          /*typeSourceBox=*/{}, allocatorIdx);
      fir::StoreOp::create(builder, loc, nullPointer, box.getAddr());
    } else {
      assert(box.isAllocatable() && "must be an allocatable");
      // For allocatables, sync the MutableBoxValue and descriptor before the
      // calls in case it is tracked locally by a set of variables.
      fir::factory::getMutableIRBox(builder, loc, box);
    }
  }

  void genAllocateObjectBounds(const Allocation &alloc,
                               const fir::MutableBoxValue &box) {
    // Set bounds for arrays
    mlir::Type idxTy = builder.getIndexType();
    mlir::Type i32Ty = builder.getIntegerType(32);
    language::Compability::lower::StatementContext stmtCtx;
    for (const auto &iter : toolchain::enumerate(alloc.getShapeSpecs())) {
      mlir::Value lb;
      const auto &bounds = iter.value().t;
      if (const std::optional<language::Compability::parser::BoundExpr> &lbExpr =
              std::get<0>(bounds))
        lb = fir::getBase(converter.genExprValue(
            loc, language::Compability::semantics::GetExpr(*lbExpr), stmtCtx));
      else
        lb = builder.createIntegerConstant(loc, idxTy, 1);
      mlir::Value ub = fir::getBase(converter.genExprValue(
          loc, language::Compability::semantics::GetExpr(std::get<1>(bounds)), stmtCtx));
      mlir::Value dimIndex =
          builder.createIntegerConstant(loc, i32Ty, iter.index());
      // Runtime call
      genRuntimeSetBounds(builder, loc, box, dimIndex, lb, ub);
    }
    if (sourceExpr && sourceExpr->Rank() > 0 &&
        alloc.getShapeSpecs().size() == 0) {
      // If the alloc object does not have shape list, get the bounds from the
      // source expression.
      mlir::Value one = builder.createIntegerConstant(loc, idxTy, 1);
      const auto *sourceBox = sourceExv.getBoxOf<fir::BoxValue>();
      assert(sourceBox && "source expression should be lowered to one box");
      for (int i = 0; i < sourceExpr->Rank(); ++i) {
        auto dimVal = builder.createIntegerConstant(loc, idxTy, i);
        auto dimInfo = fir::BoxDimsOp::create(builder, loc, idxTy, idxTy, idxTy,
                                              sourceBox->getAddr(), dimVal);
        mlir::Value lb =
            fir::factory::readLowerBound(builder, loc, sourceExv, i, one);
        mlir::Value extent = dimInfo.getResult(1);
        mlir::Value ub = mlir::arith::SubIOp::create(
            builder, loc, mlir::arith::AddIOp::create(builder, loc, extent, lb),
            one);
        mlir::Value dimIndex = builder.createIntegerConstant(loc, i32Ty, i);
        genRuntimeSetBounds(builder, loc, box, dimIndex, lb, ub);
      }
    }
  }

  void genSourceMoldAllocation(const Allocation &alloc,
                               const fir::MutableBoxValue &box, bool isSource) {
    unsigned allocatorIdx = language::Compability::lower::getAllocatorIdx(alloc.getSymbol());
    fir::ExtendedValue exv = isSource ? sourceExv : moldExv;

    // Generate a sequence of runtime calls.
    errorManager.genStatCheck(builder, loc);
    genAllocateObjectInit(box, allocatorIdx);
    if (alloc.hasCoarraySpec())
      TODO(loc, "coarray: allocation of a coarray object");
    // Set length of the allocate object if it has. Otherwise, get the length
    // from source for the deferred length parameter.
    const bool isDeferredLengthCharacter =
        box.isCharacter() && !box.hasNonDeferredLenParams();
    if (lenParams.empty() && isDeferredLengthCharacter)
      lenParams.push_back(fir::factory::readCharLen(builder, loc, exv));
    if (!isSource || alloc.type.IsPolymorphic())
      genRuntimeAllocateApplyMold(builder, loc, box, exv,
                                  alloc.getSymbol().Rank());
    if (isDeferredLengthCharacter)
      genSetDeferredLengthParameters(alloc, box);
    genAllocateObjectBounds(alloc, box);
    mlir::Value stat;
    if (language::Compability::semantics::HasCUDAAttr(alloc.getSymbol())) {
      stat =
          genCudaAllocate(builder, loc, box, errorManager, alloc.getSymbol());
    } else {
      if (isSource)
        stat = genRuntimeAllocateSource(builder, loc, box, exv, errorManager);
      else
        stat = genRuntimeAllocate(builder, loc, box, errorManager);
      setPinnedToFalse();
    }
    fir::factory::syncMutableBoxFromIRBox(builder, loc, box);
    postAllocationAction(alloc, box);
    errorManager.assignStat(builder, loc, stat);
  }

  /// Generate call to PointerNullifyDerived or AllocatableInitDerived
  /// to set the dynamic type information.
  void genInitDerived(const fir::MutableBoxValue &box, mlir::Value typeDescAddr,
                      int rank, int corank = 0) {
    mlir::func::FuncOp callee =
        box.isPointer()
            ? fir::runtime::getRuntimeFunc<mkRTKey(PointerNullifyDerived)>(
                  loc, builder)
            : fir::runtime::getRuntimeFunc<mkRTKey(
                  AllocatableInitDerivedForAllocate)>(loc, builder);

    toolchain::ArrayRef<mlir::Type> inputTypes =
        callee.getFunctionType().getInputs();
    mlir::Value rankValue =
        builder.createIntegerConstant(loc, inputTypes[2], rank);
    mlir::Value corankValue =
        builder.createIntegerConstant(loc, inputTypes[3], corank);
    const auto args = fir::runtime::createArguments(
        builder, loc, callee.getFunctionType(), box.getAddr(), typeDescAddr,
        rankValue, corankValue);
    fir::CallOp::create(builder, loc, callee, args);
  }

  /// Generate call to PointerNullifyIntrinsic or AllocatableInitIntrinsic to
  /// set the dynamic type information for a polymorphic entity from an
  /// intrinsic type spec.
  void genInitIntrinsic(const fir::MutableBoxValue &box,
                        const TypeCategory category, int64_t kind, int rank,
                        int corank = 0) {
    mlir::func::FuncOp callee =
        box.isPointer()
            ? fir::runtime::getRuntimeFunc<mkRTKey(PointerNullifyIntrinsic)>(
                  loc, builder)
            : fir::runtime::getRuntimeFunc<mkRTKey(
                  AllocatableInitIntrinsicForAllocate)>(loc, builder);

    toolchain::ArrayRef<mlir::Type> inputTypes =
        callee.getFunctionType().getInputs();
    mlir::Value categoryValue = builder.createIntegerConstant(
        loc, inputTypes[1], static_cast<int32_t>(category));
    mlir::Value kindValue =
        builder.createIntegerConstant(loc, inputTypes[2], kind);
    mlir::Value rankValue =
        builder.createIntegerConstant(loc, inputTypes[3], rank);
    mlir::Value corankValue =
        builder.createIntegerConstant(loc, inputTypes[4], corank);
    const auto args = fir::runtime::createArguments(
        builder, loc, callee.getFunctionType(), box.getAddr(), categoryValue,
        kindValue, rankValue, corankValue);
    fir::CallOp::create(builder, loc, callee, args);
  }

  /// Generate call to the AllocatableInitDerived to set up the type descriptor
  /// and other part of the descriptor for derived type.
  void genSetType(const Allocation &alloc, const fir::MutableBoxValue &box,
                  mlir::Location loc) {
    const language::Compability::semantics::DeclTypeSpec *typeSpec =
        getIfAllocateStmtTypeSpec();

    // No type spec provided in allocate statement so the declared type spec is
    // used.
    if (!typeSpec)
      typeSpec = &alloc.type;
    assert(typeSpec && "type spec missing for polymorphic allocation");

    // Set up the descriptor for allocation for intrinsic type spec on
    // unlimited polymorphic entity.
    if (typeSpec->AsIntrinsic() &&
        fir::isUnlimitedPolymorphicType(fir::getBase(box).getType())) {
      if (typeSpec->AsIntrinsic()->category() == TypeCategory::Character) {
        genRuntimeInitCharacter(
            builder, loc, box, lenParams[0],
            language::Compability::evaluate::ToInt64(typeSpec->AsIntrinsic()->kind())
                .value());
      } else {
        genInitIntrinsic(
            box, typeSpec->AsIntrinsic()->category(),
            language::Compability::evaluate::ToInt64(typeSpec->AsIntrinsic()->kind()).value(),
            alloc.getSymbol().Rank());
      }
      return;
    }

    // Do not generate calls for non derived-type type spec.
    if (!typeSpec->AsDerived())
      return;

    auto typeDescAddr = language::Compability::lower::getTypeDescAddr(
        converter, loc, typeSpec->derivedTypeSpec());
    genInitDerived(box, typeDescAddr, alloc.getSymbol().Rank());
  }

  /// Returns a pointer to the DeclTypeSpec if a type-spec is provided in the
  /// allocate statement. Returns a null pointer otherwise.
  const language::Compability::semantics::DeclTypeSpec *getIfAllocateStmtTypeSpec() const {
    if (const auto &typeSpec =
            std::get<std::optional<language::Compability::parser::TypeSpec>>(stmt.t))
      return typeSpec->declTypeSpec;
    return nullptr;
  }

  mlir::Value genCudaAllocate(fir::FirOpBuilder &builder, mlir::Location loc,
                              const fir::MutableBoxValue &box,
                              ErrorManager &errorManager,
                              const language::Compability::semantics::Symbol &sym) {
    language::Compability::lower::StatementContext stmtCtx;
    cuf::DataAttributeAttr cudaAttr =
        language::Compability::lower::translateSymbolCUFDataAttribute(builder.getContext(),
                                                        sym);
    mlir::Value errmsg = errMsgExpr ? errorManager.errMsgAddr : nullptr;
    mlir::Value stream =
        streamExpr
            ? fir::getBase(converter.genExprAddr(loc, *streamExpr, stmtCtx))
            : nullptr;
    mlir::Value pinned =
        pinnedExpr
            ? fir::getBase(converter.genExprAddr(loc, *pinnedExpr, stmtCtx))
            : nullptr;
    mlir::Value source = sourceExpr ? fir::getBase(sourceExv) : nullptr;

    // Keep return type the same as a standard AllocatableAllocate call.
    mlir::Type retTy = fir::runtime::getModel<int>()(builder.getContext());

    return cuf::AllocateOp::create(
               builder, loc, retTy, box.getAddr(), errmsg, stream, pinned,
               source, cudaAttr,
               errorManager.hasStatSpec() ? builder.getUnitAttr() : nullptr)
        .getResult();
  }

  language::Compability::lower::AbstractConverter &converter;
  fir::FirOpBuilder &builder;
  const language::Compability::parser::AllocateStmt &stmt;
  const language::Compability::lower::SomeExpr *sourceExpr{nullptr};
  const language::Compability::lower::SomeExpr *moldExpr{nullptr};
  const language::Compability::lower::SomeExpr *statExpr{nullptr};
  const language::Compability::lower::SomeExpr *errMsgExpr{nullptr};
  const language::Compability::lower::SomeExpr *pinnedExpr{nullptr};
  const language::Compability::lower::SomeExpr *streamExpr{nullptr};
  // If the allocate has a type spec, lenParams contains the
  // value of the length parameters that were specified inside.
  toolchain::SmallVector<mlir::Value> lenParams;
  ErrorManager errorManager;
  // 9.7.1.2(7) The source-expr is evaluated exactly once for each AllocateStmt.
  fir::ExtendedValue sourceExv;
  fir::ExtendedValue moldExv;

  mlir::Location loc;
};
} // namespace

void language::Compability::lower::genAllocateStmt(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::AllocateStmt &stmt, mlir::Location loc) {
  AllocateStmtHelper{converter, stmt, loc}.lower();
}

//===----------------------------------------------------------------------===//
// Deallocate statement implementation
//===----------------------------------------------------------------------===//

static void preDeallocationAction(language::Compability::lower::AbstractConverter &converter,
                                  fir::FirOpBuilder &builder,
                                  mlir::Value beginOpValue,
                                  const language::Compability::semantics::Symbol &sym) {
  if (sym.test(language::Compability::semantics::Symbol::Flag::AccDeclare))
    language::Compability::lower::attachDeclarePreDeallocAction(converter, builder,
                                                  beginOpValue, sym);
}

static void postDeallocationAction(language::Compability::lower::AbstractConverter &converter,
                                   fir::FirOpBuilder &builder,
                                   const language::Compability::semantics::Symbol &sym) {
  if (sym.test(language::Compability::semantics::Symbol::Flag::AccDeclare))
    language::Compability::lower::attachDeclarePostDeallocAction(converter, builder, sym);
}

static mlir::Value genCudaDeallocate(fir::FirOpBuilder &builder,
                                     mlir::Location loc,
                                     const fir::MutableBoxValue &box,
                                     ErrorManager &errorManager,
                                     const language::Compability::semantics::Symbol &sym) {
  cuf::DataAttributeAttr cudaAttr =
      language::Compability::lower::translateSymbolCUFDataAttribute(builder.getContext(),
                                                      sym);
  mlir::Value errmsg =
      mlir::isa<fir::AbsentOp>(errorManager.errMsgAddr.getDefiningOp())
          ? nullptr
          : errorManager.errMsgAddr;

  // Keep return type the same as a standard AllocatableAllocate call.
  mlir::Type retTy = fir::runtime::getModel<int>()(builder.getContext());
  return cuf::DeallocateOp::create(
             builder, loc, retTy, box.getAddr(), errmsg, cudaAttr,
             errorManager.hasStatSpec() ? builder.getUnitAttr() : nullptr)
      .getResult();
}

// Generate deallocation of a pointer/allocatable.
static mlir::Value
genDeallocate(fir::FirOpBuilder &builder,
              language::Compability::lower::AbstractConverter &converter, mlir::Location loc,
              const fir::MutableBoxValue &box, ErrorManager &errorManager,
              mlir::Value declaredTypeDesc = {},
              const language::Compability::semantics::Symbol *symbol = nullptr) {
  bool isCudaSymbol = symbol && language::Compability::semantics::HasCUDAAttr(*symbol);
  bool isCudaDeviceContext = cuf::isCUDADeviceContext(builder.getRegion());
  bool inlineDeallocation =
      !box.isDerived() && !box.isPolymorphic() && !box.hasAssumedRank() &&
      !box.isUnlimitedPolymorphic() && !errorManager.hasStatSpec() &&
      !useAllocateRuntime && !box.isPointer();
  // Deallocate intrinsic types inline.
  if (inlineDeallocation &&
      ((isCudaSymbol && isCudaDeviceContext) || !isCudaSymbol)) {
    // Pointers must use PointerDeallocate so that their deallocations
    // can be validated.
    mlir::Value ret = fir::factory::genFreemem(builder, loc, box);
    if (symbol)
      postDeallocationAction(converter, builder, *symbol);
    return ret;
  }
  // Use runtime calls to deallocate descriptor cases. Sync MutableBoxValue
  // with its descriptor before and after calls if needed.
  errorManager.genStatCheck(builder, loc);
  mlir::Value stat;
  if (!isCudaSymbol)
    stat =
        genRuntimeDeallocate(builder, loc, box, errorManager, declaredTypeDesc);
  else
    stat = genCudaDeallocate(builder, loc, box, errorManager, *symbol);
  fir::factory::syncMutableBoxFromIRBox(builder, loc, box);
  if (symbol)
    postDeallocationAction(converter, builder, *symbol);
  errorManager.assignStat(builder, loc, stat);
  return stat;
}

void language::Compability::lower::genDeallocateBox(
    language::Compability::lower::AbstractConverter &converter,
    const fir::MutableBoxValue &box, mlir::Location loc,
    const language::Compability::semantics::Symbol *sym, mlir::Value declaredTypeDesc) {
  const language::Compability::lower::SomeExpr *statExpr = nullptr;
  const language::Compability::lower::SomeExpr *errMsgExpr = nullptr;
  ErrorManager errorManager;
  errorManager.init(converter, loc, statExpr, errMsgExpr);
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  genDeallocate(builder, converter, loc, box, errorManager, declaredTypeDesc,
                sym);
}

void language::Compability::lower::genDeallocateIfAllocated(
    language::Compability::lower::AbstractConverter &converter,
    const fir::MutableBoxValue &box, mlir::Location loc,
    const language::Compability::semantics::Symbol *sym) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  mlir::Value isAllocated =
      fir::factory::genIsAllocatedOrAssociatedTest(builder, loc, box);
  builder.genIfThen(loc, isAllocated)
      .genThen([&]() {
        if (mlir::Type eleType = box.getEleTy();
            mlir::isa<fir::RecordType>(eleType) && box.isPolymorphic()) {
          mlir::Value declaredTypeDesc = fir::TypeDescOp::create(
              builder, loc, mlir::TypeAttr::get(eleType));
          genDeallocateBox(converter, box, loc, sym, declaredTypeDesc);
        } else {
          genDeallocateBox(converter, box, loc, sym);
        }
      })
      .end();
}

void language::Compability::lower::genDeallocateStmt(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::parser::DeallocateStmt &stmt, mlir::Location loc) {
  const language::Compability::lower::SomeExpr *statExpr = nullptr;
  const language::Compability::lower::SomeExpr *errMsgExpr = nullptr;
  for (const language::Compability::parser::StatOrErrmsg &statOrErr :
       std::get<std::list<language::Compability::parser::StatOrErrmsg>>(stmt.t))
    language::Compability::common::visit(
        language::Compability::common::visitors{
            [&](const language::Compability::parser::StatVariable &statVar) {
              statExpr = language::Compability::semantics::GetExpr(statVar);
            },
            [&](const language::Compability::parser::MsgVariable &errMsgVar) {
              errMsgExpr = language::Compability::semantics::GetExpr(errMsgVar);
            },
        },
        statOrErr.u);
  ErrorManager errorManager;
  errorManager.init(converter, loc, statExpr, errMsgExpr);
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  mlir::OpBuilder::InsertPoint insertPt = builder.saveInsertionPoint();
  for (const language::Compability::parser::AllocateObject &allocateObject :
       std::get<std::list<language::Compability::parser::AllocateObject>>(stmt.t)) {
    const language::Compability::semantics::Symbol &symbol = unwrapSymbol(allocateObject);
    fir::MutableBoxValue box =
        genMutableBoxValue(converter, loc, allocateObject);
    mlir::Value declaredTypeDesc = {};
    if (box.isPolymorphic()) {
      mlir::Type eleType = box.getEleTy();
      if (mlir::isa<fir::RecordType>(eleType))
        if (const language::Compability::semantics::DerivedTypeSpec *derivedTypeSpec =
                symbol.GetType()->AsDerived()) {
          declaredTypeDesc =
              language::Compability::lower::getTypeDescAddr(converter, loc, *derivedTypeSpec);
        }
    }
    mlir::Value beginOpValue = genDeallocate(
        builder, converter, loc, box, errorManager, declaredTypeDesc, &symbol);
    preDeallocationAction(converter, builder, beginOpValue, symbol);
  }
  builder.restoreInsertionPoint(insertPt);
}

//===----------------------------------------------------------------------===//
// MutableBoxValue creation implementation
//===----------------------------------------------------------------------===//

/// Is this symbol a pointer to a pointer array that does not have the
/// CONTIGUOUS attribute ?
static inline bool
isNonContiguousArrayPointer(const language::Compability::semantics::Symbol &sym) {
  return language::Compability::semantics::IsPointer(sym) && sym.Rank() != 0 &&
         !sym.attrs().test(language::Compability::semantics::Attr::CONTIGUOUS);
}

/// Is this symbol a polymorphic pointer?
static inline bool isPolymorphicPointer(const language::Compability::semantics::Symbol &sym) {
  return language::Compability::semantics::IsPointer(sym) &&
         language::Compability::semantics::IsPolymorphic(sym);
}

/// Is this symbol a polymorphic allocatable?
static inline bool
isPolymorphicAllocatable(const language::Compability::semantics::Symbol &sym) {
  return language::Compability::semantics::IsAllocatable(sym) &&
         language::Compability::semantics::IsPolymorphic(sym);
}

/// Is this a local procedure symbol in a procedure that contains internal
/// procedures ?
static bool mayBeCapturedInInternalProc(const language::Compability::semantics::Symbol &sym) {
  const language::Compability::semantics::Scope &owner = sym.owner();
  language::Compability::semantics::Scope::Kind kind = owner.kind();
  // Test if this is a procedure scope that contains a subprogram scope that is
  // not an interface.
  if (kind == language::Compability::semantics::Scope::Kind::Subprogram ||
      kind == language::Compability::semantics::Scope::Kind::MainProgram)
    for (const language::Compability::semantics::Scope &childScope : owner.children())
      if (childScope.kind() == language::Compability::semantics::Scope::Kind::Subprogram)
        if (const language::Compability::semantics::Symbol *childSym = childScope.symbol())
          if (const auto *details =
                  childSym->detailsIf<language::Compability::semantics::SubprogramDetails>())
            if (!details->isInterface())
              return true;
  return false;
}

/// In case it is safe to track the properties in variables outside a
/// descriptor, create the variables to hold the mutable properties of the
/// entity var. The variables are not initialized here.
static fir::MutableProperties
createMutableProperties(language::Compability::lower::AbstractConverter &converter,
                        mlir::Location loc,
                        const language::Compability::lower::pft::Variable &var,
                        mlir::ValueRange nonDeferredParams, bool alwaysUseBox) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  const language::Compability::semantics::Symbol &sym = var.getSymbol();
  // Globals and dummies may be associated, creating local variables would
  // require keeping the values and descriptor before and after every single
  // impure calls in the current scope (not only the ones taking the variable as
  // arguments. All.) Volatile means the variable may change in ways not defined
  // per Fortran, so lowering can most likely not keep the descriptor and values
  // in sync as needed.
  // Pointers to non contiguous arrays need to be represented with a fir.box to
  // account for the discontiguity.
  // Pointer/Allocatable in internal procedure are descriptors in the host link,
  // and it would increase complexity to sync this descriptor with the local
  // values every time the host link is escaping.
  if (alwaysUseBox || var.isGlobal() || language::Compability::semantics::IsDummy(sym) ||
      language::Compability::semantics::IsFunctionResult(sym) ||
      sym.attrs().test(language::Compability::semantics::Attr::VOLATILE) ||
      isNonContiguousArrayPointer(sym) || useAllocateRuntime ||
      useDescForMutableBox || mayBeCapturedInInternalProc(sym) ||
      isPolymorphicPointer(sym) || isPolymorphicAllocatable(sym))
    return {};
  fir::MutableProperties mutableProperties;
  std::string name = converter.mangleName(sym);
  mlir::Type baseAddrTy = converter.genType(sym);
  if (auto boxType = mlir::dyn_cast<fir::BaseBoxType>(baseAddrTy))
    baseAddrTy = boxType.getEleTy();
  // Allocate and set a variable to hold the address.
  // It will be set to null in setUnallocatedStatus.
  mutableProperties.addr =
      builder.allocateLocal(loc, baseAddrTy, name + ".addr", "",
                            /*shape=*/{}, /*typeparams=*/{});
  // Allocate variables to hold lower bounds and extents.
  int rank = sym.Rank();
  mlir::Type idxTy = builder.getIndexType();
  for (decltype(rank) i = 0; i < rank; ++i) {
    mlir::Value lboundVar =
        builder.allocateLocal(loc, idxTy, name + ".lb" + std::to_string(i), "",
                              /*shape=*/{}, /*typeparams=*/{});
    mlir::Value extentVar =
        builder.allocateLocal(loc, idxTy, name + ".ext" + std::to_string(i), "",
                              /*shape=*/{}, /*typeparams=*/{});
    mutableProperties.lbounds.emplace_back(lboundVar);
    mutableProperties.extents.emplace_back(extentVar);
  }

  // Allocate variable to hold deferred length parameters.
  mlir::Type eleTy = baseAddrTy;
  if (auto newTy = fir::dyn_cast_ptrEleTy(eleTy))
    eleTy = newTy;
  if (auto seqTy = mlir::dyn_cast<fir::SequenceType>(eleTy))
    eleTy = seqTy.getEleTy();
  if (auto record = mlir::dyn_cast<fir::RecordType>(eleTy))
    if (record.getNumLenParams() != 0)
      TODO(loc, "deferred length type parameters.");
  if (fir::isa_char(eleTy) && nonDeferredParams.empty()) {
    mlir::Value lenVar = builder.allocateLocal(
        loc, builder.getCharacterLengthType(), name + ".len", "", /*shape=*/{},
        /*typeparams=*/{});
    mutableProperties.deferredParams.emplace_back(lenVar);
  }
  return mutableProperties;
}

fir::MutableBoxValue language::Compability::lower::createMutableBox(
    language::Compability::lower::AbstractConverter &converter, mlir::Location loc,
    const language::Compability::lower::pft::Variable &var, mlir::Value boxAddr,
    mlir::ValueRange nonDeferredParams, bool alwaysUseBox, unsigned allocator) {
  fir::MutableProperties mutableProperties = createMutableProperties(
      converter, loc, var, nonDeferredParams, alwaysUseBox);
  fir::MutableBoxValue box(boxAddr, nonDeferredParams, mutableProperties);
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  if (!var.isGlobal() && !language::Compability::semantics::IsDummy(var.getSymbol()))
    fir::factory::disassociateMutableBox(builder, loc, box,
                                         /*polymorphicSetType=*/false,
                                         allocator);
  return box;
}

//===----------------------------------------------------------------------===//
// MutableBoxValue reading interface implementation
//===----------------------------------------------------------------------===//

bool language::Compability::lower::isArraySectionWithoutVectorSubscript(
    const language::Compability::lower::SomeExpr &expr) {
  return expr.Rank() > 0 && language::Compability::evaluate::IsVariable(expr) &&
         !language::Compability::evaluate::UnwrapWholeSymbolDataRef(expr) &&
         !language::Compability::evaluate::HasVectorSubscript(expr);
}

void language::Compability::lower::associateMutableBox(
    language::Compability::lower::AbstractConverter &converter, mlir::Location loc,
    const fir::MutableBoxValue &box, const language::Compability::lower::SomeExpr &source,
    mlir::ValueRange lbounds, language::Compability::lower::StatementContext &stmtCtx) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(source)) {
    fir::factory::disassociateMutableBox(builder, loc, box);
    cuf::genPointerSync(box.getAddr(), builder);
    return;
  }
  if (converter.getLoweringOptions().getLowerToHighLevelFIR()) {
    fir::ExtendedValue rhs = converter.genExprAddr(loc, source, stmtCtx);
    fir::factory::associateMutableBox(builder, loc, box, rhs, lbounds);
    cuf::genPointerSync(box.getAddr(), builder);
    return;
  }
  // The right hand side is not be evaluated into a temp. Array sections can
  // typically be represented as a value of type `!fir.box`. However, an
  // expression that uses vector subscripts cannot be emboxed. In that case,
  // generate a reference to avoid having to later use a fir.rebox to implement
  // the pointer association.
  fir::ExtendedValue rhs = isArraySectionWithoutVectorSubscript(source)
                               ? converter.genExprBox(loc, source, stmtCtx)
                               : converter.genExprAddr(loc, source, stmtCtx);

  fir::factory::associateMutableBox(builder, loc, box, rhs, lbounds);
}

bool language::Compability::lower::isWholeAllocatable(const language::Compability::lower::SomeExpr &expr) {
  if (const language::Compability::semantics::Symbol *sym =
          language::Compability::evaluate::UnwrapWholeSymbolOrComponentDataRef(expr))
    return language::Compability::semantics::IsAllocatable(sym->GetUltimate());
  return false;
}

bool language::Compability::lower::isWholePointer(const language::Compability::lower::SomeExpr &expr) {
  if (const language::Compability::semantics::Symbol *sym =
          language::Compability::evaluate::UnwrapWholeSymbolOrComponentDataRef(expr))
    return language::Compability::semantics::IsPointer(sym->GetUltimate());
  return false;
}

mlir::Value language::Compability::lower::getAssumedCharAllocatableOrPointerLen(
    fir::FirOpBuilder &builder, mlir::Location loc,
    const language::Compability::semantics::Symbol &sym, mlir::Value box) {
  // Read length from fir.box (explicit expr cannot safely be re-evaluated
  // here).
  auto readLength = [&]() {
    fir::BoxValue boxLoad =
        fir::LoadOp::create(builder, loc, fir::getBase(box)).getResult();
    return fir::factory::readCharLen(builder, loc, boxLoad);
  };
  if (language::Compability::semantics::IsOptional(sym)) {
    mlir::IndexType idxTy = builder.getIndexType();
    // It is not safe to unconditionally read boxes of optionals in case
    // they are absents. According to 15.5.2.12 3 (9), it is illegal to
    // inquire the length of absent optional, even if non deferred, so
    // it's fine to use undefOp in this case.
    auto isPresent = fir::IsPresentOp::create(builder, loc, builder.getI1Type(),
                                              fir::getBase(box));
    mlir::Value len =
        builder.genIfOp(loc, {idxTy}, isPresent, true)
            .genThen(
                [&]() { fir::ResultOp::create(builder, loc, readLength()); })
            .genElse([&]() {
              auto undef = fir::UndefOp::create(builder, loc, idxTy);
              fir::ResultOp::create(builder, loc, undef.getResult());
            })
            .getResults()[0];
    return len;
  }

  return readLength();
}

mlir::Value language::Compability::lower::getTypeDescAddr(
    AbstractConverter &converter, mlir::Location loc,
    const language::Compability::semantics::DerivedTypeSpec &typeSpec) {
  mlir::Type typeDesc =
      language::Compability::lower::translateDerivedTypeToFIRType(converter, typeSpec);
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  return fir::TypeDescOp::create(builder, loc, mlir::TypeAttr::get(typeDesc));
}
