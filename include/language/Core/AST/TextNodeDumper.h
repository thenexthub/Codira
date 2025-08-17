//===--- TextNodeDumper.h - Printing of AST nodes -------------------------===//
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
// This file implements AST dumping of components of individual AST nodes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_TEXTNODEDUMPER_H
#define LANGUAGE_CORE_AST_TEXTNODEDUMPER_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/ASTDumperUtils.h"
#include "language/Core/AST/AttrVisitor.h"
#include "language/Core/AST/CommentCommandTraits.h"
#include "language/Core/AST/CommentVisitor.h"
#include "language/Core/AST/DeclVisitor.h"
#include "language/Core/AST/ExprConcepts.h"
#include "language/Core/AST/ExprCXX.h"
#include "language/Core/AST/StmtVisitor.h"
#include "language/Core/AST/TemplateArgumentVisitor.h"
#include "language/Core/AST/Type.h"
#include "language/Core/AST/TypeLocVisitor.h"
#include "language/Core/AST/TypeVisitor.h"

namespace language::Core {

class APValue;

class TextTreeStructure {
  raw_ostream &OS;
  const bool ShowColors;

  /// Pending[i] is an action to dump an entity at level i.
  toolchain::SmallVector<std::function<void(bool IsLastChild)>, 32> Pending;

  /// Indicates whether we're at the top level.
  bool TopLevel = true;

  /// Indicates if we're handling the first child after entering a new depth.
  bool FirstChild = true;

  /// Prefix for currently-being-dumped entity.
  std::string Prefix;

public:
  /// Add a child of the current node.  Calls DoAddChild without arguments
  template <typename Fn> void AddChild(Fn DoAddChild) {
    return AddChild("", DoAddChild);
  }

  /// Add a child of the current node with an optional label.
  /// Calls DoAddChild without arguments.
  template <typename Fn> void AddChild(StringRef Label, Fn DoAddChild) {
    // If we're at the top level, there's nothing interesting to do; just
    // run the dumper.
    if (TopLevel) {
      TopLevel = false;
      DoAddChild();
      while (!Pending.empty()) {
        Pending.back()(true);
        Pending.pop_back();
      }
      Prefix.clear();
      OS << "\n";
      TopLevel = true;
      return;
    }

    auto DumpWithIndent = [this, DoAddChild,
                           Label(Label.str())](bool IsLastChild) {
      // Print out the appropriate tree structure and work out the prefix for
      // children of this node. For instance:
      //
      //   A        Prefix = ""
      //   |-B      Prefix = "| "
      //   | `-C    Prefix = "|   "
      //   `-D      Prefix = "  "
      //     |-E    Prefix = "  | "
      //     `-F    Prefix = "    "
      //   G        Prefix = ""
      //
      // Note that the first level gets no prefix.
      {
        OS << '\n';
        ColorScope Color(OS, ShowColors, IndentColor);
        OS << Prefix << (IsLastChild ? '`' : '|') << '-';
        if (!Label.empty())
          OS << Label << ": ";

        this->Prefix.push_back(IsLastChild ? ' ' : '|');
        this->Prefix.push_back(' ');
      }

      FirstChild = true;
      unsigned Depth = Pending.size();

      DoAddChild();

      // If any children are left, they're the last at their nesting level.
      // Dump those ones out now.
      while (Depth < Pending.size()) {
        Pending.back()(true);
        this->Pending.pop_back();
      }

      // Restore the old prefix.
      this->Prefix.resize(Prefix.size() - 2);
    };

    if (FirstChild) {
      Pending.push_back(std::move(DumpWithIndent));
    } else {
      Pending.back()(false);
      Pending.back() = std::move(DumpWithIndent);
    }
    FirstChild = false;
  }

  TextTreeStructure(raw_ostream &OS, bool ShowColors)
      : OS(OS), ShowColors(ShowColors) {}
};

class TextNodeDumper
    : public TextTreeStructure,
      public comments::ConstCommentVisitor<TextNodeDumper, void,
                                           const comments::FullComment *>,
      public ConstAttrVisitor<TextNodeDumper>,
      public ConstTemplateArgumentVisitor<TextNodeDumper>,
      public ConstStmtVisitor<TextNodeDumper>,
      public TypeVisitor<TextNodeDumper>,
      public TypeLocVisitor<TextNodeDumper>,
      public ConstDeclVisitor<TextNodeDumper> {
  raw_ostream &OS;
  const bool ShowColors;

  /// Keep track of the last location we print out so that we can
  /// print out deltas from then on out.
  const char *LastLocFilename = "";
  unsigned LastLocLine = ~0U;

  /// \p Context, \p SM, and \p Traits can be null. This is because we want
  /// to be able to call \p dump() in a debugger without having to pass the
  /// \p ASTContext to \p dump. Not all parts of the AST dump output will be
  /// available without the \p ASTContext.
  const ASTContext *Context = nullptr;
  const SourceManager *SM = nullptr;

  /// The policy to use for printing; can be defaulted.
  PrintingPolicy PrintPolicy = LangOptions();

  const comments::CommandTraits *Traits = nullptr;

  const char *getCommandName(unsigned CommandID);
  void printFPOptions(FPOptionsOverride FPO);

  void dumpAPValueChildren(const APValue &Value, QualType Ty,
                           const APValue &(*IdxToChildFun)(const APValue &,
                                                           unsigned),
                           unsigned NumChildren, StringRef LabelSingular,
                           StringRef LabelPlurial);

public:
  TextNodeDumper(raw_ostream &OS, const ASTContext &Context, bool ShowColors);
  TextNodeDumper(raw_ostream &OS, bool ShowColors);

  void Visit(const comments::Comment *C, const comments::FullComment *FC);

  void Visit(const Attr *A);

  void Visit(const TemplateArgument &TA, SourceRange R,
             const Decl *From = nullptr, StringRef Label = {});

  void Visit(const Stmt *Node);

  void Visit(const Type *T);

  void Visit(QualType T);

  void Visit(TypeLoc);

  void Visit(const Decl *D);

  void Visit(const CXXCtorInitializer *Init);

  void Visit(const OMPClause *C);

  void Visit(const OpenACCClause *C);

  void Visit(const BlockDecl::Capture &C);

  void Visit(const GenericSelectionExpr::ConstAssociation &A);

  void Visit(const ConceptReference *);

  void Visit(const concepts::Requirement *R);

  void Visit(const APValue &Value, QualType Ty);

  void dumpPointer(const void *Ptr);
  void dumpLocation(SourceLocation Loc);
  void dumpSourceRange(SourceRange R);
  void dumpBareType(QualType T, bool Desugar = true);
  void dumpType(QualType T);
  void dumpBareDeclRef(const Decl *D);
  void dumpName(const NamedDecl *ND);
  void dumpAccessSpecifier(AccessSpecifier AS);
  void dumpCleanupObject(const ExprWithCleanups::CleanupObject &C);
  void dumpTemplateSpecializationKind(TemplateSpecializationKind TSK);
  void dumpNestedNameSpecifier(NestedNameSpecifier NNS);
  void dumpConceptReference(const ConceptReference *R);
  void dumpTemplateArgument(const TemplateArgument &TA);
  void dumpBareTemplateName(TemplateName TN);
  void dumpTemplateName(TemplateName TN, StringRef Label = {});

  void dumpDeclRef(const Decl *D, StringRef Label = {});

  void visitTextComment(const comments::TextComment *C,
                        const comments::FullComment *);
  void visitInlineCommandComment(const comments::InlineCommandComment *C,
                                 const comments::FullComment *);
  void visitHTMLStartTagComment(const comments::HTMLStartTagComment *C,
                                const comments::FullComment *);
  void visitHTMLEndTagComment(const comments::HTMLEndTagComment *C,
                              const comments::FullComment *);
  void visitBlockCommandComment(const comments::BlockCommandComment *C,
                                const comments::FullComment *);
  void visitParamCommandComment(const comments::ParamCommandComment *C,
                                const comments::FullComment *FC);
  void visitTParamCommandComment(const comments::TParamCommandComment *C,
                                 const comments::FullComment *FC);
  void visitVerbatimBlockComment(const comments::VerbatimBlockComment *C,
                                 const comments::FullComment *);
  void
  visitVerbatimBlockLineComment(const comments::VerbatimBlockLineComment *C,
                                const comments::FullComment *);
  void visitVerbatimLineComment(const comments::VerbatimLineComment *C,
                                const comments::FullComment *);

// Implements Visit methods for Attrs.
#include "language/Core/AST/AttrTextNodeDump.inc"

  void VisitNullTemplateArgument(const TemplateArgument &TA);
  void VisitTypeTemplateArgument(const TemplateArgument &TA);
  void VisitDeclarationTemplateArgument(const TemplateArgument &TA);
  void VisitNullPtrTemplateArgument(const TemplateArgument &TA);
  void VisitIntegralTemplateArgument(const TemplateArgument &TA);
  void VisitStructuralValueTemplateArgument(const TemplateArgument &TA);
  void VisitTemplateTemplateArgument(const TemplateArgument &TA);
  void VisitTemplateExpansionTemplateArgument(const TemplateArgument &TA);
  void VisitExpressionTemplateArgument(const TemplateArgument &TA);
  void VisitPackTemplateArgument(const TemplateArgument &TA);

  void VisitIfStmt(const IfStmt *Node);
  void VisitSwitchStmt(const SwitchStmt *Node);
  void VisitWhileStmt(const WhileStmt *Node);
  void VisitLabelStmt(const LabelStmt *Node);
  void VisitGotoStmt(const GotoStmt *Node);
  void VisitCaseStmt(const CaseStmt *Node);
  void VisitReturnStmt(const ReturnStmt *Node);
  void VisitCoawaitExpr(const CoawaitExpr *Node);
  void VisitCoreturnStmt(const CoreturnStmt *Node);
  void VisitCompoundStmt(const CompoundStmt *Node);
  void VisitConstantExpr(const ConstantExpr *Node);
  void VisitCallExpr(const CallExpr *Node);
  void VisitCXXOperatorCallExpr(const CXXOperatorCallExpr *Node);
  void VisitCastExpr(const CastExpr *Node);
  void VisitImplicitCastExpr(const ImplicitCastExpr *Node);
  void VisitDeclRefExpr(const DeclRefExpr *Node);
  void VisitDependentScopeDeclRefExpr(const DependentScopeDeclRefExpr *Node);
  void VisitSYCLUniqueStableNameExpr(const SYCLUniqueStableNameExpr *Node);
  void VisitPredefinedExpr(const PredefinedExpr *Node);
  void VisitCharacterLiteral(const CharacterLiteral *Node);
  void VisitIntegerLiteral(const IntegerLiteral *Node);
  void VisitFixedPointLiteral(const FixedPointLiteral *Node);
  void VisitFloatingLiteral(const FloatingLiteral *Node);
  void VisitStringLiteral(const StringLiteral *Str);
  void VisitInitListExpr(const InitListExpr *ILE);
  void VisitGenericSelectionExpr(const GenericSelectionExpr *E);
  void VisitUnaryOperator(const UnaryOperator *Node);
  void VisitUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *Node);
  void VisitMemberExpr(const MemberExpr *Node);
  void VisitExtVectorElementExpr(const ExtVectorElementExpr *Node);
  void VisitBinaryOperator(const BinaryOperator *Node);
  void VisitCompoundAssignOperator(const CompoundAssignOperator *Node);
  void VisitAddrLabelExpr(const AddrLabelExpr *Node);
  void VisitCXXNamedCastExpr(const CXXNamedCastExpr *Node);
  void VisitCXXBoolLiteralExpr(const CXXBoolLiteralExpr *Node);
  void VisitCXXThisExpr(const CXXThisExpr *Node);
  void VisitCXXFunctionalCastExpr(const CXXFunctionalCastExpr *Node);
  void VisitCXXStaticCastExpr(const CXXStaticCastExpr *Node);
  void VisitCXXUnresolvedConstructExpr(const CXXUnresolvedConstructExpr *Node);
  void VisitCXXConstructExpr(const CXXConstructExpr *Node);
  void VisitCXXBindTemporaryExpr(const CXXBindTemporaryExpr *Node);
  void VisitCXXNewExpr(const CXXNewExpr *Node);
  void VisitCXXDeleteExpr(const CXXDeleteExpr *Node);
  void VisitTypeTraitExpr(const TypeTraitExpr *Node);
  void VisitArrayTypeTraitExpr(const ArrayTypeTraitExpr *Node);
  void VisitExpressionTraitExpr(const ExpressionTraitExpr *Node);
  void VisitCXXDefaultArgExpr(const CXXDefaultArgExpr *Node);
  void VisitCXXDefaultInitExpr(const CXXDefaultInitExpr *Node);
  void VisitMaterializeTemporaryExpr(const MaterializeTemporaryExpr *Node);
  void VisitExprWithCleanups(const ExprWithCleanups *Node);
  void VisitUnresolvedLookupExpr(const UnresolvedLookupExpr *Node);
  void VisitSizeOfPackExpr(const SizeOfPackExpr *Node);
  void
  VisitCXXDependentScopeMemberExpr(const CXXDependentScopeMemberExpr *Node);
  void VisitObjCAtCatchStmt(const ObjCAtCatchStmt *Node);
  void VisitObjCEncodeExpr(const ObjCEncodeExpr *Node);
  void VisitObjCMessageExpr(const ObjCMessageExpr *Node);
  void VisitObjCBoxedExpr(const ObjCBoxedExpr *Node);
  void VisitObjCSelectorExpr(const ObjCSelectorExpr *Node);
  void VisitObjCProtocolExpr(const ObjCProtocolExpr *Node);
  void VisitObjCPropertyRefExpr(const ObjCPropertyRefExpr *Node);
  void VisitObjCSubscriptRefExpr(const ObjCSubscriptRefExpr *Node);
  void VisitObjCIvarRefExpr(const ObjCIvarRefExpr *Node);
  void VisitObjCBoolLiteralExpr(const ObjCBoolLiteralExpr *Node);
  void VisitOMPIteratorExpr(const OMPIteratorExpr *Node);
  void VisitConceptSpecializationExpr(const ConceptSpecializationExpr *Node);
  void VisitRequiresExpr(const RequiresExpr *Node);

  void VisitRValueReferenceType(const ReferenceType *T);
  void VisitArrayType(const ArrayType *T);
  void VisitConstantArrayType(const ConstantArrayType *T);
  void VisitVariableArrayType(const VariableArrayType *T);
  void VisitDependentSizedArrayType(const DependentSizedArrayType *T);
  void VisitDependentSizedExtVectorType(const DependentSizedExtVectorType *T);
  void VisitVectorType(const VectorType *T);
  void VisitFunctionType(const FunctionType *T);
  void VisitFunctionProtoType(const FunctionProtoType *T);
  void VisitUnresolvedUsingType(const UnresolvedUsingType *T);
  void VisitUsingType(const UsingType *T);
  void VisitTypedefType(const TypedefType *T);
  void VisitUnaryTransformType(const UnaryTransformType *T);
  void VisitTagType(const TagType *T);
  void VisitTemplateTypeParmType(const TemplateTypeParmType *T);
  void VisitSubstTemplateTypeParmType(const SubstTemplateTypeParmType *T);
  void
  VisitSubstTemplateTypeParmPackType(const SubstTemplateTypeParmPackType *T);
  void VisitAutoType(const AutoType *T);
  void VisitDeducedTemplateSpecializationType(
      const DeducedTemplateSpecializationType *T);
  void VisitTemplateSpecializationType(const TemplateSpecializationType *T);
  void VisitInjectedClassNameType(const InjectedClassNameType *T);
  void VisitObjCInterfaceType(const ObjCInterfaceType *T);
  void VisitPackExpansionType(const PackExpansionType *T);

  void VisitTypeLoc(TypeLoc TL);

  void VisitLabelDecl(const LabelDecl *D);
  void VisitTypedefDecl(const TypedefDecl *D);
  void VisitEnumDecl(const EnumDecl *D);
  void VisitRecordDecl(const RecordDecl *D);
  void VisitEnumConstantDecl(const EnumConstantDecl *D);
  void VisitIndirectFieldDecl(const IndirectFieldDecl *D);
  void VisitFunctionDecl(const FunctionDecl *D);
  void VisitCXXDeductionGuideDecl(const CXXDeductionGuideDecl *D);
  void VisitFieldDecl(const FieldDecl *D);
  void VisitVarDecl(const VarDecl *D);
  void VisitBindingDecl(const BindingDecl *D);
  void VisitCapturedDecl(const CapturedDecl *D);
  void VisitImportDecl(const ImportDecl *D);
  void VisitPragmaCommentDecl(const PragmaCommentDecl *D);
  void VisitPragmaDetectMismatchDecl(const PragmaDetectMismatchDecl *D);
  void VisitOMPExecutableDirective(const OMPExecutableDirective *D);
  void VisitOMPDeclareReductionDecl(const OMPDeclareReductionDecl *D);
  void VisitOMPRequiresDecl(const OMPRequiresDecl *D);
  void VisitOMPCapturedExprDecl(const OMPCapturedExprDecl *D);
  void VisitNamespaceDecl(const NamespaceDecl *D);
  void VisitUsingDirectiveDecl(const UsingDirectiveDecl *D);
  void VisitNamespaceAliasDecl(const NamespaceAliasDecl *D);
  void VisitTypeAliasDecl(const TypeAliasDecl *D);
  void VisitTypeAliasTemplateDecl(const TypeAliasTemplateDecl *D);
  void VisitCXXRecordDecl(const CXXRecordDecl *D);
  void VisitFunctionTemplateDecl(const FunctionTemplateDecl *D);
  void VisitClassTemplateDecl(const ClassTemplateDecl *D);
  void VisitBuiltinTemplateDecl(const BuiltinTemplateDecl *D);
  void VisitVarTemplateDecl(const VarTemplateDecl *D);
  void VisitTemplateTypeParmDecl(const TemplateTypeParmDecl *D);
  void VisitNonTypeTemplateParmDecl(const NonTypeTemplateParmDecl *D);
  void VisitTemplateTemplateParmDecl(const TemplateTemplateParmDecl *D);
  void VisitUsingDecl(const UsingDecl *D);
  void VisitUnresolvedUsingTypenameDecl(const UnresolvedUsingTypenameDecl *D);
  void VisitUnresolvedUsingValueDecl(const UnresolvedUsingValueDecl *D);
  void VisitUsingEnumDecl(const UsingEnumDecl *D);
  void VisitUsingShadowDecl(const UsingShadowDecl *D);
  void VisitConstructorUsingShadowDecl(const ConstructorUsingShadowDecl *D);
  void VisitLinkageSpecDecl(const LinkageSpecDecl *D);
  void VisitAccessSpecDecl(const AccessSpecDecl *D);
  void VisitFriendDecl(const FriendDecl *D);
  void VisitObjCIvarDecl(const ObjCIvarDecl *D);
  void VisitObjCMethodDecl(const ObjCMethodDecl *D);
  void VisitObjCTypeParamDecl(const ObjCTypeParamDecl *D);
  void VisitObjCCategoryDecl(const ObjCCategoryDecl *D);
  void VisitObjCCategoryImplDecl(const ObjCCategoryImplDecl *D);
  void VisitObjCProtocolDecl(const ObjCProtocolDecl *D);
  void VisitObjCInterfaceDecl(const ObjCInterfaceDecl *D);
  void VisitObjCImplementationDecl(const ObjCImplementationDecl *D);
  void VisitObjCCompatibleAliasDecl(const ObjCCompatibleAliasDecl *D);
  void VisitObjCPropertyDecl(const ObjCPropertyDecl *D);
  void VisitObjCPropertyImplDecl(const ObjCPropertyImplDecl *D);
  void VisitBlockDecl(const BlockDecl *D);
  void VisitConceptDecl(const ConceptDecl *D);
  void
  VisitLifetimeExtendedTemporaryDecl(const LifetimeExtendedTemporaryDecl *D);
  void VisitHLSLBufferDecl(const HLSLBufferDecl *D);
  void VisitHLSLRootSignatureDecl(const HLSLRootSignatureDecl *D);
  void VisitHLSLOutArgExpr(const HLSLOutArgExpr *E);
  void VisitOpenACCConstructStmt(const OpenACCConstructStmt *S);
  void VisitOpenACCLoopConstruct(const OpenACCLoopConstruct *S);
  void VisitOpenACCCombinedConstruct(const OpenACCCombinedConstruct *S);
  void VisitOpenACCDataConstruct(const OpenACCDataConstruct *S);
  void VisitOpenACCEnterDataConstruct(const OpenACCEnterDataConstruct *S);
  void VisitOpenACCExitDataConstruct(const OpenACCExitDataConstruct *S);
  void VisitOpenACCHostDataConstruct(const OpenACCHostDataConstruct *S);
  void VisitOpenACCWaitConstruct(const OpenACCWaitConstruct *S);
  void VisitOpenACCInitConstruct(const OpenACCInitConstruct *S);
  void VisitOpenACCSetConstruct(const OpenACCSetConstruct *S);
  void VisitOpenACCShutdownConstruct(const OpenACCShutdownConstruct *S);
  void VisitOpenACCUpdateConstruct(const OpenACCUpdateConstruct *S);
  void VisitOpenACCAtomicConstruct(const OpenACCAtomicConstruct *S);
  void VisitOpenACCCacheConstruct(const OpenACCCacheConstruct *S);
  void VisitOpenACCAsteriskSizeExpr(const OpenACCAsteriskSizeExpr *S);
  void VisitOpenACCDeclareDecl(const OpenACCDeclareDecl *D);
  void VisitOpenACCRoutineDecl(const OpenACCRoutineDecl *D);
  void VisitOpenACCRoutineDeclAttr(const OpenACCRoutineDeclAttr *A);
  void VisitEmbedExpr(const EmbedExpr *S);
  void VisitAtomicExpr(const AtomicExpr *AE);
  void VisitConvertVectorExpr(const ConvertVectorExpr *S);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_TEXTNODEDUMPER_H
