//===--- InterfaceStubFunctionsConsumer.cpp -------------------------------===//
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

#include "language/Core/AST/Mangle.h"
#include "language/Core/AST/RecursiveASTVisitor.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendActions.h"
#include "language/Core/Sema/TemplateInstCallback.h"
#include "toolchain/BinaryFormat/ELF.h"

using namespace language::Core;

namespace {
class InterfaceStubFunctionsConsumer : public ASTConsumer {
  CompilerInstance &Instance;
  StringRef InFile;
  StringRef Format;
  std::set<std::string> ParsedTemplates;

  enum RootDeclOrigin { TopLevel = 0, FromTU = 1, IsLate = 2 };
  struct MangledSymbol {
    std::string ParentName;
    uint8_t Type;
    uint8_t Binding;
    std::vector<std::string> Names;
    MangledSymbol() = delete;

    MangledSymbol(const std::string &ParentName, uint8_t Type, uint8_t Binding,
                  std::vector<std::string> Names)
        : ParentName(ParentName), Type(Type), Binding(Binding),
          Names(std::move(Names)) {}
  };
  using MangledSymbols = std::map<const NamedDecl *, MangledSymbol>;

  bool WriteNamedDecl(const NamedDecl *ND, MangledSymbols &Symbols, int RDO) {
    // Here we filter out anything that's not set to DefaultVisibility.
    // DefaultVisibility is set on a decl when -fvisibility is not specified on
    // the command line (or specified as default) and the decl does not have
    // __attribute__((visibility("hidden"))) set or when the command line
    // argument is set to hidden but the decl explicitly has
    // __attribute__((visibility ("default"))) set. We do this so that the user
    // can have fine grain control of what they want to expose in the stub.
    auto isVisible = [](const NamedDecl *ND) -> bool {
      return ND->getVisibility() == DefaultVisibility;
    };

    auto ignoreDecl = [this, isVisible](const NamedDecl *ND) -> bool {
      if (!isVisible(ND))
        return true;

      if (const VarDecl *VD = dyn_cast<VarDecl>(ND)) {
        if (const auto *Parent = VD->getParentFunctionOrMethod())
          if (isa<BlockDecl>(Parent) || isa<CXXMethodDecl>(Parent))
            return true;

        if ((VD->getStorageClass() == StorageClass::SC_Extern) ||
            (VD->getStorageClass() == StorageClass::SC_Static &&
             VD->getParentFunctionOrMethod() == nullptr))
          return true;
      }

      if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(ND)) {
        if (FD->isInlined() && !isa<CXXMethodDecl>(FD) &&
            !Instance.getLangOpts().GNUInline)
          return true;
        if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD)) {
          if (const auto *RC = dyn_cast<CXXRecordDecl>(MD->getParent()))
            if (isa<ClassTemplateDecl>(RC->getParent()) || !isVisible(RC))
              return true;
          if (MD->isDependentContext() || !MD->hasBody())
            return true;
        }
        if (FD->getStorageClass() == StorageClass::SC_Static)
          return true;
      }
      return false;
    };

    auto getParentFunctionDecl = [](const NamedDecl *ND) -> const NamedDecl * {
      if (const VarDecl *VD = dyn_cast<VarDecl>(ND))
        if (const auto *FD =
                dyn_cast_or_null<FunctionDecl>(VD->getParentFunctionOrMethod()))
          return FD;
      return nullptr;
    };

    auto getMangledNames = [](const NamedDecl *ND) -> std::vector<std::string> {
      if (!ND)
        return {""};
      ASTNameGenerator NameGen(ND->getASTContext());
      std::vector<std::string> MangledNames = NameGen.getAllManglings(ND);
      if (isa<CXXConstructorDecl>(ND) || isa<CXXDestructorDecl>(ND))
        return MangledNames;
#ifdef EXPENSIVE_CHECKS
      assert(MangledNames.size() <= 1 && "Expected only one name mangling.");
#endif
      return {NameGen.getName(ND)};
    };

    if (!(RDO & FromTU))
      return true;
    if (Symbols.find(ND) != Symbols.end())
      return true;
    // - Currently have not figured out how to produce the names for FieldDecls.
    // - Do not want to produce symbols for function paremeters.
    if (isa<FieldDecl>(ND) || isa<ParmVarDecl>(ND))
      return true;

    const NamedDecl *ParentDecl = getParentFunctionDecl(ND);
    if ((ParentDecl && ignoreDecl(ParentDecl)) || ignoreDecl(ND))
      return true;

    if (RDO & IsLate) {
      Instance.getDiagnostics().Report(diag::err_asm_invalid_type_in_input)
          << "Generating Interface Stubs is not supported with "
             "delayed template parsing.";
    } else {
      if (const auto *FD = dyn_cast<FunctionDecl>(ND))
        if (FD->isDependentContext())
          return true;

      const bool IsWeak = (ND->hasAttr<WeakAttr>() ||
                           ND->hasAttr<WeakRefAttr>() || ND->isWeakImported());

      Symbols.insert(std::make_pair(
          ND,
          MangledSymbol(getMangledNames(ParentDecl).front(),
                        // Type:
                        isa<VarDecl>(ND) ? toolchain::ELF::STT_OBJECT
                                         : toolchain::ELF::STT_FUNC,
                        // Binding:
                        IsWeak ? toolchain::ELF::STB_WEAK : toolchain::ELF::STB_GLOBAL,
                        getMangledNames(ND))));
    }
    return true;
  }

  void
  HandleDecls(const toolchain::iterator_range<DeclContext::decl_iterator> &Decls,
              MangledSymbols &Symbols, int RDO) {
    for (const auto *D : Decls)
      HandleNamedDecl(dyn_cast<NamedDecl>(D), Symbols, RDO);
  }

  void HandleTemplateSpecializations(const FunctionTemplateDecl &FTD,
                                     MangledSymbols &Symbols, int RDO) {
    for (const auto *D : FTD.specializations())
      HandleNamedDecl(dyn_cast<NamedDecl>(D), Symbols, RDO);
  }

  void HandleTemplateSpecializations(const ClassTemplateDecl &CTD,
                                     MangledSymbols &Symbols, int RDO) {
    for (const auto *D : CTD.specializations())
      HandleNamedDecl(dyn_cast<NamedDecl>(D), Symbols, RDO);
  }

  bool HandleNamedDecl(const NamedDecl *ND, MangledSymbols &Symbols, int RDO) {
    if (!ND)
      return false;

    switch (ND->getKind()) {
    default:
      break;
    case Decl::Kind::Namespace:
      HandleDecls(cast<NamespaceDecl>(ND)->decls(), Symbols, RDO);
      return true;
    case Decl::Kind::CXXRecord:
      HandleDecls(cast<CXXRecordDecl>(ND)->decls(), Symbols, RDO);
      return true;
    case Decl::Kind::ClassTemplateSpecialization:
      HandleDecls(cast<ClassTemplateSpecializationDecl>(ND)->decls(), Symbols,
                  RDO);
      return true;
    case Decl::Kind::ClassTemplate:
      HandleTemplateSpecializations(*cast<ClassTemplateDecl>(ND), Symbols, RDO);
      return true;
    case Decl::Kind::FunctionTemplate:
      HandleTemplateSpecializations(*cast<FunctionTemplateDecl>(ND), Symbols,
                                    RDO);
      return true;
    case Decl::Kind::Record:
    case Decl::Kind::Typedef:
    case Decl::Kind::Enum:
    case Decl::Kind::EnumConstant:
    case Decl::Kind::TemplateTypeParm:
    case Decl::Kind::NonTypeTemplateParm:
    case Decl::Kind::CXXConversion:
    case Decl::Kind::UnresolvedUsingValue:
    case Decl::Kind::Using:
    case Decl::Kind::UsingShadow:
    case Decl::Kind::TypeAliasTemplate:
    case Decl::Kind::TypeAlias:
    case Decl::Kind::VarTemplate:
    case Decl::Kind::VarTemplateSpecialization:
    case Decl::Kind::UsingDirective:
    case Decl::Kind::TemplateTemplateParm:
    case Decl::Kind::ClassTemplatePartialSpecialization:
    case Decl::Kind::IndirectField:
    case Decl::Kind::ConstructorUsingShadow:
    case Decl::Kind::CXXDeductionGuide:
    case Decl::Kind::NamespaceAlias:
    case Decl::Kind::UnresolvedUsingTypename:
      return true;
    case Decl::Kind::Var: {
      // Bail on any VarDecl that either has no named symbol.
      if (!ND->getIdentifier())
        return true;
      const auto *VD = cast<VarDecl>(ND);
      // Bail on any VarDecl that is a dependent or templated type.
      if (VD->isTemplated() || VD->getType()->isDependentType())
        return true;
      if (WriteNamedDecl(ND, Symbols, RDO))
        return true;
      break;
    }
    case Decl::Kind::ParmVar:
    case Decl::Kind::CXXMethod:
    case Decl::Kind::CXXConstructor:
    case Decl::Kind::CXXDestructor:
    case Decl::Kind::Function:
    case Decl::Kind::Field:
      if (WriteNamedDecl(ND, Symbols, RDO))
        return true;
    }

    // While interface stubs are in the development stage, it's probably best to
    // catch anything that's not a VarDecl or Template/FunctionDecl.
    Instance.getDiagnostics().Report(diag::err_asm_invalid_type_in_input)
        << "Expected a function or function template decl.";
    return false;
  }

public:
  InterfaceStubFunctionsConsumer(CompilerInstance &Instance, StringRef InFile,
                                 StringRef Format)
      : Instance(Instance), InFile(InFile), Format(Format) {}

  void HandleTranslationUnit(ASTContext &context) override {
    struct Visitor : public RecursiveASTVisitor<Visitor> {
      bool VisitNamedDecl(NamedDecl *ND) {
        if (const auto *FD = dyn_cast<FunctionDecl>(ND))
          if (FD->isLateTemplateParsed()) {
            LateParsedDecls.insert(FD);
            return true;
          }

        if (const auto *VD = dyn_cast<ValueDecl>(ND)) {
          ValueDecls.insert(VD);
          return true;
        }

        NamedDecls.insert(ND);
        return true;
      }

      std::set<const NamedDecl *> LateParsedDecls;
      std::set<NamedDecl *> NamedDecls;
      std::set<const ValueDecl *> ValueDecls;
    } v;

    v.TraverseDecl(context.getTranslationUnitDecl());

    MangledSymbols Symbols;
    auto OS = Instance.createDefaultOutputFile(/*Binary=*/false, InFile, "ifs");
    if (!OS)
      return;

    if (Instance.getLangOpts().DelayedTemplateParsing) {
      language::Core::Sema &S = Instance.getSema();
      for (const auto *FD : v.LateParsedDecls) {
        language::Core::LateParsedTemplate &LPT =
            *S.LateParsedTemplateMap.find(cast<FunctionDecl>(FD))->second;
        S.LateTemplateParser(S.OpaqueParser, LPT);
        HandleNamedDecl(FD, Symbols, (FromTU | IsLate));
      }
    }

    for (const NamedDecl *ND : v.ValueDecls)
      HandleNamedDecl(ND, Symbols, FromTU);
    for (const NamedDecl *ND : v.NamedDecls)
      HandleNamedDecl(ND, Symbols, FromTU);

    auto writeIfsV1 = [this](const toolchain::Triple &T,
                             const MangledSymbols &Symbols,
                             const ASTContext &context, StringRef Format,
                             raw_ostream &OS) -> void {
      OS << "--- !" << Format << "\n";
      OS << "IfsVersion: 3.0\n";
      OS << "Target: " << T.str() << "\n";
      OS << "Symbols:\n";
      for (const auto &E : Symbols) {
        const MangledSymbol &Symbol = E.second;
        for (const auto &Name : Symbol.Names) {
          OS << "  - { Name: \""
             << (Symbol.ParentName.empty() || Instance.getLangOpts().CPlusPlus
                     ? ""
                     : (Symbol.ParentName + "."))
             << Name << "\", Type: ";
          switch (Symbol.Type) {
          default:
            toolchain_unreachable(
                "clang -emit-interface-stubs: Unexpected symbol type.");
          case toolchain::ELF::STT_NOTYPE:
            OS << "NoType";
            break;
          case toolchain::ELF::STT_OBJECT: {
            auto VD = cast<ValueDecl>(E.first)->getType();
            OS << "Object, Size: "
               << context.getTypeSizeInChars(VD).getQuantity();
            break;
          }
          case toolchain::ELF::STT_FUNC:
            OS << "Func";
            break;
          }
          if (Symbol.Binding == toolchain::ELF::STB_WEAK)
            OS << ", Weak: true";
          OS << " }\n";
        }
      }
      OS << "...\n";
      OS.flush();
    };

    assert(Format == "ifs-v1" && "Unexpected IFS Format.");
    writeIfsV1(Instance.getTarget().getTriple(), Symbols, context, Format, *OS);
  }
};
} // namespace

std::unique_ptr<ASTConsumer>
GenerateInterfaceStubsAction::CreateASTConsumer(CompilerInstance &CI,
                                                StringRef InFile) {
  return std::make_unique<InterfaceStubFunctionsConsumer>(CI, InFile, "ifs-v1");
}
