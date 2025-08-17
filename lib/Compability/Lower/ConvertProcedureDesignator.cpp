//===- ConvertProcedureDesignator.cpp -- Procedure Designator ---*- C++ -*-===//
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

#include "language/Compability/Lower/ConvertProcedureDesignator.h"
#include "language/Compability/Evaluate/intrinsics.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/CallInterface.h"
#include "language/Compability/Lower/ConvertCall.h"
#include "language/Compability/Lower/ConvertExprToHLFIR.h"
#include "language/Compability/Lower/ConvertVariable.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Lower/SymbolMap.h"
#include "language/Compability/Optimizer/Builder/Character.h"
#include "language/Compability/Optimizer/Builder/IntrinsicCall.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"

static bool areAllSymbolsInExprMapped(const language::Compability::evaluate::ExtentExpr &expr,
                                      language::Compability::lower::SymMap &symMap) {
  for (const auto &sym : language::Compability::evaluate::CollectSymbols(expr))
    if (!symMap.lookupSymbol(sym))
      return false;
  return true;
}

fir::ExtendedValue language::Compability::lower::convertProcedureDesignator(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::ProcedureDesignator &proc,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();

  if (const language::Compability::evaluate::SpecificIntrinsic *intrinsic =
          proc.GetSpecificIntrinsic()) {
    mlir::FunctionType signature =
        language::Compability::lower::translateSignature(proc, converter);
    // Intrinsic lowering is based on the generic name, so retrieve it here in
    // case it is different from the specific name. The type of the specific
    // intrinsic is retained in the signature.
    std::string genericName =
        converter.getFoldingContext().intrinsics().GetGenericIntrinsicName(
            intrinsic->name);
    mlir::SymbolRefAttr symbolRefAttr =
        fir::getUnrestrictedIntrinsicSymbolRefAttr(builder, loc, genericName,
                                                   signature);
    mlir::Value funcPtr =
        fir::AddrOfOp::create(builder, loc, signature, symbolRefAttr);
    return funcPtr;
  }
  const language::Compability::semantics::Symbol *symbol = proc.GetSymbol();
  assert(symbol && "expected symbol in ProcedureDesignator");
  mlir::Value funcPtr;
  mlir::Value funcPtrResultLength;
  if (language::Compability::semantics::IsDummy(*symbol)) {
    language::Compability::lower::SymbolBox val = symMap.lookupSymbol(*symbol);
    assert(val && "Dummy procedure not in symbol map");
    funcPtr = val.getAddr();
    if (fir::isCharacterProcedureTuple(funcPtr.getType(),
                                       /*acceptRawFunc=*/false))
      std::tie(funcPtr, funcPtrResultLength) =
          fir::factory::extractCharacterProcedureTuple(builder, loc, funcPtr);
  } else {
    mlir::func::FuncOp func =
        language::Compability::lower::getOrDeclareFunction(proc, converter);
    mlir::SymbolRefAttr nameAttr = builder.getSymbolRefAttr(func.getSymName());
    funcPtr =
        fir::AddrOfOp::create(builder, loc, func.getFunctionType(), nameAttr);
  }
  if (language::Compability::lower::mustPassLengthWithDummyProcedure(proc, converter)) {
    // The result length, if available here, must be propagated along the
    // procedure address so that call sites where the result length is assumed
    // can retrieve the length.
    language::Compability::evaluate::DynamicType resultType = proc.GetType().value();
    if (const auto &lengthExpr = resultType.GetCharLength()) {
      // The length expression may refer to dummy argument symbols that are
      // meaningless without any actual arguments. Leave the length as
      // unknown in that case, it be resolved on the call site
      // with the actual arguments.
      if (areAllSymbolsInExprMapped(*lengthExpr, symMap)) {
        mlir::Value rawLen = fir::getBase(
            converter.genExprValue(toEvExpr(*lengthExpr), stmtCtx));
        // F2018 7.4.4.2 point 5.
        funcPtrResultLength =
            fir::factory::genMaxWithZero(builder, loc, rawLen);
      }
    }
    // The caller of the function pointer will have to allocate
    // the function result with the character length specified
    // by the boxed value. If the result length cannot be
    // computed statically, set it to zero (we used to use -1,
    // but this could cause assertions in LLVM after inlining
    // exposed alloca of size -1).
    if (!funcPtrResultLength)
      funcPtrResultLength = builder.createIntegerConstant(
          loc, builder.getCharacterLengthType(), 0);
    return fir::CharBoxValue{funcPtr, funcPtrResultLength};
  }
  return funcPtr;
}

static hlfir::EntityWithAttributes designateProcedurePointerComponent(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::Symbol &procComponentSym, mlir::Value base,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  fir::FortranVariableFlagsAttr attributes =
      language::Compability::lower::translateSymbolAttributes(builder.getContext(),
                                                procComponentSym);
  /// Passed argument may be a descriptor. This is a scalar reference, so the
  /// base address can be directly addressed.
  if (mlir::isa<fir::BaseBoxType>(base.getType()))
    base = fir::BoxAddrOp::create(builder, loc, base);
  std::string fieldName = converter.getRecordTypeFieldName(procComponentSym);
  auto recordType =
      mlir::cast<fir::RecordType>(hlfir::getFortranElementType(base.getType()));
  mlir::Type fieldType = recordType.getType(fieldName);
  // Note: semantics turns x%p() into x%t%p() when the procedure pointer
  // component is part of parent component t.
  if (!fieldType)
    TODO(loc, "passing type bound procedure (extension)");
  mlir::Type designatorType = fir::ReferenceType::get(fieldType);
  mlir::Value compRef = hlfir::DesignateOp::create(
      builder, loc, designatorType, base, fieldName,
      /*compShape=*/mlir::Value{}, hlfir::DesignateOp::Subscripts{},
      /*substring=*/mlir::ValueRange{},
      /*complexPart=*/std::nullopt,
      /*shape=*/mlir::Value{}, /*typeParams=*/mlir::ValueRange{}, attributes);
  return hlfir::EntityWithAttributes{compRef};
}

static hlfir::EntityWithAttributes convertProcedurePointerComponent(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::Component &procComponent,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx) {
  fir::ExtendedValue baseExv = language::Compability::lower::convertDataRefToValue(
      loc, converter, procComponent.base(), symMap, stmtCtx);
  mlir::Value base = fir::getBase(baseExv);
  const language::Compability::semantics::Symbol &procComponentSym =
      procComponent.GetLastSymbol();
  return designateProcedurePointerComponent(loc, converter, procComponentSym,
                                            base, symMap, stmtCtx);
}

hlfir::EntityWithAttributes language::Compability::lower::convertProcedureDesignatorToHLFIR(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::ProcedureDesignator &proc,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx) {
  const auto *sym = proc.GetSymbol();
  if (sym) {
    if (sym->GetUltimate().attrs().test(language::Compability::semantics::Attr::INTRINSIC))
      TODO(loc, "Procedure pointer with intrinsic target.");
    if (std::optional<fir::FortranVariableOpInterface> varDef =
            symMap.lookupVariableDefinition(*sym))
      return *varDef;
  }

  if (const language::Compability::evaluate::Component *procComponent = proc.GetComponent())
    return convertProcedurePointerComponent(loc, converter, *procComponent,
                                            symMap, stmtCtx);

  fir::ExtendedValue procExv =
      convertProcedureDesignator(loc, converter, proc, symMap, stmtCtx);
  // Directly package the procedure address as a fir.boxproc or
  // tuple<fir.boxbroc, len> so that it can be returned as a single mlir::Value.
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();

  mlir::Value funcAddr = fir::getBase(procExv);
  if (!mlir::isa<fir::BoxProcType>(funcAddr.getType())) {
    mlir::Type boxTy =
        language::Compability::lower::getUntypedBoxProcType(&converter.getMLIRContext());
    if (auto host = language::Compability::lower::argumentHostAssocs(converter, funcAddr))
      funcAddr = fir::EmboxProcOp::create(
          builder, loc, boxTy, toolchain::ArrayRef<mlir::Value>{funcAddr, host});
    else
      funcAddr = fir::EmboxProcOp::create(builder, loc, boxTy, funcAddr);
  }

  mlir::Value res = procExv.match(
      [&](const fir::CharBoxValue &box) -> mlir::Value {
        mlir::Type tupleTy =
            fir::factory::getCharacterProcedureTupleType(funcAddr.getType());
        return fir::factory::createCharacterProcedureTuple(
            builder, loc, tupleTy, funcAddr, box.getLen());
      },
      [funcAddr](const auto &) { return funcAddr; });
  return hlfir::EntityWithAttributes{res};
}

mlir::Value language::Compability::lower::convertProcedureDesignatorInitialTarget(
    language::Compability::lower::AbstractConverter &converter, mlir::Location loc,
    const language::Compability::semantics::Symbol &sym) {
  language::Compability::lower::SymMap globalOpSymMap;
  language::Compability::lower::StatementContext stmtCtx;
  language::Compability::evaluate::ProcedureDesignator proc(sym);
  auto procVal{language::Compability::lower::convertProcedureDesignatorToHLFIR(
      loc, converter, proc, globalOpSymMap, stmtCtx)};
  return fir::getBase(language::Compability::lower::convertToAddress(
      loc, converter, procVal, stmtCtx, procVal.getType()));
}

mlir::Value language::Compability::lower::derefPassProcPointerComponent(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::ProcedureDesignator &proc, mlir::Value passedArg,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx) {
  const language::Compability::semantics::Symbol *procComponentSym = proc.GetSymbol();
  assert(procComponentSym &&
         "failed to retrieve pointer procedure component symbol");
  hlfir::EntityWithAttributes pointerComp = designateProcedurePointerComponent(
      loc, converter, *procComponentSym, passedArg, symMap, stmtCtx);
  return fir::LoadOp::create(converter.getFirOpBuilder(), loc, pointerComp);
}
