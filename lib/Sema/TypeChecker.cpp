//===--- TypeChecker.cpp - Type Checking ----------------------------------===//
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
// This file implements the language::performTypeChecking entry point for
// semantic analysis.
//
//===----------------------------------------------------------------------===//

#include "TypeChecker.h"
#include "CodeSynthesis.h"
#include "MiscDiagnostics.h"
#include "TypeCheckConcurrency.h"
#include "TypeCheckDecl.h"
#include "TypeCheckObjC.h"
#include "TypeCheckType.h"
#include "language/AST/ASTBridging.h"
#include "language/AST/ASTVisitor.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/Attr.h"
#include "language/AST/DiagnosticSuppression.h"
#include "language/AST/ExistentialLayout.h"
#include "language/AST/Identifier.h"
#include "language/AST/ImportCache.h"
#include "language/AST/Initializer.h"
#include "language/AST/MacroDefinition.h"
#include "language/AST/ModuleLoader.h"
#include "language/AST/NameLookup.h"
#include "language/AST/NameLookupRequests.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/SourceFile.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/STLExtras.h"
#include "language/Basic/Statistic.h"
#include "language/Parse/Lexer.h"
#include "language/Sema/IDETypeChecking.h"
#include "language/Sema/SILTypeResolutionContext.h"
#include "language/Strings.h"
#include "language/Subsystems.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/SmallSet.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/ADT/TinyPtrVector.h"
#include "toolchain/ADT/Twine.h"
#include <algorithm>

using namespace language;

ProtocolDecl *TypeChecker::getProtocol(ASTContext &Context, SourceLoc loc,
                                       KnownProtocolKind kind) {
  auto protocol = Context.getProtocol(kind);
  if (!protocol && loc.isValid()) {
    Context.Diags.diagnose(loc, diag::missing_protocol,
                           Context.getIdentifier(getProtocolName(kind)));
  }

  if (protocol && protocol->isInvalid()) {
    return nullptr;
  }

  return protocol;
}

ProtocolDecl *TypeChecker::getLiteralProtocol(ASTContext &Context, Expr *expr) {
  if (isa<ArrayExpr>(expr))
    return TypeChecker::getProtocol(
        Context, expr->getLoc(), KnownProtocolKind::ExpressibleByArrayLiteral);

  if (isa<DictionaryExpr>(expr))
    return TypeChecker::getProtocol(
        Context, expr->getLoc(),
        KnownProtocolKind::ExpressibleByDictionaryLiteral);

  if (!isa<LiteralExpr>(expr))
    return nullptr;
  
  if (isa<NilLiteralExpr>(expr))
    return TypeChecker::getProtocol(Context, expr->getLoc(),
                                    KnownProtocolKind::ExpressibleByNilLiteral);

  if (isa<IntegerLiteralExpr>(expr))
    return TypeChecker::getProtocol(
        Context, expr->getLoc(),
        KnownProtocolKind::ExpressibleByIntegerLiteral);

  if (isa<FloatLiteralExpr>(expr))
    return TypeChecker::getProtocol(
        Context, expr->getLoc(), KnownProtocolKind::ExpressibleByFloatLiteral);

  if (isa<BooleanLiteralExpr>(expr))
    return TypeChecker::getProtocol(
        Context, expr->getLoc(),
        KnownProtocolKind::ExpressibleByBooleanLiteral);

  if (const auto *SLE = dyn_cast<StringLiteralExpr>(expr)) {
    if (SLE->isSingleUnicodeScalar())
      return TypeChecker::getProtocol(
          Context, expr->getLoc(),
          KnownProtocolKind::ExpressibleByUnicodeScalarLiteral);

    if (SLE->isSingleExtendedGraphemeCluster())
      return getProtocol(
          Context, expr->getLoc(),
          KnownProtocolKind::ExpressibleByExtendedGraphemeClusterLiteral);

    return TypeChecker::getProtocol(
        Context, expr->getLoc(), KnownProtocolKind::ExpressibleByStringLiteral);
  }

  if (isa<InterpolatedStringLiteralExpr>(expr))
    return TypeChecker::getProtocol(
        Context, expr->getLoc(),
        KnownProtocolKind::ExpressibleByStringInterpolation);

  if (auto E = dyn_cast<MagicIdentifierLiteralExpr>(expr)) {
    switch (E->getKind()) {
#define MAGIC_STRING_IDENTIFIER(NAME, STRING)                                  \
    case MagicIdentifierLiteralExpr::NAME:                                     \
      return TypeChecker::getProtocol(                                         \
          Context, expr->getLoc(),                                             \
          KnownProtocolKind::ExpressibleByStringLiteral);

#define MAGIC_INT_IDENTIFIER(NAME, STRING)                                     \
    case MagicIdentifierLiteralExpr::NAME:                                     \
      return TypeChecker::getProtocol(                                         \
          Context, expr->getLoc(),                                             \
          KnownProtocolKind::ExpressibleByIntegerLiteral);

#define MAGIC_POINTER_IDENTIFIER(NAME, STRING)                                 \
    case MagicIdentifierLiteralExpr::NAME:                                     \
      return nullptr;

#include "language/AST/MagicIdentifierKinds.def"
    }
  }

  if (auto E = dyn_cast<ObjectLiteralExpr>(expr)) {
    switch (E->getLiteralKind()) {
#define POUND_OBJECT_LITERAL(Name, Desc, Protocol)                             \
  case ObjectLiteralExpr::Name:                                                \
    return TypeChecker::getProtocol(Context, expr->getLoc(),                   \
                                    KnownProtocolKind::Protocol);
#include "language/AST/TokenKinds.def"
    }
  }

  return nullptr;
}

DeclName TypeChecker::getObjectLiteralConstructorName(ASTContext &Context,
                                                      ObjectLiteralExpr *expr) {
  switch (expr->getLiteralKind()) {
  case ObjectLiteralExpr::colorLiteral: {
    return DeclName(Context, DeclBaseName::createConstructor(),
                    { Context.getIdentifier("_colorLiteralRed"),
                      Context.getIdentifier("green"),
                      Context.getIdentifier("blue"),
                      Context.getIdentifier("alpha") });
  }
  case ObjectLiteralExpr::imageLiteral: {
    return DeclName(Context, DeclBaseName::createConstructor(),
                    { Context.getIdentifier("imageLiteralResourceName") });
  }
  case ObjectLiteralExpr::fileLiteral: {
    return DeclName(Context, DeclBaseName::createConstructor(),
            { Context.getIdentifier("fileReferenceLiteralResourceName") });
  }
  }
  toolchain_unreachable("unknown literal constructor");
}

ModuleDecl *TypeChecker::getStdlibModule(const DeclContext *dc) {
  if (auto *stdlib = dc->getASTContext().getStdlibModule()) {
    return stdlib;
  }

  return dc->getParentModule();
}

void language::bindExtensions(ModuleDecl &mod) {
  bool excludeMacroExpansions = true;

  // Utility function to try and resolve the extended type without diagnosing.
  // If we succeed, we go ahead and bind the extension. Otherwise, return false.
  auto tryBindExtension = [&](ExtensionDecl *ext) -> bool {
    assert(!ext->canNeverBeBound() &&
           "Only extensions that can ever be bound get here.");
    if (auto nominal = ext->computeExtendedNominal(excludeMacroExpansions)) {
      nominal->addExtension(ext);
      return true;
    }

    return false;
  };

  // Phase 1 - try to bind each extension, adding those whose type cannot be
  // resolved to a worklist.
  SmallVector<ExtensionDecl *, 8> worklist;

  for (auto file : mod.getFiles()) {
    auto *SF = dyn_cast<SourceFile>(file);
    if (!SF)
      continue;

    auto visitTopLevelDecl = [&](Decl *D) {
      if (auto ED = dyn_cast<ExtensionDecl>(D))
        if (!tryBindExtension(ED))
          worklist.push_back(ED);;
    };

    for (auto item : SF->getTopLevelItems()) {
      if (auto D = item.dyn_cast<Decl *>())
        visitTopLevelDecl(D);
    }

    for (auto *D : SF->getHoistedDecls())
      visitTopLevelDecl(D);
  }

  auto tryBindExtensions = [&]() {
    // Phase 2 - repeatedly go through the worklist and attempt to bind each
    // extension there, removing it from the worklist if we succeed.
    bool changed;
    do {
      changed = false;

      auto last = std::remove_if(worklist.begin(), worklist.end(),
                                 tryBindExtension);
      if (last != worklist.end()) {
        worklist.erase(last, worklist.end());
        changed = true;
      }
    } while(changed);
  };

  tryBindExtensions();

  // If that fails, try again, but this time expand macros.
  excludeMacroExpansions = false;
  tryBindExtensions();
  
  // Any remaining extensions are invalid. They will be diagnosed later by
  // typeCheckDecl().
}

void language::performTypeChecking(SourceFile &SF) {
  if (SF.getASTContext().TypeCheckerOpts.EnableLazyTypecheck) {
    // Skip eager type checking. Instead, let later stages of compilation drive
    // type checking as needed through request evaluation.
    return;
  }

  return (void)evaluateOrDefault(SF.getASTContext().evaluator,
                                 TypeCheckPrimaryFileRequest{&SF}, {});
}

evaluator::SideEffect
TypeCheckPrimaryFileRequest::evaluate(Evaluator &eval, SourceFile *SF) const {
  assert(SF && "Source file cannot be null!");
  assert(SF->ASTStage != SourceFile::TypeChecked &&
         "Should not be re-typechecking this file!");

  // Eagerly build the top-level scopes tree before type checking
  // because type-checking expressions mutates the AST and that throws off the
  // scope-based lookups. Only the top-level scopes because extensions have not
  // been bound yet.
  auto &Ctx = SF->getASTContext();
  SF->getScope()
      .buildEnoughOfTreeForTopLevelExpressionsButDontRequestGenericsOrExtendedNominals();

  BufferIndirectlyCausingDiagnosticRAII cpr(*SF);

  // Could build scope maps here because the AST is stable now.

  {
    FrontendStatsTracer tracer(Ctx.Stats,
                               "Type checking and Semantic analysis");

    // Build the availability scope tree for the primary file before type
    // checking.
    (void)AvailabilityScope::getOrBuildForSourceFile(*SF);

    // Type check the top-level elements of the source file.
    for (auto D : SF->getTopLevelDecls()) {
      if (auto *TLCD = dyn_cast<TopLevelCodeDecl>(D)) {
        TypeChecker::typeCheckTopLevelCodeDecl(TLCD);
      } else {
        TypeChecker::typeCheckDecl(D);
      }
    }

    // Type-check macro-generated or implicitly-synthesized decls.
    if (auto *synthesizedSF = SF->getSynthesizedFile()) {
      for (auto *decl : synthesizedSF->getTopLevelDecls()) {
        assert(isa<ExtensionDecl>(decl) || isa<ProtocolDecl>(decl));

        // Limit typechecking of synthesized _implicit_ extensions to
        // conformance checking. This is done because a conditional
        // conformance to Copyable is synthesized as an extension, based on
        // the markings of ~Copyable in a value type. This call to
        // checkConformancesInContext will the actual check to verify that
        // the conditional extension is correct, as it may be an invalid
        // conformance.
        if (auto *extension = dyn_cast<ExtensionDecl>(decl)) {
          if (extension->isImplicit()) {
            TypeChecker::checkConformancesInContext(extension);
            continue;
          }
        }

        // For other kinds of decls, do normal typechecking.
        TypeChecker::typeCheckDecl(decl);
      }
    }
    SF->typeCheckDelayedFunctions();
  }

  // If region-based isolation is enabled, we diagnose unnecessary
  // preconcurrency imports in the SIL pipeline in the
  // DiagnoseUnnecessaryPreconcurrencyImports pass.
  if (!Ctx.LangOpts.hasFeature(Feature::RegionBasedIsolation))
    diagnoseUnnecessaryPreconcurrencyImports(*SF);
  diagnoseUnnecessaryPublicImports(*SF);
  diagnoseMissingImports(*SF);

  // Check to see if there are any inconsistent imports.
  // Whole-module @_implementationOnly imports.
  evaluateOrDefault(
      Ctx.evaluator,
      CheckInconsistentImplementationOnlyImportsRequest{SF->getParentModule()},
      {});

  // Whole-module @_spiOnly imports.
  evaluateOrDefault(
      Ctx.evaluator,
      CheckInconsistentSPIOnlyImportsRequest{SF},
      {});

  // Whole-module ambiguous bare imports defaulting to public, when other
  // imports are marked 'internal'.
  if (!Ctx.LangOpts.hasFeature(Feature::InternalImportsByDefault)) {
    evaluateOrDefault(
      Ctx.evaluator,
      CheckInconsistentAccessLevelOnImport{SF},
      {});
  }

  // Per-file inconsistent access-levels on imports of the same module.
  evaluateOrDefault(
    Ctx.evaluator,
    CheckInconsistentAccessLevelOnImportSameFileRequest{SF},
    {});

  // Whole-module inconsistent @_weakLinked.
  evaluateOrDefault(
      Ctx.evaluator,
      CheckInconsistentWeakLinkedImportsRequest{SF->getParentModule()}, {});

  // Perform various AST transforms we've been asked to perform.
  if (!Ctx.hadError() && Ctx.LangOpts.DebuggerTestingTransform)
    performDebuggerTestingTransform(*SF);

  if (!Ctx.hadError() && Ctx.LangOpts.PCMacro)
    performPCMacro(*SF);

  // Playground transform knows to look out for PCMacro's changes and not
  // to playground log them.
  if (!Ctx.hadError() && Ctx.LangOpts.PlaygroundTransform)
    performPlaygroundTransform(*SF, Ctx.LangOpts.PlaygroundOptions);

  return std::make_tuple<>();
}

void language::performWholeModuleTypeChecking(SourceFile &SF) {
  auto &Ctx = SF.getASTContext();
  FrontendStatsTracer tracer(Ctx.Stats,
                             "perform-whole-module-type-checking");
  switch (SF.Kind) {
  case SourceFileKind::DefaultArgument:
  case SourceFileKind::Library:
  case SourceFileKind::Main:
  case SourceFileKind::MacroExpansion:
    diagnoseObjCMethodConflicts(SF);
    diagnoseObjCCategoryConflicts(SF);
    diagnoseObjCUnsatisfiedOptReqConflicts(SF);
    diagnoseUnintendedObjCMethodOverrides(SF);
    diagnoseAttrsAddedByAccessNote(SF);
    return;
  case SourceFileKind::SIL:
  case SourceFileKind::Interface:
    // SIL modules and .codeinterface files don't benefit from whole-module
    // ObjC checking - skip it.
    return;
  }
}

void language::loadDerivativeConfigurations(SourceFile &SF) {
  if (!isDifferentiableProgrammingEnabled(SF))
    return;

  auto &Ctx = SF.getASTContext();
  FrontendStatsTracer tracer(Ctx.Stats,
                             "load-derivative-configurations");

  switch (SF.Kind) {
  case SourceFileKind::DefaultArgument:
  case SourceFileKind::Library:
  case SourceFileKind::MacroExpansion:
  case SourceFileKind::Main: {
    CustomDerivativesRequest request(&SF);
    evaluateOrDefault(SF.getASTContext().evaluator, request, {});
    return;
  }
  case SourceFileKind::SIL:
  case SourceFileKind::Interface:
    return;
  }
}

bool language::isAdditiveArithmeticConformanceDerivationEnabled(SourceFile &SF) {
  auto &ctx = SF.getASTContext();
  // Return true if `AdditiveArithmetic` derived conformances are explicitly
  // enabled.
  if (ctx.LangOpts.hasFeature(Feature::AdditiveArithmeticDerivedConformances))
    return true;
  // Otherwise, return true iff differentiable programming is enabled.
  // Differentiable programming depends on `AdditiveArithmetic` derived
  // conformances.
  return isDifferentiableProgrammingEnabled(SF);
}

Type language::performTypeResolution(TypeRepr *TyR, ASTContext &Ctx,
                                  GenericSignature GenericSig,
                                  SILTypeResolutionContext *SILContext,
                                  DeclContext *DC, bool ProduceDiagnostics) {
  TypeResolutionOptions options = std::nullopt;
  if (SILContext) {
    options |= TypeResolutionFlags::SILMode;
    if (SILContext->IsSILType)
      options |= TypeResolutionFlags::SILType;
  }

  std::optional<DiagnosticSuppression> suppression;
  if (!ProduceDiagnostics)
    suppression.emplace(Ctx.Diags);

  return TypeResolution::forInterface(
             DC, GenericSig, options,
             [](auto unboundTy) {
               // FIXME: Don't let unbound generic types escape type resolution.
               // For now, just return the unbound generic type.
               return unboundTy;
             },
             // FIXME: Don't let placeholder types escape type resolution.
             // For now, just return the placeholder type.
             PlaceholderType::get,
             /*packElementOpener*/ nullptr)
      .resolveType(TyR, SILContext);
}

namespace {
  class BindGenericParamsWalker : public ASTWalker {
    DeclContext *dc;
    GenericParamList *params;

  public:
    BindGenericParamsWalker(DeclContext *dc,
                            GenericParamList *params)
        : dc(dc), params(params) {}

    MacroWalking getMacroWalkingBehavior() const override {
      return MacroWalking::Expansion;
    }

    PreWalkAction walkToTypeReprPre(TypeRepr *T) override {
      // Only unqualified identifiers can reference generic parameters.
      auto *unqualIdentTR = dyn_cast<UnqualifiedIdentTypeRepr>(T);
      if (unqualIdentTR && !unqualIdentTR->hasGenericArgList()) {
        auto name = unqualIdentTR->getNameRef().getBaseIdentifier();
        if (auto *paramDecl = params->lookUpGenericParam(name)) {
          unqualIdentTR->setValue(paramDecl, dc);
        }
      }

      return Action::Continue();
    }
  };
}

/// Expose TypeChecker's handling of GenericParamList to SIL parsing.
GenericSignature
language::handleSILGenericParams(GenericParamList *genericParams,
                              DeclContext *DC, bool allowInverses) {
  if (genericParams == nullptr)
    return nullptr;

  SmallVector<GenericParamList *, 2> nestedList;
  for (auto *innerParams = genericParams;
       innerParams != nullptr;
       innerParams = innerParams->getOuterParameters()) {
    nestedList.push_back(innerParams);
  }

  std::reverse(nestedList.begin(), nestedList.end());

  BindGenericParamsWalker walker(DC, genericParams);

  for (unsigned i = 0, e = nestedList.size(); i < e; ++i) {
    auto genericParams = nestedList[i];
    genericParams->setDepth(i);

    genericParams->walk(walker);
  }

  auto request = InferredGenericSignatureRequest{
      /*parentSig=*/nullptr,
      nestedList.back(), WhereClauseOwner(),
      {}, {}, genericParams->getLAngleLoc(),
      /*forExtension=*/nullptr,
      allowInverses};
  return evaluateOrDefault(DC->getASTContext().evaluator, request,
                           GenericSignatureWithError()).getPointer();
}

void language::typeCheckPatternBinding(PatternBindingDecl *PBD,
                                    unsigned bindingIndex) {
  assert(!PBD->isInitializerChecked(bindingIndex) &&
         PBD->getInit(bindingIndex));

  auto &Ctx = PBD->getASTContext();
  DiagnosticSuppression suppression(Ctx.Diags);

  TypeChecker::typeCheckPatternBinding(PBD, bindingIndex,
                                       /*patternType=*/Type());
}

bool language::typeCheckASTNodeAtLoc(TypeCheckASTNodeAtLocContext TypeCheckCtx,
                                  SourceLoc TargetLoc) {
  auto &Ctx = TypeCheckCtx.getDeclContext()->getASTContext();
  DiagnosticSuppression suppression(Ctx.Diags);
  return !evaluateOrDefault(
      Ctx.evaluator, TypeCheckASTNodeAtLocRequest{TypeCheckCtx, TargetLoc},
      true);
}

bool language::typeCheckForCodeCompletion(
    constraints::SyntacticElementTarget &target, bool needsPrecheck,
    toolchain::function_ref<void(const constraints::Solution &)> callback) {
  return TypeChecker::typeCheckForCodeCompletion(target, needsPrecheck,
                                                 callback);
}

Expr *language::resolveDeclRefExpr(UnresolvedDeclRefExpr *UDRE,
                                DeclContext *Context) {
  return TypeChecker::resolveDeclRefExpr(UDRE, Context);
}

void TypeChecker::checkForForbiddenPrefix(ASTContext &C, DeclBaseName Name) {
  if (C.TypeCheckerOpts.DebugForbidTypecheckPrefixes.empty())
    return;

  // Don't touch special names or empty names.
  if (Name.isSpecial() || Name.empty())
    return;

  StringRef Str = Name.getIdentifier().str();
  for (auto forbiddenPrefix : C.TypeCheckerOpts.DebugForbidTypecheckPrefixes) {
    if (Str.starts_with(forbiddenPrefix)) {
      toolchain::report_fatal_error(Twine("forbidden typecheck occurred: ") + Str);
    }
  }
}

DeclTypeCheckingSemantics
TypeChecker::getDeclTypeCheckingSemantics(ValueDecl *decl) {
  // Check for a @_semantics attribute.
  if (auto semantics = decl->getAttrs().getAttribute<SemanticsAttr>()) {
    if (semantics->Value == "typechecker.type(of:)")
      return DeclTypeCheckingSemantics::TypeOf;
    if (semantics->Value == "typechecker.withoutActuallyEscaping(_:do:)")
      return DeclTypeCheckingSemantics::WithoutActuallyEscaping;
    if (semantics->Value == "typechecker._openExistential(_:do:)")
      return DeclTypeCheckingSemantics::OpenExistential;
  }
  return DeclTypeCheckingSemantics::Normal;
}

bool TypeChecker::isDifferentiable(Type type, bool tangentVectorEqualsSelf,
                                   DeclContext *dc,
                                   std::optional<TypeResolutionStage> stage) {
  if (stage)
    type = dc->mapTypeIntoContext(type);
  auto tanSpace = type->getAutoDiffTangentSpace(
      LookUpConformanceInModule());
  if (!tanSpace)
    return false;
  // If no `Self == Self.TangentVector` requirement, return true.
  if (!tangentVectorEqualsSelf)
    return true;
  // Otherwise, return true if `Self == Self.TangentVector`.
  return type->getCanonicalType() == tanSpace->getCanonicalType();
}

bool TypeChecker::diagnoseInvalidFunctionType(
    FunctionType *fnTy, SourceLoc loc, std::optional<FunctionTypeRepr *> repr,
    DeclContext *dc, std::optional<TypeResolutionStage> stage) {
  // Some of the below checks trigger cycles if we don't have a generic
  // signature yet; we'll run the checks again in
  // TypeResolutionStage::Interface.
  if (stage == TypeResolutionStage::Structural)
    return false;

  // If the type has a placeholder, don't try to diagnose anything now since
  // we'll produce a better diagnostic when (if) the expression successfully
  // typechecks.
  if (fnTy->hasPlaceholder())
    return false;

  // If the type is a block or C function pointer, it must be representable in
  // ObjC.
  auto representation = fnTy->getRepresentation();
  auto extInfo = fnTy->getExtInfo();
  auto &ctx = dc->getASTContext();

  bool hadAnyError = false;

  switch (representation) {
  case AnyFunctionType::Representation::Block:
  case AnyFunctionType::Representation::CFunctionPointer:
    if (!fnTy->isRepresentableIn(ForeignLanguage::ObjectiveC, dc)) {
      StringRef strName =
        (representation == AnyFunctionType::Representation::Block)
        ? "block"
        : "c";
      auto extInfo2 =
        extInfo.withRepresentation(AnyFunctionType::Representation::Codira);
      auto simpleFnTy = FunctionType::get(fnTy->getParams(), fnTy->getResult(),
                                          extInfo2);
      ctx.Diags.diagnose(loc, diag::objc_convention_invalid,
                         simpleFnTy, strName);
      hadAnyError = true;
    }
    break;

  case AnyFunctionType::Representation::Thin:
  case AnyFunctionType::Representation::Codira:
    break;
  }

  // `@differentiable` function types must return a differentiable type and have
  // differentiable (or `@noDerivative`) parameters.
  if (extInfo.isDifferentiable()) {
    auto result = fnTy->getResult();
    auto params = fnTy->getParams();
    auto diffKind = extInfo.getDifferentiabilityKind();
    bool isLinear = diffKind == DifferentiabilityKind::Linear;

    // Check the params.

    // Emit `@noDerivative` fixit only if there is at least one valid
    // differentiability parameter. Otherwise, adding `@noDerivative` produces
    // an ill-formed function type.
    auto hasValidDifferentiabilityParam =
    toolchain::find_if(params, [&](AnyFunctionType::Param param) {
      if (param.isNoDerivative())
        return false;
      return TypeChecker::isDifferentiable(param.getPlainType(),
                                           /*tangentVectorEqualsSelf*/ isLinear,
                                           dc, stage);
    }) != params.end();
    bool alreadyDiagnosedOneParam = false;
    for (unsigned i = 0, end = fnTy->getNumParams(); i != end; ++i) {
      auto param = params[i];
      if (param.isNoDerivative())
        continue;
      auto paramType = param.getPlainType();
      if (TypeChecker::isDifferentiable(paramType, isLinear, dc, stage))
        continue;
      auto diagLoc =
          repr ? (*repr)->getArgsTypeRepr()->getElement(i).Type->getLoc() : loc;
      auto paramTypeString = paramType->getString();
      auto diagnostic = ctx.Diags.diagnose(
          diagLoc, diag::differentiable_function_type_invalid_parameter,
          paramTypeString, isLinear, hasValidDifferentiabilityParam);
      alreadyDiagnosedOneParam = true;
      hadAnyError = true;
      if (hasValidDifferentiabilityParam)
        diagnostic.fixItInsert(diagLoc, "@noDerivative ");
    }
    // Reject the case where all parameters have '@noDerivative'.
    if (!alreadyDiagnosedOneParam && !hasValidDifferentiabilityParam) {
      auto diagLoc = repr ? (*repr)->getArgsTypeRepr()->getLoc() : loc;
      auto diag = ctx.Diags.diagnose(
          diagLoc,
          diag::differentiable_function_type_no_differentiability_parameters,
          isLinear);
      hadAnyError = true;

      if (repr) {
          diag.highlight((*repr)->getSourceRange());
      }
    }

    // Check the result
    bool differentiable = isDifferentiable(result,
                                           /*tangentVectorEqualsSelf*/ isLinear,
                                           dc, stage);
    if (!differentiable) {
      auto diagLoc = repr ? (*repr)->getResultTypeRepr()->getLoc() : loc;
      auto resultStr = fnTy->getResult()->getString();
      auto diag = ctx.Diags.diagnose(
          diagLoc, diag::differentiable_function_type_invalid_result, resultStr,
          isLinear);
      hadAnyError = true;

      if (repr) {
          diag.highlight((*repr)->getResultTypeRepr()->getSourceRange());
      }
    }

    // If the result type is void, we need to have at least one differentiable
    // inout argument
    if (result->isVoid() &&
        toolchain::find_if(params,
                      [&](AnyFunctionType::Param param) {
                        if (param.isNoDerivative())
                          return false;
                        return param.isInOut() &&
                          TypeChecker::isDifferentiable(param.getPlainType(),
                                                        /*tangentVectorEqualsSelf*/ isLinear,
                                                        dc, stage);
                      }) == params.end()) {
      auto diagLoc = repr ? (*repr)->getResultTypeRepr()->getLoc() : loc;
      auto resultStr = fnTy->getResult()->getString();
      auto diag = ctx.Diags.diagnose(
        diagLoc, diag::differentiable_function_type_void_result);
      hadAnyError = true;

      if (repr)
        diag.highlight((*repr)->getResultTypeRepr()->getSourceRange());
    }
  }

  return hadAnyError;
}

extern "C" intptr_t language_ASTGen_evaluatePoundIfCondition(
                        BridgedASTContext astContext,
                        void *_Nonnull diagEngine,
                        BridgedStringRef sourceFileBuffer,
                        BridgedStringRef conditionText,
                        bool);

std::pair<bool, bool> EvaluateIfConditionRequest::evaluate(
    Evaluator &evaluator, SourceFile *sourceFile, SourceRange conditionRange,
    bool shouldEvaluate
) const {
#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
  // FIXME: When we migrate to CodiraParser, use the parsed syntax tree.
  ASTContext &ctx = sourceFile->getASTContext();
  auto &sourceMgr = ctx.SourceMgr;

  // Extract the full buffer containing the condition.
  auto bufferID = sourceMgr.findBufferContainingLoc(conditionRange.Start);
  StringRef sourceFileText = sourceMgr.getEntireTextForBuffer(bufferID);

  // Extract the condition text from that buffer.
  auto conditionCharRange = Lexer::getCharSourceRangeFromSourceRange(sourceMgr, conditionRange);
  StringRef conditionText = sourceMgr.extractText(conditionCharRange, bufferID);

  // Evaluate the condition.
  intptr_t evalResult = language_ASTGen_evaluatePoundIfCondition(
      ctx, &ctx.Diags, sourceFileText, conditionText, shouldEvaluate
  );

  bool isActive = (evalResult & 0x01) != 0;
  bool allowSyntaxErrors = (evalResult & 0x02) != 0;
  return std::pair(isActive, allowSyntaxErrors);
#else
  toolchain_unreachable("Must not be used in C++-only build");
#endif
}
