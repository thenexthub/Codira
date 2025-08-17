//===--- ASTConsumers.cpp - ASTConsumer implementations -------------------===//
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
// AST Consumer Implementations.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Frontend/ASTConsumers.h"
#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/PrettyPrinter.h"
#include "language/Core/AST/RecordLayout.h"
#include "language/Core/AST/RecursiveASTVisitor.h"
#include "language/Core/Basic/Diagnostic.h"
#include "toolchain/Support/Timer.h"
#include "toolchain/Support/raw_ostream.h"
using namespace language::Core;

//===----------------------------------------------------------------------===//
/// ASTPrinter - Pretty-printer and dumper of ASTs

namespace {
  class ASTPrinter : public ASTConsumer,
                     public RecursiveASTVisitor<ASTPrinter> {
    typedef RecursiveASTVisitor<ASTPrinter> base;

  public:
    enum Kind { DumpFull, Dump, Print, None };
    ASTPrinter(std::unique_ptr<raw_ostream> Out, Kind K,
               ASTDumpOutputFormat Format, StringRef FilterString,
               bool DumpLookups = false, bool DumpDeclTypes = false)
        : Out(Out ? *Out : toolchain::outs()), OwnedOut(std::move(Out)),
          OutputKind(K), OutputFormat(Format), FilterString(FilterString),
          DumpLookups(DumpLookups), DumpDeclTypes(DumpDeclTypes) {}

    ASTPrinter(raw_ostream &Out, Kind K, ASTDumpOutputFormat Format,
               StringRef FilterString, bool DumpLookups = false,
               bool DumpDeclTypes = false)
        : Out(Out), OwnedOut(nullptr), OutputKind(K), OutputFormat(Format),
          FilterString(FilterString), DumpLookups(DumpLookups),
          DumpDeclTypes(DumpDeclTypes) {}

    void HandleTranslationUnit(ASTContext &Context) override {
      TranslationUnitDecl *D = Context.getTranslationUnitDecl();

      if (FilterString.empty())
        return print(D);

      TraverseDecl(D);
    }

    bool shouldWalkTypesOfTypeLocs() const { return false; }

    bool TraverseDecl(Decl *D) {
      if (D && filterMatches(D)) {
        bool ShowColors = Out.has_colors();
        if (ShowColors)
          Out.changeColor(raw_ostream::BLUE);

        if (OutputFormat == ADOF_Default)
          Out << (OutputKind != Print ? "Dumping " : "Printing ") << getName(D)
              << ":\n";

        if (ShowColors)
          Out.resetColor();
        print(D);
        Out << "\n";
        // Don't traverse child nodes to avoid output duplication.
        return true;
      }
      return base::TraverseDecl(D);
    }

  private:
    std::string getName(Decl *D) {
      if (isa<NamedDecl>(D))
        return cast<NamedDecl>(D)->getQualifiedNameAsString();
      return "";
    }
    bool filterMatches(Decl *D) {
      return getName(D).find(FilterString) != std::string::npos;
    }
    void print(Decl *D) {
      if (DumpLookups) {
        if (DeclContext *DC = dyn_cast<DeclContext>(D)) {
          if (DC == DC->getPrimaryContext())
            DC->dumpLookups(Out, OutputKind != None, OutputKind == DumpFull);
          else
            Out << "Lookup map is in primary DeclContext "
                << DC->getPrimaryContext() << "\n";
        } else
          Out << "Not a DeclContext\n";
      } else if (OutputKind == Print) {
        PrintingPolicy Policy(D->getASTContext().getLangOpts());
        Policy.IncludeTagDefinition = true;
        D->print(Out, Policy, /*Indentation=*/0, /*PrintInstantiation=*/true);
      } else if (OutputKind != None) {
        D->dump(Out, OutputKind == DumpFull, OutputFormat);
      }

      if (DumpDeclTypes) {
        Decl *InnerD = D;
        if (auto *TD = dyn_cast<TemplateDecl>(D))
          if (Decl *TempD = TD->getTemplatedDecl())
            InnerD = TempD;

        // FIXME: Support OutputFormat in type dumping.
        // FIXME: Support combining -ast-dump-decl-types with -ast-dump-lookups.
        if (auto *VD = dyn_cast<ValueDecl>(InnerD))
          VD->getType().dump(Out, VD->getASTContext());
        if (auto *TD = dyn_cast<TypeDecl>(InnerD)) {
          const ASTContext &Ctx = TD->getASTContext();
          Ctx.getTypeDeclType(TD)->dump(Out, Ctx);
        }
      }
    }

    raw_ostream &Out;
    std::unique_ptr<raw_ostream> OwnedOut;

    /// How to output individual declarations.
    Kind OutputKind;

    /// What format should the output take?
    ASTDumpOutputFormat OutputFormat;

    /// Which declarations or DeclContexts to display.
    std::string FilterString;

    /// Whether the primary output is lookup results or declarations. Individual
    /// results will be output with a format determined by OutputKind. This is
    /// incompatible with OutputKind == Print.
    bool DumpLookups;

    /// Whether to dump the type for each declaration dumped.
    bool DumpDeclTypes;
  };

  class ASTDeclNodeLister : public ASTConsumer,
                     public RecursiveASTVisitor<ASTDeclNodeLister> {
  public:
    ASTDeclNodeLister(raw_ostream *Out = nullptr)
        : Out(Out ? *Out : toolchain::outs()) {}

    void HandleTranslationUnit(ASTContext &Context) override {
      TraverseDecl(Context.getTranslationUnitDecl());
    }

    bool shouldWalkTypesOfTypeLocs() const { return false; }

    bool VisitNamedDecl(NamedDecl *D) {
      D->printQualifiedName(Out);
      Out << '\n';
      return true;
    }

  private:
    raw_ostream &Out;
  };
} // end anonymous namespace

std::unique_ptr<ASTConsumer>
language::Core::CreateASTPrinter(std::unique_ptr<raw_ostream> Out,
                        StringRef FilterString) {
  return std::make_unique<ASTPrinter>(std::move(Out), ASTPrinter::Print,
                                       ADOF_Default, FilterString);
}

std::unique_ptr<ASTConsumer>
language::Core::CreateASTDumper(std::unique_ptr<raw_ostream> Out, StringRef FilterString,
                       bool DumpDecls, bool Deserialize, bool DumpLookups,
                       bool DumpDeclTypes, ASTDumpOutputFormat Format) {
  assert((DumpDecls || Deserialize || DumpLookups) && "nothing to dump");
  return std::make_unique<ASTPrinter>(
      std::move(Out),
      Deserialize ? ASTPrinter::DumpFull
                  : DumpDecls ? ASTPrinter::Dump : ASTPrinter::None,
      Format, FilterString, DumpLookups, DumpDeclTypes);
}

std::unique_ptr<ASTConsumer>
language::Core::CreateASTDumper(raw_ostream &Out, StringRef FilterString, bool DumpDecls,
                       bool Deserialize, bool DumpLookups, bool DumpDeclTypes,
                       ASTDumpOutputFormat Format) {
  assert((DumpDecls || Deserialize || DumpLookups) && "nothing to dump");
  return std::make_unique<ASTPrinter>(Out,
                                      Deserialize ? ASTPrinter::DumpFull
                                      : DumpDecls ? ASTPrinter::Dump
                                                  : ASTPrinter::None,
                                      Format, FilterString, DumpLookups,
                                      DumpDeclTypes);
}

std::unique_ptr<ASTConsumer> language::Core::CreateASTDeclNodeLister() {
  return std::make_unique<ASTDeclNodeLister>(nullptr);
}

//===----------------------------------------------------------------------===//
/// ASTViewer - AST Visualization

namespace {
class ASTViewer : public ASTConsumer {
  ASTContext *Context = nullptr;

public:
  void Initialize(ASTContext &Context) override { this->Context = &Context; }

  bool HandleTopLevelDecl(DeclGroupRef D) override {
    for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
      HandleTopLevelSingleDecl(*I);
    return true;
  }

  void HandleTopLevelSingleDecl(Decl *D);
};
}

void ASTViewer::HandleTopLevelSingleDecl(Decl *D) {
  if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)) {
    D->print(toolchain::errs());

    if (Stmt *Body = D->getBody()) {
      toolchain::errs() << '\n';
      Body->viewAST();
      toolchain::errs() << '\n';
    }
  }
}

std::unique_ptr<ASTConsumer> language::Core::CreateASTViewer() {
  return std::make_unique<ASTViewer>();
}
