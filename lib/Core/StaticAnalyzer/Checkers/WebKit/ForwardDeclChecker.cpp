//=======- ForwardDeclChecker.cpp --------------------------------*- C++ -*-==//
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

#include "ASTUtils.h"
#include "DiagOutputUtils.h"
#include "PtrTypesSemantics.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/RecursiveASTVisitor.h"
#include "language/Core/Analysis/DomainSpecific/CocoaConventions.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/Support/SaveAndRestore.h"

using namespace language::Core;
using namespace ento;

namespace {

class ForwardDeclChecker : public Checker<check::ASTDecl<TranslationUnitDecl>> {
  BugType Bug;
  mutable BugReporter *BR = nullptr;
  mutable RetainTypeChecker RTC;
  mutable toolchain::DenseSet<const Type *> SystemTypes;

public:
  ForwardDeclChecker()
      : Bug(this, "Forward declared member or local variable or parameter",
            "WebKit coding guidelines") {}

  void checkASTDecl(const TranslationUnitDecl *TUD, AnalysisManager &MGR,
                    BugReporter &BRArg) const {
    BR = &BRArg;

    // The calls to checkAST* from AnalysisConsumer don't
    // visit template instantiations or lambda classes. We
    // want to visit those, so we make our own RecursiveASTVisitor.
    struct LocalVisitor : public RecursiveASTVisitor<LocalVisitor> {
      using Base = RecursiveASTVisitor<LocalVisitor>;

      const ForwardDeclChecker *Checker;
      Decl *DeclWithIssue{nullptr};

      explicit LocalVisitor(const ForwardDeclChecker *Checker)
          : Checker(Checker) {
        assert(Checker);
      }

      bool shouldVisitTemplateInstantiations() const { return true; }
      bool shouldVisitImplicitCode() const { return false; }

      bool VisitTypedefDecl(TypedefDecl *TD) {
        Checker->visitTypedef(TD);
        return true;
      }

      bool VisitRecordDecl(const RecordDecl *RD) {
        Checker->visitRecordDecl(RD, DeclWithIssue);
        return true;
      }

      bool TraverseDecl(Decl *D) {
        toolchain::SaveAndRestore SavedDecl(DeclWithIssue);
        if (D && (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)))
          DeclWithIssue = D;
        return Base::TraverseDecl(D);
      }

      bool VisitVarDecl(VarDecl *V) {
        if (V->isLocalVarDecl())
          Checker->visitVarDecl(V, DeclWithIssue);
        return true;
      }

      bool VisitCallExpr(const CallExpr *CE) {
        Checker->visitCallExpr(CE, DeclWithIssue);
        return true;
      }

      bool VisitCXXConstructExpr(const CXXConstructExpr *CE) {
        Checker->visitConstructExpr(CE, DeclWithIssue);
        return true;
      }

      bool VisitObjCMessageExpr(const ObjCMessageExpr *ObjCMsgExpr) {
        Checker->visitObjCMessageExpr(ObjCMsgExpr, DeclWithIssue);
        return true;
      }
    };

    LocalVisitor visitor(this);
    RTC.visitTranslationUnitDecl(TUD);
    visitor.TraverseDecl(const_cast<TranslationUnitDecl *>(TUD));
  }

  void visitTypedef(const TypedefDecl *TD) const {
    RTC.visitTypedef(TD);
    auto QT = TD->getUnderlyingType().getCanonicalType();
    assert(BR && "expected nonnull BugReporter");
    if (BR->getSourceManager().isInSystemHeader(TD->getBeginLoc())) {
      if (auto *Type = QT.getTypePtrOrNull())
        SystemTypes.insert(Type);
    }
  }

  bool isUnknownType(QualType QT) const {
    auto *CanonicalType = QT.getCanonicalType().getTypePtrOrNull();
    if (!CanonicalType)
      return false;
    auto PointeeQT = CanonicalType->getPointeeType();
    auto *PointeeType = PointeeQT.getTypePtrOrNull();
    if (!PointeeType)
      return false;
    auto *R = PointeeType->getAsCXXRecordDecl();
    if (!R) // Forward declaration of a Objective-C interface is safe.
      return false;
    auto Name = R->getName();
    if (R->hasDefinition())
      return false;
    // Find a definition amongst template declarations.
    if (auto *Specialization = dyn_cast<ClassTemplateSpecializationDecl>(R)) {
      if (auto *S = Specialization->getSpecializedTemplate()) {
        for (S = S->getMostRecentDecl(); S; S = S->getPreviousDecl()) {
          if (S->isThisDeclarationADefinition())
            return false;
        }
      }
    }
    return !RTC.isUnretained(QT) && !SystemTypes.contains(CanonicalType) &&
           !SystemTypes.contains(PointeeType) && !Name.starts_with("Opaque") &&
           Name != "_NSZone";
  }

  void visitRecordDecl(const RecordDecl *RD, const Decl *DeclWithIssue) const {
    if (!RD->isThisDeclarationADefinition())
      return;

    if (RD->isImplicit() || RD->isLambda())
      return;

    const auto RDLocation = RD->getLocation();
    if (!RDLocation.isValid())
      return;

    const auto Kind = RD->getTagKind();
    if (Kind != TagTypeKind::Struct && Kind != TagTypeKind::Class)
      return;

    assert(BR && "expected nonnull BugReporter");
    if (BR->getSourceManager().isInSystemHeader(RDLocation))
      return;

    // Ref-counted smartpointers actually have raw-pointer to uncounted type as
    // a member but we trust them to handle it correctly.
    auto R = toolchain::dyn_cast_or_null<CXXRecordDecl>(RD);
    if (!R || isRefCounted(R) || isCheckedPtr(R) || isRetainPtr(R))
      return;

    for (auto *Member : RD->fields()) {
      auto QT = Member->getType();
      if (isUnknownType(QT)) {
        SmallString<100> Buf;
        toolchain::raw_svector_ostream Os(Buf);

        const std::string TypeName = QT.getAsString();
        Os << "Member variable ";
        printQuotedName(Os, Member);
        Os << " uses a forward declared type '" << TypeName << "'";

        const SourceLocation SrcLocToReport = Member->getBeginLoc();
        PathDiagnosticLocation BSLoc(SrcLocToReport, BR->getSourceManager());
        auto Report = std::make_unique<BasicBugReport>(Bug, Os.str(), BSLoc);
        Report->addRange(Member->getSourceRange());
        Report->setDeclWithIssue(DeclWithIssue);
        BR->emitReport(std::move(Report));
      }
    }
  }

  void visitVarDecl(const VarDecl *V, const Decl *DeclWithIssue) const {
    assert(BR && "expected nonnull BugReporter");
    if (BR->getSourceManager().isInSystemHeader(V->getBeginLoc()))
      return;

    auto QT = V->getType();
    if (!isUnknownType(QT))
      return;

    SmallString<100> Buf;
    toolchain::raw_svector_ostream Os(Buf);
    Os << "Local variable ";
    printQuotedQualifiedName(Os, V);

    reportBug(V->getBeginLoc(), V->getSourceRange(), DeclWithIssue, Os.str(),
              QT);
  }

  void visitCallExpr(const CallExpr *CE, const Decl *DeclWithIssue) const {
    assert(BR && "expected nonnull BugReporter");
    if (BR->getSourceManager().isInSystemHeader(CE->getExprLoc()))
      return;

    if (auto *F = CE->getDirectCallee()) {
      // Skip the first argument for overloaded member operators (e. g. lambda
      // or std::function call operator).
      unsigned ArgIdx =
          isa<CXXOperatorCallExpr>(CE) && isa_and_nonnull<CXXMethodDecl>(F);

      for (auto P = F->param_begin();
           P < F->param_end() && ArgIdx < CE->getNumArgs(); ++P, ++ArgIdx)
        visitCallArg(CE->getArg(ArgIdx), *P, DeclWithIssue);
    }
  }

  void visitConstructExpr(const CXXConstructExpr *CE,
                          const Decl *DeclWithIssue) const {
    assert(BR && "expected nonnull BugReporter");
    if (BR->getSourceManager().isInSystemHeader(CE->getExprLoc()))
      return;

    if (auto *F = CE->getConstructor()) {
      // Skip the first argument for overloaded member operators (e. g. lambda
      // or std::function call operator).
      unsigned ArgIdx =
          isa<CXXOperatorCallExpr>(CE) && isa_and_nonnull<CXXMethodDecl>(F);

      for (auto P = F->param_begin();
           P < F->param_end() && ArgIdx < CE->getNumArgs(); ++P, ++ArgIdx)
        visitCallArg(CE->getArg(ArgIdx), *P, DeclWithIssue);
    }
  }

  void visitObjCMessageExpr(const ObjCMessageExpr *E,
                            const Decl *DeclWithIssue) const {
    assert(BR && "expected nonnull BugReporter");
    if (BR->getSourceManager().isInSystemHeader(E->getExprLoc()))
      return;

    if (auto *Receiver = E->getInstanceReceiver()) {
      Receiver = Receiver->IgnoreParenCasts();
      if (isUnknownType(E->getReceiverType()))
        reportUnknownRecieverType(Receiver, DeclWithIssue);
    }

    auto *MethodDecl = E->getMethodDecl();
    if (!MethodDecl)
      return;

    auto ArgCount = E->getNumArgs();
    for (unsigned i = 0; i < ArgCount && i < MethodDecl->param_size(); ++i)
      visitCallArg(E->getArg(i), MethodDecl->getParamDecl(i), DeclWithIssue);
  }

  void visitCallArg(const Expr *Arg, const ParmVarDecl *Param,
                    const Decl *DeclWithIssue) const {
    auto *ArgExpr = Arg->IgnoreParenCasts();
    if (auto *InnerCE = dyn_cast<CallExpr>(Arg)) {
      auto *InnerCallee = InnerCE->getDirectCallee();
      if (InnerCallee && InnerCallee->isInStdNamespace() &&
          safeGetName(InnerCallee) == "move" && InnerCE->getNumArgs() == 1) {
        ArgExpr = InnerCE->getArg(0);
        if (ArgExpr)
          ArgExpr = ArgExpr->IgnoreParenCasts();
      }
    }
    if (isa<CXXNullPtrLiteralExpr>(ArgExpr) || isa<IntegerLiteral>(ArgExpr) ||
        isa<CXXDefaultArgExpr>(ArgExpr))
      return;
    if (auto *DRE = dyn_cast<DeclRefExpr>(ArgExpr)) {
      if (auto *ValDecl = DRE->getDecl()) {
        if (isa<ParmVarDecl>(ValDecl))
          return;
      }
    }

    QualType ArgType = Param->getType();
    if (!isUnknownType(ArgType))
      return;

    reportUnknownArgType(Arg, Param, DeclWithIssue);
  }

  void reportUnknownArgType(const Expr *CA, const ParmVarDecl *Param,
                            const Decl *DeclWithIssue) const {
    assert(CA);

    SmallString<100> Buf;
    toolchain::raw_svector_ostream Os(Buf);

    const std::string paramName = safeGetName(Param);
    Os << "Call argument";
    if (!paramName.empty()) {
      Os << " for parameter ";
      printQuotedQualifiedName(Os, Param);
    }

    reportBug(CA->getExprLoc(), CA->getSourceRange(), DeclWithIssue, Os.str(),
              Param->getType());
  }

  void reportUnknownRecieverType(const Expr *Receiver,
                                 const Decl *DeclWithIssue) const {
    assert(Receiver);
    reportBug(Receiver->getExprLoc(), Receiver->getSourceRange(), DeclWithIssue,
              "Receiver", Receiver->getType());
  }

  void reportBug(const SourceLocation &SrcLoc, const SourceRange &SrcRange,
                 const Decl *DeclWithIssue, const StringRef &Description,
                 QualType Type) const {
    SmallString<100> Buf;
    toolchain::raw_svector_ostream Os(Buf);

    const std::string TypeName = Type.getAsString();
    Os << Description << " uses a forward declared type '" << TypeName << "'";

    assert(BR && "expected nonnull BugReporter");
    PathDiagnosticLocation BSLoc(SrcLoc, BR->getSourceManager());
    auto Report = std::make_unique<BasicBugReport>(Bug, Os.str(), BSLoc);
    Report->addRange(SrcRange);
    Report->setDeclWithIssue(DeclWithIssue);
    BR->emitReport(std::move(Report));
  }
};

} // namespace

void ento::registerForwardDeclChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<ForwardDeclChecker>();
}

bool ento::shouldRegisterForwardDeclChecker(const CheckerManager &) {
  return true;
}
