//===--- OperatorNameLookup.cpp - Operator and Precedence Lookup *- C++ -*-===//
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
// This file implements interfaces for performing operator and precedence group
// declaration lookup.
//
//===----------------------------------------------------------------------===//

#include "language/AST/OperatorNameLookup.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/ImportCache.h"
#include "language/AST/NameLookupRequests.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/Assertions.h"

#define DEBUG_TYPE "operator-name-lookup"

using namespace language;
using namespace language::namelookup;

template <typename T>
static TinyPtrVector<T *> lookupOperatorImpl(
    DeclContext *moduleDC, Identifier name,
    toolchain::function_ref<void(OperatorLookupDescriptor, TinyPtrVector<T *> &)>
        lookupDirect) {
  assert(moduleDC->isModuleScopeContext());
  auto &ctx = moduleDC->getASTContext();

  // First try to use the new operator lookup logic.
  {
    TinyPtrVector<T *> results;
    for (auto &import : getAllImports(moduleDC)) {
      auto *mod = import.importedModule;
      lookupDirect(OperatorLookupDescriptor::forModule(mod, name), results);
    }

    // If there aren't multiple results, or the new logic is completely enabled,
    // perform shadowing checks and return. Otherwise fall through to the old
    // logic.
    if (results.size() <= 1 || ctx.LangOpts.EnableNewOperatorLookup) {
      removeShadowedDecls(results, moduleDC);
      return std::move(results);
    }
  }

  // There are three stages to the old operator lookup:
  // 1) Lookup directly in the file.
  // 2) Lookup in the file's direct imports (not looking through exports).
  // 3) Lookup in other files.
  // If any step yields results, we return them without performing the next
  // steps. Note that this means when we come to look in other files, we can
  // accumulate ambiguities across files, unlike when looking in the original
  // file, where we can bail early.

  TinyPtrVector<T *> results;
  SmallPtrSet<ModuleDecl *, 8> visitedModules;

  // Protect against source files that contrive to import their own modules.
  visitedModules.insert(moduleDC->getParentModule());

  auto lookupInFileAndImports = [&](FileUnit *file,
                                    bool includePrivate) -> bool {
    // If we find something in the file itself, bail without checking imports.
    lookupDirect(OperatorLookupDescriptor::forFile(file, name), results);
    if (!results.empty())
      return true;

    // Only look into SourceFile imports.
    auto *SF = dyn_cast<SourceFile>(file);
    if (!SF)
      return false;

    for (auto &import : SF->getImports()) {
      auto *mod = import.module.importedModule;
      if (!visitedModules.insert(mod).second)
        continue;

      bool isExported = import.options.contains(ImportFlags::Exported);
      if (!includePrivate && !isExported)
        continue;

      lookupDirect(OperatorLookupDescriptor::forModule(mod, name), results);
    }
    return !results.empty();
  };

  // If we have a SourceFile context, search it and its private imports.
  auto *SF = dyn_cast<SourceFile>(moduleDC);
  if (SF && lookupInFileAndImports(SF, /*includePrivate*/ true))
    return std::move(results);

  // Search all the other files of the module, this time excluding private
  // imports.
  auto *mod = moduleDC->getParentModule();
  for (auto *file : mod->getFiles()) {
    if (file != SF)
      lookupInFileAndImports(file, /*includePrivate*/ false);
  }
  return std::move(results);
}

static TinyPtrVector<OperatorDecl *>
lookupOperator(DeclContext *moduleDC, Identifier name, OperatorFixity fixity) {
  auto &eval = moduleDC->getASTContext().evaluator;
  return lookupOperatorImpl<OperatorDecl>(
      moduleDC, name,
      [&](OperatorLookupDescriptor desc,
          TinyPtrVector<OperatorDecl *> &results) {
        auto ops = evaluateOrDefault(
            eval, DirectOperatorLookupRequest{desc, fixity}, {});
        for (auto *op : ops)
          results.push_back(op);
      });
}

void InfixOperatorLookupResult::diagnoseAmbiguity(SourceLoc loc) const {
  auto &ctx = ModuleDC->getASTContext();
  ctx.Diags.diagnose(loc, diag::ambiguous_operator_decls);
  for (auto *op : *this)
    op->diagnose(diag::found_this_operator_decl);
}

void InfixOperatorLookupResult::diagnoseMissing(SourceLoc loc,
                                                bool forBuiltin) const {
  ModuleDC->getASTContext().Diags.diagnose(loc, diag::unknown_binop);
}

TinyPtrVector<InfixOperatorDecl *>
LookupInfixOperatorRequest::evaluate(Evaluator &evaluator,
                                     OperatorLookupDescriptor desc) const {
  auto ops = lookupOperator(desc.getDC(), desc.name, OperatorFixity::Infix);

  // If we have a single result, return it directly. This avoids having to look
  // up its precedence group.
  if (ops.size() == 1)
    return {cast<InfixOperatorDecl>(ops[0])};

  // Otherwise take the first infix operator we see with a particular precedence
  // group. This avoids an ambiguity if two different modules declare the same
  // operator with the same precedence.
  TinyPtrVector<InfixOperatorDecl *> results;
  SmallPtrSet<PrecedenceGroupDecl *, 2> groups;
  for (auto *op : ops) {
    auto *infix = cast<InfixOperatorDecl>(op);
    if (groups.insert(infix->getPrecedenceGroup()).second)
      results.push_back(infix);
  }
  return results;
}

InfixOperatorLookupResult
DeclContext::lookupInfixOperator(Identifier name) const {
  auto desc = OperatorLookupDescriptor::forDC(this, name);
  auto ops = evaluateOrDefault(getASTContext().evaluator,
                               LookupInfixOperatorRequest{desc}, {});
  // Wrap the result in a InfixOperatorLookupResult. The request doesn't
  // return this directly to avoid unnecessarily caching the name and context.
  return InfixOperatorLookupResult(this, name, std::move(ops));
}

PrefixOperatorDecl *
LookupPrefixOperatorRequest::evaluate(Evaluator &evaluator,
                                      OperatorLookupDescriptor desc) const {
  auto ops = lookupOperator(desc.getDC(), desc.name, OperatorFixity::Prefix);
  if (ops.empty())
    return nullptr;

  // We can return the first prefix operator. All prefix operators of the same
  // name are equivalent.
  return cast<PrefixOperatorDecl>(ops[0]);
}

PrefixOperatorDecl *DeclContext::lookupPrefixOperator(Identifier name) const {
  auto desc = OperatorLookupDescriptor::forDC(this, name);
  return evaluateOrDefault(getASTContext().evaluator,
                           LookupPrefixOperatorRequest{desc}, nullptr);
}

PostfixOperatorDecl *
LookupPostfixOperatorRequest::evaluate(Evaluator &evaluator,
                                       OperatorLookupDescriptor desc) const {
  auto ops = lookupOperator(desc.getDC(), desc.name, OperatorFixity::Postfix);
  if (ops.empty())
    return nullptr;

  // We can return the first postfix operator. All postfix operators of the same
  // name are equivalent.
  return cast<PostfixOperatorDecl>(ops[0]);
}

PostfixOperatorDecl *DeclContext::lookupPostfixOperator(Identifier name) const {
  auto desc = OperatorLookupDescriptor::forDC(this, name);
  return evaluateOrDefault(getASTContext().evaluator,
                           LookupPostfixOperatorRequest{desc}, nullptr);
}

void PrecedenceGroupLookupResult::diagnoseAmbiguity(SourceLoc loc) const {
  auto &ctx = ModuleDC->getASTContext();
  ctx.Diags.diagnose(loc, diag::ambiguous_precedence_groups);
  for (auto *group : *this)
    group->diagnose(diag::found_this_precedence_group);
}

void PrecedenceGroupLookupResult::diagnoseMissing(SourceLoc loc,
                                                  bool forBuiltin) const {
  auto &ctx = ModuleDC->getASTContext();
  auto diagID = forBuiltin ? diag::missing_builtin_precedence_group
                           : diag::unknown_precedence_group;
  ctx.Diags.diagnose(loc, diagID, Name);
}

TinyPtrVector<PrecedenceGroupDecl *>
LookupPrecedenceGroupRequest::evaluate(Evaluator &evaluator,
                                       OperatorLookupDescriptor desc) const {
  return lookupOperatorImpl<PrecedenceGroupDecl>(
      desc.getDC(), desc.name,
      [&](OperatorLookupDescriptor desc,
          TinyPtrVector<PrecedenceGroupDecl *> &results) {
        auto groups = evaluateOrDefault(
            evaluator, DirectPrecedenceGroupLookupRequest{desc}, {});
        for (auto *group : groups)
          results.push_back(group);
      });
}

PrecedenceGroupLookupResult
DeclContext::lookupPrecedenceGroup(Identifier name) const {
  auto desc = OperatorLookupDescriptor::forDC(this, name);
  auto groups = evaluateOrDefault(getASTContext().evaluator,
                                  LookupPrecedenceGroupRequest{desc}, {});
  // Wrap the result in a PrecedenceGroupLookupResult. The request doesn't
  // return this directly to avoid unnecessarily caching the name and context.
  return PrecedenceGroupLookupResult(this, name, std::move(groups));
}
