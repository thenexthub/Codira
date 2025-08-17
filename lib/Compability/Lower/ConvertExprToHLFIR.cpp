//===-- ConvertExprToHLFIR.cpp --------------------------------------------===//
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

#include "language/Compability/Lower/ConvertExprToHLFIR.h"
#include "language/Compability/Evaluate/shape.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/Allocatable.h"
#include "language/Compability/Lower/CallInterface.h"
#include "language/Compability/Lower/ConvertArrayConstructor.h"
#include "language/Compability/Lower/ConvertCall.h"
#include "language/Compability/Lower/ConvertConstant.h"
#include "language/Compability/Lower/ConvertProcedureDesignator.h"
#include "language/Compability/Lower/ConvertType.h"
#include "language/Compability/Lower/ConvertVariable.h"
#include "language/Compability/Lower/StatementContext.h"
#include "language/Compability/Lower/SymbolMap.h"
#include "language/Compability/Optimizer/Builder/Complex.h"
#include "language/Compability/Optimizer/Builder/IntrinsicCall.h"
#include "language/Compability/Optimizer/Builder/MutableBox.h"
#include "language/Compability/Optimizer/Builder/Runtime/Character.h"
#include "language/Compability/Optimizer/Builder/Runtime/Derived.h"
#include "language/Compability/Optimizer/Builder/Runtime/Pointer.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "mlir/IR/IRMapping.h"
#include "toolchain/ADT/TypeSwitch.h"
#include <optional>

namespace {

/// Lower Designators to HLFIR.
class HlfirDesignatorBuilder {
private:
  /// Internal entry point on the rightest part of a evaluate::Designator.
  template <typename T>
  hlfir::EntityWithAttributes
  genLeafPartRef(const T &designatorNode,
                 bool vectorSubscriptDesignatorToValue) {
    hlfir::EntityWithAttributes result = gen(designatorNode);
    if (vectorSubscriptDesignatorToValue)
      return turnVectorSubscriptedDesignatorIntoValue(result);
    return result;
  }

  hlfir::EntityWithAttributes
  genDesignatorExpr(const language::Compability::lower::SomeExpr &designatorExpr,
                    bool vectorSubscriptDesignatorToValue = true);

public:
  HlfirDesignatorBuilder(mlir::Location loc,
                         language::Compability::lower::AbstractConverter &converter,
                         language::Compability::lower::SymMap &symMap,
                         language::Compability::lower::StatementContext &stmtCtx)
      : converter{converter}, symMap{symMap}, stmtCtx{stmtCtx}, loc{loc} {}

  /// Public entry points to lower a Designator<T> (given its .u member, to
  /// avoid the template arguments which does not matter here).
  /// This lowers a designator to an hlfir variable SSA value (that can be
  /// assigned to), except for vector subscripted designators that are
  /// lowered by default to hlfir.expr value since they cannot be
  /// represented as HLFIR variable SSA values.

  // Character designators variant contains substrings
  using CharacterDesignators =
      decltype(language::Compability::evaluate::Designator<language::Compability::evaluate::Type<
                   language::Compability::evaluate::TypeCategory::Character, 1>>::u);
  hlfir::EntityWithAttributes
  gen(const CharacterDesignators &designatorVariant,
      bool vectorSubscriptDesignatorToValue = true) {
    return language::Compability::common::visit(
        [&](const auto &x) -> hlfir::EntityWithAttributes {
          return genLeafPartRef(x, vectorSubscriptDesignatorToValue);
        },
        designatorVariant);
  }
  // Character designators variant contains complex parts
  using RealDesignators =
      decltype(language::Compability::evaluate::Designator<language::Compability::evaluate::Type<
                   language::Compability::evaluate::TypeCategory::Real, 4>>::u);
  hlfir::EntityWithAttributes
  gen(const RealDesignators &designatorVariant,
      bool vectorSubscriptDesignatorToValue = true) {
    return language::Compability::common::visit(
        [&](const auto &x) -> hlfir::EntityWithAttributes {
          return genLeafPartRef(x, vectorSubscriptDesignatorToValue);
        },
        designatorVariant);
  }
  // All other designators are similar
  using OtherDesignators =
      decltype(language::Compability::evaluate::Designator<language::Compability::evaluate::Type<
                   language::Compability::evaluate::TypeCategory::Integer, 4>>::u);
  hlfir::EntityWithAttributes
  gen(const OtherDesignators &designatorVariant,
      bool vectorSubscriptDesignatorToValue = true) {
    return language::Compability::common::visit(
        [&](const auto &x) -> hlfir::EntityWithAttributes {
          return genLeafPartRef(x, vectorSubscriptDesignatorToValue);
        },
        designatorVariant);
  }

  hlfir::EntityWithAttributes
  genNamedEntity(const language::Compability::evaluate::NamedEntity &namedEntity,
                 bool vectorSubscriptDesignatorToValue = true) {
    if (namedEntity.IsSymbol())
      return genLeafPartRef(
          language::Compability::evaluate::SymbolRef{namedEntity.GetLastSymbol()},
          vectorSubscriptDesignatorToValue);
    return genLeafPartRef(namedEntity.GetComponent(),
                          vectorSubscriptDesignatorToValue);
  }

  /// Public entry point to lower a vector subscripted designator to
  /// an hlfir::ElementalAddrOp.
  hlfir::ElementalAddrOp convertVectorSubscriptedExprToElementalAddr(
      const language::Compability::lower::SomeExpr &designatorExpr);

  std::tuple<mlir::Type, fir::FortranVariableFlagsEnum>
  genComponentDesignatorTypeAndAttributes(
      const language::Compability::semantics::Symbol &componentSym, mlir::Type fieldType,
      bool isVolatile) {
    if (mayHaveNonDefaultLowerBounds(componentSym)) {
      mlir::Type boxType = fir::BoxType::get(fieldType, isVolatile);
      return std::make_tuple(boxType,
                             fir::FortranVariableFlagsEnum::contiguous);
    }
    auto refType = fir::ReferenceType::get(fieldType, isVolatile);
    return std::make_tuple(refType, fir::FortranVariableFlagsEnum{});
  }

  mlir::Value genComponentShape(const language::Compability::semantics::Symbol &componentSym,
                                mlir::Type fieldType) {
    // For pointers and allocatable components, the
    // shape is deferred and should not be loaded now to preserve
    // pointer/allocatable aspects.
    if (componentSym.Rank() == 0 ||
        language::Compability::semantics::IsAllocatableOrObjectPointer(&componentSym) ||
        language::Compability::semantics::IsProcedurePointer(&componentSym))
      return mlir::Value{};

    fir::FirOpBuilder &builder = getBuilder();
    mlir::Location loc = getLoc();
    mlir::Type idxTy = builder.getIndexType();
    toolchain::SmallVector<mlir::Value> extents;
    auto seqTy = mlir::cast<fir::SequenceType>(
        hlfir::getFortranElementOrSequenceType(fieldType));
    for (auto extent : seqTy.getShape()) {
      if (extent == fir::SequenceType::getUnknownExtent()) {
        // We have already generated invalid hlfir.declare
        // without the type parameters and probably invalid storage
        // for the variable (e.g. fir.alloca without type parameters).
        // So this TODO here is a little bit late, but it matches
        // the non-HLFIR path.
        TODO(loc, "array component shape depending on length parameters");
      }
      extents.push_back(builder.createIntegerConstant(loc, idxTy, extent));
    }
    if (!mayHaveNonDefaultLowerBounds(componentSym))
      return fir::ShapeOp::create(builder, loc, extents);

    toolchain::SmallVector<mlir::Value> lbounds;
    if (const auto *objDetails =
            componentSym.detailsIf<language::Compability::semantics::ObjectEntityDetails>())
      for (const language::Compability::semantics::ShapeSpec &bounds : objDetails->shape())
        if (auto lb = bounds.lbound().GetExplicit())
          if (auto constant = language::Compability::evaluate::ToInt64(*lb))
            lbounds.push_back(
                builder.createIntegerConstant(loc, idxTy, *constant));
    assert(extents.size() == lbounds.size() &&
           "extents and lower bounds must match");
    return builder.genShape(loc, lbounds, extents);
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::evaluate::DataRef &dataRef) {
    return language::Compability::common::visit(
        language::Compability::common::visitors{[&](const auto &x) { return gen(x); }},
        dataRef.u);
  }

private:
  /// Struct that is filled while visiting a part-ref (in the "visit" member
  /// function) before the top level "gen" generates an hlfir.declare for the
  /// part ref. It contains the lowered pieces of the part-ref that will
  /// become the operands of an hlfir.declare.
  struct PartInfo {
    std::optional<hlfir::Entity> base;
    std::string componentName{};
    mlir::Value componentShape;
    hlfir::DesignateOp::Subscripts subscripts;
    std::optional<bool> complexPart;
    mlir::Value resultShape;
    toolchain::SmallVector<mlir::Value> typeParams;
    toolchain::SmallVector<mlir::Value, 2> substring;
  };

  // Given the value type of a designator (T or fir.array<T>) and the front-end
  // node for the designator, compute the memory type (fir.class, fir.ref, or
  // fir.box)...
  template <typename T>
  mlir::Type computeDesignatorType(mlir::Type resultValueType,
                                   PartInfo &partInfo,
                                   const T &designatorNode) {
    // Get base's shape if its a sequence type with no previously computed
    // result shape
    if (partInfo.base && mlir::isa<fir::SequenceType>(resultValueType) &&
        !partInfo.resultShape)
      partInfo.resultShape =
          hlfir::genShape(getLoc(), getBuilder(), *partInfo.base);

    // Enable volatility on the designatory type if it has the VOLATILE
    // attribute or if the base is volatile.
    bool isVolatile = false;

    // Check if this should be a volatile reference
    if constexpr (std::is_same_v<std::decay_t<T>,
                                 language::Compability::evaluate::SymbolRef>) {
      if (designatorNode.get().GetUltimate().attrs().test(
              language::Compability::semantics::Attr::VOLATILE))
        isVolatile = true;
    } else if constexpr (std::is_same_v<std::decay_t<T>,
                                        language::Compability::evaluate::ArrayRef>) {
      if (designatorNode.base().GetLastSymbol().attrs().test(
              language::Compability::semantics::Attr::VOLATILE))
        isVolatile = true;
    } else if constexpr (std::is_same_v<std::decay_t<T>,
                                        language::Compability::evaluate::Component>) {
      if (designatorNode.GetLastSymbol().attrs().test(
              language::Compability::semantics::Attr::VOLATILE))
        isVolatile = true;
    }

    // Check if the base type is volatile
    if (partInfo.base.has_value()) {
      mlir::Type baseType = partInfo.base.value().getType();
      isVolatile = isVolatile || fir::isa_volatile_type(baseType);
    }

    // Dynamic type of polymorphic base must be kept if the designator is
    // polymorphic.
    if (isPolymorphic(designatorNode))
      return fir::ClassType::get(resultValueType, isVolatile);

    // Character scalar with dynamic length needs a fir.boxchar to hold the
    // designator length.
    auto charType = mlir::dyn_cast<fir::CharacterType>(resultValueType);
    if (charType && charType.hasDynamicLen())
      return fir::BoxCharType::get(charType.getContext(), charType.getFKind());

    // Arrays with non default lower bounds or dynamic length or dynamic extent
    // need a fir.box to hold the dynamic or lower bound information.
    if (fir::hasDynamicSize(resultValueType) ||
        mayHaveNonDefaultLowerBounds(partInfo))
      return fir::BoxType::get(resultValueType, isVolatile);

    // Non simply contiguous ref require a fir.box to carry the byte stride.
    if (mlir::isa<fir::SequenceType>(resultValueType) &&
        !language::Compability::evaluate::IsSimplyContiguous(
            designatorNode, getConverter().getFoldingContext(),
            /*namedConstantSectionsAreAlwaysContiguous=*/false))
      return fir::BoxType::get(resultValueType, isVolatile);

    // Other designators can be handled as raw addresses.
    return fir::ReferenceType::get(resultValueType, isVolatile);
  }

  template <typename T>
  static bool isPolymorphic(const T &designatorNode) {
    if constexpr (!std::is_same_v<T, language::Compability::evaluate::Substring>) {
      return language::Compability::semantics::IsPolymorphic(designatorNode.GetLastSymbol());
    }
    return false;
  }

  template <typename T>
  /// Generate an hlfir.designate for a part-ref given a filled PartInfo and the
  /// FIR type for this part-ref.
  fir::FortranVariableOpInterface genDesignate(mlir::Type resultValueType,
                                               PartInfo &partInfo,
                                               const T &designatorNode) {
    mlir::Type designatorType =
        computeDesignatorType(resultValueType, partInfo, designatorNode);
    return genDesignate(designatorType, partInfo, /*attributes=*/{});
  }
  fir::FortranVariableOpInterface
  genDesignate(mlir::Type designatorType, PartInfo &partInfo,
               fir::FortranVariableFlagsAttr attributes) {
    fir::FirOpBuilder &builder = getBuilder();
    // Once a part with vector subscripts has been lowered, the following
    // hlfir.designator (for the parts on the right of the designator) must
    // be lowered inside the hlfir.elemental_addr because they depend on the
    // hlfir.elemental_addr indices.
    // All the subsequent Fortran indices however, should be lowered before
    // the hlfir.elemental_addr because they should only be evaluated once,
    // hence, the insertion point is restored outside of the
    // hlfir.elemental_addr after generating the hlfir.designate. Example: in
    // "X(VECTOR)%COMP(FOO(), BAR())", the calls to bar() and foo() must be
    // generated outside of the hlfir.elemental, but the related hlfir.designate
    // that depends on the scalar hlfir.designate of X(VECTOR) that was
    // generated inside the hlfir.elemental_addr should be generated in the
    // hlfir.elemental_addr.
    if (auto elementalAddrOp = getVectorSubscriptElementAddrOp())
      builder.setInsertionPointToEnd(&elementalAddrOp->getBody().front());
    auto designate = hlfir::DesignateOp::create(
        builder, getLoc(), designatorType, partInfo.base.value().getBase(),
        partInfo.componentName, partInfo.componentShape, partInfo.subscripts,
        partInfo.substring, partInfo.complexPart, partInfo.resultShape,
        partInfo.typeParams, attributes);
    if (auto elementalAddrOp = getVectorSubscriptElementAddrOp())
      builder.setInsertionPoint(*elementalAddrOp);
    return mlir::cast<fir::FortranVariableOpInterface>(
        designate.getOperation());
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::evaluate::SymbolRef &symbolRef) {
    if (std::optional<fir::FortranVariableOpInterface> varDef =
            getSymMap().lookupVariableDefinition(symbolRef)) {
      if (symbolRef.get().GetUltimate().test(
              language::Compability::semantics::Symbol::Flag::CrayPointee)) {
        // The pointee is represented with a descriptor inheriting
        // the shape and type parameters of the pointee.
        // We have to update the base_addr to point to the current
        // value of the Cray pointer variable.
        fir::FirOpBuilder &builder = getBuilder();
        fir::FortranVariableOpInterface ptrVar =
            gen(language::Compability::semantics::GetCrayPointer(symbolRef));
        mlir::Value ptrAddr = ptrVar.getBase();

        // Reinterpret the reference to a Cray pointer so that
        // we have a pointer-compatible value after loading
        // the Cray pointer value.
        mlir::Type refPtrType = builder.getRefType(
            fir::PointerType::get(fir::dyn_cast_ptrEleTy(ptrAddr.getType())));
        mlir::Value cast = builder.createConvert(loc, refPtrType, ptrAddr);
        mlir::Value ptrVal = fir::LoadOp::create(builder, loc, cast);

        // Update the base_addr to the value of the Cray pointer.
        // This is a hacky way to do the update, and it may harm
        // performance around Cray pointer references.
        // TODO: we should introduce an operation that updates
        // just the base_addr of the given box. The CodeGen
        // will just convert it into a single store.
        fir::runtime::genPointerAssociateScalar(builder, loc, varDef->getBase(),
                                                ptrVal);
      }
      return *varDef;
    }
    toolchain::errs() << *symbolRef << "\n";
    TODO(getLoc(), "lowering symbol to HLFIR");
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::semantics::Symbol &symbol) {
    language::Compability::evaluate::SymbolRef symref{symbol};
    return gen(symref);
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::evaluate::Component &component) {
    if (language::Compability::semantics::IsAllocatableOrPointer(component.GetLastSymbol()))
      return genWholeAllocatableOrPointerComponent(component);
    PartInfo partInfo;
    mlir::Type resultType = visit(component, partInfo);
    return genDesignate(resultType, partInfo, component);
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::evaluate::ArrayRef &arrayRef) {
    PartInfo partInfo;
    mlir::Type resultType = visit(arrayRef, partInfo);
    return genDesignate(resultType, partInfo, arrayRef);
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::evaluate::CoarrayRef &coarrayRef) {
    TODO(getLoc(), "coarray: lowering a reference to a coarray object");
  }

  mlir::Type visit(const language::Compability::evaluate::CoarrayRef &, PartInfo &) {
    TODO(getLoc(), "coarray: lowering a reference to a coarray object");
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::evaluate::ComplexPart &complexPart) {
    PartInfo partInfo;
    fir::factory::Complex cmplxHelper(getBuilder(), getLoc());

    bool complexBit =
        complexPart.part() == language::Compability::evaluate::ComplexPart::Part::IM;
    partInfo.complexPart = {complexBit};

    mlir::Type resultType = visit(complexPart.complex(), partInfo);

    // Determine complex part type
    mlir::Type base = hlfir::getFortranElementType(resultType);
    mlir::Type cmplxValueType = cmplxHelper.getComplexPartType(base);
    mlir::Type designatorType = changeElementType(resultType, cmplxValueType);

    return genDesignate(designatorType, partInfo, complexPart);
  }

  fir::FortranVariableOpInterface
  gen(const language::Compability::evaluate::Substring &substring) {
    PartInfo partInfo;
    mlir::Type baseStringType = language::Compability::common::visit(
        [&](const auto &x) { return visit(x, partInfo); }, substring.parent());
    assert(partInfo.typeParams.size() == 1 && "expect base string length");
    // Compute the substring lower and upper bound.
    partInfo.substring.push_back(genSubscript(substring.lower()));
    if (language::Compability::evaluate::MaybeExtentExpr upperBound = substring.upper())
      partInfo.substring.push_back(genSubscript(*upperBound));
    else
      partInfo.substring.push_back(partInfo.typeParams[0]);
    fir::FirOpBuilder &builder = getBuilder();
    mlir::Location loc = getLoc();
    mlir::Type idxTy = builder.getIndexType();
    partInfo.substring[0] =
        builder.createConvert(loc, idxTy, partInfo.substring[0]);
    partInfo.substring[1] =
        builder.createConvert(loc, idxTy, partInfo.substring[1]);
    // Try using constant length if available. mlir::arith folding would
    // most likely be able to fold "max(ub-lb+1,0)" too, but getting
    // the constant length in the FIR types would be harder.
    std::optional<int64_t> cstLen =
        language::Compability::evaluate::ToInt64(language::Compability::evaluate::Fold(
            getConverter().getFoldingContext(), substring.LEN()));
    if (cstLen) {
      partInfo.typeParams[0] =
          builder.createIntegerConstant(loc, idxTy, *cstLen);
    } else {
      // Compute "len = max(ub-lb+1,0)" (Fortran 2018 9.4.1).
      mlir::Value one = builder.createIntegerConstant(loc, idxTy, 1);
      auto boundsDiff = mlir::arith::SubIOp::create(
          builder, loc, partInfo.substring[1], partInfo.substring[0]);
      auto rawLen = mlir::arith::AddIOp::create(builder, loc, boundsDiff, one);
      partInfo.typeParams[0] =
          fir::factory::genMaxWithZero(builder, loc, rawLen);
    }
    auto kind = mlir::cast<fir::CharacterType>(
                    hlfir::getFortranElementType(baseStringType))
                    .getFKind();
    auto newCharTy = fir::CharacterType::get(
        baseStringType.getContext(), kind,
        cstLen ? *cstLen : fir::CharacterType::unknownLen());
    mlir::Type resultType = changeElementType(baseStringType, newCharTy);
    return genDesignate(resultType, partInfo, substring);
  }

  static mlir::Type changeElementType(mlir::Type type, mlir::Type newEleTy) {
    return toolchain::TypeSwitch<mlir::Type, mlir::Type>(type)
        .Case<fir::SequenceType>([&](fir::SequenceType seqTy) -> mlir::Type {
          return fir::SequenceType::get(seqTy.getShape(), newEleTy);
        })
        .Case<fir::ReferenceType, fir::BoxType, fir::ClassType>(
            [&](auto t) -> mlir::Type {
              using FIRT = decltype(t);
              return FIRT::get(changeElementType(t.getEleTy(), newEleTy),
                               t.isVolatile());
            })
        .Case<fir::PointerType, fir::HeapType>([&](auto t) -> mlir::Type {
          using FIRT = decltype(t);
          return FIRT::get(changeElementType(t.getEleTy(), newEleTy));
        })
        .Default([newEleTy](mlir::Type t) -> mlir::Type { return newEleTy; });
  }

  fir::FortranVariableOpInterface genWholeAllocatableOrPointerComponent(
      const language::Compability::evaluate::Component &component) {
    // Generate whole allocatable or pointer component reference. The
    // hlfir.designate result will be a pointer/allocatable.
    PartInfo partInfo;
    mlir::Type componentType = visitComponentImpl(component, partInfo).second;
    const auto isVolatile =
        fir::isa_volatile_type(partInfo.base.value().getBase().getType());
    mlir::Type designatorType =
        fir::ReferenceType::get(componentType, isVolatile);
    fir::FortranVariableFlagsAttr attributes =
        language::Compability::lower::translateSymbolAttributes(getBuilder().getContext(),
                                                  component.GetLastSymbol());
    return genDesignate(designatorType, partInfo, attributes);
  }

  mlir::Type visit(const language::Compability::evaluate::DataRef &dataRef,
                   PartInfo &partInfo) {
    return language::Compability::common::visit(
        [&](const auto &x) { return visit(x, partInfo); }, dataRef.u);
  }

  mlir::Type
  visit(const language::Compability::evaluate::StaticDataObject::Pointer &staticObject,
        PartInfo &partInfo) {
    fir::FirOpBuilder &builder = getBuilder();
    mlir::Location loc = getLoc();
    std::optional<std::string> string = staticObject->AsString();
    // TODO: see if StaticDataObject can be replaced by something based on
    // Constant<T> to avoid dealing with endianness here for KIND>1.
    // This will also avoid making string copies here.
    if (!string)
      TODO(loc, "StaticDataObject::Pointer substring with kind > 1");
    fir::ExtendedValue exv =
        fir::factory::createStringLiteral(builder, getLoc(), *string);
    auto flags = fir::FortranVariableFlagsAttr::get(
        builder.getContext(), fir::FortranVariableFlagsEnum::parameter);
    partInfo.base = hlfir::genDeclare(loc, builder, exv, ".stringlit", flags);
    partInfo.typeParams.push_back(fir::getLen(exv));
    return partInfo.base->getElementOrSequenceType();
  }

  mlir::Type visit(const language::Compability::evaluate::SymbolRef &symbolRef,
                   PartInfo &partInfo) {
    // A symbol is only visited if there is a following array, substring, or
    // complex reference. If the entity is a pointer or allocatable, this
    // reference designates the target, so the pointer, allocatable must be
    // dereferenced here.
    partInfo.base =
        hlfir::derefPointersAndAllocatables(loc, getBuilder(), gen(symbolRef));
    hlfir::genLengthParameters(loc, getBuilder(), *partInfo.base,
                               partInfo.typeParams);
    return partInfo.base->getElementOrSequenceType();
  }

  mlir::Type visit(const language::Compability::evaluate::ArrayRef &arrayRef,
                   PartInfo &partInfo) {
    mlir::Type baseType;
    if (const auto *component = arrayRef.base().UnwrapComponent()) {
      // Pointers and allocatable components must be dereferenced since the
      // array ref designates the target (this is done in "visit"). Other
      // components need special care to deal with the array%array_comp(indices)
      // case.
      if (language::Compability::semantics::IsAllocatableOrObjectPointer(
              &component->GetLastSymbol()))
        baseType = visit(*component, partInfo);
      else
        baseType = hlfir::getFortranElementOrSequenceType(
            visitComponentImpl(*component, partInfo).second);
    } else {
      baseType = visit(arrayRef.base().GetLastSymbol(), partInfo);
    }

    fir::FirOpBuilder &builder = getBuilder();
    mlir::Location loc = getLoc();
    mlir::Type idxTy = builder.getIndexType();
    toolchain::SmallVector<std::pair<mlir::Value, mlir::Value>> bounds;
    auto getBaseBounds = [&](unsigned i) {
      if (bounds.empty()) {
        if (partInfo.componentName.empty()) {
          bounds = hlfir::genBounds(loc, builder, partInfo.base.value());
        } else {
          assert(
              partInfo.componentShape &&
              "implicit array section bounds must come from component shape");
          bounds = hlfir::genBounds(loc, builder, partInfo.componentShape);
        }
        assert(!bounds.empty() &&
               "failed to compute implicit array section bounds");
      }
      return bounds[i];
    };
    auto frontEndResultShape =
        language::Compability::evaluate::GetShape(converter.getFoldingContext(), arrayRef);
    auto tryGettingExtentFromFrontEnd =
        [&](unsigned dim) -> std::pair<mlir::Value, fir::SequenceType::Extent> {
      // Use constant extent if possible. The main advantage to do this now
      // is to get the best FIR array types as possible while lowering.
      if (frontEndResultShape)
        if (auto maybeI64 =
                language::Compability::evaluate::ToInt64(frontEndResultShape->at(dim)))
          return {builder.createIntegerConstant(loc, idxTy, *maybeI64),
                  *maybeI64};
      return {mlir::Value{}, fir::SequenceType::getUnknownExtent()};
    };
    toolchain::SmallVector<mlir::Value> resultExtents;
    fir::SequenceType::Shape resultTypeShape;
    bool sawVectorSubscripts = false;
    for (auto subscript : toolchain::enumerate(arrayRef.subscript())) {
      if (const auto *triplet =
              std::get_if<language::Compability::evaluate::Triplet>(&subscript.value().u)) {
        mlir::Value lb, ub;
        if (const auto &lbExpr = triplet->lower())
          lb = genSubscript(*lbExpr);
        else
          lb = getBaseBounds(subscript.index()).first;
        if (const auto &ubExpr = triplet->upper())
          ub = genSubscript(*ubExpr);
        else
          ub = getBaseBounds(subscript.index()).second;
        lb = builder.createConvert(loc, idxTy, lb);
        ub = builder.createConvert(loc, idxTy, ub);
        mlir::Value stride = genSubscript(triplet->stride());
        stride = builder.createConvert(loc, idxTy, stride);
        auto [extentValue, shapeExtent] =
            tryGettingExtentFromFrontEnd(resultExtents.size());
        resultTypeShape.push_back(shapeExtent);
        if (!extentValue)
          extentValue =
              builder.genExtentFromTriplet(loc, lb, ub, stride, idxTy);
        resultExtents.push_back(extentValue);
        partInfo.subscripts.emplace_back(
            hlfir::DesignateOp::Triplet{lb, ub, stride});
      } else {
        const auto &expr =
            std::get<language::Compability::evaluate::IndirectSubscriptIntegerExpr>(
                subscript.value().u)
                .value();
        hlfir::Entity subscript = genSubscript(expr);
        partInfo.subscripts.push_back(subscript);
        if (expr.Rank() > 0) {
          sawVectorSubscripts = true;
          auto [extentValue, shapeExtent] =
              tryGettingExtentFromFrontEnd(resultExtents.size());
          resultTypeShape.push_back(shapeExtent);
          if (!extentValue)
            extentValue = hlfir::genExtent(loc, builder, subscript, /*dim=*/0);
          resultExtents.push_back(extentValue);
        }
      }
    }
    assert(resultExtents.size() == resultTypeShape.size() &&
           "inconsistent hlfir.designate shape");

    // For vector subscripts, create an hlfir.elemental_addr and continue
    // lowering the designator inside it as if it was addressing an element of
    // the vector subscripts.
    if (sawVectorSubscripts)
      return createVectorSubscriptElementAddrOp(partInfo, baseType,
                                                resultExtents);

    mlir::Type resultType =
        mlir::cast<fir::SequenceType>(baseType).getElementType();
    if (!resultTypeShape.empty()) {
      // Ranked array section. The result shape comes from the array section
      // subscripts.
      resultType = fir::SequenceType::get(resultTypeShape, resultType);
      assert(!partInfo.resultShape &&
             "Fortran designator can only have one ranked part");
      partInfo.resultShape = builder.genShape(loc, resultExtents);
    } else if (!partInfo.componentName.empty() &&
               partInfo.base.value().isArray()) {
      // This is an array%array_comp(indices) reference. Keep the
      // shape of the base array and not the array_comp.
      auto compBaseTy = partInfo.base->getElementOrSequenceType();
      resultType = changeElementType(compBaseTy, resultType);
      assert(!partInfo.resultShape && "should not have been computed already");
      partInfo.resultShape = hlfir::genShape(loc, builder, *partInfo.base);
    }
    return resultType;
  }

  static bool
  mayHaveNonDefaultLowerBounds(const language::Compability::semantics::Symbol &componentSym) {
    if (const auto *objDetails =
            componentSym.detailsIf<language::Compability::semantics::ObjectEntityDetails>())
      for (const language::Compability::semantics::ShapeSpec &bounds : objDetails->shape())
        if (auto lb = bounds.lbound().GetExplicit())
          if (auto constant = language::Compability::evaluate::ToInt64(*lb))
            if (!constant || *constant != 1)
              return true;
    return false;
  }
  static bool mayHaveNonDefaultLowerBounds(const PartInfo &partInfo) {
    return partInfo.resultShape &&
           mlir::isa<fir::ShiftType, fir::ShapeShiftType>(
               partInfo.resultShape.getType());
  }

  mlir::Type visit(const language::Compability::evaluate::Component &component,
                   PartInfo &partInfo) {
    if (language::Compability::semantics::IsAllocatableOrPointer(component.GetLastSymbol())) {
      // In a visit, the following reference will address the target. Insert
      // the dereference here.
      partInfo.base = genWholeAllocatableOrPointerComponent(component);
      partInfo.base = hlfir::derefPointersAndAllocatables(loc, getBuilder(),
                                                          *partInfo.base);
      hlfir::genLengthParameters(loc, getBuilder(), *partInfo.base,
                                 partInfo.typeParams);
      return partInfo.base->getElementOrSequenceType();
    }
    // This function must be called from contexts where the component is not the
    // base of an ArrayRef. In these cases, the component cannot be an array
    // if the base is an array. The code below determines the shape of the
    // component reference if any.
    auto [baseType, componentType] = visitComponentImpl(component, partInfo);
    mlir::Type componentBaseType =
        hlfir::getFortranElementOrSequenceType(componentType);
    if (partInfo.base.value().isArray()) {
      // For array%scalar_comp, the result shape is
      // the one of the base. Compute it here. Note that the lower bounds of the
      // base are not the ones of the resulting reference (that are default
      // ones).
      partInfo.resultShape = hlfir::genShape(loc, getBuilder(), *partInfo.base);
      assert(!partInfo.componentShape &&
             "Fortran designators can only have one ranked part");
      return changeElementType(baseType, componentBaseType);
    }

    if (partInfo.complexPart && partInfo.componentShape) {
      // Treat ...array_comp%im/re as ...array_comp(:,:,...)%im/re
      // so that the codegen has the full slice triples for the component
      // readily available.
      fir::FirOpBuilder &builder = getBuilder();
      mlir::Type idxTy = builder.getIndexType();
      mlir::Value one = builder.createIntegerConstant(loc, idxTy, 1);

      toolchain::SmallVector<mlir::Value> resultExtents;
      // Collect <lb, ub> pairs from the component shape.
      auto bounds = hlfir::genBounds(loc, builder, partInfo.componentShape);
      for (auto &boundPair : bounds) {
        // The default subscripts are <lb, ub, 1>:
        partInfo.subscripts.emplace_back(hlfir::DesignateOp::Triplet{
            boundPair.first, boundPair.second, one});
        auto extentValue = builder.genExtentFromTriplet(
            loc, boundPair.first, boundPair.second, one, idxTy);
        resultExtents.push_back(extentValue);
      }
      // The result shape is: <max((ub - lb + 1) / 1, 0), ...>.
      partInfo.resultShape = builder.genShape(loc, resultExtents);
      return componentBaseType;
    }

    // scalar%array_comp or scalar%scalar. In any case the shape of this
    // part-ref is coming from the component.
    partInfo.resultShape = partInfo.componentShape;
    partInfo.componentShape = {};
    return componentBaseType;
  }

  // Returns the <BaseType, ComponentType> pair, computes partInfo.base,
  // partInfo.componentShape and partInfo.typeParams, but does not set the
  // partInfo.resultShape yet. The result shape will be computed after
  // processing a following ArrayRef, if any, and in "visit" otherwise.
  std::pair<mlir::Type, mlir::Type>
  visitComponentImpl(const language::Compability::evaluate::Component &component,
                     PartInfo &partInfo) {
    fir::FirOpBuilder &builder = getBuilder();
    // Break the Designator visit here: if the base is an array-ref, a
    // coarray-ref, or another component, this creates another hlfir.designate
    // for it.  hlfir.designate is not meant to represent more than one
    // part-ref.
    partInfo.base = gen(component.base());
    // If the base is an allocatable/pointer, dereference it here since the
    // component ref designates its target.
    partInfo.base =
        hlfir::derefPointersAndAllocatables(loc, builder, *partInfo.base);
    assert(partInfo.typeParams.empty() && "should not have been computed yet");

    hlfir::genLengthParameters(getLoc(), getBuilder(), *partInfo.base,
                               partInfo.typeParams);
    mlir::Type baseType = partInfo.base->getElementOrSequenceType();

    // Lower the information about the component (type, length parameters and
    // shape).
    const language::Compability::semantics::Symbol &componentSym = component.GetLastSymbol();
    partInfo.componentName = converter.getRecordTypeFieldName(componentSym);
    auto recordType =
        mlir::cast<fir::RecordType>(hlfir::getFortranElementType(baseType));
    if (recordType.isDependentType())
      TODO(getLoc(), "Designate derived type with length parameters in HLFIR");
    mlir::Type fieldType = recordType.getType(partInfo.componentName);
    assert(fieldType && "component name is not known");
    mlir::Type fieldBaseType =
        hlfir::getFortranElementOrSequenceType(fieldType);
    partInfo.componentShape = genComponentShape(componentSym, fieldBaseType);

    mlir::Type fieldEleType = hlfir::getFortranElementType(fieldBaseType);
    if (fir::isRecordWithTypeParameters(fieldEleType))
      TODO(loc,
           "lower a component that is a parameterized derived type to HLFIR");
    if (auto charTy = mlir::dyn_cast<fir::CharacterType>(fieldEleType)) {
      mlir::Location loc = getLoc();
      mlir::Type idxTy = builder.getIndexType();
      if (charTy.hasConstantLen())
        partInfo.typeParams.push_back(
            builder.createIntegerConstant(loc, idxTy, charTy.getLen()));
      else if (!language::Compability::semantics::IsAllocatableOrObjectPointer(&componentSym))
        TODO(loc, "compute character length of automatic character component "
                  "in a PDT");
      // Otherwise, the length of the component is deferred and will only
      // be read when the component is dereferenced.
    }
    return {baseType, fieldType};
  }

  // Compute: "lb + (i-1)*step".
  mlir::Value computeTripletPosition(mlir::Location loc,
                                     fir::FirOpBuilder &builder,
                                     hlfir::DesignateOp::Triplet &triplet,
                                     mlir::Value oneBasedIndex) {
    mlir::Type idxTy = builder.getIndexType();
    mlir::Value lb = builder.createConvert(loc, idxTy, std::get<0>(triplet));
    mlir::Value step = builder.createConvert(loc, idxTy, std::get<2>(triplet));
    mlir::Value one = builder.createIntegerConstant(loc, idxTy, 1);
    oneBasedIndex = builder.createConvert(loc, idxTy, oneBasedIndex);
    mlir::Value zeroBased =
        mlir::arith::SubIOp::create(builder, loc, oneBasedIndex, one);
    mlir::Value offset =
        mlir::arith::MulIOp::create(builder, loc, zeroBased, step);
    return mlir::arith::AddIOp::create(builder, loc, lb, offset);
  }

  /// Create an hlfir.element_addr operation to deal with vector subscripted
  /// entities. This transforms the current vector subscripted array-ref into a
  /// a scalar array-ref that is addressing the vector subscripted part given
  /// the one based indices of the hlfir.element_addr.
  /// The rest of the designator lowering will continue lowering any further
  /// parts inside the hlfir.elemental as a scalar reference.
  /// At the end of the designator lowering, the hlfir.elemental_addr will
  /// be turned into an hlfir.elemental value, unless the caller of this
  /// utility requested to get the hlfir.elemental_addr instead of lowering
  /// the designator to an mlir::Value.
  mlir::Type createVectorSubscriptElementAddrOp(
      PartInfo &partInfo, mlir::Type baseType,
      toolchain::ArrayRef<mlir::Value> resultExtents) {
    fir::FirOpBuilder &builder = getBuilder();
    mlir::Value shape = builder.genShape(loc, resultExtents);
    // The type parameters to be added on the hlfir.elemental_addr are the ones
    // of the whole designator (not the ones of the vector subscripted part).
    // These are not yet known and will be added when finalizing the designator
    // lowering.
    // The resulting designator may be polymorphic, in which case the resulting
    // type is the base of the vector subscripted part because
    // allocatable/pointer components cannot be referenced after a vector
    // subscripted part. Set the mold to the current base. It will be erased if
    // the resulting designator is not polymorphic.
    assert(partInfo.base.has_value() &&
           "vector subscripted part must have a base");
    mlir::Value mold = *partInfo.base;
    auto elementalAddrOp = hlfir::ElementalAddrOp::create(
        builder, loc, shape, mold, mlir::ValueRange{},
        /*isUnordered=*/true);
    setVectorSubscriptElementAddrOp(elementalAddrOp);
    builder.setInsertionPointToEnd(&elementalAddrOp.getBody().front());
    mlir::Region::BlockArgListType indices = elementalAddrOp.getIndices();
    auto indicesIterator = indices.begin();
    auto getNextOneBasedIndex = [&]() -> mlir::Value {
      assert(indicesIterator != indices.end() && "ill formed ElementalAddrOp");
      return *(indicesIterator++);
    };
    // Transform the designator into a scalar designator computing the vector
    // subscripted entity element address given one based indices (for the shape
    // of the vector subscripted designator).
    for (hlfir::DesignateOp::Subscript &subscript : partInfo.subscripts) {
      if (auto *triplet =
              std::get_if<hlfir::DesignateOp::Triplet>(&subscript)) {
        // subscript = (lb + (i-1)*step)
        mlir::Value scalarSubscript = computeTripletPosition(
            loc, builder, *triplet, getNextOneBasedIndex());
        subscript = scalarSubscript;
      } else {
        hlfir::Entity valueSubscript{std::get<mlir::Value>(subscript)};
        if (valueSubscript.isScalar())
          continue;
        // subscript = vector(i + (vector_lb-1))
        hlfir::Entity scalarSubscript = hlfir::getElementAt(
            loc, builder, valueSubscript, {getNextOneBasedIndex()});
        scalarSubscript =
            hlfir::loadTrivialScalar(loc, builder, scalarSubscript);
        subscript = scalarSubscript;
      }
    }
    builder.setInsertionPoint(elementalAddrOp);
    return mlir::cast<fir::SequenceType>(baseType).getElementType();
  }

  /// Yield the designator for the final part-ref inside the
  /// hlfir.elemental_addr.
  void finalizeElementAddrOp(hlfir::ElementalAddrOp elementalAddrOp,
                             hlfir::EntityWithAttributes elementAddr) {
    fir::FirOpBuilder &builder = getBuilder();
    builder.setInsertionPointToEnd(&elementalAddrOp.getBody().front());
    if (!elementAddr.isPolymorphic())
      elementalAddrOp.getMoldMutable().clear();
    hlfir::YieldOp::create(builder, loc, elementAddr);
    builder.setInsertionPointAfter(elementalAddrOp);
  }

  /// If the lowered designator has vector subscripts turn it into an
  /// ElementalOp, otherwise, return the lowered designator. This should
  /// only be called if the user did not request to get the
  /// hlfir.elemental_addr. In Fortran, vector subscripted designators are only
  /// writable on the left-hand side of an assignment and in input IO
  /// statements. Otherwise, they are not variables (cannot be modified, their
  /// value is taken at the place they appear).
  hlfir::EntityWithAttributes turnVectorSubscriptedDesignatorIntoValue(
      hlfir::EntityWithAttributes loweredDesignator) {
    std::optional<hlfir::ElementalAddrOp> elementalAddrOp =
        getVectorSubscriptElementAddrOp();
    if (!elementalAddrOp)
      return loweredDesignator;
    finalizeElementAddrOp(*elementalAddrOp, loweredDesignator);
    // This vector subscript designator is only being read, transform the
    // hlfir.elemental_addr into an hlfir.elemental.  The content of the
    // hlfir.elemental_addr is cloned, and the resulting address is loaded to
    // get the new element value.
    fir::FirOpBuilder &builder = getBuilder();
    mlir::Location loc = getLoc();
    mlir::Value elemental =
        hlfir::cloneToElementalOp(loc, builder, *elementalAddrOp);
    (*elementalAddrOp)->erase();
    setVectorSubscriptElementAddrOp(std::nullopt);
    fir::FirOpBuilder *bldr = &builder;
    getStmtCtx().attachCleanup(
        [=]() { hlfir::DestroyOp::create(*bldr, loc, elemental); });
    return hlfir::EntityWithAttributes{elemental};
  }

  /// Lower a subscript expression. If it is a scalar subscript that is a
  /// variable, it is loaded into an integer value. If it is an array (for
  /// vector subscripts) it is dereferenced if this is an allocatable or
  /// pointer.
  template <typename T>
  hlfir::Entity genSubscript(const language::Compability::evaluate::Expr<T> &expr);

  const std::optional<hlfir::ElementalAddrOp> &
  getVectorSubscriptElementAddrOp() const {
    return vectorSubscriptElementAddrOp;
  }
  void setVectorSubscriptElementAddrOp(
      std::optional<hlfir::ElementalAddrOp> elementalAddrOp) {
    vectorSubscriptElementAddrOp = elementalAddrOp;
  }

  mlir::Location getLoc() const { return loc; }
  language::Compability::lower::AbstractConverter &getConverter() { return converter; }
  fir::FirOpBuilder &getBuilder() { return converter.getFirOpBuilder(); }
  language::Compability::lower::SymMap &getSymMap() { return symMap; }
  language::Compability::lower::StatementContext &getStmtCtx() { return stmtCtx; }

  language::Compability::lower::AbstractConverter &converter;
  language::Compability::lower::SymMap &symMap;
  language::Compability::lower::StatementContext &stmtCtx;
  // If there is a vector subscript, an elementalAddrOp is created
  // to compute the address of the designator elements.
  std::optional<hlfir::ElementalAddrOp> vectorSubscriptElementAddrOp{};
  mlir::Location loc;
};

hlfir::EntityWithAttributes HlfirDesignatorBuilder::genDesignatorExpr(
    const language::Compability::lower::SomeExpr &designatorExpr,
    bool vectorSubscriptDesignatorToValue) {
  // Expr<SomeType> plumbing to unwrap Designator<T> and call
  // gen(Designator<T>.u).
  return language::Compability::common::visit(
      [&](const auto &x) -> hlfir::EntityWithAttributes {
        using T = std::decay_t<decltype(x)>;
        if constexpr (language::Compability::common::HasMember<
                          T, language::Compability::lower::CategoryExpression>) {
          if constexpr (T::Result::category ==
                        language::Compability::common::TypeCategory::Derived) {
            return gen(std::get<language::Compability::evaluate::Designator<
                           language::Compability::evaluate::SomeDerived>>(x.u)
                           .u,
                       vectorSubscriptDesignatorToValue);
          } else {
            return language::Compability::common::visit(
                [&](const auto &preciseKind) {
                  using TK =
                      typename std::decay_t<decltype(preciseKind)>::Result;
                  return gen(
                      std::get<language::Compability::evaluate::Designator<TK>>(preciseKind.u)
                          .u,
                      vectorSubscriptDesignatorToValue);
                },
                x.u);
          }
        } else {
          fir::emitFatalError(loc, "unexpected typeless Designator");
        }
      },
      designatorExpr.u);
}

hlfir::ElementalAddrOp
HlfirDesignatorBuilder::convertVectorSubscriptedExprToElementalAddr(
    const language::Compability::lower::SomeExpr &designatorExpr) {

  hlfir::EntityWithAttributes elementAddrEntity = genDesignatorExpr(
      designatorExpr, /*vectorSubscriptDesignatorToValue=*/false);
  assert(getVectorSubscriptElementAddrOp().has_value() &&
         "expected vector subscripts");
  hlfir::ElementalAddrOp elementalAddrOp = *getVectorSubscriptElementAddrOp();
  // Now that the type parameters have been computed, add then to the
  // hlfir.elemental_addr.
  fir::FirOpBuilder &builder = getBuilder();
  toolchain::SmallVector<mlir::Value, 1> lengths;
  hlfir::genLengthParameters(loc, builder, elementAddrEntity, lengths);
  if (!lengths.empty())
    elementalAddrOp.getTypeparamsMutable().assign(lengths);
  if (!elementAddrEntity.isPolymorphic())
    elementalAddrOp.getMoldMutable().clear();
  // Create the hlfir.yield terminator inside the hlfir.elemental_body.
  builder.setInsertionPointToEnd(&elementalAddrOp.getBody().front());
  hlfir::YieldOp::create(builder, loc, elementAddrEntity);
  builder.setInsertionPointAfter(elementalAddrOp);
  // Reset the HlfirDesignatorBuilder state, in case it is used on a new
  // designator.
  setVectorSubscriptElementAddrOp(std::nullopt);
  return elementalAddrOp;
}

//===--------------------------------------------------------------------===//
// Binary Operation implementation
//===--------------------------------------------------------------------===//

template <typename T>
struct BinaryOp {};

#undef GENBIN
#define GENBIN(GenBinEvOp, GenBinTyCat, GenBinFirOp)                           \
  template <int KIND>                                                          \
  struct BinaryOp<language::Compability::evaluate::GenBinEvOp<language::Compability::evaluate::Type<       \
      language::Compability::common::TypeCategory::GenBinTyCat, KIND>>> {                    \
    using Op = language::Compability::evaluate::GenBinEvOp<language::Compability::evaluate::Type<          \
        language::Compability::common::TypeCategory::GenBinTyCat, KIND>>;                    \
    static hlfir::EntityWithAttributes gen(mlir::Location loc,                 \
                                           fir::FirOpBuilder &builder,         \
                                           const Op &, hlfir::Entity lhs,      \
                                           hlfir::Entity rhs) {                \
      if constexpr (language::Compability::common::TypeCategory::GenBinTyCat ==              \
                    language::Compability::common::TypeCategory::Unsigned) {                 \
        return hlfir::EntityWithAttributes{                                    \
            builder.createUnsigned<GenBinFirOp>(loc, lhs.getType(), lhs,       \
                                                rhs)};                         \
      } else {                                                                 \
        return hlfir::EntityWithAttributes{                                    \
            GenBinFirOp::create(builder, loc, lhs, rhs)};                      \
      }                                                                        \
    }                                                                          \
  };

GENBIN(Add, Integer, mlir::arith::AddIOp)
GENBIN(Add, Unsigned, mlir::arith::AddIOp)
GENBIN(Add, Real, mlir::arith::AddFOp)
GENBIN(Add, Complex, fir::AddcOp)
GENBIN(Subtract, Integer, mlir::arith::SubIOp)
GENBIN(Subtract, Unsigned, mlir::arith::SubIOp)
GENBIN(Subtract, Real, mlir::arith::SubFOp)
GENBIN(Subtract, Complex, fir::SubcOp)
GENBIN(Multiply, Integer, mlir::arith::MulIOp)
GENBIN(Multiply, Unsigned, mlir::arith::MulIOp)
GENBIN(Multiply, Real, mlir::arith::MulFOp)
GENBIN(Multiply, Complex, fir::MulcOp)
GENBIN(Divide, Integer, mlir::arith::DivSIOp)
GENBIN(Divide, Unsigned, mlir::arith::DivUIOp)
GENBIN(Divide, Real, mlir::arith::DivFOp)

template <int KIND>
struct BinaryOp<language::Compability::evaluate::Divide<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Complex, KIND>>> {
  using Op = language::Compability::evaluate::Divide<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Complex, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs, hlfir::Entity rhs) {
    mlir::Type ty = language::Compability::lower::getFIRType(
        builder.getContext(), language::Compability::common::TypeCategory::Complex, KIND,
        /*params=*/{});

    // TODO: Ideally, complex number division operations should always be
    // lowered to MLIR. However, converting them to the runtime via MLIR causes
    // ABI issues.
    if (builder.getComplexDivisionToRuntimeFlag()) {
      return hlfir::EntityWithAttributes{
          fir::genDivC(builder, loc, ty, lhs, rhs)};
    } else {
      return hlfir::EntityWithAttributes{
          mlir::complex::DivOp::create(builder, loc, lhs, rhs)};
    }
  }
};

template <language::Compability::common::TypeCategory TC, int KIND>
struct BinaryOp<language::Compability::evaluate::Power<language::Compability::evaluate::Type<TC, KIND>>> {
  using Op = language::Compability::evaluate::Power<language::Compability::evaluate::Type<TC, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs, hlfir::Entity rhs) {
    mlir::Type ty = language::Compability::lower::getFIRType(builder.getContext(), TC, KIND,
                                               /*params=*/{});
    return hlfir::EntityWithAttributes{fir::genPow(builder, loc, ty, lhs, rhs)};
  }
};

template <language::Compability::common::TypeCategory TC, int KIND>
struct BinaryOp<
    language::Compability::evaluate::RealToIntPower<language::Compability::evaluate::Type<TC, KIND>>> {
  using Op =
      language::Compability::evaluate::RealToIntPower<language::Compability::evaluate::Type<TC, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs, hlfir::Entity rhs) {
    mlir::Type ty = language::Compability::lower::getFIRType(builder.getContext(), TC, KIND,
                                               /*params=*/{});
    return hlfir::EntityWithAttributes{fir::genPow(builder, loc, ty, lhs, rhs)};
  }
};

template <language::Compability::common::TypeCategory TC, int KIND>
struct BinaryOp<
    language::Compability::evaluate::Extremum<language::Compability::evaluate::Type<TC, KIND>>> {
  using Op = language::Compability::evaluate::Extremum<language::Compability::evaluate::Type<TC, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs,
                                         hlfir::Entity rhs) {
    toolchain::SmallVector<mlir::Value, 2> args{lhs, rhs};
    fir::ExtendedValue res = op.ordering == language::Compability::evaluate::Ordering::Greater
                                 ? fir::genMax(builder, loc, args)
                                 : fir::genMin(builder, loc, args);
    return hlfir::EntityWithAttributes{fir::getBase(res)};
  }
};

// evaluate::Extremum is only created by the front-end when building compiler
// generated expressions (like when folding LEN() or shape/bounds inquiries).
// MIN and MAX are represented as evaluate::ProcedureRef and are not going
// through here. So far the frontend does not generate character Extremum so
// there is no way to test it.
template <int KIND>
struct BinaryOp<language::Compability::evaluate::Extremum<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Character, KIND>>> {
  using Op = language::Compability::evaluate::Extremum<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Character, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &, const Op &,
                                         hlfir::Entity, hlfir::Entity) {
    fir::emitFatalError(loc, "language::Compability::evaluate::Extremum are unexpected");
  }
  static void genResultTypeParams(mlir::Location loc, fir::FirOpBuilder &,
                                  hlfir::Entity, hlfir::Entity,
                                  toolchain::SmallVectorImpl<mlir::Value> &) {
    fir::emitFatalError(loc, "language::Compability::evaluate::Extremum are unexpected");
  }
};

/// Convert parser's INTEGER relational operators to MLIR.
static mlir::arith::CmpIPredicate
translateSignedRelational(language::Compability::common::RelationalOperator rop) {
  switch (rop) {
  case language::Compability::common::RelationalOperator::LT:
    return mlir::arith::CmpIPredicate::slt;
  case language::Compability::common::RelationalOperator::LE:
    return mlir::arith::CmpIPredicate::sle;
  case language::Compability::common::RelationalOperator::EQ:
    return mlir::arith::CmpIPredicate::eq;
  case language::Compability::common::RelationalOperator::NE:
    return mlir::arith::CmpIPredicate::ne;
  case language::Compability::common::RelationalOperator::GT:
    return mlir::arith::CmpIPredicate::sgt;
  case language::Compability::common::RelationalOperator::GE:
    return mlir::arith::CmpIPredicate::sge;
  }
  toolchain_unreachable("unhandled INTEGER relational operator");
}

static mlir::arith::CmpIPredicate
translateUnsignedRelational(language::Compability::common::RelationalOperator rop) {
  switch (rop) {
  case language::Compability::common::RelationalOperator::LT:
    return mlir::arith::CmpIPredicate::ult;
  case language::Compability::common::RelationalOperator::LE:
    return mlir::arith::CmpIPredicate::ule;
  case language::Compability::common::RelationalOperator::EQ:
    return mlir::arith::CmpIPredicate::eq;
  case language::Compability::common::RelationalOperator::NE:
    return mlir::arith::CmpIPredicate::ne;
  case language::Compability::common::RelationalOperator::GT:
    return mlir::arith::CmpIPredicate::ugt;
  case language::Compability::common::RelationalOperator::GE:
    return mlir::arith::CmpIPredicate::uge;
  }
  toolchain_unreachable("unhandled UNSIGNED relational operator");
}

/// Convert parser's REAL relational operators to MLIR.
/// The choice of order (O prefix) vs unorder (U prefix) follows Fortran 2018
/// requirements in the IEEE context (table 17.1 of F2018). This choice is
/// also applied in other contexts because it is easier and in line with
/// other Fortran compilers.
/// FIXME: The signaling/quiet aspect of the table 17.1 requirement is not
/// fully enforced. FIR and LLVM `fcmp` instructions do not give any guarantee
/// whether the comparison will signal or not in case of quiet NaN argument.
static mlir::arith::CmpFPredicate
translateFloatRelational(language::Compability::common::RelationalOperator rop) {
  switch (rop) {
  case language::Compability::common::RelationalOperator::LT:
    return mlir::arith::CmpFPredicate::OLT;
  case language::Compability::common::RelationalOperator::LE:
    return mlir::arith::CmpFPredicate::OLE;
  case language::Compability::common::RelationalOperator::EQ:
    return mlir::arith::CmpFPredicate::OEQ;
  case language::Compability::common::RelationalOperator::NE:
    return mlir::arith::CmpFPredicate::UNE;
  case language::Compability::common::RelationalOperator::GT:
    return mlir::arith::CmpFPredicate::OGT;
  case language::Compability::common::RelationalOperator::GE:
    return mlir::arith::CmpFPredicate::OGE;
  }
  toolchain_unreachable("unhandled REAL relational operator");
}

template <int KIND>
struct BinaryOp<language::Compability::evaluate::Relational<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Integer, KIND>>> {
  using Op = language::Compability::evaluate::Relational<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Integer, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs,
                                         hlfir::Entity rhs) {
    auto cmp = mlir::arith::CmpIOp::create(
        builder, loc, translateSignedRelational(op.opr), lhs, rhs);
    return hlfir::EntityWithAttributes{cmp};
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::Relational<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Unsigned, KIND>>> {
  using Op = language::Compability::evaluate::Relational<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Unsigned, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs,
                                         hlfir::Entity rhs) {
    int bits = language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Integer,
                                       KIND>::Scalar::bits;
    auto signlessType = mlir::IntegerType::get(
        builder.getContext(), bits,
        mlir::IntegerType::SignednessSemantics::Signless);
    mlir::Value lhsSL = builder.createConvert(loc, signlessType, lhs);
    mlir::Value rhsSL = builder.createConvert(loc, signlessType, rhs);
    auto cmp = mlir::arith::CmpIOp::create(
        builder, loc, translateUnsignedRelational(op.opr), lhsSL, rhsSL);
    return hlfir::EntityWithAttributes{cmp};
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::Relational<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Real, KIND>>> {
  using Op = language::Compability::evaluate::Relational<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Real, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs,
                                         hlfir::Entity rhs) {
    auto cmp = mlir::arith::CmpFOp::create(
        builder, loc, translateFloatRelational(op.opr), lhs, rhs);
    return hlfir::EntityWithAttributes{cmp};
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::Relational<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Complex, KIND>>> {
  using Op = language::Compability::evaluate::Relational<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Complex, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs,
                                         hlfir::Entity rhs) {
    auto cmp = fir::CmpcOp::create(builder, loc,
                                   translateFloatRelational(op.opr), lhs, rhs);
    return hlfir::EntityWithAttributes{cmp};
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::Relational<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Character, KIND>>> {
  using Op = language::Compability::evaluate::Relational<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Character, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs,
                                         hlfir::Entity rhs) {
    auto [lhsExv, lhsCleanUp] =
        hlfir::translateToExtendedValue(loc, builder, lhs);
    auto [rhsExv, rhsCleanUp] =
        hlfir::translateToExtendedValue(loc, builder, rhs);
    auto cmp = fir::runtime::genCharCompare(
        builder, loc, translateSignedRelational(op.opr), lhsExv, rhsExv);
    if (lhsCleanUp)
      (*lhsCleanUp)();
    if (rhsCleanUp)
      (*rhsCleanUp)();
    return hlfir::EntityWithAttributes{cmp};
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::LogicalOperation<KIND>> {
  using Op = language::Compability::evaluate::LogicalOperation<KIND>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs,
                                         hlfir::Entity rhs) {
    mlir::Type i1Type = builder.getI1Type();
    mlir::Value i1Lhs = builder.createConvert(loc, i1Type, lhs);
    mlir::Value i1Rhs = builder.createConvert(loc, i1Type, rhs);
    switch (op.logicalOperator) {
    case language::Compability::evaluate::LogicalOperator::And:
      return hlfir::EntityWithAttributes{
          mlir::arith::AndIOp::create(builder, loc, i1Lhs, i1Rhs)};
    case language::Compability::evaluate::LogicalOperator::Or:
      return hlfir::EntityWithAttributes{
          mlir::arith::OrIOp::create(builder, loc, i1Lhs, i1Rhs)};
    case language::Compability::evaluate::LogicalOperator::Eqv:
      return hlfir::EntityWithAttributes{mlir::arith::CmpIOp::create(
          builder, loc, mlir::arith::CmpIPredicate::eq, i1Lhs, i1Rhs)};
    case language::Compability::evaluate::LogicalOperator::Neqv:
      return hlfir::EntityWithAttributes{mlir::arith::CmpIOp::create(
          builder, loc, mlir::arith::CmpIPredicate::ne, i1Lhs, i1Rhs)};
    case language::Compability::evaluate::LogicalOperator::Not:
      // lib/evaluate expression for .NOT. is language::Compability::evaluate::Not<KIND>.
      toolchain_unreachable(".NOT. is not a binary operator");
    }
    toolchain_unreachable("unhandled logical operation");
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::ComplexConstructor<KIND>> {
  using Op = language::Compability::evaluate::ComplexConstructor<KIND>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs, hlfir::Entity rhs) {
    mlir::Value res =
        fir::factory::Complex{builder, loc}.createComplex(lhs, rhs);
    return hlfir::EntityWithAttributes{res};
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::SetLength<KIND>> {
  using Op = language::Compability::evaluate::SetLength<KIND>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity string,
                                         hlfir::Entity length) {
    // The input length may be a user input and needs to be sanitized as per
    // Fortran 2018 7.4.4.2 point 5.
    mlir::Value safeLength = fir::factory::genMaxWithZero(builder, loc, length);
    return hlfir::EntityWithAttributes{
        hlfir::SetLengthOp::create(builder, loc, string, safeLength)};
  }
  static void
  genResultTypeParams(mlir::Location, fir::FirOpBuilder &, hlfir::Entity,
                      hlfir::Entity rhs,
                      toolchain::SmallVectorImpl<mlir::Value> &resultTypeParams) {
    resultTypeParams.push_back(rhs);
  }
};

template <int KIND>
struct BinaryOp<language::Compability::evaluate::Concat<KIND>> {
  using Op = language::Compability::evaluate::Concat<KIND>;
  hlfir::EntityWithAttributes gen(mlir::Location loc,
                                  fir::FirOpBuilder &builder, const Op &,
                                  hlfir::Entity lhs, hlfir::Entity rhs) {
    assert(len && "genResultTypeParams must have been called");
    auto concat =
        hlfir::ConcatOp::create(builder, loc, mlir::ValueRange{lhs, rhs}, len);
    return hlfir::EntityWithAttributes{concat.getResult()};
  }
  void
  genResultTypeParams(mlir::Location loc, fir::FirOpBuilder &builder,
                      hlfir::Entity lhs, hlfir::Entity rhs,
                      toolchain::SmallVectorImpl<mlir::Value> &resultTypeParams) {
    toolchain::SmallVector<mlir::Value> lengths;
    hlfir::genLengthParameters(loc, builder, lhs, lengths);
    hlfir::genLengthParameters(loc, builder, rhs, lengths);
    assert(lengths.size() == 2 && "lacks rhs or lhs length");
    mlir::Type idxType = builder.getIndexType();
    mlir::Value lhsLen = builder.createConvert(loc, idxType, lengths[0]);
    mlir::Value rhsLen = builder.createConvert(loc, idxType, lengths[1]);
    len = mlir::arith::AddIOp::create(builder, loc, lhsLen, rhsLen);
    resultTypeParams.push_back(len);
  }

private:
  mlir::Value len{};
};

//===--------------------------------------------------------------------===//
// Unary Operation implementation
//===--------------------------------------------------------------------===//

template <typename T>
struct UnaryOp {};

template <int KIND>
struct UnaryOp<language::Compability::evaluate::Not<KIND>> {
  using Op = language::Compability::evaluate::Not<KIND>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs) {
    mlir::Value one = builder.createBool(loc, true);
    mlir::Value val = builder.createConvert(loc, builder.getI1Type(), lhs);
    return hlfir::EntityWithAttributes{
        mlir::arith::XOrIOp::create(builder, loc, val, one)};
  }
};

template <int KIND>
struct UnaryOp<language::Compability::evaluate::Negate<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Integer, KIND>>> {
  using Op = language::Compability::evaluate::Negate<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Integer, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs) {
    // Like LLVM, integer negation is the binary op "0 - value"
    mlir::Type type = language::Compability::lower::getFIRType(
        builder.getContext(), language::Compability::common::TypeCategory::Integer, KIND,
        /*params=*/{});
    mlir::Value zero = builder.createIntegerConstant(loc, type, 0);
    return hlfir::EntityWithAttributes{
        mlir::arith::SubIOp::create(builder, loc, zero, lhs)};
  }
};

template <int KIND>
struct UnaryOp<language::Compability::evaluate::Negate<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Unsigned, KIND>>> {
  using Op = language::Compability::evaluate::Negate<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Unsigned, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs) {
    int bits = language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Integer,
                                       KIND>::Scalar::bits;
    mlir::Type signlessType = mlir::IntegerType::get(
        builder.getContext(), bits,
        mlir::IntegerType::SignednessSemantics::Signless);
    mlir::Value zero = builder.createIntegerConstant(loc, signlessType, 0);
    mlir::Value signless = builder.createConvert(loc, signlessType, lhs);
    mlir::Value negated =
        mlir::arith::SubIOp::create(builder, loc, zero, signless);
    return hlfir::EntityWithAttributes(
        builder.createConvert(loc, lhs.getType(), negated));
  }
};

template <int KIND>
struct UnaryOp<language::Compability::evaluate::Negate<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Real, KIND>>> {
  using Op = language::Compability::evaluate::Negate<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Real, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs) {
    return hlfir::EntityWithAttributes{
        mlir::arith::NegFOp::create(builder, loc, lhs)};
  }
};

template <int KIND>
struct UnaryOp<language::Compability::evaluate::Negate<
    language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Complex, KIND>>> {
  using Op = language::Compability::evaluate::Negate<
      language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Complex, KIND>>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs) {
    return hlfir::EntityWithAttributes{fir::NegcOp::create(builder, loc, lhs)};
  }
};

template <int KIND>
struct UnaryOp<language::Compability::evaluate::ComplexComponent<KIND>> {
  using Op = language::Compability::evaluate::ComplexComponent<KIND>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs) {
    mlir::Value res = fir::factory::Complex{builder, loc}.extractComplexPart(
        lhs, op.isImaginaryPart);
    return hlfir::EntityWithAttributes{res};
  }
};

template <typename T>
struct UnaryOp<language::Compability::evaluate::Parentheses<T>> {
  using Op = language::Compability::evaluate::Parentheses<T>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder,
                                         const Op &op, hlfir::Entity lhs) {
    if (lhs.isVariable())
      return hlfir::EntityWithAttributes{
          hlfir::AsExprOp::create(builder, loc, lhs)};
    return hlfir::EntityWithAttributes{
        hlfir::NoReassocOp::create(builder, loc, lhs.getType(), lhs)};
  }

  static void
  genResultTypeParams(mlir::Location loc, fir::FirOpBuilder &builder,
                      hlfir::Entity lhs,
                      toolchain::SmallVectorImpl<mlir::Value> &resultTypeParams) {
    hlfir::genLengthParameters(loc, builder, lhs, resultTypeParams);
  }
};

template <language::Compability::common::TypeCategory TC1, int KIND,
          language::Compability::common::TypeCategory TC2>
struct UnaryOp<
    language::Compability::evaluate::Convert<language::Compability::evaluate::Type<TC1, KIND>, TC2>> {
  using Op =
      language::Compability::evaluate::Convert<language::Compability::evaluate::Type<TC1, KIND>, TC2>;
  static hlfir::EntityWithAttributes gen(mlir::Location loc,
                                         fir::FirOpBuilder &builder, const Op &,
                                         hlfir::Entity lhs) {
    if constexpr (TC1 == language::Compability::common::TypeCategory::Character &&
                  TC2 == TC1) {
      return hlfir::convertCharacterKind(loc, builder, lhs, KIND);
    }
    mlir::Type type = language::Compability::lower::getFIRType(builder.getContext(), TC1,
                                                 KIND, /*params=*/{});
    mlir::Value res = builder.convertWithSemantics(loc, type, lhs);
    return hlfir::EntityWithAttributes{res};
  }

  static void
  genResultTypeParams(mlir::Location loc, fir::FirOpBuilder &builder,
                      hlfir::Entity lhs,
                      toolchain::SmallVectorImpl<mlir::Value> &resultTypeParams) {
    hlfir::genLengthParameters(loc, builder, lhs, resultTypeParams);
  }
};

static bool hasDeferredCharacterLength(const language::Compability::semantics::Symbol &sym) {
  const language::Compability::semantics::DeclTypeSpec *type = sym.GetType();
  return type &&
         type->category() ==
             language::Compability::semantics::DeclTypeSpec::Category::Character &&
         type->characterTypeSpec().length().isDeferred();
}

/// Lower Expr to HLFIR.
class HlfirBuilder {
public:
  HlfirBuilder(mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
               language::Compability::lower::SymMap &symMap,
               language::Compability::lower::StatementContext &stmtCtx)
      : converter{converter}, symMap{symMap}, stmtCtx{stmtCtx}, loc{loc} {}

  template <typename T>
  hlfir::EntityWithAttributes gen(const language::Compability::evaluate::Expr<T> &expr) {
    if (const language::Compability::lower::ExprToValueMap *map =
            getConverter().getExprOverrides()) {
      if constexpr (std::is_same_v<T, language::Compability::evaluate::SomeType>) {
        if (auto match = map->find(&expr); match != map->end())
          return hlfir::EntityWithAttributes{match->second};
      } else {
        language::Compability::lower::SomeExpr someExpr = toEvExpr(expr);
        if (auto match = map->find(&someExpr); match != map->end())
          return hlfir::EntityWithAttributes{match->second};
      }
    }
    return language::Compability::common::visit([&](const auto &x) { return gen(x); },
                                  expr.u);
  }

private:
  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::BOZLiteralConstant &expr) {
    TODO(getLoc(), "BOZ");
  }

  hlfir::EntityWithAttributes gen(const language::Compability::evaluate::NullPointer &expr) {
    auto nullop = hlfir::NullOp::create(getBuilder(), getLoc());
    return mlir::cast<fir::FortranVariableOpInterface>(nullop.getOperation());
  }

  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::ProcedureDesignator &proc) {
    return language::Compability::lower::convertProcedureDesignatorToHLFIR(
        getLoc(), getConverter(), proc, getSymMap(), getStmtCtx());
  }

  hlfir::EntityWithAttributes gen(const language::Compability::evaluate::ProcedureRef &expr) {
    language::Compability::evaluate::ProcedureDesignator proc{expr.proc()};
    auto procTy{language::Compability::lower::translateSignature(proc, getConverter())};
    auto result = language::Compability::lower::convertCallToHLFIR(getLoc(), getConverter(),
                                                     expr, procTy.getResult(0),
                                                     getSymMap(), getStmtCtx());
    assert(result.has_value());
    return *result;
  }

  template <typename T>
  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::Designator<T> &designator) {
    return HlfirDesignatorBuilder(getLoc(), getConverter(), getSymMap(),
                                  getStmtCtx())
        .gen(designator.u);
  }

  template <typename T>
  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::FunctionRef<T> &expr) {
    mlir::Type resType =
        language::Compability::lower::TypeBuilder<T>::genType(getConverter(), expr);
    auto result = language::Compability::lower::convertCallToHLFIR(
        getLoc(), getConverter(), expr, resType, getSymMap(), getStmtCtx());
    assert(result.has_value());
    return *result;
  }

  template <typename T>
  hlfir::EntityWithAttributes gen(const language::Compability::evaluate::Constant<T> &expr) {
    mlir::Location loc = getLoc();
    fir::FirOpBuilder &builder = getBuilder();
    fir::ExtendedValue exv = language::Compability::lower::convertConstant(
        converter, loc, expr, /*outlineBigConstantInReadOnlyMemory=*/true);
    if (const auto *scalarBox = exv.getUnboxed())
      if (fir::isa_trivial(scalarBox->getType()))
        return hlfir::EntityWithAttributes(*scalarBox);
    if (auto addressOf = fir::getBase(exv).getDefiningOp<fir::AddrOfOp>()) {
      auto flags = fir::FortranVariableFlagsAttr::get(
          builder.getContext(), fir::FortranVariableFlagsEnum::parameter);
      return hlfir::genDeclare(
          loc, builder, exv,
          addressOf.getSymbol().getRootReference().getValue(), flags);
    }
    fir::emitFatalError(loc, "Constant<T> was lowered to unexpected format");
  }

  template <typename T>
  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::ArrayConstructor<T> &arrayCtor) {
    return language::Compability::lower::ArrayConstructorBuilder<T>::gen(
        getLoc(), getConverter(), arrayCtor, getSymMap(), getStmtCtx());
  }

  template <typename D, typename R, typename O>
  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::Operation<D, R, O> &op) {
    auto &builder = getBuilder();
    mlir::Location loc = getLoc();
    const int rank = op.Rank();
    UnaryOp<D> unaryOp;
    auto left = hlfir::loadTrivialScalar(loc, builder, gen(op.left()));
    toolchain::SmallVector<mlir::Value, 1> typeParams;
    if constexpr (R::category == language::Compability::common::TypeCategory::Character) {
      unaryOp.genResultTypeParams(loc, builder, left, typeParams);
    }
    if (rank == 0)
      return unaryOp.gen(loc, builder, op.derived(), left);

    // Elemental expression.
    mlir::Type elementType;
    if constexpr (R::category == language::Compability::common::TypeCategory::Derived) {
      if (op.derived().GetType().IsUnlimitedPolymorphic())
        elementType = mlir::NoneType::get(builder.getContext());
      else
        elementType = language::Compability::lower::translateDerivedTypeToFIRType(
            getConverter(), op.derived().GetType().GetDerivedTypeSpec());
    } else {
      elementType =
          language::Compability::lower::getFIRType(builder.getContext(), R::category, R::kind,
                                     /*params=*/{});
    }
    mlir::Value shape = hlfir::genShape(loc, builder, left);
    auto genKernel = [&op, &left, &unaryOp](
                         mlir::Location l, fir::FirOpBuilder &b,
                         mlir::ValueRange oneBasedIndices) -> hlfir::Entity {
      auto leftElement = hlfir::getElementAt(l, b, left, oneBasedIndices);
      auto leftVal = hlfir::loadTrivialScalar(l, b, leftElement);
      return unaryOp.gen(l, b, op.derived(), leftVal);
    };
    mlir::Value elemental = hlfir::genElementalOp(
        loc, builder, elementType, shape, typeParams, genKernel,
        /*isUnordered=*/true, left.isPolymorphic() ? left : mlir::Value{});
    fir::FirOpBuilder *bldr = &builder;
    getStmtCtx().attachCleanup(
        [=]() { hlfir::DestroyOp::create(*bldr, loc, elemental); });
    return hlfir::EntityWithAttributes{elemental};
  }

  template <typename D, typename R, typename LO, typename RO>
  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::Operation<D, R, LO, RO> &op) {
    auto &builder = getBuilder();
    mlir::Location loc = getLoc();
    const int rank = op.Rank();
    BinaryOp<D> binaryOp;
    auto left = hlfir::loadTrivialScalar(loc, builder, gen(op.left()));
    auto right = hlfir::loadTrivialScalar(loc, builder, gen(op.right()));
    toolchain::SmallVector<mlir::Value, 1> typeParams;
    if constexpr (R::category == language::Compability::common::TypeCategory::Character) {
      binaryOp.genResultTypeParams(loc, builder, left, right, typeParams);
    }
    if (rank == 0)
      return binaryOp.gen(loc, builder, op.derived(), left, right);

    // Elemental expression.
    mlir::Type elementType =
        language::Compability::lower::getFIRType(builder.getContext(), R::category, R::kind,
                                   /*params=*/{});
    // TODO: "merge" shape, get cst shape from front-end if possible.
    mlir::Value shape;
    if (left.isArray()) {
      shape = hlfir::genShape(loc, builder, left);
    } else {
      assert(right.isArray() && "must have at least one array operand");
      shape = hlfir::genShape(loc, builder, right);
    }
    auto genKernel = [&op, &left, &right, &binaryOp](
                         mlir::Location l, fir::FirOpBuilder &b,
                         mlir::ValueRange oneBasedIndices) -> hlfir::Entity {
      auto leftElement = hlfir::getElementAt(l, b, left, oneBasedIndices);
      auto rightElement = hlfir::getElementAt(l, b, right, oneBasedIndices);
      auto leftVal = hlfir::loadTrivialScalar(l, b, leftElement);
      auto rightVal = hlfir::loadTrivialScalar(l, b, rightElement);
      return binaryOp.gen(l, b, op.derived(), leftVal, rightVal);
    };
    auto iofBackup = builder.getIntegerOverflowFlags();
    // nsw is never added to operations on vector subscripts
    // even if -fno-wrapv is enabled.
    builder.setIntegerOverflowFlags(mlir::arith::IntegerOverflowFlags::none);
    mlir::Value elemental = hlfir::genElementalOp(loc, builder, elementType,
                                                  shape, typeParams, genKernel,
                                                  /*isUnordered=*/true);
    builder.setIntegerOverflowFlags(iofBackup);
    fir::FirOpBuilder *bldr = &builder;
    getStmtCtx().attachCleanup(
        [=]() { hlfir::DestroyOp::create(*bldr, loc, elemental); });
    return hlfir::EntityWithAttributes{elemental};
  }

  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::Relational<language::Compability::evaluate::SomeType> &op) {
    return language::Compability::common::visit([&](const auto &x) { return gen(x); }, op.u);
  }

  hlfir::EntityWithAttributes gen(const language::Compability::evaluate::TypeParamInquiry &) {
    TODO(getLoc(), "lowering type parameter inquiry to HLFIR");
  }

  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::DescriptorInquiry &desc) {
    mlir::Location loc = getLoc();
    auto &builder = getBuilder();
    hlfir::EntityWithAttributes entity =
        HlfirDesignatorBuilder(getLoc(), getConverter(), getSymMap(),
                               getStmtCtx())
            .genNamedEntity(desc.base());
    using ResTy = language::Compability::evaluate::DescriptorInquiry::Result;
    mlir::Type resultType =
        getConverter().genType(ResTy::category, ResTy::kind);
    auto castResult = [&](mlir::Value v) {
      return hlfir::EntityWithAttributes{
          builder.createConvert(loc, resultType, v)};
    };
    switch (desc.field()) {
    case language::Compability::evaluate::DescriptorInquiry::Field::Len:
      return castResult(hlfir::genCharLength(loc, builder, entity));
    case language::Compability::evaluate::DescriptorInquiry::Field::LowerBound:
      return castResult(
          hlfir::genLBound(loc, builder, entity, desc.dimension()));
    case language::Compability::evaluate::DescriptorInquiry::Field::Extent:
      return castResult(
          hlfir::genExtent(loc, builder, entity, desc.dimension()));
    case language::Compability::evaluate::DescriptorInquiry::Field::Rank:
      return castResult(hlfir::genRank(loc, builder, entity, resultType));
    case language::Compability::evaluate::DescriptorInquiry::Field::Stride:
      // So far the front end does not generate this inquiry.
      TODO(loc, "stride inquiry");
    }
    toolchain_unreachable("unknown descriptor inquiry");
  }

  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::ImpliedDoIndex &var) {
    mlir::Value value = symMap.lookupImpliedDo(toStringRef(var.name));
    if (!value)
      fir::emitFatalError(getLoc(), "ac-do-variable has no binding");
    // The index value generated by the implied-do has Index type,
    // while computations based on it inside the loop body are using
    // the original data type. So we need to cast it appropriately.
    mlir::Type varTy = getConverter().genType(toEvExpr(var));
    value = getBuilder().createConvert(getLoc(), varTy, value);
    return hlfir::EntityWithAttributes{value};
  }

  static bool
  isDerivedTypeWithLenParameters(const language::Compability::semantics::Symbol &sym) {
    if (const language::Compability::semantics::DeclTypeSpec *declTy = sym.GetType())
      if (const language::Compability::semantics::DerivedTypeSpec *derived =
              declTy->AsDerived())
        return language::Compability::semantics::CountLenParameters(*derived) > 0;
    return false;
  }

  // Construct an entity holding the value specified by the
  // StructureConstructor. The initialization of the temporary entity
  // is done component by component with the help of HLFIR operations
  // DesignateOp and AssignOp.
  hlfir::EntityWithAttributes
  gen(const language::Compability::evaluate::StructureConstructor &ctor) {
    mlir::Location loc = getLoc();
    fir::FirOpBuilder &builder = getBuilder();
    mlir::Type ty = translateSomeExprToFIRType(converter, toEvExpr(ctor));
    auto recTy = mlir::cast<fir::RecordType>(ty);

    if (recTy.isDependentType())
      TODO(loc, "structure constructor for derived type with length parameters "
                "in HLFIR");

    // Allocate scalar temporary that will be initialized
    // with the values specified by the constructor.
    mlir::Value storagePtr = builder.createTemporary(loc, recTy);
    auto varOp = hlfir::EntityWithAttributes{hlfir::DeclareOp::create(
        builder, loc, storagePtr, "ctor.temp", /*shape=*/nullptr,
        /*typeparams=*/mlir::ValueRange{}, /*dummy_scope=*/nullptr,
        fir::FortranVariableFlagsAttr{})};

    // Initialize any components that need initialization.
    mlir::Value box = builder.createBox(loc, fir::ExtendedValue{varOp});
    fir::runtime::genDerivedTypeInitialize(builder, loc, box);

    // StructureConstructor values may relate to name of components in parent
    // types. These components cannot be addressed directly, the parent
    // components must be addressed first. The loop below creates all the
    // required chains of hlfir.designate to address the parent components so
    // that the StructureConstructor can later be lowered by addressing these
    // parent components if needed. Note: the front-end orders the components in
    // structure constructors.
    using ValueAndParent = std::tuple<const language::Compability::lower::SomeExpr &,
                                      const language::Compability::semantics::Symbol &,
                                      hlfir::EntityWithAttributes>;
    toolchain::SmallVector<ValueAndParent> valuesAndParents;
    for (const auto &value : toolchain::reverse(ctor.values())) {
      const language::Compability::semantics::Symbol &compSym = *value.first;
      hlfir::EntityWithAttributes currentParent = varOp;
      for (language::Compability::lower::ComponentReverseIterator compIterator(
               ctor.result().derivedTypeSpec());
           !compIterator.lookup(compSym.name());) {
        // Private parent components have mangled names. Get the name from the
        // parent symbol.
        const language::Compability::semantics::Symbol *parentCompSym =
            compIterator.getParentComponent();
        assert(parentCompSym && "failed to get parent component symbol");
        std::string parentName =
            converter.getRecordTypeFieldName(*parentCompSym);
        // Advance the iterator, but don't use its return value.
        compIterator.advanceToParentType();
        auto baseRecTy = mlir::cast<fir::RecordType>(
            hlfir::getFortranElementType(currentParent.getType()));
        auto parentCompType = baseRecTy.getType(parentName);
        assert(parentCompType && "failed to retrieve parent component type");
        mlir::Type designatorType = builder.getRefType(parentCompType);
        mlir::Value newParent = hlfir::DesignateOp::create(
            builder, loc, designatorType, currentParent, parentName,
            /*compShape=*/mlir::Value{}, hlfir::DesignateOp::Subscripts{},
            /*substring=*/mlir::ValueRange{},
            /*complexPart=*/std::nullopt,
            /*shape=*/mlir::Value{}, /*typeParams=*/mlir::ValueRange{},
            fir::FortranVariableFlagsAttr{});
        currentParent = hlfir::EntityWithAttributes{newParent};
      }
      valuesAndParents.emplace_back(
          ValueAndParent{value.second.value(), compSym, currentParent});
    }

    HlfirDesignatorBuilder designatorBuilder(loc, converter, symMap, stmtCtx);
    for (const auto &iter : toolchain::reverse(valuesAndParents)) {
      auto &sym = std::get<const language::Compability::semantics::Symbol &>(iter);
      auto &expr = std::get<const language::Compability::lower::SomeExpr &>(iter);
      auto &baseOp = std::get<hlfir::EntityWithAttributes>(iter);
      std::string name = converter.getRecordTypeFieldName(sym);

      // Generate DesignateOp for the component.
      // The designator's result type is just a reference to the component type,
      // because the whole component is being designated.
      auto baseRecTy = mlir::cast<fir::RecordType>(
          hlfir::getFortranElementType(baseOp.getType()));
      auto compType = baseRecTy.getType(name);
      assert(compType && "failed to retrieve component type");
      mlir::Value compShape =
          designatorBuilder.genComponentShape(sym, compType);
      const bool isDesignatorVolatile =
          fir::isa_volatile_type(baseOp.getType());
      auto [designatorType, extraAttributeFlags] =
          designatorBuilder.genComponentDesignatorTypeAndAttributes(
              sym, compType, isDesignatorVolatile);

      mlir::Type fieldElemType = hlfir::getFortranElementType(compType);
      toolchain::SmallVector<mlir::Value, 1> typeParams;
      if (auto charType = mlir::dyn_cast<fir::CharacterType>(fieldElemType)) {
        if (charType.hasConstantLen()) {
          mlir::Type idxType = builder.getIndexType();
          typeParams.push_back(
              builder.createIntegerConstant(loc, idxType, charType.getLen()));
        } else if (!hasDeferredCharacterLength(sym)) {
          // If the length is not deferred, this is a parametrized derived type
          // where the character length depends on the derived type length
          // parameters. Otherwise, this is a pointer/allocatable component and
          // the length will be set during the assignment.
          TODO(loc, "automatic character component in structure constructor");
        }
      }

      // Convert component symbol attributes to variable attributes.
      fir::FortranVariableFlagsAttr attrs =
          language::Compability::lower::translateSymbolAttributes(builder.getContext(), sym,
                                                    extraAttributeFlags);

      // Get the component designator.
      auto lhs = hlfir::DesignateOp::create(
          builder, loc, designatorType, baseOp, name, compShape,
          hlfir::DesignateOp::Subscripts{},
          /*substring=*/mlir::ValueRange{},
          /*complexPart=*/std::nullopt,
          /*shape=*/compShape, typeParams, attrs);

      if (attrs && bitEnumContainsAny(attrs.getFlags(),
                                      fir::FortranVariableFlagsEnum::pointer)) {
        if (language::Compability::semantics::IsProcedure(sym)) {
          // Procedure pointer components.
          if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(
                  expr)) {
            auto boxTy{
                language::Compability::lower::getUntypedBoxProcType(builder.getContext())};
            hlfir::Entity rhs(
                fir::factory::createNullBoxProc(builder, loc, boxTy));
            builder.createStoreWithConvert(loc, rhs, lhs);
            continue;
          }
          hlfir::Entity rhs(getBase(language::Compability::lower::convertExprToAddress(
              loc, converter, expr, symMap, stmtCtx)));
          builder.createStoreWithConvert(loc, rhs, lhs);
          continue;
        }
        // Pointer component construction is just a copy of the box contents.
        fir::ExtendedValue lhsExv =
            hlfir::translateToExtendedValue(loc, builder, lhs);
        auto *toBox = lhsExv.getBoxOf<fir::MutableBoxValue>();
        if (!toBox)
          fir::emitFatalError(loc, "pointer component designator could not be "
                                   "lowered to mutable box");
        language::Compability::lower::associateMutableBox(converter, loc, *toBox, expr,
                                            /*lbounds=*/{}, stmtCtx);
        continue;
      }

      // Use generic assignment for all the other cases.
      bool allowRealloc =
          attrs &&
          bitEnumContainsAny(attrs.getFlags(),
                             fir::FortranVariableFlagsEnum::allocatable);
      // If the component is allocatable, then we have to check
      // whether the RHS value is allocatable or not.
      // If it is not allocatable, then AssignOp can be used directly.
      // If it is allocatable, then using AssignOp for unallocated RHS
      // will cause illegal dereference. When an unallocated allocatable
      // value is used to construct an allocatable component, the component
      // must just stay unallocated (see Fortran 2018 7.5.10 point 7).

      // If the component is allocatable and RHS is NULL() expression, then
      // we can just skip it: the LHS must remain unallocated with its
      // defined rank.
      if (allowRealloc &&
          language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(expr))
        continue;

      bool keepLhsLength = false;
      if (allowRealloc)
        if (const language::Compability::semantics::DeclTypeSpec *declType = sym.GetType())
          keepLhsLength =
              declType->category() ==
                  language::Compability::semantics::DeclTypeSpec::Category::Character &&
              !declType->characterTypeSpec().length().isDeferred();
      // Handle special case when the initializer expression is
      // '{%SET_LENGTH(x,const_kind)}'. In structure constructor,
      // SET_LENGTH is used for initializers of non-allocatable character
      // components so that the front-end can better
      // fold and work with these structure constructors.
      // Here, they are just noise since the assignment semantics will deal
      // with any length mismatch, and creating an extra temp with the lhs
      // length is useless.
      // TODO: should this be moved into an hlfir.assign + hlfir.set_length
      // pattern rewrite?
      hlfir::Entity rhs = gen(expr);
      if (auto set_length = rhs.getDefiningOp<hlfir::SetLengthOp>())
        rhs = hlfir::Entity{set_length.getString()};

      // lambda to generate `lhs = rhs` and deal with potential rhs implicit
      // cast
      auto genAssign = [&] {
        rhs = hlfir::loadTrivialScalar(loc, builder, rhs);
        auto rhsCastAndCleanup =
            hlfir::genTypeAndKindConvert(loc, builder, rhs, lhs.getType(),
                                         /*preserveLowerBounds=*/allowRealloc);
        hlfir::AssignOp::create(builder, loc, rhsCastAndCleanup.first, lhs,
                                allowRealloc,
                                allowRealloc ? keepLhsLength : false,
                                /*temporary_lhs=*/true);
        if (rhsCastAndCleanup.second)
          (*rhsCastAndCleanup.second)();
      };

      if (!allowRealloc || !rhs.isMutableBox()) {
        genAssign();
        continue;
      }

      auto [rhsExv, cleanup] =
          hlfir::translateToExtendedValue(loc, builder, rhs);
      assert(!cleanup && "unexpected cleanup");
      auto *fromBox = rhsExv.getBoxOf<fir::MutableBoxValue>();
      if (!fromBox)
        fir::emitFatalError(loc, "allocatable entity could not be lowered "
                                 "to mutable box");
      mlir::Value isAlloc =
          fir::factory::genIsAllocatedOrAssociatedTest(builder, loc, *fromBox);
      builder.genIfThen(loc, isAlloc).genThen(genAssign).end();
    }

    if (fir::isRecordWithAllocatableMember(recTy)) {
      // Deallocate allocatable components without calling final subroutines.
      // The Fortran 2018 section 9.7.3.2 about deallocation is not ruling
      // about the fate of allocatable components of structure constructors,
      // and there is no behavior consensus in other compilers.
      fir::FirOpBuilder *bldr = &builder;
      getStmtCtx().attachCleanup([=]() {
        fir::runtime::genDerivedTypeDestroyWithoutFinalization(*bldr, loc, box);
      });
    }
    return varOp;
  }

  mlir::Location getLoc() const { return loc; }
  language::Compability::lower::AbstractConverter &getConverter() { return converter; }
  fir::FirOpBuilder &getBuilder() { return converter.getFirOpBuilder(); }
  language::Compability::lower::SymMap &getSymMap() { return symMap; }
  language::Compability::lower::StatementContext &getStmtCtx() { return stmtCtx; }

  language::Compability::lower::AbstractConverter &converter;
  language::Compability::lower::SymMap &symMap;
  language::Compability::lower::StatementContext &stmtCtx;
  mlir::Location loc;
};

template <typename T>
hlfir::Entity
HlfirDesignatorBuilder::genSubscript(const language::Compability::evaluate::Expr<T> &expr) {
  fir::FirOpBuilder &builder = getBuilder();
  mlir::arith::IntegerOverflowFlags iofBackup{};
  if (!getConverter().getLoweringOptions().getIntegerWrapAround()) {
    iofBackup = builder.getIntegerOverflowFlags();
    builder.setIntegerOverflowFlags(mlir::arith::IntegerOverflowFlags::nsw);
  }
  auto loweredExpr =
      HlfirBuilder(getLoc(), getConverter(), getSymMap(), getStmtCtx())
          .gen(expr);
  if (!getConverter().getLoweringOptions().getIntegerWrapAround())
    builder.setIntegerOverflowFlags(iofBackup);
  // Skip constant conversions that litters designators and makes generated
  // IR harder to read: directly use index constants for constant subscripts.
  mlir::Type idxTy = builder.getIndexType();
  if (!loweredExpr.isArray() && loweredExpr.getType() != idxTy)
    if (auto cstIndex = fir::getIntIfConstant(loweredExpr))
      return hlfir::EntityWithAttributes{
          builder.createIntegerConstant(getLoc(), idxTy, *cstIndex)};
  return hlfir::loadTrivialScalar(loc, builder, loweredExpr);
}

} // namespace

hlfir::EntityWithAttributes language::Compability::lower::convertExprToHLFIR(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &expr, language::Compability::lower::SymMap &symMap,
    language::Compability::lower::StatementContext &stmtCtx) {
  return HlfirBuilder(loc, converter, symMap, stmtCtx).gen(expr);
}

fir::ExtendedValue language::Compability::lower::convertToBox(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    hlfir::Entity entity, language::Compability::lower::StatementContext &stmtCtx,
    mlir::Type fortranType) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  auto [exv, cleanup] = hlfir::convertToBox(loc, builder, entity, fortranType);
  if (cleanup)
    stmtCtx.attachCleanup(*cleanup);
  return exv;
}

fir::ExtendedValue language::Compability::lower::convertExprToBox(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &expr, language::Compability::lower::SymMap &symMap,
    language::Compability::lower::StatementContext &stmtCtx) {
  hlfir::EntityWithAttributes loweredExpr =
      HlfirBuilder(loc, converter, symMap, stmtCtx).gen(expr);
  return convertToBox(loc, converter, loweredExpr, stmtCtx,
                      converter.genType(expr));
}

fir::ExtendedValue language::Compability::lower::convertToAddress(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    hlfir::Entity entity, language::Compability::lower::StatementContext &stmtCtx,
    mlir::Type fortranType) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();
  auto [exv, cleanup] =
      hlfir::convertToAddress(loc, builder, entity, fortranType);
  if (cleanup)
    stmtCtx.attachCleanup(*cleanup);
  return exv;
}

fir::ExtendedValue language::Compability::lower::convertExprToAddress(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &expr, language::Compability::lower::SymMap &symMap,
    language::Compability::lower::StatementContext &stmtCtx) {
  hlfir::EntityWithAttributes loweredExpr =
      HlfirBuilder(loc, converter, symMap, stmtCtx).gen(expr);
  return convertToAddress(loc, converter, loweredExpr, stmtCtx,
                          converter.genType(expr));
}

fir::ExtendedValue language::Compability::lower::convertToValue(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    hlfir::Entity entity, language::Compability::lower::StatementContext &stmtCtx) {
  auto &builder = converter.getFirOpBuilder();
  auto [exv, cleanup] = hlfir::convertToValue(loc, builder, entity);
  if (cleanup)
    stmtCtx.attachCleanup(*cleanup);
  return exv;
}

fir::ExtendedValue language::Compability::lower::convertExprToValue(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &expr, language::Compability::lower::SymMap &symMap,
    language::Compability::lower::StatementContext &stmtCtx) {
  hlfir::EntityWithAttributes loweredExpr =
      HlfirBuilder(loc, converter, symMap, stmtCtx).gen(expr);
  return convertToValue(loc, converter, loweredExpr, stmtCtx);
}

fir::ExtendedValue language::Compability::lower::convertDataRefToValue(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::DataRef &dataRef, language::Compability::lower::SymMap &symMap,
    language::Compability::lower::StatementContext &stmtCtx) {
  fir::FortranVariableOpInterface loweredExpr =
      HlfirDesignatorBuilder(loc, converter, symMap, stmtCtx).gen(dataRef);
  return convertToValue(loc, converter, loweredExpr, stmtCtx);
}

fir::MutableBoxValue language::Compability::lower::convertExprToMutableBox(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &expr, language::Compability::lower::SymMap &symMap) {
  // Pointers and Allocatable cannot be temporary expressions. Temporaries may
  // be created while lowering it (e.g. if any indices expression of a
  // designator create temporaries), but they can be destroyed before using the
  // lowered pointer or allocatable;
  language::Compability::lower::StatementContext localStmtCtx;
  hlfir::EntityWithAttributes loweredExpr =
      HlfirBuilder(loc, converter, symMap, localStmtCtx).gen(expr);
  fir::ExtendedValue exv = language::Compability::lower::translateToExtendedValue(
      loc, converter.getFirOpBuilder(), loweredExpr, localStmtCtx);
  auto *mutableBox = exv.getBoxOf<fir::MutableBoxValue>();
  assert(mutableBox && "expression could not be lowered to mutable box");
  return *mutableBox;
}

hlfir::ElementalAddrOp
language::Compability::lower::convertVectorSubscriptedExprToElementalAddr(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &designatorExpr,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx) {
  return HlfirDesignatorBuilder(loc, converter, symMap, stmtCtx)
      .convertVectorSubscriptedExprToElementalAddr(designatorExpr);
}

hlfir::Entity language::Compability::lower::genVectorSubscriptedDesignatorFirstElementAddress(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &expr, language::Compability::lower::SymMap &symMap,
    language::Compability::lower::StatementContext &stmtCtx) {
  fir::FirOpBuilder &builder = converter.getFirOpBuilder();

  // Get a hlfir.elemental_addr op describing the address of the value
  // indexed from the original array.
  // Note: the hlfir.elemental_addr op verifier requires it to be inside
  // of a hlfir.region_assign op. This operation is never seen by the
  // verifier because it is immediately inlined.
  hlfir::ElementalAddrOp addrOp = convertVectorSubscriptedExprToElementalAddr(
      loc, converter, expr, symMap, stmtCtx);
  if (!addrOp.getCleanup().empty())
    TODO(converter.getCurrentLocation(),
         "Vector subscript requring a cleanup region");

  // hlfir.elemental_addr doesn't have a normal lowering because it
  // can't return a value. Instead we need to inline it here using
  // values for the first element. Similar to hlfir::inlineElementalOp.

  mlir::Value one = builder.createIntegerConstant(
      converter.getCurrentLocation(), builder.getIndexType(), 1);
  mlir::SmallVector<mlir::Value> oneBasedIndices;
  oneBasedIndices.resize(addrOp.getIndices().size(), one);

  mlir::IRMapping mapper;
  mapper.map(addrOp.getIndices(), oneBasedIndices);
  assert(addrOp.getElementalRegion().hasOneBlock());
  mlir::Operation *newOp;
  for (mlir::Operation &op : addrOp.getElementalRegion().back().getOperations())
    newOp = builder.clone(op, mapper);
  auto yield = mlir::cast<hlfir::YieldOp>(newOp);

  addrOp->erase();

  if (!yield.getCleanup().empty())
    TODO(converter.getCurrentLocation(),
         "Vector subscript requring element cleanup");

  hlfir::Entity result{yield.getEntity()};
  yield->erase();
  return result;
}
