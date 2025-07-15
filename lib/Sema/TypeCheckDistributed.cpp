//===--- TypeCheckDistributed.cpp - Distributed ---------------------------===//
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
//
// This file implements type checking support for Codira's concurrency model.
//
//===----------------------------------------------------------------------===//
#include "TypeCheckConcurrency.h"
#include "TypeCheckDistributed.h"
#include "TypeChecker.h"
#include "language/Strings.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/ConformanceLookup.h"
#include "language/AST/Initializer.h"
#include "language/AST/ParameterList.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/DistributedDecl.h"
#include "language/AST/NameLookupRequests.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/AST/TypeVisitor.h"
#include "language/AST/ImportCache.h"
#include "language/AST/ExistentialLayout.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/AST/ASTPrinter.h"

using namespace language;

// ==== ------------------------------------------------------------------------

bool language::ensureDistributedModuleLoaded(const ValueDecl *decl) {
  auto &C = decl->getASTContext();
  auto moduleAvailable = evaluateOrDefault(
      C.evaluator, DistributedModuleIsAvailableRequest{decl}, false);
  return moduleAvailable;
}

bool
DistributedModuleIsAvailableRequest::evaluate(Evaluator &evaluator,
                                              const ValueDecl *decl) const {
  auto &C = decl->getASTContext();

  auto DistributedModule = C.getLoadedModule(C.Id_Distributed);
  if (!DistributedModule) {
    decl->diagnose(diag::distributed_decl_needs_explicit_distributed_import,
                   decl)
        .fixItAddImport("Distributed");
    return false;
  }

  auto &importCache = C.getImportCache();
  if (importCache.isImportedBy(DistributedModule, decl->getDeclContext())) {
    return true;
  }

  // seems we're missing the Distributed module, ask to import it explicitly
  decl->diagnose(diag::distributed_decl_needs_explicit_distributed_import,
                 decl);
  return false;
}

/******************************************************************************/
/************ LOCATING AD-HOC PROTOCOL REQUIREMENT IMPLS **********************/
/******************************************************************************/

static AbstractFunctionDecl *findDistributedAdHocRequirement(
    NominalTypeDecl *decl, Identifier identifier,
    std::function<bool(AbstractFunctionDecl *)> matchFn) {
  auto &C = decl->getASTContext();

  // It would be nice to check if this is a DistributedActorSystem
  // "conforming" type, but we can't do this as we invoke this function WHILE
  // deciding if the type conforms or not;

  // Not via `ensureDistributedModuleLoaded` to avoid generating a warning,
  // we won't be emitting the offending decl after all.
  if (!C.getLoadedModule(C.Id_Distributed)) {
    return nullptr;
  }

  toolchain::SmallVector<ValueDecl *, 2> results;
  decl->lookupQualified(decl, DeclNameRef(identifier),
                        SourceLoc(), NL_QualifiedDefault, results);
  for (auto value : results) {
    auto fn = dyn_cast<AbstractFunctionDecl>(value);
    if (fn && matchFn(fn))
      return fn;
  }

  return nullptr;
}

AbstractFunctionDecl *
GetDistributedActorSystemRemoteCallFunctionRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *decl, bool isVoidReturn) const {
  auto &C = decl->getASTContext();
  auto callId = isVoidReturn ? C.Id_remoteCallVoid : C.Id_remoteCall;

  return findDistributedAdHocRequirement(
      decl, callId, [isVoidReturn](AbstractFunctionDecl *fn) {
        return fn->isDistributedActorSystemRemoteCall(isVoidReturn);
      });
}

AbstractFunctionDecl *
GetDistributedTargetInvocationEncoderRecordArgumentFunctionRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *decl) const {
  auto &C = decl->getASTContext();

  return findDistributedAdHocRequirement(
      decl, C.Id_recordArgument, [](AbstractFunctionDecl *fn) {
        return fn->isDistributedTargetInvocationEncoderRecordArgument();
      });
}

AbstractFunctionDecl *
GetDistributedTargetInvocationEncoderRecordReturnTypeFunctionRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *decl) const {
  auto &C = decl->getASTContext();

  return findDistributedAdHocRequirement(
      decl, C.Id_recordReturnType, [](AbstractFunctionDecl *fn) {
        return fn->isDistributedTargetInvocationEncoderRecordReturnType();
      });
}

AbstractFunctionDecl *
GetDistributedTargetInvocationEncoderRecordErrorTypeFunctionRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *decl) const {
  auto &C = decl->getASTContext();
  return findDistributedAdHocRequirement(
      decl, C.Id_recordErrorType, [](AbstractFunctionDecl *fn) {
        return fn->isDistributedTargetInvocationEncoderRecordErrorType();
      });
}

AbstractFunctionDecl *
GetDistributedTargetInvocationDecoderDecodeNextArgumentFunctionRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *decl) const {
  auto &C = decl->getASTContext();
  return findDistributedAdHocRequirement(
      decl, C.Id_decodeNextArgument, [](AbstractFunctionDecl *fn) {
        return fn->isDistributedTargetInvocationDecoderDecodeNextArgument();
      });
}

AbstractFunctionDecl *
GetDistributedTargetInvocationResultHandlerOnReturnFunctionRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *decl) const {
  auto &C = decl->getASTContext();
  return findDistributedAdHocRequirement(
      decl, C.Id_onReturn, [](AbstractFunctionDecl *fn) {
        return fn->isDistributedTargetInvocationResultHandlerOnReturn();
      });
}

// ==== ------------------------------------------------------------------------

/// Add Fix-It text for the given protocol type to inherit DistributedActor.
void language::diagnoseDistributedFunctionInNonDistributedActorProtocol(
    const ProtocolDecl *proto, InFlightDiagnostic &diag) {
  if (proto->getInherited().empty()) {
    SourceLoc fixItLoc = proto->getBraces().Start;
    diag.fixItInsert(fixItLoc, ": DistributedActor");
  } else {
    // Similar to how Sendable FitIts do this, we insert at the end of
    // the inherited types.
    SourceLoc fixItLoc = proto->getInherited().getEndLoc();
    diag.fixItInsertAfter(fixItLoc, ", DistributedActor");
  }
}


/// Add Fix-It text for the given nominal type to adopt Codable.
///
/// Useful when 'Codable' is the 'SerializationRequirement' and a non-Codable
/// function parameter or return value type is detected.
void language::addCodableFixIt(
    const NominalTypeDecl *nominal, InFlightDiagnostic &diag) {
  if (nominal->getInherited().empty()) {
    SourceLoc fixItLoc = nominal->getBraces().Start;
    diag.fixItInsert(fixItLoc, ": Codable");
  } else {
    SourceLoc fixItLoc = nominal->getInherited().getEndLoc();
    diag.fixItInsertAfter(fixItLoc, ", Codable");
  }
}

// ==== ------------------------------------------------------------------------

bool IsDistributedActorRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *nominal) const {
  // Protocols are actors if they inherit from `DistributedActor`.
  if (auto protocol = dyn_cast<ProtocolDecl>(nominal)) {
    auto &ctx = protocol->getASTContext();
    auto *distributedActorProtocol = ctx.getDistributedActorDecl();
    if (!distributedActorProtocol)
      return false;

    return (protocol == distributedActorProtocol ||
            protocol->inheritsFrom(distributedActorProtocol));
  }

  // Class declarations are 'distributed actors' if they are declared with
  // 'distributed actor'
  auto classDecl = dyn_cast<ClassDecl>(nominal);
  if(!classDecl)
    return false;

  return classDecl->isExplicitDistributedActor();
}

// ==== ------------------------------------------------------------------------

static bool checkAdHocRequirementAccessControl(
    NominalTypeDecl *decl,
    ProtocolDecl *proto,
    AbstractFunctionDecl *fn) {
  if (!fn)
    return true;

  // === check access control
  if (fn->getEffectiveAccess() == decl->getEffectiveAccess()) {
    return false;
  }

  fn->diagnose(diag::witness_not_accessible_type, diag::RequirementKind::Func,
                 fn, /*isSetter=*/false,
                 /*requiredAccess=*/AccessLevel::Public, AccessLevel::Public,
                 proto);
      return true;
}

static bool diagnoseMissingAdHocProtocolRequirement(ASTContext &C, Identifier identifier, NominalTypeDecl *decl) {
  assert(decl);
  auto FixitLocation = decl->getBraces().Start;

  // Prepare the indent (same as `printRequirementStub`)
  StringRef ExtraIndent;
  StringRef CurrentIndent =
      Lexer::getIndentationForLine(C.SourceMgr, decl->getStartLoc(), &ExtraIndent);

  toolchain::SmallString<128> Text;
  toolchain::raw_svector_ostream OS(Text);
  ExtraIndentStreamPrinter Printer(OS, CurrentIndent);

  Printer.printNewline();
  Printer.printIndent();
  Printer << (decl->getFormalAccess() == AccessLevel::Public ? "public " : "");

  if (identifier == C.Id_remoteCall) {
    Printer << "fn remoteCall<Act, Err, Res>("
               "on actor: Act, "
               "target: RemoteCallTarget, "
               "invocation: inout InvocationEncoder, "
               "throwing: Err.Type, "
               "returning: Res.Type) "
               "async throws -> Res "
               "where Act: DistributedActor, "
               "Act.ID == ActorID, "
               "Err: Error, "
               "Res: SerializationRequirement";
  } else if (identifier == C.Id_remoteCallVoid) {
    Printer << "fn remoteCallVoid<Act, Err>("
               "on actor: Act, "
               "target: RemoteCallTarget, "
               "invocation: inout InvocationEncoder, "
               "throwing: Err.Type"
               ") async throws "
               "where Act: DistributedActor, "
               "Act.ID == ActorID, "
               "Err: Error";
  } else if (identifier == C.Id_recordArgument) {
    Printer << "mutating fn recordArgument<Value: SerializationRequirement>(_ argument: RemoteCallArgument<Value>) throws";
  } else if (identifier == C.Id_recordReturnType) {
    Printer << "mutating fn recordReturnType<Res: SerializationRequirement>(_ resultType: Res.Type) throws";
  } else if (identifier == C.Id_decodeNextArgument) {
    Printer << "mutating fn decodeNextArgument<Argument: SerializationRequirement>() throws -> Argument";
  } else if (identifier == C.Id_onReturn) {
    Printer << "fn onReturn<Success: SerializationRequirement>(value: Success) async throws";
  } else {
    toolchain_unreachable("Unknown identifier for diagnosing ad-hoc missing requirement.");
  }

  /// Print the "{ <#code#> }" placeholder body
  Printer << " {\n";
  Printer << ExtraIndent << getCodePlaceholder();
  Printer.printNewline();
  Printer.printIndent();
  Printer << "}\n";

  decl->diagnose(
      diag::distributed_actor_system_conformance_missing_adhoc_requirement,
      decl, identifier);
  decl->diagnose(diag::missing_witnesses_general)
      .fixItInsertAfter(FixitLocation, Text.str());

  return true;
}

bool language::checkDistributedActorSystemAdHocProtocolRequirements(
    ASTContext &C,
    ProtocolDecl *Proto,
    NormalProtocolConformance *Conformance,
    Type Adoptee,
    bool diagnose) {
  auto decl = Adoptee->getAnyNominal();
  auto anyMissingAdHocRequirements = false;

  // ==== ----------------------------------------------------------------------
  // Check the ad-hoc requirements of 'DistributedActorSystem":
  if (Proto->isSpecificProtocol(KnownProtocolKind::DistributedActorSystem)) {
    // - remoteCall
    auto remoteCallDecl =
        getRemoteCallOnDistributedActorSystem(decl, /*isVoidReturn=*/false);
    if (!remoteCallDecl && diagnose) {
      anyMissingAdHocRequirements = diagnoseMissingAdHocProtocolRequirement(C, C.Id_remoteCall, decl);
    }
    if (checkAdHocRequirementAccessControl(decl, Proto, remoteCallDecl)) {
      anyMissingAdHocRequirements = true;
    }

    // - remoteCallVoid
    auto remoteCallVoidDecl =
        getRemoteCallOnDistributedActorSystem(decl, /*isVoidReturn=*/true);
    if (!remoteCallVoidDecl && diagnose) {
      anyMissingAdHocRequirements = diagnoseMissingAdHocProtocolRequirement(C, C.Id_remoteCallVoid, decl);
    }
    if (checkAdHocRequirementAccessControl(decl, Proto, remoteCallVoidDecl)) {
      anyMissingAdHocRequirements = true;
    }

    return anyMissingAdHocRequirements;
  }

  // ==== ----------------------------------------------------------------------
  // Check the ad-hoc requirements of 'DistributedTargetInvocationEncoder'
  if (Proto->isSpecificProtocol(KnownProtocolKind::DistributedTargetInvocationEncoder)) {
    // - recordArgument
    auto recordArgumentDecl =
        getRecordArgumentOnDistributedInvocationEncoder(decl);
    if (!recordArgumentDecl) {
      anyMissingAdHocRequirements = diagnoseMissingAdHocProtocolRequirement(C, C.Id_recordArgument, decl);
    }
    if (checkAdHocRequirementAccessControl(decl, Proto, recordArgumentDecl)) {
      anyMissingAdHocRequirements = true;
    }

    // - recordReturnType
    auto recordReturnTypeDecl =
        getRecordReturnTypeOnDistributedInvocationEncoder(decl);
    if (!recordReturnTypeDecl) {
      anyMissingAdHocRequirements = diagnoseMissingAdHocProtocolRequirement(C, C.Id_recordReturnType, decl);
    }
    if (checkAdHocRequirementAccessControl(decl, Proto, recordReturnTypeDecl)) {
      anyMissingAdHocRequirements = true;
    }

    return anyMissingAdHocRequirements;
  }

  // ==== ----------------------------------------------------------------------
  // Check the ad-hoc requirements of 'DistributedTargetInvocationDecoder'
  if (Proto->isSpecificProtocol(KnownProtocolKind::DistributedTargetInvocationDecoder)) {
    // - decodeNextArgument
    auto decodeNextArgumentDecl =
        getDecodeNextArgumentOnDistributedInvocationDecoder(decl);
    if (!decodeNextArgumentDecl) {
      anyMissingAdHocRequirements = diagnoseMissingAdHocProtocolRequirement(C, C.Id_decodeNextArgument, decl);
    }
    if (checkAdHocRequirementAccessControl(decl, Proto, decodeNextArgumentDecl)) {
      anyMissingAdHocRequirements = true;
    }

    return anyMissingAdHocRequirements;
  }

  // === -----------------------------------------------------------------------
  // Check the ad-hoc requirements of 'DistributedTargetInvocationResultHandler'
  if (Proto->isSpecificProtocol(KnownProtocolKind::DistributedTargetInvocationResultHandler)) {
    // - onReturn
    auto onReturnDecl =
        getOnReturnOnDistributedTargetInvocationResultHandler(decl);
    if (!onReturnDecl) {
      anyMissingAdHocRequirements = diagnoseMissingAdHocProtocolRequirement(C, C.Id_onReturn, decl);
    }
    if (checkAdHocRequirementAccessControl(decl, Proto, onReturnDecl)) {
      anyMissingAdHocRequirements = true;
    }

    return anyMissingAdHocRequirements;
  }

  assert(!anyMissingAdHocRequirements &&
         "Should have returned in appropriate type checking block earlier!");
  return false;
}

static bool checkDistributedTargetResultType(
    ValueDecl *valueDecl,
    Type serializationRequirement,
    bool diagnose) {
  auto &C = valueDecl->getASTContext();

  if (serializationRequirement && serializationRequirement->hasError()) {
    return false;
  }
  if (!serializationRequirement || serializationRequirement->hasError()) {
    return false; // error of the type would be diagnosed elsewhere
  }

  Type resultType;
  if (auto fn = dyn_cast<FuncDecl>(valueDecl)) {
    resultType = fn->mapTypeIntoContext(fn->getResultInterfaceType());
  } else if (auto var = dyn_cast<VarDecl>(valueDecl)) {
    // Distributed computed properties are always getters,
    // so get the get accessor for mapping the type into context:
    auto getter = var->getAccessor(language::AccessorKind::Get);
    resultType = getter->mapTypeIntoContext(var->getInterfaceType());
  } else {
    toolchain_unreachable("Unsupported distributed target");
  }

  if (resultType->isVoid())
    return false;

  SmallVector<ProtocolDecl *, 4> serializationRequirements;
  // Collect extra "SerializationRequirement: SomeProtocol" requirements
  if (serializationRequirement && !serializationRequirement->hasError()) {
    auto srl = serializationRequirement->getExistentialLayout();
    toolchain::copy(srl.getProtocols(),
               std::back_inserter(serializationRequirements));
  }

  auto isCodableRequirement =
      checkDistributedSerializationRequirementIsExactlyCodable(
          C, serializationRequirement);

  for (auto serializationReq: serializationRequirements) {
    auto conformance = checkConformance(resultType, serializationReq);
    if (conformance.isInvalid()) {
      if (diagnose) {
        toolchain::StringRef conformanceToSuggest = isCodableRequirement ?
                                               "Codable" : // Codable is a typealias, easier to diagnose like that
                                               serializationReq->getNameStr();

        auto diag = valueDecl->diagnose(
            diag::distributed_actor_target_result_not_codable,
            resultType,
            valueDecl,
            conformanceToSuggest
        );

        if (isCodableRequirement) {
          if (auto resultNominalType = resultType->getAnyNominal()) {
            addCodableFixIt(resultNominalType, diag);
          }
        }
      } // end if: diagnose

      return true;
    }
  }

    return false;
}

bool language::checkDistributedActorSystem(const NominalTypeDecl *system) {
  auto nominal = const_cast<NominalTypeDecl *>(system);

  // ==== Ensure the Distributed module is available,
  // without it there's no reason to check the decl in more detail anyway.
  if (!language::ensureDistributedModuleLoaded(nominal))
    return true;

  // === AssociatedTypes
  // --- SerializationRequirement MUST be a protocol TODO(distributed): rdar://91663941
  // we may lift this in the future and allow classes but this requires more
  // work to enable associatedtypes to be constrained to class or protocol,
  // which then will unlock using them as generic constraints in protocols.
  Type requirementTy = getDistributedActorSystemSerializationType(nominal);

  if (auto existentialTy = requirementTy->getAs<ExistentialType>()) {
    requirementTy = existentialTy->getConstraintType();
  }

  if (auto alias = dyn_cast<TypeAliasType>(requirementTy.getPointer())) {
    auto concreteReqTy = alias->getDesugaredType();
    if (isa<ProtocolCompositionType>(concreteReqTy)) {
      // ok, protocol composition is fine as requirement,
      // since special case of just a single protocol
    } else if (isa<ProtocolType>(concreteReqTy)) {
      // ok, protocols is exactly what we want to be used as constraints here
    } else {
      nominal->diagnose(diag::distributed_actor_system_serialization_req_must_be_protocol,
                        requirementTy);
      return true;
    }
  }

  // all good, didn't find any errors
  return false;
}

/// Check whether the function is a proper distributed function
///
/// \returns \c true if there was a problem with adding the attribute, \c false
/// otherwise.
bool language::checkDistributedFunction(AbstractFunctionDecl *fn) {
  if (!fn->isDistributed())
    return false;

  // ==== Ensure the Distributed module is available,
  if (!language::ensureDistributedModuleLoaded(fn))
    return true;

  auto &C = fn->getASTContext();
  return evaluateOrDefault(C.evaluator,
                           CheckDistributedFunctionRequest{fn},
                           false); // no error if cycle
}

bool CheckDistributedFunctionRequest::evaluate(
    Evaluator &evaluator, AbstractFunctionDecl *fn) const {
  if (auto *accessor = dyn_cast<AccessorDecl>(fn)) {
    auto *var = cast<VarDecl>(accessor->getStorage());
    assert(var->isDistributed() && accessor->isDistributedGetter());
  } else {
    assert(fn->isDistributed());
  }

  auto &C = fn->getASTContext();

  /// If no distributed module is available, then no reason to even try checks.
  if (!C.getLoadedModule(C.Id_Distributed)) {
    fn->diagnose(diag::distributed_decl_needs_explicit_distributed_import,
                   fn);
    return true;
  }

  Type serializationReqType =
      getDistributedActorSerializationType(fn->getDeclContext());

  for (auto param: *fn->getParameters()) {
    // --- Check the parameter conforming to serialization requirements
    if (serializationReqType && !serializationReqType->hasError()) {
      // If the requirement is exactly `Codable` we diagnose it ia bit nicer.
      auto serializationRequirementIsCodable =
          checkDistributedSerializationRequirementIsExactlyCodable(
              C, serializationReqType);

      // --- Check parameters for 'SerializationRequirement' conformance
      auto paramTy = fn->mapTypeIntoContext(param->getInterfaceType());

      auto srl = serializationReqType->getExistentialLayout();
      for (auto req: srl.getProtocols()) {
        if (checkConformance(paramTy, req).isInvalid()) {
          auto diag = fn->diagnose(
              diag::distributed_actor_func_param_not_codable,
              param->getArgumentName(), param->getInterfaceType(), fn,
              serializationRequirementIsCodable ? "Codable"
                                                : req->getNameStr());

          if (auto paramNominalTy = paramTy->getAnyNominal()) {
            addCodableFixIt(paramNominalTy, diag);
          } // else, no nominal type to suggest the fixit for, e.g. a closure

          return true;
        }
      }
    }

    // --- Check parameters for various illegal modifiers
    if (param->isInOut()) {
      param->diagnose(
          diag::distributed_actor_func_inout,
          param->getName(),
          fn
      ).fixItRemove(SourceRange(param->getTypeSourceRangeForDiagnostics().Start,
                                param->getTypeSourceRangeForDiagnostics().Start.getAdvancedLoc(1)));
      // FIXME(distributed): the fixIt should be on param->getSpecifierLoc(), but that Loc is invalid for some reason?
      return true;
    }

    if (param->getSpecifier() == ParamSpecifier::LegacyShared ||
        param->getSpecifier() == ParamSpecifier::LegacyOwned ||
        param->getSpecifier() == ParamSpecifier::Consuming ||
        param->getSpecifier() == ParamSpecifier::Borrowing) {
      param->diagnose(
          diag::distributed_actor_func_unsupported_specifier,
          ParamDecl::getSpecifierSpelling(param->getSpecifier()),
          param->getName(),
          fn);
      return true;
    }

    if (param->isVariadic()) {
      param->diagnose(
          diag::distributed_actor_func_variadic,
          param->getName(),
          fn
      );
    }
  }

  // --- Result type must be either void or a serialization requirement conforming type
  if (checkDistributedTargetResultType(fn, serializationReqType,
                                       /*diagnose=*/true)) {
    return true;
  }

  return false;
}

/// Check whether the function is a proper distributed computed property
///
/// \param diagnose Whether to emit a diagnostic when a problem is encountered.
///
/// \returns \c true if there was a problem with adding the attribute, \c false
/// otherwise.
bool language::checkDistributedActorProperty(VarDecl *var, bool diagnose) {
  // without the distributed module, we can't check any of these.
  if (!ensureDistributedModuleLoaded(var))
    return true;

  /// === Check if the declaration is a valid combination of attributes
  if (var->isStatic()) {
    if (diagnose)
      var->diagnose(diag::distributed_property_cannot_be_static,
                    var->getName());
    // TODO(distributed): fixit, offer removing the static keyword
    return true;
  }

  // it is not a computed property
  if (var->isLet() || var->hasStorageOrWrapsStorage()) {
    if (diagnose)
      var->diagnose(diag::distributed_property_can_only_be_computed, var);
    return true;
  }

  // distributed properties cannot have setters
  if (var->getWriteImpl() != language::WriteImplKind::Immutable) {
    if (diagnose)
      var->diagnose(diag::distributed_property_can_only_be_computed_get_only,
                    var->getName());
    return true;
  }

  auto serializationRequirement =
      getDistributedActorSerializationType(var->getDeclContext());

  if (checkDistributedTargetResultType(var, serializationRequirement, diagnose)) {
    return true;
  }

  return false;
}

void language::checkDistributedActorProperties(const NominalTypeDecl *decl) {
  auto &C = decl->getASTContext();

  if (!decl->getDeclContext()->getParentSourceFile()) {
    // Don't diagnose when checking without source file (e.g. from module, importer etc).
    return;
  }

  if (decl->getDeclContext()->isInCodirainterface()) {
    // Don't diagnose properties in languageinterfaces.
    return;
  }

  if (isa<ProtocolDecl>(decl)) {
    // protocols don't matter for stored property checking
    return;
  }

  for (auto member : decl->getMembers()) {
    if (auto prop = dyn_cast<VarDecl>(member)) {
      if (prop->isSynthesized())
        continue;

      auto id = prop->getName();
      if (id == C.Id_actorSystem || id == C.Id_id) {
        prop->diagnose(diag::distributed_actor_user_defined_special_property,
                      id);
        prop->setInvalid();
      }
    }
  }
}

// ==== ------------------------------------------------------------------------

void TypeChecker::checkDistributedActor(SourceFile *SF, NominalTypeDecl *nominal) {
  if (!nominal || !nominal->isDistributedActor())
    return;

  // ==== Ensure the Distributed module is available,
  // without it there's no reason to check the decl in more detail anyway.
  if (!language::ensureDistributedModuleLoaded(nominal))
    return;

  auto &C = nominal->getASTContext();
  auto loc = nominal->getLoc();
  recordRequiredImportAccessLevelForDecl(
    C.getDistributedActorDecl(), nominal, nominal->getEffectiveAccess(),
    [&](AttributedImport<ImportedModule> attributedImport) {
  ModuleDecl *importedVia = attributedImport.module.importedModule,
             *sourceModule = nominal->getModuleContext();
  C.Diags.diagnose(loc, diag::module_api_import, nominal, importedVia,
                         sourceModule, importedVia == sourceModule,
                         /*isImplicit*/ false);
});

  // ==== Constructors
  // --- Get the default initializer
  // If applicable, this will create the default 'init(transport:)' initializer
  (void)nominal->getDefaultInitializer();

  for (auto member : nominal->getMembers()) {
    // --- Ensure 'distributed fn' all thunks
    if (auto *var = dyn_cast<VarDecl>(member)) {
      if (!var->isDistributed())
        continue;

      if (auto thunk = var->getDistributedThunk())
        SF->addDelayedFunction(thunk);

      continue;
    }

    // --- Ensure 'distributed fn' all thunks
    if (auto fn = dyn_cast<AbstractFunctionDecl>(member)) {
      if (auto dtor = dyn_cast<DestructorDecl>(fn)) {
        ASTContext &C = dtor->getASTContext();
        auto selfDecl = dtor->getImplicitSelfDecl();
        selfDecl->getAttrs().add(new (C) KnownToBeLocalAttr(true));
      }
      if (!fn->isDistributed())
        continue;

      if (!isa<ProtocolDecl>(nominal)) {
        auto systemTy = getConcreteReplacementForProtocolActorSystemType(fn);
        if (!systemTy || systemTy->hasError()) {
          nominal->diagnose(
              diag::distributed_actor_conformance_missing_system_type,
              nominal->getName());
          return;
        }
      }

      if (auto thunk = fn->getDistributedThunk()) {
        SF->addDelayedFunction(thunk);
      }
    }
  }

  // ==== Properties
  checkDistributedActorProperties(nominal);
  // --- Synthesize the 'id' property here rather than via derived conformance
  //     because the 'DerivedConformanceDistributedActor' won't trigger for 'id'
  //     because it has a default impl via 'Identifiable' (ObjectIdentifier)
  //     which we do not want.
  // Also, the 'id' var must be added before the 'actorSystem'.
  // See NOTE (id-before-actorSystem) for more details.
  (void)nominal->getDistributedActorIDProperty();
}

bool TypeChecker::checkDistributedFunc(FuncDecl *fn) {
  return language::checkDistributedFunction(fn);
}

ConstructorDecl*
GetDistributedRemoteCallTargetInitFunctionRequest::evaluate(
    Evaluator &evaluator,
    NominalTypeDecl *nominal) const {
  auto &C = nominal->getASTContext();

  // not via `ensureDistributedModuleLoaded` to avoid generating a warning,
  // we won't be emitting the offending decl after all.
  if (!C.getLoadedModule(C.Id_Distributed))
    return nullptr;

  if (!nominal->getDeclaredInterfaceType()->isEqual(
          C.getRemoteCallTargetType()))
    return nullptr;

  for (auto value : nominal->getMembers()) {
    auto ctor = dyn_cast<ConstructorDecl>(value);
    if (!ctor)
      continue;

    auto params = ctor->getParameters();
    if (params->size() != 1)
      return nullptr;

    // _ identifier
    if (params->get(0)->getArgumentName().empty())
      return ctor;

    return nullptr;
  }

  return nullptr;
}

ConstructorDecl*
GetDistributedRemoteCallArgumentInitFunctionRequest::evaluate(
    Evaluator &evaluator,
    NominalTypeDecl *nominal) const {
  auto &C = nominal->getASTContext();

  // not via `ensureDistributedModuleLoaded` to avoid generating a warning,
  // we won't be emitting the offending decl after all.
  if (!C.getLoadedModule(C.Id_Distributed))
    return nullptr;

  if (!nominal->getDeclaredInterfaceType()->isEqual(
          C.getRemoteCallArgumentType()))
    return nullptr;

  for (auto value : nominal->getMembers()) {
    auto ctor = dyn_cast<ConstructorDecl>(value);
    if (!ctor)
      continue;

    auto params = ctor->getParameters();
    if (params->size() != 3)
      return nullptr;

    // --- param: label
    if (!params->get(0)->getArgumentName().is("label"))
      return nullptr;

    // --- param: name
    if (!params->get(1)->getArgumentName().is("name"))
      return nullptr;

    // --- param: value
    if (params->get(2)->getArgumentName() != C.Id_value)
      return nullptr;

    return ctor;
  }

  return nullptr;
}

NominalTypeDecl *
GetDistributedActorInvocationDecoderRequest::evaluate(Evaluator &evaluator,
                                                      NominalTypeDecl *actor) const {
  auto &ctx = actor->getASTContext();
  auto decoderTy = getAssociatedTypeOfDistributedSystemOfActor(
      actor, ctx.Id_InvocationDecoder);
  return decoderTy ? decoderTy->getAnyNominal() : nullptr;
}

FuncDecl *
GetDistributedActorConcreteArgumentDecodingMethodRequest::evaluate(
    Evaluator &evaluator, NominalTypeDecl *decl) const {
  auto &ctx = decl->getASTContext();

  if (auto actor = dyn_cast<ClassDecl>(decl)) {
    auto *decoder = getDistributedActorInvocationDecoder(actor);
    // If distributed actor is generic over actor system, there is not
    // going to be a concrete decoder.
    if (!decoder)
      return nullptr;

    auto decoderTy = decoder->getDeclaredInterfaceType();

    auto members =
        TypeChecker::lookupMember(actor->getDeclContext(), decoderTy,
                                  DeclNameRef(ctx.Id_decodeNextArgument));

    // typealias SerializationRequirement = any ...
    auto serializationTy = getAssociatedTypeOfDistributedSystemOfActor(
        actor, ctx.Id_SerializationRequirement);

    if (!serializationTy || !serializationTy->is<ExistentialType>())
      return nullptr;

    SmallVector<ProtocolDecl *, 4> serializationRequirements;
    {
      auto layout = serializationTy->getExistentialLayout();
      toolchain::copy(layout.getProtocols(),
                 std::back_inserter(serializationRequirements));
    }

    SmallVector<FuncDecl *, 2> candidates;
    // Looking for `decodeNextArgument<Arg: <SerializationReq>>() throws -> Arg`
    for (auto &member : members) {
      auto *FD = dyn_cast<FuncDecl>(member.getValueDecl());
      if (!FD || FD->hasAsync() || !FD->hasThrows())
        continue;

      auto *params = FD->getParameters();
      // No arguments.
      if (params->size() != 0)
        continue;

      auto genericParamList = FD->getGenericParams();
      // A single generic parameter.
      if (genericParamList->size() != 1)
        continue;

      auto paramTy =
          genericParamList->getParams()[0]->getDeclaredInterfaceType();

      // `decodeNextArgument` should return its generic parameter value
      if (!FD->getResultInterfaceType()->isEqual(paramTy))
        continue;

      // Let's find out how many serialization requirements does this method cover e.g. `Codable` is two requirements - `Encodable` and `Decodable`.
      auto nextArgumentSig = FD->getGenericSignature();
      bool okay =
          toolchain::all_of(serializationRequirements, [&](ProtocolDecl *p) -> bool {
            return nextArgumentSig->requiresProtocol(paramTy, p);
          });

      // If the current method covers all of the serialization requirements,
      // it's a match. Note that it might also have other requirements, but
      // we let that go as long as there are no two candidates that differ
      // only in generic requirements.
      if (okay)
        candidates.push_back(FD);
    }

    // Type-checker should reject any definition of invocation decoder
    // that doesn't have a correct version of `decodeNextArgument` declared.
    assert(candidates.size() == 1);
    return candidates.front();
  }

  /// No concrete candidate found, return null and perform the call via a
  /// witness
  return nullptr;
}

toolchain::ArrayRef<ValueDecl *>
GetDistributedMethodWitnessedProtocolRequirements::evaluate(
    Evaluator &evaluator,
    AbstractFunctionDecl *afd) const {
  // Only a 'distributed' decl can witness 'distributed' protocol
  assert(afd->isDistributed());
  auto &C = afd->getASTContext();

  auto result = toolchain::SmallVector<ValueDecl *, 1>();
  for (auto witnessedRequirement : afd->getSatisfiedProtocolRequirements()) {
    if (witnessedRequirement->isDistributed()) {
      result.push_back(witnessedRequirement);
    }
  }

  return C.AllocateCopy(result);
}
