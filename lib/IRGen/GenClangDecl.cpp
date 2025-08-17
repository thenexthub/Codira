//===--- GenClangDecl.cpp - Codira IRGen for imported Clang declarations ---===//
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

#include "IRGenModule.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ClangModuleLoader.h"
#include "language/AST/Expr.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/Stmt.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclGroup.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExprCXX.h"
#include "language/Core/AST/GlobalDecl.h"
#include "language/Core/AST/RecordLayout.h"
#include "language/Core/AST/RecursiveASTVisitor.h"
#include "language/Core/CodeGen/ModuleBuilder.h"
#include "language/Core/Sema/Sema.h"
#include "toolchain/ADT/SmallPtrSet.h"

using namespace language;
using namespace irgen;

namespace {
class ClangDeclFinder
    : public language::Core::RecursiveASTVisitor<ClangDeclFinder> {
  std::function<void(const language::Core::Decl *)> callback;
  ClangModuleLoader *clangModuleLoader;

public:
  template <typename Fn>
  explicit ClangDeclFinder(Fn fn, ClangModuleLoader *clangModuleLoader)
      : callback(fn), clangModuleLoader(clangModuleLoader) {}

  bool VisitDeclRefExpr(language::Core::DeclRefExpr *DRE) {
    if (isa<language::Core::FunctionDecl>(DRE->getDecl()) ||
        isa<language::Core::VarDecl>(DRE->getDecl())) {
      callback(DRE->getDecl());
    }

    return true;
  }

  bool VisitMemberExpr(language::Core::MemberExpr *ME) {
    if (isa<language::Core::FunctionDecl>(ME->getMemberDecl()) ||
        isa<language::Core::VarDecl>(ME->getMemberDecl()) || 
        isa<language::Core::FieldDecl>(ME->getMemberDecl())) {
      callback(ME->getMemberDecl());
    }
    return true;
  }

  bool VisitFunctionDecl(language::Core::FunctionDecl *functionDecl) {
    for (auto paramDecl : functionDecl->parameters()) {
      if (paramDecl->hasDefaultArg()) {
        if (FuncDecl *defaultArgGenerator =
                clangModuleLoader->getDefaultArgGenerator(paramDecl)) {
          // Deconstruct the Codira function that was created in
          // CodiraDeclSynthesizer::makeDefaultArgument and extract the
          // underlying Clang function that was also synthesized.
          BraceStmt *body = defaultArgGenerator->getTypecheckedBody();
          auto returnStmt =
              cast<ReturnStmt>(body->getSingleActiveElement().get<Stmt *>());
          auto callExpr = cast<CallExpr>(returnStmt->getResult());
          auto calledFuncDecl = cast<FuncDecl>(callExpr->getCalledValue());
          auto calledClangFuncDecl =
              cast<language::Core::FunctionDecl>(calledFuncDecl->getClangDecl());
          callback(calledClangFuncDecl);
        }
      }
    }

    return true;
  }

  bool VisitCXXConstructorDecl(language::Core::CXXConstructorDecl *CXXCD) {
    callback(CXXCD);
    for (language::Core::CXXCtorInitializer *CXXCI : CXXCD->inits()) {
      if (language::Core::FieldDecl *FD = CXXCI->getMember()) {
        callback(FD);
        // A throwing constructor might throw after the field is initialized,
        // emitting additional cleanup code that destroys the field. Make sure
        // we record the destructor of the field in that case as it might need
        // to be potentially emitted.
        if (auto *recordType = FD->getType()->getAsCXXRecordDecl()) {
          if (auto *destructor = recordType->getDestructor()) {
            if (!destructor->isDeleted())
              callback(destructor);
          }
        }
      }
    }
    return true;
  }

  bool VisitCXXConstructExpr(language::Core::CXXConstructExpr *CXXCE) {
    callback(CXXCE->getConstructor());
    return true;
  }

  bool VisitCXXDeleteExpr(language::Core::CXXDeleteExpr *deleteExpr) {
    if (auto cxxRecord = deleteExpr->getDestroyedType()->getAsCXXRecordDecl())
      if (auto dtor = cxxRecord->getDestructor())
        callback(dtor);
    return true;
  }

  bool VisitVarDecl(language::Core::VarDecl *VD) {
    if (auto cxxRecord = VD->getType()->getAsCXXRecordDecl())
      if (auto dtor = cxxRecord->getDestructor())
        callback(dtor);

    return true;
  }

  bool VisitCXXBindTemporaryExpr(language::Core::CXXBindTemporaryExpr *BTE) {
    // This is a temporary value with a custom destructor. C++ will implicitly
    // call the destructor at some point. Make sure we emit IR for it.
    callback(BTE->getTemporary()->getDestructor());
    return true;
  }

  bool VisitCXXNewExpr(language::Core::CXXNewExpr *NE) {
    callback(NE->getOperatorNew());
    return true;
  }

  bool VisitBindingDecl(language::Core::BindingDecl *BD) {
    if (auto *holdingVar = BD->getHoldingVar()) {
      if (holdingVar->hasInit())
        TraverseStmt(holdingVar->getInit());
    }
    return true;
  }

  bool VisitCXXInheritedCtorInitExpr(language::Core::CXXInheritedCtorInitExpr *CIE) {
    if (auto ctor = CIE->getConstructor())
      callback(ctor);
    return true;
  }

  // Do not traverse unevaluated expressions. Doing to might result in compile
  // errors if we try to instantiate an un-instantiatable template.

  bool TraverseCXXNoexceptExpr(language::Core::CXXNoexceptExpr *NEE) { return true; }

  bool TraverseCXXTypeidExpr(language::Core::CXXTypeidExpr *TIE) {
    if (TIE->isPotentiallyEvaluated())
      language::Core::RecursiveASTVisitor<ClangDeclFinder>::TraverseCXXTypeidExpr(TIE);
    return true;
  }

  bool TraverseRequiresExpr(language::Core::RequiresExpr *RE) { return true; }

  // Do not traverse type locs, as they might contain expressions that reference
  // code that should not be instantiated and/or emitted.
  bool TraverseTypeLoc(language::Core::TypeLoc TL) { return true; }

  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitImplicitCode() const { return true; }
};

// If any (re)declaration of `decl` contains executable code, returns that
// redeclaration; otherwise, returns nullptr.
// In the case of a function, executable code is contained in the function
// definition. In the case of a variable, executable code can be contained in
// the initializer of the variable.
language::Core::Decl *getDeclWithExecutableCode(language::Core::Decl *decl) {
  if (auto fd = dyn_cast<language::Core::FunctionDecl>(decl)) {
    const language::Core::FunctionDecl *definition;
    if (fd->hasBody(definition)) {
      return const_cast<language::Core::FunctionDecl *>(definition);
    }

    // If this is a potentially not-yet-instantiated template, we might
    // still have a body.
    if (fd->getTemplateInstantiationPattern())
      return fd;
  } else if (auto vd = dyn_cast<language::Core::VarDecl>(decl)) {
    language::Core::VarDecl *initializingDecl = vd->getInitializingDeclaration();
    if (initializingDecl) {
      return initializingDecl;
    }
  } else if (auto fd = dyn_cast<language::Core::FieldDecl>(decl)) {
    if(fd->hasInClassInitializer()) {
      return fd;
    }
  }
  return nullptr;
}

} // end anonymous namespace

void IRGenModule::emitClangDecl(const language::Core::Decl *decl) {
  // Ignore this decl if we've seen it before.
  if (!GlobalClangDecls.insert(decl->getCanonicalDecl()).second)
    return;

  // Fast path for the case where `decl` doesn't contain executable code, so it
  // can't reference any other declarations that we would need to emit.
  if (getDeclWithExecutableCode(const_cast<language::Core::Decl *>(decl)) == nullptr) {
    ClangCodeGen->HandleTopLevelDecl(
                          language::Core::DeclGroupRef(const_cast<language::Core::Decl*>(decl)));
    return;
  }

  SmallVector<const language::Core::Decl *, 8> stack;
  stack.push_back(decl);

  auto callback = [&](const language::Core::Decl *D) {
    for (auto *DC = D->getDeclContext();; DC = DC->getParent()) {
      // Check that this is not a local declaration inside a function.
      if (DC->isFunctionOrMethod()) {
        return;
      }
      if (DC->isFileContext()) {
        break;
      }
      if (isa<language::Core::TagDecl>(DC)) {
        break;
      }
      if (isa<language::Core::LinkageSpecDecl>(DC)) {
        break;
      }
      D = cast<const language::Core::Decl>(DC);
    }
    if (!GlobalClangDecls.insert(D->getCanonicalDecl()).second) {
      return;
    }

    stack.push_back(D);
  };

  ClangModuleLoader *clangModuleLoader = Context.getClangModuleLoader();
  ClangDeclFinder refFinder(callback, clangModuleLoader);

  auto &clangSema = clangModuleLoader->getClangSema();

  while (!stack.empty()) {
    auto *next = const_cast<language::Core::Decl *>(stack.pop_back_val());

    // If this is a static member of a class, it might be defined out of line.
    // If the class is templated, the definition of its static member might be
    // templated as well. If it is, instantiate it here.
    if (auto var = dyn_cast<language::Core::VarDecl>(next)) {
      if (var->isStaticDataMember() &&
          var->getTemplateSpecializationKind() ==
              language::Core::TemplateSpecializationKind::TSK_ImplicitInstantiation)
        clangSema.InstantiateVariableDefinition(var->getLocation(), var);
    }

    // If a function calls another method in a class template specialization, we
    // need to instantiate that other function. Do that here.
    if (auto *fn = dyn_cast<language::Core::FunctionDecl>(next)) {
      // Make sure that this method is part of a class template specialization.
      if (fn->getTemplateInstantiationPattern())
        clangSema.InstantiateFunctionDefinition(fn->getLocation(), fn);
    }

    if (language::Core::Decl *executableDecl = getDeclWithExecutableCode(next)) {
        refFinder.TraverseDecl(executableDecl);
        next = executableDecl;
    }

    // Unfortunately, implicitly defined CXXDestructorDecls don't have a real
    // body, so we need to traverse these manually.
    if (auto *dtor = dyn_cast<language::Core::CXXDestructorDecl>(next)) {
      if (dtor->isImplicit() && dtor->isDefaulted() && !dtor->isDeleted() &&
          !dtor->doesThisDeclarationHaveABody())
        clangSema.DefineImplicitDestructor(dtor->getLocation(), dtor);

      if (dtor->isImplicit() || dtor->hasBody()) {
        auto cxxRecord = dtor->getParent();

        for (auto field : cxxRecord->fields()) {
          if (auto fieldCxxRecord = field->getType()->getAsCXXRecordDecl())
            if (auto *fieldDtor = fieldCxxRecord->getDestructor())
              callback(fieldDtor);
        }

        for (auto base : cxxRecord->bases()) {
          if (auto baseCxxRecord = base.getType()->getAsCXXRecordDecl())
            if (auto *baseDtor = baseCxxRecord->getDestructor())
              callback(baseDtor);
        }
      }
    }

    // If something from a C++ class is used, emit all virtual methods of this
    // class because they might be emitted in the vtable even if not used
    // directly from Codira.
    if (auto *record = dyn_cast<language::Core::CXXRecordDecl>(next->getDeclContext())) {
      if (auto *destructor = record->getDestructor()) {
        // Ensure virtual destructors have the body defined, even if they're
        // not used directly, as they might be referenced by the emitted vtable.
        if (destructor->isVirtual() && !destructor->isDeleted())
          ensureImplicitCXXDestructorBodyIsDefined(destructor);
      }
      for (auto *method : record->methods()) {
        if (method->isVirtual()) {
          callback(method);
        }
      }
    }

    if (auto var = dyn_cast<language::Core::VarDecl>(next))
      if (!var->isFileVarDecl())
        continue;
    if (isa<language::Core::FieldDecl>(next)) {
      continue;
    }

    ClangCodeGen->HandleTopLevelDecl(language::Core::DeclGroupRef(next));
  }
}

toolchain::Constant *
IRGenModule::getAddrOfClangGlobalDecl(language::Core::GlobalDecl global,
                                      ForDefinition_t forDefinition) {
  // Register the decl with the clang code generator.
  if (auto decl = global.getDecl())
    emitClangDecl(decl);

  return ClangCodeGen->GetAddrOfGlobal(global, (bool) forDefinition);
}

void IRGenModule::finalizeClangCodeGen() {
  // FIXME: We try to avoid looking for PragmaCommentDecls unless we need to,
  // since language::Core::DeclContext::decls_begin() can trigger expensive
  // de-serialization.
  if (Triple.isWindowsMSVCEnvironment() || Triple.isWindowsItaniumEnvironment() ||
      IRGen.Opts.LLVMLTOKind != IRGenLLVMLTOKind::None) {
    // Ensure that code is emitted for any `PragmaCommentDecl`s. (These are
    // always guaranteed to be directly below the TranslationUnitDecl.)
    // In Clang, this happens automatically during the Sema phase, but here we
    // need to take care of it manually because our Clang CodeGenerator is not
    // attached to Clang Sema as an ASTConsumer.
    for (const auto *D : ClangASTContext->getTranslationUnitDecl()->decls()) {
      if (const auto *PCD = dyn_cast<language::Core::PragmaCommentDecl>(D)) {
        emitClangDecl(PCD);
      }
    }
  }

  ClangCodeGen->HandleTranslationUnit(
      *const_cast<language::Core::ASTContext *>(ClangASTContext));
}

void IRGenModule::ensureImplicitCXXDestructorBodyIsDefined(
    language::Core::CXXDestructorDecl *destructor) {
  if (destructor->isUserProvided() ||
      destructor->doesThisDeclarationHaveABody())
    return;
  assert(!destructor->isDeleted() &&
         "Codira cannot handle a type with no known destructor.");
  // Make sure we define the destructor so we have something to call.
  auto &sema = Context.getClangModuleLoader()->getClangSema();
  sema.DefineImplicitDestructor(language::Core::SourceLocation(), destructor);
}

/// Retrieves the base classes of a C++ struct/class ordered by their offset in
/// the derived type's memory layout.
SmallVector<CXXBaseRecordLayout, 1>
irgen::getBasesAndOffsets(const language::Core::CXXRecordDecl *decl) {
  auto &layout = decl->getASTContext().getASTRecordLayout(decl);

  // Collect the offsets and sizes of base classes within the memory layout
  // of the derived class.
  SmallVector<CXXBaseRecordLayout, 1> baseOffsetsAndSizes;
  for (auto base : decl->bases()) {
    if (base.isVirtual())
      continue;

    auto baseType = base.getType().getCanonicalType();
    auto baseRecordType = cast<language::Core::RecordType>(baseType);
    auto baseRecord = baseRecordType->getAsCXXRecordDecl();
    assert(baseRecord && "expected a base C++ record");

    if (baseRecord->isEmpty())
      continue;

    auto offset = Size(layout.getBaseClassOffset(baseRecord).getQuantity());
    // A base type might have different size and data size (sizeof != dsize).
    // Make sure we are using data size here, since fields of the derived type
    // might be packed into the base's tail padding.
    auto size = Size(decl->getASTContext()
                         .getTypeInfoDataSizeInChars(baseType)
                         .Width.getQuantity());

    baseOffsetsAndSizes.push_back({baseRecord, offset, size});
  }

  // In C++, base classes might get reordered if the primary base was not
  // the first base type on the declaration of the class.
  toolchain::sort(baseOffsetsAndSizes, [](const CXXBaseRecordLayout &lhs,
                                     const CXXBaseRecordLayout &rhs) {
    return lhs.offset < rhs.offset;
  });

  return baseOffsetsAndSizes;
}
