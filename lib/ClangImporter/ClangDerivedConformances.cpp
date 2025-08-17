//===--- ClangDerivedConformances.cpp -------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "ClangDerivedConformances.h"
#include "ImporterImpl.h"
#include "language/AST/ConformanceLookup.h"
#include "language/AST/ParameterList.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/ProtocolConformance.h"
#include "language/Basic/Assertions.h"
#include "language/ClangImporter/ClangImporterRequests.h"
#include "language/Core/Sema/DelayedDiagnostic.h"
#include "language/Core/Sema/Overload.h"

using namespace language;
using namespace language::importer;

/// Alternative to `NominalTypeDecl::lookupDirect`.
/// This function does not attempt to load extensions of the nominal decl.
static TinyPtrVector<ValueDecl *>
lookupDirectWithoutExtensions(NominalTypeDecl *decl, Identifier id) {
  ASTContext &ctx = decl->getASTContext();
  auto *importer = static_cast<ClangImporter *>(ctx.getClangModuleLoader());

  TinyPtrVector<ValueDecl *> result;

  if (id.isOperator()) {
    auto underlyingId = getOperatorName(ctx, id);
    TinyPtrVector<ValueDecl *> underlyingFuncs = evaluateOrDefault(
        ctx.evaluator, ClangRecordMemberLookup({decl, underlyingId}), {});
    for (auto it : underlyingFuncs) {
      if (auto synthesizedFunc =
              importer->getCXXSynthesizedOperatorFunc(cast<FuncDecl>(it)))
        result.push_back(synthesizedFunc);
    }
  } else {
    // See if there is a Clang decl with the given name.
    result = evaluateOrDefault(ctx.evaluator,
                               ClangRecordMemberLookup({decl, id}), {});
  }

  // Check if there are any synthesized Codira members that match the name.
  for (auto member : decl->getCurrentMembersWithoutLoading()) {
    if (auto namedMember = dyn_cast<ValueDecl>(member)) {
      if (namedMember->hasName() && !namedMember->getName().isSpecial() &&
          namedMember->getName().getBaseIdentifier().is(id.str()) &&
          // Make sure we don't add duplicate entries, as that would wrongly
          // imply that lookup is ambiguous.
          !toolchain::is_contained(result, namedMember)) {
        result.push_back(namedMember);
      }
    }
  }
  return result;
}

template <typename Decl>
static Decl *lookupDirectSingleWithoutExtensions(NominalTypeDecl *decl,
                                                 Identifier id) {
  auto results = lookupDirectWithoutExtensions(decl, id);
  if (results.size() != 1)
    return nullptr;
  return dyn_cast<Decl>(results.front());
}

static FuncDecl *getInsertFunc(NominalTypeDecl *decl,
                               TypeAliasDecl *valueType) {
  ASTContext &ctx = decl->getASTContext();

  auto insertId = ctx.getIdentifier("__insertUnsafe");
  auto inserts = lookupDirectWithoutExtensions(decl, insertId);
  FuncDecl *insert = nullptr;
  for (auto candidate : inserts) {
    if (auto candidateMethod = dyn_cast<FuncDecl>(candidate)) {
      auto params = candidateMethod->getParameters();
      if (params->size() != 1)
        continue;
      auto param = params->front();
      if (param->getTypeInContext()->getCanonicalType() !=
          valueType->getUnderlyingType()->getCanonicalType())
        continue;
      insert = candidateMethod;
      break;
    }
  }
  return insert;
}

static bool isStdDecl(const language::Core::CXXRecordDecl *clangDecl,
                      toolchain::ArrayRef<StringRef> names) {
  if (!clangDecl->isInStdNamespace())
    return false;
  if (!clangDecl->getIdentifier())
    return false;
  StringRef name = clangDecl->getName();
  return toolchain::is_contained(names, name);
}

static language::Core::TypeDecl *
lookupNestedClangTypeDecl(const language::Core::CXXRecordDecl *clangDecl,
                          StringRef name) {
  language::Core::IdentifierInfo *nestedDeclName =
      &clangDecl->getASTContext().Idents.get(name);
  auto nestedDecls = clangDecl->lookup(nestedDeclName);
  // If this is a templated typedef, Clang might have instantiated several
  // equivalent typedef decls. If they aren't equivalent, Clang has already
  // complained about this. Let's assume that they are equivalent. (see
  // filterNonConflictingPreviousTypedefDecls in clang/Sema/SemaDecl.cpp)
  if (nestedDecls.empty())
    return nullptr;
  auto nestedDecl = nestedDecls.front();
  return dyn_cast_or_null<language::Core::TypeDecl>(nestedDecl);
}

static language::Core::TypeDecl *
getIteratorCategoryDecl(const language::Core::CXXRecordDecl *clangDecl) {
  return lookupNestedClangTypeDecl(clangDecl, "iterator_category");
}

static language::Core::TypeDecl *
getIteratorConceptDecl(const language::Core::CXXRecordDecl *clangDecl) {
  return lookupNestedClangTypeDecl(clangDecl, "iterator_concept");
}

static ValueDecl *lookupOperator(NominalTypeDecl *decl, Identifier id,
                                 function_ref<bool(ValueDecl *)> isValid) {
  // First look for operator declared as a member.
  auto memberResults = lookupDirectWithoutExtensions(decl, id);
  for (const auto &member : memberResults) {
    if (isValid(member))
      return member;
  }

  // If no member operator was found, look for out-of-class definitions in the
  // same module.
  auto module = decl->getModuleContext();
  SmallVector<ValueDecl *> nonMemberResults;
  module->lookupValue(id, NLKind::UnqualifiedLookup, nonMemberResults);
  for (const auto &nonMember : nonMemberResults) {
    if (isValid(nonMember))
      return nonMember;
  }

  return nullptr;
}

static ValueDecl *getEqualEqualOperator(NominalTypeDecl *decl) {
  auto isValid = [&](ValueDecl *equalEqualOp) -> bool {
    auto equalEqual = dyn_cast<FuncDecl>(equalEqualOp);
    if (!equalEqual)
      return false;
    auto params = equalEqual->getParameters();
    if (params->size() != 2)
      return false;
    auto lhs = params->get(0);
    auto rhs = params->get(1);
    if (lhs->isInOut() || rhs->isInOut())
      return false;
    auto lhsTy = lhs->getTypeInContext();
    auto rhsTy = rhs->getTypeInContext();
    if (!lhsTy || !rhsTy)
      return false;
    auto lhsNominal = lhsTy->getAnyNominal();
    auto rhsNominal = rhsTy->getAnyNominal();
    if (lhsNominal != rhsNominal || lhsNominal != decl)
      return false;
    return true;
  };

  return lookupOperator(decl, decl->getASTContext().Id_EqualsOperator, isValid);
}

static FuncDecl *getMinusOperator(NominalTypeDecl *decl) {
  auto binaryIntegerProto =
      decl->getASTContext().getProtocol(KnownProtocolKind::BinaryInteger);

  auto isValid = [&](ValueDecl *minusOp) -> bool {
    auto minus = dyn_cast<FuncDecl>(minusOp);
    if (!minus)
      return false;
    auto params = minus->getParameters();
    if (params->size() != 2)
      return false;
    auto lhs = params->get(0);
    auto rhs = params->get(1);
    if (lhs->isInOut() || rhs->isInOut())
      return false;
    auto lhsTy = lhs->getTypeInContext();
    auto rhsTy = rhs->getTypeInContext();
    if (!lhsTy || !rhsTy)
      return false;
    auto lhsNominal = lhsTy->getAnyNominal();
    auto rhsNominal = rhsTy->getAnyNominal();
    if (lhsNominal != rhsNominal || lhsNominal != decl)
      return false;
    auto returnTy = minus->getResultInterfaceType();
    if (!checkConformance(returnTy, binaryIntegerProto))
      return false;
    return true;
  };

  ValueDecl *result =
      lookupOperator(decl, decl->getASTContext().getIdentifier("-"), isValid);
  return dyn_cast_or_null<FuncDecl>(result);
}

static FuncDecl *getPlusEqualOperator(NominalTypeDecl *decl, Type distanceTy) {
  auto isValid = [&](ValueDecl *plusEqualOp) -> bool {
    auto plusEqual = dyn_cast<FuncDecl>(plusEqualOp);
    if (!plusEqual)
      return false;
    auto params = plusEqual->getParameters();
    if (params->size() != 2)
      return false;
    auto lhs = params->get(0);
    auto rhs = params->get(1);
    if (rhs->isInOut())
      return false;
    auto lhsTy = lhs->getTypeInContext();
    auto rhsTy = rhs->getTypeInContext();
    if (!lhsTy || !rhsTy)
      return false;
    if (rhsTy->getCanonicalType() != distanceTy->getCanonicalType())
      return false;
    auto lhsNominal = lhsTy->getAnyNominal();
    if (lhsNominal != decl)
      return false;
    auto returnTy = plusEqual->getResultInterfaceType();
    if (!returnTy->isVoid())
      return false;
    return true;
  };

  ValueDecl *result =
      lookupOperator(decl, decl->getASTContext().getIdentifier("+="), isValid);
  return dyn_cast_or_null<FuncDecl>(result);
}

static language::Core::FunctionDecl *
instantiateTemplatedOperator(ClangImporter::Implementation &impl,
                             const language::Core::CXXRecordDecl *classDecl,
                             language::Core::BinaryOperatorKind operatorKind) {

  language::Core::ASTContext &clangCtx = impl.getClangASTContext();
  language::Core::Sema &clangSema = impl.getClangSema();

  language::Core::UnresolvedSet<1> ops;
  auto qualType = language::Core::QualType(classDecl->getTypeForDecl(), 0);
  auto arg = language::Core::CXXThisExpr::Create(clangCtx, language::Core::SourceLocation(),
                                        qualType, false);
  arg->setType(language::Core::QualType(classDecl->getTypeForDecl(), 0));

  language::Core::OverloadedOperatorKind opKind =
      language::Core::BinaryOperator::getOverloadedOperator(operatorKind);
  language::Core::OverloadCandidateSet candidateSet(
      classDecl->getLocation(), language::Core::OverloadCandidateSet::CSK_Operator,
      language::Core::OverloadCandidateSet::OperatorRewriteInfo(opKind,
                                              language::Core::SourceLocation(), false));
  std::array<language::Core::Expr *, 2> args{arg, arg};
  clangSema.LookupOverloadedBinOp(candidateSet, opKind, ops, args, true);

  language::Core::OverloadCandidateSet::iterator best;
  switch (candidateSet.BestViableFunction(clangSema, language::Core::SourceLocation(),
                                          best)) {
  case language::Core::OR_Success: {
    if (auto clangCallee = best->Function) {
      auto lookupTable = impl.findLookupTable(classDecl);
      addEntryToLookupTable(*lookupTable, clangCallee, impl.getNameImporter());
      return clangCallee;
    }
    break;
  }
  case language::Core::OR_No_Viable_Function:
  case language::Core::OR_Ambiguous:
  case language::Core::OR_Deleted:
    break;
  }

  return nullptr;
}

/// Warning: This function emits an error and stops compilation if the
/// underlying operator function is unavailable in Codira for the current target
/// (see `language::Core::Sema::DiagnoseAvailabilityOfDecl`).
static bool synthesizeCXXOperator(ClangImporter::Implementation &impl,
                                  const language::Core::CXXRecordDecl *classDecl,
                                  language::Core::BinaryOperatorKind operatorKind,
                                  language::Core::QualType lhsTy, language::Core::QualType rhsTy,
                                  language::Core::QualType returnTy) {
  auto &clangCtx = impl.getClangASTContext();
  auto &clangSema = impl.getClangSema();

  language::Core::OverloadedOperatorKind opKind =
      language::Core::BinaryOperator::getOverloadedOperator(operatorKind);
  const char *opSpelling = language::Core::getOperatorSpelling(opKind);

  auto declName = language::Core::DeclarationName(&clangCtx.Idents.get(opSpelling));

  // Determine the Clang decl context where the new operator function will be
  // created. We use the translation unit as the decl context of the new
  // operator, otherwise, the operator might get imported as a static member
  // function of a different type (e.g. an operator declared inside of a C++
  // namespace would get imported as a member function of a Codira enum), which
  // would make the operator un-discoverable to Codira name lookup.
  auto declContext =
      const_cast<language::Core::CXXRecordDecl *>(classDecl)->getDeclContext();
  while (!declContext->isTranslationUnit()) {
    declContext = declContext->getParent();
  }

  auto equalEqualTy = clangCtx.getFunctionType(
      returnTy, {lhsTy, rhsTy}, language::Core::FunctionProtoType::ExtProtoInfo());

  // Create a `bool operator==(T, T)` function.
  auto equalEqualDecl = language::Core::FunctionDecl::Create(
      clangCtx, declContext, language::Core::SourceLocation(), language::Core::SourceLocation(),
      declName, equalEqualTy, clangCtx.getTrivialTypeSourceInfo(returnTy),
      language::Core::StorageClass::SC_Static);
  equalEqualDecl->setImplicit();
  equalEqualDecl->setImplicitlyInline();
  // If this is a static member function of a class, it needs to be public.
  equalEqualDecl->setAccess(language::Core::AccessSpecifier::AS_public);

  // Create the parameters of the function. They are not referenced from source
  // code, so they don't need to have a name.
  auto lhsParamId = nullptr;
  auto lhsTyInfo = clangCtx.getTrivialTypeSourceInfo(lhsTy);
  auto lhsParamDecl = language::Core::ParmVarDecl::Create(
      clangCtx, equalEqualDecl, language::Core::SourceLocation(),
      language::Core::SourceLocation(), lhsParamId, lhsTy, lhsTyInfo,
      language::Core::StorageClass::SC_None, /*DefArg*/ nullptr);
  auto lhsParamRefExpr = new (clangCtx) language::Core::DeclRefExpr(
      clangCtx, lhsParamDecl, false, lhsTy, language::Core::ExprValueKind::VK_LValue,
      language::Core::SourceLocation());

  auto rhsParamId = nullptr;
  auto rhsTyInfo = clangCtx.getTrivialTypeSourceInfo(rhsTy);
  auto rhsParamDecl = language::Core::ParmVarDecl::Create(
      clangCtx, equalEqualDecl, language::Core::SourceLocation(),
      language::Core::SourceLocation(), rhsParamId, rhsTy, rhsTyInfo,
      language::Core::StorageClass::SC_None, nullptr);
  auto rhsParamRefExpr = new (clangCtx) language::Core::DeclRefExpr(
      clangCtx, rhsParamDecl, false, rhsTy, language::Core::ExprValueKind::VK_LValue,
      language::Core::SourceLocation());

  equalEqualDecl->setParams({lhsParamDecl, rhsParamDecl});

  // Lookup the `operator==` function that will be called under the hood.
  language::Core::UnresolvedSet<16> operators;
  language::Core::sema::DelayedDiagnosticPool diagPool{
      impl.getClangSema().DelayedDiagnostics.getCurrentPool()};
  auto diagState = impl.getClangSema().DelayedDiagnostics.push(diagPool);
  // Note: calling `CreateOverloadedBinOp` emits an error if the looked up
  // function is unavailable for the current target.
  auto underlyingCallResult = clangSema.CreateOverloadedBinOp(
      language::Core::SourceLocation(), operatorKind, operators, lhsParamRefExpr,
      rhsParamRefExpr);
  impl.getClangSema().DelayedDiagnostics.popWithoutEmitting(diagState);

  if (!diagPool.empty())
    return false;
  if (!underlyingCallResult.isUsable())
    return false;
  auto underlyingCall = underlyingCallResult.get();

  auto equalEqualBody = language::Core::ReturnStmt::Create(
      clangCtx, language::Core::SourceLocation(), underlyingCall, nullptr);
  equalEqualDecl->setBody(equalEqualBody);

  impl.synthesizedAndAlwaysVisibleDecls.insert(equalEqualDecl);
  auto lookupTable = impl.findLookupTable(classDecl);
  addEntryToLookupTable(*lookupTable, equalEqualDecl, impl.getNameImporter());
  return true;
}

bool language::isIterator(const language::Core::CXXRecordDecl *clangDecl) {
  return getIteratorCategoryDecl(clangDecl);
}

ValueDecl *
language::importer::getImportedMemberOperator(const DeclBaseName &name,
                                           NominalTypeDecl *selfType,
                                           std::optional<Type> parameterType) {
  assert(name.isOperator());
  // Handle ==, -, and += operators, that are required operators for C++
  // iterator types to conform to the corresponding Cxx iterator protocols.
  // These operators can be instantiated and synthesized by clang importer below,
  // and thus require additional lookup logic when they're being deserialized.
  if (name.getIdentifier() == selfType->getASTContext().Id_EqualsOperator) {
    return getEqualEqualOperator(selfType);
  }
  if (name.getIdentifier() == selfType->getASTContext().getIdentifier("-")) {
    return getMinusOperator(selfType);
  }
  if (name.getIdentifier() == selfType->getASTContext().getIdentifier("+=") &&
      parameterType) {
    return getPlusEqualOperator(selfType, *parameterType);
  }
  return nullptr;
}

void language::conformToCxxIteratorIfNeeded(
    ClangImporter::Implementation &impl, NominalTypeDecl *decl,
    const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to UnsafeCxxInputIterator", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();
  language::Core::ASTContext &clangCtx = clangDecl->getASTContext();

  if (!ctx.getProtocol(KnownProtocolKind::UnsafeCxxInputIterator))
    return;

  // We consider a type to be an input iterator if it defines an
  // `iterator_category` that inherits from `std::input_iterator_tag`, e.g.
  // `using iterator_category = std::input_iterator_tag`.
  auto iteratorCategory = getIteratorCategoryDecl(clangDecl);
  if (!iteratorCategory)
    return;

  auto unwrapUnderlyingTypeDecl =
      [](language::Core::TypeDecl *typeDecl) -> language::Core::CXXRecordDecl * {
    language::Core::CXXRecordDecl *underlyingDecl = nullptr;
    if (auto typedefDecl = dyn_cast<language::Core::TypedefNameDecl>(typeDecl)) {
      auto type = typedefDecl->getUnderlyingType();
      underlyingDecl = type->getAsCXXRecordDecl();
    } else {
      underlyingDecl = dyn_cast<language::Core::CXXRecordDecl>(typeDecl);
    }
    if (underlyingDecl) {
      underlyingDecl = underlyingDecl->getDefinition();
    }
    return underlyingDecl;
  };

  // If `iterator_category` is a typedef or a using-decl, retrieve the
  // underlying struct decl.
  auto underlyingCategoryDecl = unwrapUnderlyingTypeDecl(iteratorCategory);
  if (!underlyingCategoryDecl)
    return;

  auto isIteratorTagDecl = [&](const language::Core::CXXRecordDecl *base,
                               StringRef tag) {
    return base->isInStdNamespace() && base->getIdentifier() &&
           base->getName() == tag;
  };
  auto isInputIteratorDecl = [&](const language::Core::CXXRecordDecl *base) {
    return isIteratorTagDecl(base, "input_iterator_tag");
  };
  auto isRandomAccessIteratorDecl = [&](const language::Core::CXXRecordDecl *base) {
    return isIteratorTagDecl(base, "random_access_iterator_tag");
  };
  auto isContiguousIteratorDecl = [&](const language::Core::CXXRecordDecl *base) {
    return isIteratorTagDecl(base, "contiguous_iterator_tag"); // C++20
  };

  // Traverse all transitive bases of `underlyingDecl` to check if
  // it inherits from `std::input_iterator_tag`.
  bool isInputIterator = isInputIteratorDecl(underlyingCategoryDecl);
  bool isRandomAccessIterator =
      isRandomAccessIteratorDecl(underlyingCategoryDecl);
  underlyingCategoryDecl->forallBases([&](const language::Core::CXXRecordDecl *base) {
    if (isInputIteratorDecl(base)) {
      isInputIterator = true;
    }
    if (isRandomAccessIteratorDecl(base)) {
      isRandomAccessIterator = true;
      isInputIterator = true;
      return false;
    }
    return true;
  });

  if (!isInputIterator)
    return;

  bool isContiguousIterator = false;
  // In C++20, `std::contiguous_iterator_tag` is specified as a type called
  // `iterator_concept`. It is not possible to detect a contiguous iterator
  // based on its `iterator_category`. The type might not have an
  // `iterator_concept` defined.
  if (auto iteratorConcept = getIteratorConceptDecl(clangDecl)) {
    if (auto underlyingConceptDecl =
            unwrapUnderlyingTypeDecl(iteratorConcept)) {
      isContiguousIterator = isContiguousIteratorDecl(underlyingConceptDecl);
      if (!isContiguousIterator)
        underlyingConceptDecl->forallBases(
            [&](const language::Core::CXXRecordDecl *base) {
              if (isContiguousIteratorDecl(base)) {
                isContiguousIterator = true;
                return false;
              }
              return true;
            });
    }
  }

  // Check if present: `var pointee: Pointee { get }`
  auto pointeeId = ctx.getIdentifier("pointee");
  auto pointee = lookupDirectSingleWithoutExtensions<VarDecl>(decl, pointeeId);
  if (!pointee || pointee->isGetterMutating() || pointee->getTypeInContext()->hasError())
    return;

  // Check if `var pointee: Pointee` is settable. This is required for the
  // conformance to UnsafeCxxMutableInputIterator but is not necessary for
  // UnsafeCxxInputIterator.
  bool pointeeSettable = pointee->isSettable(nullptr);

  // Check if present: `fn successor() -> Self`
  auto successorId = ctx.getIdentifier("successor");
  auto successor =
      lookupDirectSingleWithoutExtensions<FuncDecl>(decl, successorId);
  if (!successor || successor->isMutating())
    return;
  auto successorTy = successor->getResultInterfaceType();
  if (!successorTy || successorTy->getAnyNominal() != decl)
    return;

  // Check if present: `fn ==`
  auto equalEqual = getEqualEqualOperator(decl);
  if (!equalEqual) {
    // If this class is inherited, `operator==` might be defined for a base
    // class. If this is a templated class, `operator==` might be templated as
    // well. Try to instantiate it.
    language::Core::FunctionDecl *instantiated = instantiateTemplatedOperator(
        impl, clangDecl, language::Core::BinaryOperatorKind::BO_EQ);
    if (instantiated && !impl.isUnavailableInCodira(instantiated)) {
      // If `operator==` was instantiated successfully, try to find `fn ==`
      // again.
      equalEqual = getEqualEqualOperator(decl);
      if (!equalEqual) {
        // If `fn ==` still can't be found, it might be defined for a base
        // class of the current class.
        auto paramTy = clangCtx.getRecordType(clangDecl);
        synthesizeCXXOperator(impl, clangDecl, language::Core::BinaryOperatorKind::BO_EQ,
                              paramTy, paramTy, clangCtx.BoolTy);
        equalEqual = getEqualEqualOperator(decl);
      }
    }
  }
  if (!equalEqual)
    return;

  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Pointee"),
                               pointee->getTypeInContext());
  if (pointeeSettable)
    impl.addSynthesizedProtocolAttrs(
        decl, {KnownProtocolKind::UnsafeCxxMutableInputIterator});
  else
    impl.addSynthesizedProtocolAttrs(
        decl, {KnownProtocolKind::UnsafeCxxInputIterator});

  if (!isRandomAccessIterator ||
      !ctx.getProtocol(KnownProtocolKind::UnsafeCxxRandomAccessIterator))
    return;

  // Try to conform to UnsafeCxxRandomAccessIterator if possible.

  // Check if present: `fn -`
  auto minus = getMinusOperator(decl);
  if (!minus) {
    language::Core::FunctionDecl *instantiated = instantiateTemplatedOperator(
        impl, clangDecl, language::Core::BinaryOperatorKind::BO_Sub);
    if (instantiated && !impl.isUnavailableInCodira(instantiated)) {
      minus = getMinusOperator(decl);
      if (!minus) {
        language::Core::QualType returnTy = instantiated->getReturnType();
        auto paramTy = clangCtx.getRecordType(clangDecl);
        synthesizeCXXOperator(impl, clangDecl,
                              language::Core::BinaryOperatorKind::BO_Sub, paramTy,
                              paramTy, returnTy);
        minus = getMinusOperator(decl);
      }
    }
  }
  if (!minus)
    return;
  auto distanceTy = minus->getResultInterfaceType();
  // distanceTy conforms to BinaryInteger, this is ensured by getMinusOperator.

  auto plusEqual = getPlusEqualOperator(decl, distanceTy);
  if (!plusEqual) {
    language::Core::FunctionDecl *instantiated = instantiateTemplatedOperator(
        impl, clangDecl, language::Core::BinaryOperatorKind::BO_AddAssign);
    if (instantiated && !impl.isUnavailableInCodira(instantiated)) {
      plusEqual = getPlusEqualOperator(decl, distanceTy);
      if (!plusEqual) {
        language::Core::QualType returnTy = instantiated->getReturnType();
        auto clangMinus = cast<language::Core::FunctionDecl>(minus->getClangDecl());
        auto lhsTy = clangCtx.getRecordType(clangDecl);
        auto rhsTy = clangMinus->getReturnType();
        synthesizeCXXOperator(impl, clangDecl,
                              language::Core::BinaryOperatorKind::BO_AddAssign, lhsTy,
                              rhsTy, returnTy);
        plusEqual = getPlusEqualOperator(decl, distanceTy);
      }
    }
  }
  if (!plusEqual)
    return;

  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Distance"), distanceTy);
  if (pointeeSettable)
    impl.addSynthesizedProtocolAttrs(
        decl, {KnownProtocolKind::UnsafeCxxMutableRandomAccessIterator});
  else
    impl.addSynthesizedProtocolAttrs(
        decl, {KnownProtocolKind::UnsafeCxxRandomAccessIterator});

  if (isContiguousIterator) {
    if (pointeeSettable)
      impl.addSynthesizedProtocolAttrs(
          decl, {KnownProtocolKind::UnsafeCxxMutableContiguousIterator});
    else
      impl.addSynthesizedProtocolAttrs(
          decl, {KnownProtocolKind::UnsafeCxxContiguousIterator});
  }
}

void language::conformToCxxConvertibleToBoolIfNeeded(
    ClangImporter::Implementation &impl, language::NominalTypeDecl *decl,
    const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxConvertibleToBool", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();

  auto conversionId = ctx.getIdentifier("__convertToBool");
  auto conversions = lookupDirectWithoutExtensions(decl, conversionId);

  // Find a non-mutating overload of `__convertToBool`.
  FuncDecl *conversion = nullptr;
  for (auto c : conversions) {
    auto candidate = dyn_cast<FuncDecl>(c);
    if (!candidate || candidate->isMutating())
      continue;
    if (conversion)
      // Overload ambiguity?
      return;
    conversion = candidate;
  }
  if (!conversion)
    return;
  auto conversionTy = conversion->getResultInterfaceType();
  if (!conversionTy->isBool())
    return;

  impl.addSynthesizedProtocolAttrs(decl,
                                   {KnownProtocolKind::CxxConvertibleToBool});
}

void language::conformToCxxOptionalIfNeeded(
    ClangImporter::Implementation &impl, NominalTypeDecl *decl,
    const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxOptional", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();
  language::Core::ASTContext &clangCtx = impl.getClangASTContext();
  language::Core::Sema &clangSema = impl.getClangSema();

  if (!isStdDecl(clangDecl, {"optional"}))
    return;

  ProtocolDecl *cxxOptionalProto =
      ctx.getProtocol(KnownProtocolKind::CxxOptional);
  // If the Cxx module is missing, or does not include one of the necessary
  // protocol, bail.
  if (!cxxOptionalProto)
    return;

  auto pointeeId = ctx.getIdentifier("pointee");
  auto pointees = lookupDirectWithoutExtensions(decl, pointeeId);
  if (pointees.size() != 1)
    return;
  auto pointee = dyn_cast<VarDecl>(pointees.front());
  if (!pointee)
    return;
  auto pointeeTy = pointee->getInterfaceType();

  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Wrapped"), pointeeTy);
  impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxOptional});

  // `std::optional` has a C++ constructor that takes the wrapped value as a
  // parameter. Unfortunately this constructor has templated parameter type, so
  // it isn't directly usable from Codira. Let's explicitly instantiate a
  // constructor with the wrapped value type, and then import it into Codira.

  auto valueTypeDecl = lookupNestedClangTypeDecl(clangDecl, "value_type");
  if (!valueTypeDecl)
    // `std::optional` without a value_type?!
    return;
  auto valueType = clangCtx.getTypeDeclType(valueTypeDecl);

  auto constRefValueType =
      clangCtx.getLValueReferenceType(valueType.withConst());
  // Create a fake variable with type of the wrapped value.
  auto fakeValueVarDecl = language::Core::VarDecl::Create(
      clangCtx, /*DC*/ clangCtx.getTranslationUnitDecl(),
      language::Core::SourceLocation(), language::Core::SourceLocation(), /*Id*/ nullptr,
      constRefValueType, clangCtx.getTrivialTypeSourceInfo(constRefValueType),
      language::Core::StorageClass::SC_None);
  auto fakeValueRefExpr = new (clangCtx) language::Core::DeclRefExpr(
      clangCtx, fakeValueVarDecl, false,
      constRefValueType.getNonReferenceType(), language::Core::ExprValueKind::VK_LValue,
      language::Core::SourceLocation());

  auto clangDeclTyInfo = clangCtx.getTrivialTypeSourceInfo(
      language::Core::QualType(clangDecl->getTypeForDecl(), 0));
  SmallVector<language::Core::Expr *, 1> constructExprArgs = {fakeValueRefExpr};

  // Instantiate the templated constructor that would accept this fake variable.
  language::Core::Sema::SFINAETrap trap(clangSema);
  auto constructExprResult = clangSema.BuildCXXTypeConstructExpr(
      clangDeclTyInfo, clangDecl->getLocation(), constructExprArgs,
      clangDecl->getLocation(), /*ListInitialization*/ false);
  if (!constructExprResult.isUsable() || trap.hasErrorOccurred())
    return;

  auto castExpr = dyn_cast_or_null<language::Core::CastExpr>(constructExprResult.get());
  if (!castExpr)
    return;

  // The temporary bind expression will only be present for some non-trivial C++
  // types.
  auto bindTempExpr =
      dyn_cast_or_null<language::Core::CXXBindTemporaryExpr>(castExpr->getSubExpr());

  auto constructExpr = dyn_cast_or_null<language::Core::CXXConstructExpr>(
      bindTempExpr ? bindTempExpr->getSubExpr() : castExpr->getSubExpr());
  if (!constructExpr)
    return;

  auto constructorDecl = constructExpr->getConstructor();

  auto importedConstructor =
      impl.importDecl(constructorDecl, impl.CurrentVersion);
  if (!importedConstructor)
    return;
  decl->addMember(importedConstructor);
}

void language::conformToCxxSequenceIfNeeded(
    ClangImporter::Implementation &impl, NominalTypeDecl *decl,
    const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxSequence", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();

  ProtocolDecl *cxxIteratorProto =
      ctx.getProtocol(KnownProtocolKind::UnsafeCxxInputIterator);
  ProtocolDecl *cxxSequenceProto =
      ctx.getProtocol(KnownProtocolKind::CxxSequence);
  ProtocolDecl *cxxConvertibleProto =
      ctx.getProtocol(KnownProtocolKind::CxxConvertibleToCollection);
  // If the Cxx module is missing, or does not include one of the necessary
  // protocols, bail.
  if (!cxxIteratorProto || !cxxSequenceProto)
    return;

  // Check if present: `fn __beginUnsafe() -> RawIterator`
  auto beginId = ctx.getIdentifier("__beginUnsafe");
  auto begin = lookupDirectSingleWithoutExtensions<FuncDecl>(decl, beginId);
  if (!begin)
    return;
  auto rawIteratorTy = begin->getResultInterfaceType();

  // Check if present: `fn __endUnsafe() -> RawIterator`
  auto endId = ctx.getIdentifier("__endUnsafe");
  auto end = lookupDirectSingleWithoutExtensions<FuncDecl>(decl, endId);
  if (!end)
    return;

  // Check if `begin()` and `end()` are non-mutating.
  if (begin->isMutating() || end->isMutating())
    return;

  // Check if `__beginUnsafe` and `__endUnsafe` have the same return type.
  auto endTy = end->getResultInterfaceType();
  if (!endTy || endTy->getCanonicalType() != rawIteratorTy->getCanonicalType())
    return;

  // Check if RawIterator conforms to UnsafeCxxInputIterator.
  auto rawIteratorConformanceRef =
      checkConformance(rawIteratorTy, cxxIteratorProto);
  if (!rawIteratorConformanceRef)
    return;
  auto rawIteratorConformance = rawIteratorConformanceRef.getConcrete();
  auto pointeeDecl =
      cxxIteratorProto->getAssociatedType(ctx.getIdentifier("Pointee"));
  assert(pointeeDecl &&
         "UnsafeCxxInputIterator must have a Pointee associated type");
  auto pointeeTy = rawIteratorConformance->getTypeWitness(pointeeDecl);
  assert(pointeeTy && "valid conformance must have a Pointee witness");

  // Take the default definition of `Iterator` from CxxSequence protocol. This
  // type is currently `CxxIterator<Self>`.
  auto iteratorDecl = cxxSequenceProto->getAssociatedType(ctx.Id_Iterator);
  auto iteratorTy = iteratorDecl->getDefaultDefinitionType();
  // Substitute generic `Self` parameter.
  auto cxxSequenceSelfTy = cxxSequenceProto->getSelfInterfaceType();
  auto declSelfTy = decl->getDeclaredInterfaceType();
  iteratorTy = iteratorTy.subst(
      [&](SubstitutableType *dependentType) {
        if (dependentType->isEqual(cxxSequenceSelfTy))
          return declSelfTy;
        return Type(dependentType);
      },
      LookUpConformanceInModule());

  impl.addSynthesizedTypealias(decl, ctx.Id_Element, pointeeTy);
  impl.addSynthesizedTypealias(decl, ctx.Id_Iterator, iteratorTy);
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("RawIterator"),
                               rawIteratorTy);
  // Not conforming the type to CxxSequence protocol here:
  // The current implementation of CxxSequence triggers extra copies of the C++
  // collection when creating a CxxIterator instance. It needs a more efficient
  // implementation, which is not possible with the existing Codira features.
  // impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxSequence});

  // Try to conform to CxxRandomAccessCollection if possible.

  auto tryToConformToRandomAccessCollection = [&]() -> bool {
    auto cxxRAIteratorProto =
        ctx.getProtocol(KnownProtocolKind::UnsafeCxxRandomAccessIterator);
    if (!cxxRAIteratorProto ||
        !ctx.getProtocol(KnownProtocolKind::CxxRandomAccessCollection))
      return false;

    // Check if RawIterator conforms to UnsafeCxxRandomAccessIterator.
    if (!checkConformance(rawIteratorTy, cxxRAIteratorProto))
      return false;

    // CxxRandomAccessCollection always uses Int as an Index.
    auto indexTy = ctx.getIntType();

    auto sliceTy = ctx.getSliceType();
    sliceTy = sliceTy.subst(
        [&](SubstitutableType *dependentType) {
          if (dependentType->isEqual(cxxSequenceSelfTy))
            return declSelfTy;
          return Type(dependentType);
        },
        LookUpConformanceInModule());

    auto indicesTy = ctx.getRangeType();
    indicesTy = indicesTy.subst(
        [&](SubstitutableType *dependentType) {
          if (dependentType->isEqual(cxxSequenceSelfTy))
            return indexTy;
          return Type(dependentType);
        },
        LookUpConformanceInModule());

    impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Element"), pointeeTy);
    impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Index"), indexTy);
    impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Indices"), indicesTy);
    impl.addSynthesizedTypealias(decl, ctx.getIdentifier("SubSequence"),
                                 sliceTy);

    auto tryToConformToMutatingRACollection = [&]() -> bool {
      auto rawMutableIteratorProto = ctx.getProtocol(
          KnownProtocolKind::UnsafeCxxMutableRandomAccessIterator);
      if (!rawMutableIteratorProto)
        return false;

      // Check if present: `fn __beginMutatingUnsafe() -> RawMutableIterator`
      auto beginMutatingId = ctx.getIdentifier("__beginMutatingUnsafe");
      auto beginMutating =
          lookupDirectSingleWithoutExtensions<FuncDecl>(decl, beginMutatingId);
      if (!beginMutating)
        return false;
      auto rawMutableIteratorTy = beginMutating->getResultInterfaceType();

      // Check if present: `fn __endMutatingUnsafe() -> RawMutableIterator`
      auto endMutatingId = ctx.getIdentifier("__endMutatingUnsafe");
      auto endMutating =
          lookupDirectSingleWithoutExtensions<FuncDecl>(decl, endMutatingId);
      if (!endMutating)
        return false;

      if (!checkConformance(rawMutableIteratorTy, rawMutableIteratorProto))
        return false;

      impl.addSynthesizedTypealias(
          decl, ctx.getIdentifier("RawMutableIterator"), rawMutableIteratorTy);
      impl.addSynthesizedProtocolAttrs(
          decl, {KnownProtocolKind::CxxMutableRandomAccessCollection});
      return true;
    };

    bool conformedToMutableRAC = tryToConformToMutatingRACollection();

    if (!conformedToMutableRAC)
      impl.addSynthesizedProtocolAttrs(
          decl, {KnownProtocolKind::CxxRandomAccessCollection});

    return true;
  };

  bool conformedToRAC = tryToConformToRandomAccessCollection();

  // If the collection does not support random access, let's still allow the
  // developer to explicitly convert a C++ sequence to a Codira Array (making a
  // copy of the sequence's elements) by conforming the type to
  // CxxCollectionConvertible. This enables an overload of Array.init declared
  // in the Cxx module.
  if (!conformedToRAC && cxxConvertibleProto) {
    impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Element"), pointeeTy);
    impl.addSynthesizedProtocolAttrs(
        decl, {KnownProtocolKind::CxxConvertibleToCollection});
  }
}

static bool isStdSetType(const language::Core::CXXRecordDecl *clangDecl) {
  return isStdDecl(clangDecl, {"set", "unordered_set", "multiset"});
}

static bool isStdMapType(const language::Core::CXXRecordDecl *clangDecl) {
  return isStdDecl(clangDecl, {"map", "unordered_map", "multimap"});
}

bool language::isUnsafeStdMethod(const language::Core::CXXMethodDecl *methodDecl) {
  auto parentDecl =
      dyn_cast<language::Core::CXXRecordDecl>(methodDecl->getDeclContext());
  if (!parentDecl)
    return false;
  if (!isStdSetType(parentDecl) && !isStdMapType(parentDecl))
    return false;
  if (methodDecl->getDeclName().isIdentifier() &&
      methodDecl->getName() == "insert")
    return true;
  return false;
}

void language::conformToCxxSetIfNeeded(ClangImporter::Implementation &impl,
                                    NominalTypeDecl *decl,
                                    const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxSet", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();

  // Only auto-conform types from the C++ standard library. Custom user types
  // might have a similar interface but different semantics.
  if (!isStdSetType(clangDecl))
    return;

  auto valueType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("value_type"));
  auto sizeType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("size_type"));
  if (!valueType || !sizeType)
    return;

  auto insert = getInsertFunc(decl, valueType);
  if (!insert)
    return;

  impl.addSynthesizedTypealias(decl, ctx.Id_Element,
                               valueType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.Id_ArrayLiteralElement,
                               valueType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Size"),
                               sizeType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("InsertionResult"),
                               insert->getResultInterfaceType());
  impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxSet});

  // If this isn't a std::multiset, try to also synthesize the conformance to
  // CxxUniqueSet.
  if (!isStdDecl(clangDecl, {"set", "unordered_set"}))
    return;

  ProtocolDecl *cxxInputIteratorProto =
      ctx.getProtocol(KnownProtocolKind::UnsafeCxxInputIterator);
  if (!cxxInputIteratorProto)
    return;

  auto rawIteratorType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("const_iterator"));
  auto rawMutableIteratorType =
      lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
          decl, ctx.getIdentifier("iterator"));
  if (!rawIteratorType || !rawMutableIteratorType)
    return;

  auto rawIteratorTy = rawIteratorType->getUnderlyingType();
  auto rawMutableIteratorTy = rawMutableIteratorType->getUnderlyingType();

  if (!checkConformance(rawIteratorTy, cxxInputIteratorProto) ||
      !checkConformance(rawMutableIteratorTy, cxxInputIteratorProto))
    return;

  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("RawIterator"),
                               rawIteratorTy);
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("RawMutableIterator"),
                               rawMutableIteratorTy);
  impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxUniqueSet});
}

void language::conformToCxxPairIfNeeded(ClangImporter::Implementation &impl,
                                     NominalTypeDecl *decl,
                                     const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxPair", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();

  // Only auto-conform types from the C++ standard library. Custom user types
  // might have a similar interface but different semantics.
  if (!isStdDecl(clangDecl, {"pair"}))
    return;

  auto firstType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("first_type"));
  auto secondType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("second_type"));
  if (!firstType || !secondType)
    return;

  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("First"),
                               firstType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Second"),
                               secondType->getUnderlyingType());
  impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxPair});
}

void language::conformToCxxDictionaryIfNeeded(
    ClangImporter::Implementation &impl, NominalTypeDecl *decl,
    const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxDictionary", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();

  // Only auto-conform types from the C++ standard library. Custom user types
  // might have a similar interface but different semantics.
  if (!isStdMapType(clangDecl))
    return;

  auto keyType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("key_type"));
  auto valueType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("mapped_type"));
  auto iterType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("const_iterator"));
  auto mutableIterType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("iterator"));
  auto sizeType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("size_type"));
  auto keyValuePairType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("value_type"));
  if (!keyType || !valueType || !iterType || !mutableIterType || !sizeType ||
      !keyValuePairType)
    return;

  auto insert = getInsertFunc(decl, keyValuePairType);
  if (!insert)
    return;

  ProtocolDecl *cxxInputIteratorProto =
      ctx.getProtocol(KnownProtocolKind::UnsafeCxxInputIterator);
  ProtocolDecl *cxxMutableInputIteratorProto =
      ctx.getProtocol(KnownProtocolKind::UnsafeCxxMutableInputIterator);
  if (!cxxInputIteratorProto || !cxxMutableInputIteratorProto)
    return;

  auto rawIteratorTy = iterType->getUnderlyingType();
  auto rawMutableIteratorTy = mutableIterType->getUnderlyingType();

  // Check if RawIterator conforms to UnsafeCxxInputIterator.
  if (!checkConformance(rawIteratorTy, cxxInputIteratorProto))
    return;

  // Check if RawMutableIterator conforms to UnsafeCxxMutableInputIterator.
  if (!checkConformance(rawMutableIteratorTy, cxxMutableInputIteratorProto))
    return;

  // Make the original subscript that returns a non-optional value unavailable.
  // CxxDictionary adds another subscript that returns an optional value,
  // similarly to Codira.Dictionary.
  for (auto member : decl->getCurrentMembersWithoutLoading()) {
    if (auto subscript = dyn_cast<SubscriptDecl>(member)) {
      impl.markUnavailable(subscript,
                           "use subscript with optional return value");
    }
  }

  impl.addSynthesizedTypealias(decl, ctx.Id_Key, keyType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.Id_Value,
                               valueType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.Id_Element,
                               keyValuePairType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("RawIterator"),
                               rawIteratorTy);
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("RawMutableIterator"),
                               rawMutableIteratorTy);
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Size"),
                               sizeType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("InsertionResult"),
                               insert->getResultInterfaceType());
  impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxDictionary});
}

void language::conformToCxxVectorIfNeeded(ClangImporter::Implementation &impl,
                                       NominalTypeDecl *decl,
                                       const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxVector", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();

  // Only auto-conform types from the C++ standard library. Custom user types
  // might have a similar interface but different semantics.
  if (!isStdDecl(clangDecl, {"vector"}))
    return;

  auto valueType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("value_type"));
  auto iterType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("const_iterator"));
  auto sizeType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("size_type"));
  if (!valueType || !iterType || !sizeType)
    return;

  ProtocolDecl *cxxRandomAccessIteratorProto =
      ctx.getProtocol(KnownProtocolKind::UnsafeCxxRandomAccessIterator);
  if (!cxxRandomAccessIteratorProto)
    return;

  auto rawIteratorTy = iterType->getUnderlyingType();

  // Check if RawIterator conforms to UnsafeCxxRandomAccessIterator.
  if (!checkConformance(rawIteratorTy, cxxRandomAccessIteratorProto))
    return;

  impl.addSynthesizedTypealias(decl, ctx.Id_Element,
                               valueType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.Id_ArrayLiteralElement,
                               valueType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Size"),
                               sizeType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("RawIterator"),
                               rawIteratorTy);
  impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxVector});
}

void language::conformToCxxFunctionIfNeeded(
    ClangImporter::Implementation &impl, NominalTypeDecl *decl,
    const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxFunction", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();
  language::Core::ASTContext &clangCtx = impl.getClangASTContext();
  language::Core::Sema &clangSema = impl.getClangSema();

  // Only auto-conform types from the C++ standard library. Custom user types
  // might have a similar interface but different semantics.
  if (!isStdDecl(clangDecl, {"function"}))
    return;

  // There is no typealias for the argument types on the C++ side, so to
  // retrieve the argument types we look at the overload of `operator()` that
  // got imported into Codira.

  auto callAsFunctionDecl = lookupDirectSingleWithoutExtensions<FuncDecl>(
      decl, ctx.getIdentifier("callAsFunction"));
  if (!callAsFunctionDecl)
    return;

  auto operatorCallDecl = dyn_cast_or_null<language::Core::CXXMethodDecl>(
      callAsFunctionDecl->getClangDecl());
  if (!operatorCallDecl)
    return;

  std::vector<language::Core::QualType> operatorCallParamTypes;
  toolchain::transform(
      operatorCallDecl->parameters(),
      std::back_inserter(operatorCallParamTypes),
      [](const language::Core::ParmVarDecl *paramDecl) { return paramDecl->getType(); });

  auto funcPointerType = clangCtx.getPointerType(clangCtx.getFunctionType(
      operatorCallDecl->getReturnType(), operatorCallParamTypes,
      language::Core::FunctionProtoType::ExtProtoInfo())).withConst();

  // Create a fake variable with a function type that matches the type of
  // `operator()`.
  auto fakeFuncPointerVarDecl = language::Core::VarDecl::Create(
      clangCtx, /*DC*/ clangCtx.getTranslationUnitDecl(),
      language::Core::SourceLocation(), language::Core::SourceLocation(), /*Id*/ nullptr,
      funcPointerType, clangCtx.getTrivialTypeSourceInfo(funcPointerType),
      language::Core::StorageClass::SC_None);
  auto fakeFuncPointerRefExpr = new (clangCtx) language::Core::DeclRefExpr(
      clangCtx, fakeFuncPointerVarDecl, false, funcPointerType,
      language::Core::ExprValueKind::VK_LValue, language::Core::SourceLocation());

  auto clangDeclTyInfo = clangCtx.getTrivialTypeSourceInfo(
      language::Core::QualType(clangDecl->getTypeForDecl(), 0));
  SmallVector<language::Core::Expr *, 1> constructExprArgs = {fakeFuncPointerRefExpr};

  // Instantiate the templated constructor that would accept this fake variable.
  auto constructExprResult = clangSema.BuildCXXTypeConstructExpr(
      clangDeclTyInfo, clangDecl->getLocation(), constructExprArgs,
      clangDecl->getLocation(), /*ListInitialization*/ false);
  if (!constructExprResult.isUsable())
    return;

  auto castExpr = dyn_cast_or_null<language::Core::CastExpr>(constructExprResult.get());
  if (!castExpr)
    return;

  auto bindTempExpr =
      dyn_cast_or_null<language::Core::CXXBindTemporaryExpr>(castExpr->getSubExpr());
  if (!bindTempExpr)
    return;

  auto constructExpr =
      dyn_cast_or_null<language::Core::CXXConstructExpr>(bindTempExpr->getSubExpr());
  if (!constructExpr)
    return;

  auto constructorDecl = constructExpr->getConstructor();

  auto importedConstructor =
      impl.importDecl(constructorDecl, impl.CurrentVersion);
  if (!importedConstructor)
    return;
  decl->addMember(importedConstructor);

  // TODO: actually conform to some form of CxxFunction protocol

}

void language::conformToCxxSpanIfNeeded(ClangImporter::Implementation &impl,
                                     NominalTypeDecl *decl,
                                     const language::Core::CXXRecordDecl *clangDecl) {
  PrettyStackTraceDecl trace("conforming to CxxSpan", decl);

  assert(decl);
  assert(clangDecl);
  ASTContext &ctx = decl->getASTContext();
  language::Core::ASTContext &clangCtx = impl.getClangASTContext();
  language::Core::Sema &clangSema = impl.getClangSema();

  // Only auto-conform types from the C++ standard library. Custom user types
  // might have a similar interface but different semantics.
  if (!isStdDecl(clangDecl, {"span"}))
    return;

  auto elementType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("element_type"));
  auto sizeType = lookupDirectSingleWithoutExtensions<TypeAliasDecl>(
      decl, ctx.getIdentifier("size_type"));

  if (!elementType || !sizeType)
    return;

  auto pointerTypeDecl = lookupNestedClangTypeDecl(clangDecl, "pointer");
  auto countTypeDecl = lookupNestedClangTypeDecl(clangDecl, "size_type");

  if (!pointerTypeDecl || !countTypeDecl)
    return;

  // create fake variable for pointer (constructor arg 1)
  language::Core::QualType pointerType = clangCtx.getTypeDeclType(pointerTypeDecl);
  auto fakePointerVarDecl = language::Core::VarDecl::Create(
      clangCtx, /*DC*/ clangCtx.getTranslationUnitDecl(),
      language::Core::SourceLocation(), language::Core::SourceLocation(), /*Id*/ nullptr,
      pointerType, clangCtx.getTrivialTypeSourceInfo(pointerType),
      language::Core::StorageClass::SC_None);

  auto fakePointer = new (clangCtx) language::Core::DeclRefExpr(
      clangCtx, fakePointerVarDecl, false, pointerType,
      language::Core::ExprValueKind::VK_LValue, language::Core::SourceLocation());

  // create fake variable for count (constructor arg 2)
  auto countType = clangCtx.getTypeDeclType(countTypeDecl);
  auto fakeCountVarDecl = language::Core::VarDecl::Create(
      clangCtx, /*DC*/ clangCtx.getTranslationUnitDecl(),
      language::Core::SourceLocation(), language::Core::SourceLocation(), /*Id*/ nullptr,
      countType, clangCtx.getTrivialTypeSourceInfo(countType),
      language::Core::StorageClass::SC_None);

  auto fakeCount = new (clangCtx) language::Core::DeclRefExpr(
      clangCtx, fakeCountVarDecl, false, countType,
      language::Core::ExprValueKind::VK_LValue, language::Core::SourceLocation());

  // Use clangSema.BuildCxxTypeConstructExpr to create a CXXTypeConstructExpr,
  // passing constPointer and count
  SmallVector<language::Core::Expr *, 2> constructExprArgs = {fakePointer, fakeCount};

  auto clangDeclTyInfo = clangCtx.getTrivialTypeSourceInfo(
      language::Core::QualType(clangDecl->getTypeForDecl(), 0));

  // Instantiate the templated constructor that would accept this fake variable.
  auto constructExprResult = clangSema.BuildCXXTypeConstructExpr(
      clangDeclTyInfo, clangDecl->getLocation(), constructExprArgs,
      clangDecl->getLocation(), /*ListInitialization*/ false);
  if (!constructExprResult.isUsable())
    return;

  auto constructExpr =
      dyn_cast_or_null<language::Core::CXXConstructExpr>(constructExprResult.get());
  if (!constructExpr)
    return;

  auto constructorDecl = constructExpr->getConstructor();
  auto importedConstructor =
      impl.importDecl(constructorDecl, impl.CurrentVersion);
  if (!importedConstructor)
    return;

  auto attr = AvailableAttr::createUniversallyDeprecated(
      importedConstructor->getASTContext(), "use 'init(_:)' instead.", "");
  importedConstructor->getAttrs().add(attr);

  decl->addMember(importedConstructor);

  impl.addSynthesizedTypealias(decl, ctx.Id_Element,
                               elementType->getUnderlyingType());
  impl.addSynthesizedTypealias(decl, ctx.getIdentifier("Size"),
                               sizeType->getUnderlyingType());

  if (pointerType->getPointeeType().isConstQualified()) {
    impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxSpan});
  } else {
    impl.addSynthesizedProtocolAttrs(decl, {KnownProtocolKind::CxxMutableSpan});
  }
}
