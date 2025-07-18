//===--- SymbolGraph.cpp - Symbol Graph Data Structure -------------------===//
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

#include "clang/AST/DeclObjC.h"
#include "clang/Basic/Module.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Comment.h"
#include "language/AST/Decl.h"
#include "language/AST/Module.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/USRGeneration.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Version.h"
#include "language/Sema/IDETypeChecking.h"
#include "language/SymbolGraphGen/DocumentationCategory.h"

#include "DeclarationFragmentPrinter.h"
#include "FormatVersion.h"
#include "Symbol.h"
#include "SymbolGraph.h"
#include "SymbolGraphASTWalker.h"

using namespace language;
using namespace symbolgraphgen;

SymbolGraph::SymbolGraph(SymbolGraphASTWalker &Walker, ModuleDecl &M,
                         std::optional<ModuleDecl *> ExtendedModule,
                         markup::MarkupContext &Ctx,
                         std::optional<toolchain::VersionTuple> ModuleVersion,
                         bool IsForSingleNode)
    : Walker(Walker), M(M), ExtendedModule(ExtendedModule), Ctx(Ctx),
      ModuleVersion(ModuleVersion), IsForSingleNode(IsForSingleNode) {
  if (auto *DM = M.getDeclaringModuleIfCrossImportOverlay()) {
    DeclaringModule = DM;
    SmallVector<Identifier, 1> Bystanders;
    if (M.getRequiredBystandersIfCrossImportOverlay(DM, Bystanders)) {
      BystanderModules = Bystanders;
    }
  }
}

// MARK: - Utilities

PrintOptions SymbolGraph::getDeclarationFragmentsPrintOptions() const {
  PrintOptions Opts;
  Opts.FunctionDefinitions = false;
  Opts.ArgAndParamPrinting =
    PrintOptions::ArgAndParamPrintingMode::MatchSource;
  Opts.PrintGetSetOnRWProperties = true;
  Opts.PrintPropertyAccessors = true;
  Opts.PrintSubscriptAccessors = true;
  Opts.SkipUnderscoredKeywords = true;
  Opts.SkipAttributes = false;
  Opts.PrintOverrideKeyword = true;
  Opts.PrintImplicitAttrs = false;
  Opts.PrintFunctionRepresentationAttrs =
    PrintOptions::FunctionRepresentationMode::None;
  Opts.PrintUserInaccessibleAttrs = false;
  Opts.SkipPrivateSystemDecls = !Walker.Options.PrintPrivateSystemSymbols;
  Opts.SkipUnderscoredSystemProtocols =
      !Walker.Options.PrintPrivateSystemSymbols;
  Opts.PrintGenericRequirements = true;
  Opts.PrintInherited = false;
  Opts.ExplodeEnumCaseDecls = true;
  Opts.PrintFactoryInitializerComment = false;
  Opts.PrintMacroDefinitions = false;

  Opts.ExclusiveAttrList.clear();

  toolchain::StringMap<AnyAttrKind> ExcludeAttrs;

#define TYPE_ATTR(X, C)                                                        \
  ExcludeAttrs.insert(std::make_pair("TypeAttrKind::" #C, TypeAttrKind::C));
#include "language/AST/TypeAttr.def"

  // Allow the following type attributes:
  ExcludeAttrs.erase("TypeAttrKind::Autoclosure");
  ExcludeAttrs.erase("TypeAttrKind::Convention");
  ExcludeAttrs.erase("TypeAttrKind::NoEscape");
  ExcludeAttrs.erase("TypeAttrKind::Escaping");
  ExcludeAttrs.erase("TypeAttrKind::Inout");
  ExcludeAttrs.erase("TypeAttrKind::Sendable");

  // Don't allow the following decl attributes:
  // These can be large and are already included elsewhere in
  // symbol graphs.
  ExcludeAttrs.insert(
      std::make_pair("DeclAttrKind::Available", DeclAttrKind::Available));
  ExcludeAttrs.insert(
      std::make_pair("DeclAttrKind::Inline", DeclAttrKind::Inline));
  ExcludeAttrs.insert(
      std::make_pair("DeclAttrKind::Inlinable", DeclAttrKind::Inlinable));
  ExcludeAttrs.insert(
      std::make_pair("DeclAttrKind::Prefix", DeclAttrKind::Prefix));
  ExcludeAttrs.insert(
      std::make_pair("DeclAttrKind::Postfix", DeclAttrKind::Postfix));
  ExcludeAttrs.insert(
      std::make_pair("DeclAttrKind::Infix", DeclAttrKind::Infix));

  // In "emit modules separately" jobs, access modifiers show up as attributes,
  // but we don't want them to be printed in declarations
  ExcludeAttrs.insert(std::make_pair("DeclAttrKind::AccessControl",
                                     DeclAttrKind::AccessControl));
  ExcludeAttrs.insert(
      std::make_pair("DeclAttrKind::SetterAccess", DeclAttrKind::SetterAccess));

  for (const auto &Entry : ExcludeAttrs) {
    Opts.ExcludeAttrList.push_back(Entry.getValue());
  }

  return Opts;
}

PrintOptions
SymbolGraph::getSubHeadingDeclarationFragmentsPrintOptions() const {
  auto Options = getDeclarationFragmentsPrintOptions();
  Options.ArgAndParamPrinting =
    PrintOptions::ArgAndParamPrintingMode::ArgumentOnly;

  //--------------------------------------------------------------------------//
  // Although we want these in the full declaration presentation,
  // particularly for protocol requirements,
  // we don't want to show these in subheadings.
  Options.PrintGetSetOnRWProperties = false;
  Options.PrintPropertyAccessors = false;
  Options.PrintSubscriptAccessors = false;
  //--------------------------------------------------------------------------//

  Options.SkipAttributes = true;
  Options.VarInitializers = false;
  Options.PrintDefaultArgumentValue = false;
  Options.PrintEmptyArgumentNames = false;
  Options.PrintOverrideKeyword = false;
  Options.PrintGenericRequirements = false;

#define DECL_ATTR(SPELLING, CLASS, ...)                                        \
  Options.ExcludeAttrList.push_back(DeclAttrKind::CLASS);
#define TYPE_ATTR(X, C) Options.ExcludeAttrList.push_back(TypeAttrKind::C);
#include "language/AST/DeclAttr.def"

  // Don't include these attributes in subheadings.
  Options.ExcludeAttrList.push_back(DeclAttrKind::Final);
  Options.ExcludeAttrList.push_back(DeclAttrKind::Mutating);
  Options.ExcludeAttrList.push_back(DeclAttrKind::NonMutating);
  Options.ExcludeAttrList.push_back(TypeAttrKind::Escaping);

  return Options;
}

bool
SymbolGraph::isRequirementOrDefaultImplementation(const ValueDecl *VD) const {
  const auto *DC = VD->getDeclContext();

  if (isa<ProtocolDecl>(DC) && VD->isProtocolRequirement()) {
    return true;
  }

  // At this point, VD is either a default implementation of a requirement
  // or a freestanding implementation from a protocol extension without
  // a corresponding requirement.

  auto *Proto = DC->getSelfProtocolDecl();
  if (!Proto) {
    return false;
  }

  /// Try to find a member of the owning protocol with the same name
  /// that is a requirement.
  auto FoundRequirementMemberNamed = [](DeclName Name,
                                        ProtocolDecl *Proto) -> bool {
    for (const auto *Member : Proto->lookupDirect(Name)) {
      if (isa<ProtocolDecl>(Member->getDeclContext()) &&
          Member->isProtocolRequirement()) {
        return true;
      }
    }
    return false;
  };

  if (FoundRequirementMemberNamed(VD->getName(), Proto)) {
    return true;
  }

  for (auto *Inherited : Proto->getAllInheritedProtocols()) {
    if (FoundRequirementMemberNamed(VD->getName(), Inherited)) {
      return true;
    }
  }

  // Couldn't find any requirement members of a protocol by this name.
  // It's not a requirement or default implementation of a requirement.
  return false;
}

// MARK: - Symbols (Nodes)

void SymbolGraph::recordNode(Symbol S) {
  Nodes.insert(S);

  // Record all of the possible relationships (edges) originating
  // with this declaration.
  recordMemberRelationship(S);
  recordConformanceSynthesizedMemberRelationships(S);
  recordConformanceRelationships(S);
  recordInheritanceRelationships(S);
  recordDefaultImplementationRelationships(S);
  recordOverrideRelationship(S);
  recordRequirementRelationships(S);
  recordOptionalRequirementRelationships(S);
}

// MARK: - Relationships (Edges)

void SymbolGraph::recordEdge(Symbol Source,
                             Symbol Target,
                             RelationshipKind Kind,
                             const ExtensionDecl *ConformanceExtension) {
  if (isImplicitlyPrivate(Target.getSymbolDecl())) {
    // Don't record relationships to privately named things because
    // we'll never be able to look up the target anyway.
    return;
  }
  Edges.insert({this, Kind, Source, Target, ConformanceExtension});
}

void SymbolGraph::recordMemberRelationship(Symbol S) {
  const auto *DC = S.getLocalSymbolDecl()->getDeclContext();
  const ValueDecl *ParentDecl = DC->getSelfNominalTypeDecl();
  if (!ParentDecl) {
    // If we couldn't look up the type the member is declared on (e.g.
    // because the member is declared in an extension whose extended type
    // doesn't exist), don't record a memberOf relationship.
    return;
  }
  if (const auto *PublicDecl = Walker.PublicPrivateTypeAliases.lookup(ParentDecl)) {
    // If our member target is a private type that has a public type alias,
    // point the membership to that type alias instead.
    ParentDecl = PublicDecl;
  }
  switch (DC->getContextKind()) {
    case DeclContextKind::GenericTypeDecl:
    case DeclContextKind::ExtensionDecl:
    case language::DeclContextKind::EnumElementDecl:
      /*
       If this symbol is itself a protocol requirement, or
       is a default implementation of a protocol requirement,
       don't record a memberOf relationship.

       This is to allow distinguishing between requirements,
       default implementations of requirements, and just other
       things added to protocols in extensions not related to their
       requirements.
       */
      if (isRequirementOrDefaultImplementation(S.getSymbolDecl())) {
        return;
      }

      // If this is an extension to an external type, we use the extension
      // symbol itself as the target.
      if (auto const *Extension =
              dyn_cast_or_null<ExtensionDecl>(DC->getAsDecl())) {

        if (this->Walker.shouldBeRecordedAsExtension(Extension)) {
          return recordEdge(S, Symbol(this, Extension, nullptr),
                            RelationshipKind::MemberOf());
        }
      }

      return recordEdge(S, Symbol(this, ParentDecl, nullptr),
                        RelationshipKind::MemberOf());
    case language::DeclContextKind::AbstractClosureExpr:
    case language::DeclContextKind::SerializedAbstractClosure:
    case language::DeclContextKind::Initializer:
    case language::DeclContextKind::TopLevelCodeDecl:
    case language::DeclContextKind::SerializedTopLevelCodeDecl:
    case language::DeclContextKind::SubscriptDecl:
    case language::DeclContextKind::AbstractFunctionDecl:
    case language::DeclContextKind::Package:
    case language::DeclContextKind::Module:
    case language::DeclContextKind::FileUnit:
    case language::DeclContextKind::MacroDecl:
      break;
  }
}

bool SymbolGraph::synthesizedMemberIsBestCandidate(const ValueDecl *VD,
    const NominalTypeDecl *Owner) const {
  DeclName Name;
  if (const auto *FD = dyn_cast<FuncDecl>(VD)) {
    Name = FD->getEffectiveFullName();
  } else {
    Name = VD->getName();
  }

  if (!Name) {
    return true;
  }

  auto *DC = const_cast<DeclContext*>(Owner->getDeclContext());

  ResolvedMemberResult Result =
    resolveValueMember(*DC, Owner->getSelfTypeInContext(), Name);

  const auto ViableCandidates =
    Result.getMemberDecls(InterestedMemberKind::All);

  if (ViableCandidates.size() < 2) {
    return true;
  }

  return !(Result.hasBestOverload() && Result.getBestOverload() != VD);
}

void SymbolGraph::recordConformanceSynthesizedMemberRelationships(Symbol S) {
  // Even if we don't want to emit synthesized members or protocol
  // implementations, we still want to emit synthesized members from hidden
  // underscored protocols. Save this check so we can skip emitting members
  // later if needed.
  bool dropSynthesizedMembers = !Walker.Options.EmitSynthesizedMembers ||
                                Walker.Options.SkipProtocolImplementations;

  const auto *D = S.getLocalSymbolDecl();

  // If this symbol is a public type alias to a private symbol, collect
  // synthesized members for the underlying type.
  if (const auto *TD = dyn_cast<TypeAliasDecl>(D)) {
    const auto *NTD = TD->getUnderlyingType()->getAnyNominal();
    if (NTD && Walker.PublicPrivateTypeAliases.lookup(NTD) == D)
        D = NTD;
  }

  const NominalTypeDecl *OwningNominal = nullptr;
  if (const auto *ThisNominal = dyn_cast<NominalTypeDecl>(D)) {
    OwningNominal = ThisNominal;
  } else if (const auto *Extension = dyn_cast<ExtensionDecl>(D)) {
    if (const auto *ExtendedNominal = Extension->getExtendedNominal()) {
      if (ExtendedNominal->getModuleContext()->getNameStr() != M.getNameStr()) {
        OwningNominal = ExtendedNominal;
      } else {
        return;
      }
    } else {
      return;
    }
  } else {
    return;
  }

  SynthesizedExtensionAnalyzer ExtensionAnalyzer(
      const_cast<NominalTypeDecl *>(OwningNominal),
      PrintOptions::printModuleInterface(
          OwningNominal->getASTContext().TypeCheckerOpts.PrintFullConvention));
  auto MergeGroupKind = SynthesizedExtensionAnalyzer::MergeGroupKind::All;
  ExtensionAnalyzer.forEachExtensionMergeGroup(
      MergeGroupKind, [&](ArrayRef<ExtensionInfo> ExtensionInfos) {
        const auto StdlibModule =
            OwningNominal->getASTContext().getStdlibModule(
                /*loadIfAbsent=*/true);

        for (const auto &Info : ExtensionInfos) {
          if (!Info.IsSynthesized) {
            continue;
          }

          // We are only interested in synthesized members that come from an
          // extension that we defined in our module.
          if (Info.EnablingExt) {
            const auto *ExtM = Info.EnablingExt->getModuleContext();
            if (!Walker.isOurModule(ExtM))
              continue;
          }

          // If D is not the OwningNominal, it is an ExtensionDecl. In that case
          // we only want to get members that were enabled by this exact
          // extension.
          if (D != OwningNominal && Info.EnablingExt != D) {
            continue;
          }

          // Extensions to protocols should generate synthesized members only if
          // that protocol would otherwise be hidden.
          if (auto *Nominal = Info.Ext->getExtendedNominal()) {
            if (dropSynthesizedMembers &&
                !isImplicitlyPrivate(Nominal, /*IgnoreContext =*/
                                     [&](const Decl *P) {
                                       return Nominal->getModuleContext() ==
                                              StdlibModule;
                                     }))
              continue;
          } else if (dropSynthesizedMembers) {
            continue;
          }

          for (const auto ExtensionMember : Info.Ext->getMembers()) {
            if (const auto SynthMember = dyn_cast<ValueDecl>(ExtensionMember)) {
              if (SynthMember->isObjC()) {
                continue;
              }

              // There can be synthesized members on effectively private
              // protocols or things that conform to them. We don't want to
              // include those.
              if (isImplicitlyPrivate(
                      SynthMember,
                      /*IgnoreContext =*/
                      [&](const Decl *P) {
                        return SynthMember->getModuleContext() == StdlibModule;
                      })) {
                continue;
              }

              if (!synthesizedMemberIsBestCandidate(SynthMember,
                                                    OwningNominal)) {
                continue;
              }

              const ValueDecl *BaseDecl = OwningNominal;
              if (const auto *PublicDecl = Walker.PublicPrivateTypeAliases.lookup(BaseDecl))
                BaseDecl = PublicDecl;

              Symbol Source(this, SynthMember, BaseDecl);

              if (auto *InheritedDecl = Source.getInheritedDecl()) {
                if (auto *ParentDecl =
                        InheritedDecl->getDeclContext()->getAsDecl()) {
                  if (dropSynthesizedMembers &&
                      !isImplicitlyPrivate(
                          ParentDecl,
                          /*IgnoreContext =*/
                          [&](const Decl *P) {
                            return ParentDecl->getModuleContext() ==
                                   StdlibModule;
                          })) {
                    continue;
                  }
                }
              }

              auto ExtendedSG = Walker.getModuleSymbolGraph(BaseDecl);

              ExtendedSG->Nodes.insert(Source);

              ExtendedSG->recordEdge(Source, S, RelationshipKind::MemberOf());
            }
          }
        }
      });
}

void
SymbolGraph::recordInheritanceRelationships(Symbol S) {
  const auto *D = S.getLocalSymbolDecl();

  // If this is a public type alias for a private symbol, gather inheritance
  // for the underlying type instead.
  if (const auto *TD = dyn_cast<TypeAliasDecl>(D)) {
    const auto *NTD = TD->getUnderlyingType()->getAnyNominal();
    if (NTD && Walker.PublicPrivateTypeAliases.lookup(NTD) == D)
      D = NTD;
  }

  ClassDecl *Super = nullptr;
  if (auto *CD = dyn_cast<ClassDecl>(D))
    Super = CD->getSuperclassDecl();
  else if (auto *PD = dyn_cast<ProtocolDecl>(D))
    Super = PD->getSuperclassDecl();

  if (Super) {
    recordEdge(S, Symbol(this, Super, nullptr),
               RelationshipKind::InheritsFrom());
  }
}

void SymbolGraph::recordDefaultImplementationRelationships(Symbol S) {
  const auto *VD = S.getSymbolDecl();

  /// Claim a protocol `P`'s members as default implementation targets
  /// for `VD`.
  auto HandleProtocol = [=](const ProtocolDecl *P) {
    for (const auto *Member : P->getMembers()) {
      if (const auto *MemberVD = dyn_cast<ValueDecl>(Member)) {
        if (MemberVD->getName().compare(VD->getName()) == 0) {
          recordEdge(Symbol(this, VD, nullptr),
                     Symbol(this, MemberVD, nullptr),
                     RelationshipKind::DefaultImplementationOf());

          // If P is from a different module, and it's being added to a type
          // from the current module, add a `memberOf` relation to the extended
          // protocol or the respective extension block.
          if (!Walker.isOurModule(MemberVD->getModuleContext()) && VD->getDeclContext()) {
            if (const auto *Extension =
                    dyn_cast_or_null<ExtensionDecl>(VD->getDeclContext())) {
              if (this->Walker.shouldBeRecordedAsExtension(Extension)) {
                recordEdge(Symbol(this, VD, nullptr),
                           Symbol(this, Extension, nullptr),
                           RelationshipKind::MemberOf());
                continue;
              }
            }
            if (auto *ExtendedProtocol =
                    VD->getDeclContext()->getSelfNominalTypeDecl()) {
              recordEdge(Symbol(this, VD, nullptr),
                         Symbol(this, ExtendedProtocol, nullptr),
                         RelationshipKind::MemberOf());
            }
          }
        }
      }
    }
  };

  if (const auto *Extension = dyn_cast<ExtensionDecl>(VD->getDeclContext())) {
    if (const auto *ExtendedProtocol = Extension->getExtendedProtocolDecl()) {
      HandleProtocol(ExtendedProtocol);
      for (const auto *Inherited : ExtendedProtocol->getAllInheritedProtocols()) {
        HandleProtocol(Inherited);
      }
    }
  }
}

void
SymbolGraph::recordRequirementRelationships(Symbol S) {
  const auto VD = S.getSymbolDecl();
  if (const auto *Protocol = dyn_cast<ProtocolDecl>(VD->getDeclContext())) {
    if (VD->isProtocolRequirement() &&
        !VD->getAttrs().hasAttribute<OptionalAttr>()) {
      recordEdge(Symbol(this, VD, nullptr),
                 Symbol(this, Protocol, nullptr),
                 RelationshipKind::RequirementOf());
    }
  }
}

void SymbolGraph::recordOptionalRequirementRelationships(Symbol S) {
  const auto VD = S.getSymbolDecl();
  if (const auto *Protocol = dyn_cast<ProtocolDecl>(VD->getDeclContext())) {
    if (VD->isProtocolRequirement() &&
        VD->getAttrs().hasAttribute<OptionalAttr>()) {
      recordEdge(Symbol(this, VD, nullptr),
                 Symbol(this, Protocol, nullptr),
                 RelationshipKind::OptionalRequirementOf());
    }
  }
}

void SymbolGraph::recordConformanceRelationships(Symbol S) {
  const auto *D = S.getLocalSymbolDecl();

  // If this is a public type alias for a private symbol, gather conformances
  // for the underlying type instead.
  if (const auto *TD = dyn_cast<TypeAliasDecl>(D)) {
    const auto *NTD = TD->getUnderlyingType()->getAnyNominal();
    if (NTD && Walker.PublicPrivateTypeAliases.lookup(NTD) == D)
      D = NTD;
  }

  if (const auto *NTD = dyn_cast<NominalTypeDecl>(D)) {
    if (auto *PD = dyn_cast<ProtocolDecl>(NTD)) {
      for (auto *inherited : PD->getAllInheritedProtocols()) {
        // FIXME(noncopyable_generics): Figure out what we want here.
        if (inherited->getInvertibleProtocolKind())
          continue;

        recordEdge(S, Symbol(this, inherited, nullptr),
                   RelationshipKind::ConformsTo(), nullptr);
      }
    } else {
      for (const auto *Conformance : NTD->getAllConformances()) {
        // FIXME(noncopyable_generics): Figure out what we want here.
        if (Conformance->getProtocol()->getInvertibleProtocolKind())
          continue;

        // Check to make sure that this conformance wasn't declared via an
        // unconditionally-unavailable extension. If so, don't add that to the graph.
        if (const auto *ED = dyn_cast_or_null<ExtensionDecl>(Conformance->getDeclContext())) {
          if (isUnconditionallyUnavailableOnAllPlatforms(ED)) {
            continue;
          }
        }
        recordEdge(
            S, Symbol(this, Conformance->getProtocol(), nullptr),
            RelationshipKind::ConformsTo(),
            dyn_cast_or_null<ExtensionDecl>(Conformance->getDeclContext()));
      }
    }
  }
}

void SymbolGraph::recordOverrideRelationship(Symbol S) {
  const auto VD = S.getSymbolDecl();
  if (const auto *Override = VD->getOverriddenDecl()) {
    recordEdge(Symbol(this, VD, nullptr),
               Symbol(this, Override, nullptr),
               RelationshipKind::Overrides());
  }
}

// MARK: - Serialization

void SymbolGraph::serialize(toolchain::json::OStream &OS) {
  OS.object([&](){
    OS.attributeObject("metadata", [&](){
      {
        AttributeRAII FV("formatVersion", OS);
        toolchain::VersionTuple FormatVersion(LANGUAGE_SYMBOLGRAPH_FORMAT_MAJOR,
                                         LANGUAGE_SYMBOLGRAPH_FORMAT_MINOR,
                                         LANGUAGE_SYMBOLGRAPH_FORMAT_PATCH);
        symbolgraphgen::serialize(FormatVersion, OS);
      } // end formatVersion:

      auto VersionString = version::getCodiraFullVersion();
      StringRef VersionStringRef(VersionString.c_str(), VersionString.size());
      OS.attribute("generator", VersionStringRef);
    }); // end metadata:

    OS.attributeObject("module", [&](){
      if (DeclaringModule) {
        // A cross-import overlay can be considered part of its declaring module
        OS.attribute("name", getFullModuleName(*DeclaringModule));
        std::vector<StringRef> B;
        for (auto BModule : BystanderModules) {
          B.push_back(BModule.str());
        }
        OS.attribute("bystanders", B);
      } else {
        OS.attribute("name", getFullModuleName(&M));
      }
      AttributeRAII Platform("platform", OS);

      symbolgraphgen::serialize(M, OS, Walker.Options.Target);
    });

    if (ModuleVersion) {
      AttributeRAII MV("moduleVersion", OS);
      symbolgraphgen::serialize(*ModuleVersion, OS);
    }

    OS.attributeArray("symbols", [&](){
      for (const auto &S: Nodes) {
        S.serialize(OS);
      }
    });

    OS.attributeArray("relationships", [&](){
      for (const auto &Relationship : Edges) {
        Relationship.serialize(OS);
      }
    });
  });
}

void
SymbolGraph::serializeDeclarationFragments(StringRef Key,
                                           const Symbol &S,
                                           toolchain::json::OStream &OS) {
  DeclarationFragmentPrinter Printer(this, OS, Key);
  auto Options = getDeclarationFragmentsPrintOptions();
  if (S.getBaseType()) {
    Options.setBaseType(S.getBaseType());
    Options.PrintAsMember = true;
  }
  S.getLocalSymbolDecl()->print(Printer, Options);
}

void
SymbolGraph::serializeNavigatorDeclarationFragments(StringRef Key,
                                                    const Symbol &S,
                                                    toolchain::json::OStream &OS) {
  if (const auto *TD = dyn_cast<GenericTypeDecl>(S.getSymbolDecl())) {
    DeclarationFragmentPrinter Printer(this, OS, Key);
    Printer.printAbridgedType(TD, /*PrintKeyword=*/false);
  }
}

void
SymbolGraph::serializeSubheadingDeclarationFragments(StringRef Key,
                                                     const Symbol &S,
                                                     toolchain::json::OStream &OS) {
  DeclarationFragmentPrinter Printer(this, OS, Key);

  if (const auto *TD = dyn_cast<GenericTypeDecl>(S.getLocalSymbolDecl())) {
    Printer.printAbridgedType(TD, /*PrintKeyword=*/true);
  } else {
    auto Options = getSubHeadingDeclarationFragmentsPrintOptions();
    if (S.getBaseType()) {
      Options.setBaseType(S.getBaseType());
      Options.PrintAsMember = true;
    }
    S.getLocalSymbolDecl()->print(Printer, Options);
  }
}

void
SymbolGraph::serializeDeclarationFragments(StringRef Key, Type T,
                                           Type BaseType,
                                           toolchain::json::OStream &OS) {
  DeclarationFragmentPrinter Printer(this, OS, Key);
  auto Options = getDeclarationFragmentsPrintOptions();
  if (BaseType) {
    Options.setBaseType(BaseType);
    Options.PrintAsMember = true;
  }
  T->print(Printer, Options);
}

namespace {

const ValueDecl *getProtocolRequirement(const ValueDecl *VD) {
  auto reqs = VD->getSatisfiedProtocolRequirements();

  if (!reqs.empty())
    return reqs.front();
  else
    return nullptr;
}

}

bool SymbolGraph::isImplicitlyPrivate(
    const Decl *D, toolchain::function_ref<bool(const Decl *)> IgnoreContext) const {
  // Don't record unconditionally private declarations
  if (D->isPrivateSystemDecl(/*treatNonBuiltinProtocolsAsPublic=*/false)) {
    return true;
  }

  // If the decl has a `@_documentation(visibility: <access>)` attribute, override any other heuristic
  auto DocVisibility = documentationVisibilityForDecl(D);
  if (DocVisibility) {
    return Walker.Options.MinimumAccessLevel > (*DocVisibility);
  }

  // Don't record effectively internal declarations if specified
  if (D->hasUnderscoredNaming()) {
    // Some implicit decls from Clang with underscored names sneak in, so throw those out
    if (const auto *clangD = D->getClangDecl()) {
      if (clangD->isImplicit())
        return true;
    }

    AccessLevel symLevel = AccessLevel::Public;
    if (const ValueDecl *VD = dyn_cast<ValueDecl>(D)) {
      symLevel = VD->getFormalAccess();
    }

    // Underscored symbols should be treated as `internal`, unless they're already
    // marked that way - in that case, treat them as `private`
    AccessLevel effectiveLevel;
    if (symLevel > AccessLevel::Internal) {
      effectiveLevel = AccessLevel::Internal;
    } else {
      effectiveLevel = AccessLevel::Private;
    }

    if (Walker.Options.MinimumAccessLevel > effectiveLevel)
      return true;
  }

  // Don't include declarations with the @_spi attribute unless the
  // access control filter is internal or below.
  if (D->isSPI() && !Walker.Options.IncludeSPISymbols) {
    return Walker.Options.MinimumAccessLevel > AccessLevel::Internal;
  }

  if (const auto *Extension = dyn_cast<ExtensionDecl>(D)) {
    if (const auto *Nominal = Extension->getExtendedNominal()) {
      return isImplicitlyPrivate(Nominal, IgnoreContext) ||
             Symbol::getEffectiveAccessLevel(Extension) <
                 Walker.Options.MinimumAccessLevel;
    }
  }

  if (const auto *VD = dyn_cast<ValueDecl>(D)) {
    // Symbols must meet the minimum access level to be included in the graph.
    if (VD->getFormalAccess() < Walker.Options.MinimumAccessLevel) {
      return true;
    }

    // Special cases below.

    // If we've been asked to skip protocol implementations, filter them out here.
    if (Walker.Options.SkipProtocolImplementations) {
      if (const auto *ProtocolRequirement = getProtocolRequirement(VD)) {
        if (const auto *Protocol = ProtocolRequirement->getDeclContext()->getSelfProtocolDecl()) {
          if (!Protocol->hasUnderscoredNaming()) {
            // Allow them to stay if they have their own doc comment
            const auto *DocCommentProvidingDecl = VD->getDocCommentProvidingDecl();
            if (DocCommentProvidingDecl != VD)
              return true;
          }
        }
      }
    }

    // Symbols from exported-imported modules should only be included if they
    // were originally public.
    // We force compiler-equality here to ensure that the presence of an underlying
    // Clang module does not prevent internal Codira symbols from being emitted when
    // MinimumAccessLevel is set to `internal` or below.
    if (Walker.isFromExportedImportedModule(D, /*countUnderlyingClangModule*/false) &&
        VD->getFormalAccess() < AccessLevel::Public) {
      return true;
    }

    auto BaseName = VD->getBaseName().userFacingName();

    // ${MODULE}Version{Number,String} in ${Module}.h
    SmallString<32> VersionNameIdentPrefix { M.getName().str() };
    VersionNameIdentPrefix.append("Version");
    if (BaseName.starts_with(VersionNameIdentPrefix.str())) {
      return true;
    }

    // Automatically mapped SIMD types
    auto IsGlobalSIMDType = toolchain::StringSwitch<bool>(BaseName)
  #define MAP_SIMD_TYPE(C_TYPE, _, __) \
  .Case("language_" #C_TYPE "2", true) \
  .Case("language_" #C_TYPE "3", true) \
  .Case("language_" #C_TYPE "4", true)
  #include "language/ClangImporter/SIMDMappedTypes.def"
    .Case("LANGUAGE_TYPEDEFS", true)
    .Case("char16_t", true)
    .Case("char32_t", true)
    .Default(false);

    if (IsGlobalSIMDType) {
      return true;
    }
  }

  // Check up the parent chain. Anything inside a privately named
  // thing is also private. We could be looking at the `B` of `_A.B`.
  if (const auto *DC = D->getDeclContext()) {
    if (const auto *Parent = DC->getAsDecl()) {
      if (IgnoreContext && IgnoreContext(Parent))
        return false;

      // Exception: Children of underscored protocols should be considered
      // public, even though the protocols themselves aren't. This way,
      // synthesized copies of those symbols are correctly added to the public
      // API of public types that conform to underscored protocols.
      if (isa<ProtocolDecl>(Parent) && Parent->hasUnderscoredNaming()) {
        return false;
      }
      if (const auto *ParentExtension = dyn_cast<ExtensionDecl>(Parent)) {
        if (const auto *ParentNominal = ParentExtension->getExtendedNominal()) {
          if (isa<ProtocolDecl>(ParentNominal) &&
              ParentNominal->hasUnderscoredNaming()) {
            return false;
          }
        }
      }
      return isImplicitlyPrivate(Parent, IgnoreContext);
    }
  }
  return false;
}

/// FIXME: [availability] This should use Decl::getUnavailableAttr() or similar.
bool SymbolGraph::isUnconditionallyUnavailableOnAllPlatforms(const Decl *D) const {
  for (auto Attr : D->getSemanticAvailableAttrs()) {
    if (!Attr.isPlatformSpecific() && Attr.isUnconditionallyUnavailable())
      return true;
  }

  return false;
}

/// Returns `true` if the symbol should be included as a node in the graph.
bool SymbolGraph::canIncludeDeclAsNode(const Decl *D,
                                       const Decl *PublicAncestorDecl) const {
  if (PublicAncestorDecl && D == PublicAncestorDecl)
    return true;

  // If this decl isn't in this module or module that this module imported with `@_exported`, don't record it,
  // as it will appear elsewhere in its module's symbol graph.

  // If a Clang decl was declared in a submodule, the Codira decl's context will still point to the
  // top-level module. Instead, we need to probe the owning module on the Clang side, which will
  // correctly point to the submodule.
  auto RealModuleName = (std::string)D->getModuleContext()->getName();
  if (auto *ClangDecl = D->getClangDecl())
    if (auto *ClangModule = ClangDecl->getOwningModule())
      RealModuleName = ClangModule->Name;
  if (RealModuleName != (std::string)M.getName() && !Walker.isConsideredExportedImported(D)) {
    return false;
  }

  if (const auto *VD = dyn_cast<ValueDecl>(D)) {
    if (VD->getOverriddenDecl() && D->isImplicit()) {
      return false;
    }
  } else {
    return false;
  }
  return !isImplicitlyPrivate(
             cast<ValueDecl>(D), /*IgnoreContext=*/
             [&](const Decl *P) { return P == PublicAncestorDecl; }) &&
         !isUnconditionallyUnavailableOnAllPlatforms(cast<ValueDecl>(D));
}
