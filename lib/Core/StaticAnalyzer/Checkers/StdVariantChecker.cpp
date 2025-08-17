//===- StdVariantChecker.cpp -------------------------------------*- C++ -*-==//
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

#include "language/Core/AST/Type.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/StringRef.h"
#include <optional>

#include "TaggedUnionModeling.h"

using namespace language::Core;
using namespace ento;
using namespace tagged_union_modeling;

REGISTER_MAP_WITH_PROGRAMSTATE(VariantHeldTypeMap, const MemRegion *, QualType)

namespace language::Core::ento::tagged_union_modeling {

static const CXXConstructorDecl *
getConstructorDeclarationForCall(const CallEvent &Call) {
  const auto *ConstructorCall = dyn_cast<CXXConstructorCall>(&Call);
  if (!ConstructorCall)
    return nullptr;

  return ConstructorCall->getDecl();
}

bool isCopyConstructorCall(const CallEvent &Call) {
  if (const CXXConstructorDecl *ConstructorDecl =
          getConstructorDeclarationForCall(Call))
    return ConstructorDecl->isCopyConstructor();
  return false;
}

bool isCopyAssignmentCall(const CallEvent &Call) {
  const Decl *CopyAssignmentDecl = Call.getDecl();

  if (const auto *AsMethodDecl =
          dyn_cast_or_null<CXXMethodDecl>(CopyAssignmentDecl))
    return AsMethodDecl->isCopyAssignmentOperator();
  return false;
}

bool isMoveConstructorCall(const CallEvent &Call) {
  const CXXConstructorDecl *ConstructorDecl =
      getConstructorDeclarationForCall(Call);
  if (!ConstructorDecl)
    return false;

  return ConstructorDecl->isMoveConstructor();
}

bool isMoveAssignmentCall(const CallEvent &Call) {
  const Decl *CopyAssignmentDecl = Call.getDecl();

  const auto *AsMethodDecl =
      dyn_cast_or_null<CXXMethodDecl>(CopyAssignmentDecl);
  if (!AsMethodDecl)
    return false;

  return AsMethodDecl->isMoveAssignmentOperator();
}

static bool isStdType(const Type *Type, toolchain::StringRef TypeName) {
  auto *Decl = Type->getAsRecordDecl();
  if (!Decl)
    return false;
  return (Decl->getName() == TypeName) && Decl->isInStdNamespace();
}

bool isStdVariant(const Type *Type) {
  return isStdType(Type, toolchain::StringLiteral("variant"));
}

} // end of namespace language::Core::ento::tagged_union_modeling

static std::optional<ArrayRef<TemplateArgument>>
getTemplateArgsFromVariant(const Type *VariantType) {
  const auto *TempSpecType = VariantType->getAs<TemplateSpecializationType>();
  if (!TempSpecType)
    return {};

  return TempSpecType->template_arguments();
}

static std::optional<QualType>
getNthTemplateTypeArgFromVariant(const Type *varType, unsigned i) {
  std::optional<ArrayRef<TemplateArgument>> VariantTemplates =
      getTemplateArgsFromVariant(varType);
  if (!VariantTemplates)
    return {};

  return (*VariantTemplates)[i].getAsType();
}

static bool isVowel(char a) {
  switch (a) {
  case 'a':
  case 'e':
  case 'i':
  case 'o':
  case 'u':
    return true;
  default:
    return false;
  }
}

static toolchain::StringRef indefiniteArticleBasedOnVowel(char a) {
  if (isVowel(a))
    return "an";
  return "a";
}

class StdVariantChecker : public Checker<eval::Call, check::RegionChanges> {
  // Call descriptors to find relevant calls
  CallDescription VariantConstructor{CDM::CXXMethod,
                                     {"std", "variant", "variant"}};
  CallDescription VariantAssignmentOperator{CDM::CXXMethod,
                                            {"std", "variant", "operator="}};
  CallDescription StdGet{CDM::SimpleFunc, {"std", "get"}, 1, 1};

  BugType BadVariantType{this, "BadVariantType", "BadVariantType"};

public:
  ProgramStateRef checkRegionChanges(ProgramStateRef State,
                                     const InvalidatedSymbols *,
                                     ArrayRef<const MemRegion *>,
                                     ArrayRef<const MemRegion *> Regions,
                                     const LocationContext *,
                                     const CallEvent *Call) const {
    if (!Call)
      return State;

    return removeInformationStoredForDeadInstances<VariantHeldTypeMap>(
        *Call, State, Regions);
  }

  bool evalCall(const CallEvent &Call, CheckerContext &C) const {
    // Check if the call was not made from a system header. If it was then
    // we do an early return because it is part of the implementation.
    if (Call.isCalledFromSystemHeader())
      return false;

    if (StdGet.matches(Call))
      return handleStdGetCall(Call, C);

    // First check if a constructor call is happening. If it is a
    // constructor call, check if it is an std::variant constructor call.
    bool IsVariantConstructor =
        isa<CXXConstructorCall>(Call) && VariantConstructor.matches(Call);
    bool IsVariantAssignmentOperatorCall =
        isa<CXXMemberOperatorCall>(Call) &&
        VariantAssignmentOperator.matches(Call);

    if (IsVariantConstructor || IsVariantAssignmentOperatorCall) {
      if (Call.getNumArgs() == 0 && IsVariantConstructor) {
        handleDefaultConstructor(cast<CXXConstructorCall>(&Call), C);
        return true;
      }

      // FIXME Later this checker should be extended to handle constructors
      // with multiple arguments.
      if (Call.getNumArgs() != 1)
        return false;

      SVal ThisSVal;
      if (IsVariantConstructor) {
        const auto &AsConstructorCall = cast<CXXConstructorCall>(Call);
        ThisSVal = AsConstructorCall.getCXXThisVal();
      } else if (IsVariantAssignmentOperatorCall) {
        const auto &AsMemberOpCall = cast<CXXMemberOperatorCall>(Call);
        ThisSVal = AsMemberOpCall.getCXXThisVal();
      } else {
        return false;
      }

      handleConstructorAndAssignment<VariantHeldTypeMap>(Call, C, ThisSVal);
      return true;
    }
    return false;
  }

private:
  // The default constructed std::variant must be handled separately
  // by default the std::variant is going to hold a default constructed instance
  // of the first type of the possible types
  void handleDefaultConstructor(const CXXConstructorCall *ConstructorCall,
                                CheckerContext &C) const {
    SVal ThisSVal = ConstructorCall->getCXXThisVal();

    const auto *const ThisMemRegion = ThisSVal.getAsRegion();
    if (!ThisMemRegion)
      return;

    std::optional<QualType> DefaultType = getNthTemplateTypeArgFromVariant(
        ThisSVal.getType(C.getASTContext())->getPointeeType().getTypePtr(), 0);
    if (!DefaultType)
      return;

    ProgramStateRef State = ConstructorCall->getState();
    State = State->set<VariantHeldTypeMap>(ThisMemRegion, *DefaultType);
    C.addTransition(State);
  }

  bool handleStdGetCall(const CallEvent &Call, CheckerContext &C) const {
    ProgramStateRef State = Call.getState();

    const auto &ArgType = Call.getArgSVal(0)
                              .getType(C.getASTContext())
                              ->getPointeeType()
                              .getTypePtr();
    // We have to make sure that the argument is an std::variant.
    // There is another std::get with std::pair argument
    if (!isStdVariant(ArgType))
      return false;

    // Get the mem region of the argument std::variant and look up the type
    // information that we know about it.
    const MemRegion *ArgMemRegion = Call.getArgSVal(0).getAsRegion();
    const QualType *StoredType = State->get<VariantHeldTypeMap>(ArgMemRegion);
    if (!StoredType)
      return false;

    const CallExpr *CE = cast<CallExpr>(Call.getOriginExpr());
    const FunctionDecl *FD = CE->getDirectCallee();
    if (FD->getTemplateSpecializationArgs()->size() < 1)
      return false;

    const auto &TypeOut = FD->getTemplateSpecializationArgs()->asArray()[0];
    // std::get's first template parameter can be the type we want to get
    // out of the std::variant or a natural number which is the position of
    // the requested type in the argument type list of the std::variant's
    // argument.
    QualType RetrievedType;
    switch (TypeOut.getKind()) {
    case TemplateArgument::ArgKind::Type:
      RetrievedType = TypeOut.getAsType();
      break;
    case TemplateArgument::ArgKind::Integral:
      // In the natural number case we look up which type corresponds to the
      // number.
      if (std::optional<QualType> NthTemplate =
              getNthTemplateTypeArgFromVariant(
                  ArgType, TypeOut.getAsIntegral().getSExtValue())) {
        RetrievedType = *NthTemplate;
        break;
      }
      [[fallthrough]];
    default:
      return false;
    }

    QualType RetrievedCanonicalType = RetrievedType.getCanonicalType();
    QualType StoredCanonicalType = StoredType->getCanonicalType();
    if (RetrievedCanonicalType == StoredCanonicalType)
      return true;

    ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode)
      return false;
    toolchain::SmallString<128> Str;
    toolchain::raw_svector_ostream OS(Str);
    std::string StoredTypeName = StoredType->getAsString();
    std::string RetrievedTypeName = RetrievedType.getAsString();
    OS << "std::variant " << ArgMemRegion->getDescriptiveName() << " held "
       << indefiniteArticleBasedOnVowel(StoredTypeName[0]) << " \'"
       << StoredTypeName << "\', not "
       << indefiniteArticleBasedOnVowel(RetrievedTypeName[0]) << " \'"
       << RetrievedTypeName << "\'";
    auto R = std::make_unique<PathSensitiveBugReport>(BadVariantType, OS.str(),
                                                      ErrNode);
    C.emitReport(std::move(R));
    return true;
  }
};

bool language::Core::ento::shouldRegisterStdVariantChecker(
    language::Core::ento::CheckerManager const &mgr) {
  return true;
}

void language::Core::ento::registerStdVariantChecker(language::Core::ento::CheckerManager &mgr) {
  mgr.registerChecker<StdVariantChecker>();
}
