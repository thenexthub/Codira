/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file implements Search apis for TypeChecker.
 */

#include <thread>

#include "TypeCheckerImpl.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Query.h"
#include "Codira/AST/ScopeManagerApi.h"
#include "Codira/AST/Searcher.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Utils/Utils.h"

using namespace Codira;
using namespace AST;

namespace {
constexpr unsigned int CORES_REQUIRED_WARMUP = 8;
}

void TypeChecker::TypeCheckerImpl::WarmupCache(const ASTContext& ctx) const
{
    auto numProcessors = std::thread::hardware_concurrency();
    if (numProcessors < CORES_REQUIRED_WARMUP) {
        return;
    }
    auto scopeNames = ctx.searcher->GetScopeNamesByPrefix(ctx, TOPLEVEL_SCOPE_NAME);
    auto numScopeNames = scopeNames.size();
    if (numProcessors < numScopeNames) {
        return;
    }
    std::unordered_map<std::string, std::vector<Symbol*>> cache;
    std::vector<std::thread> threads(numScopeNames);
    std::vector<std::unique_ptr<Searcher>> searchers(numScopeNames);
    for (size_t i = 0; i < numScopeNames; i++) {
        searchers[i] = std::make_unique<Searcher>();
    }
    for (size_t i = 0; i < numScopeNames; i++) {
        std::string scopeName = scopeNames[i];
        Searcher* s = searchers[i].get();
        threads[i] = std::thread([scopeName, s, &ctx]() {
            Query q(Operator::NOT);
            q.left = std::make_unique<Query>(Operator::AND);
            q.left->left = std::make_unique<Query>("scope_name", scopeName);
            q.left->right = std::make_unique<Query>(Operator::OR);
            q.left->right->left = std::make_unique<Query>("ast_kind", "decl");
            q.left->right->left->matchKind = MatchKind::SUFFIX;
            q.left->right->right = std::make_unique<Query>("ast_kind", "func_param");
            q.right = std::make_unique<Query>("ast_kind", "extend_decl");
            s->Search(ctx, &q);
        });
    }
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    for (auto& searcher : searchers) {
        cache.merge(searcher->GetCache());
    }
    ctx.searcher->SetCache(cache);
}

std::vector<Symbol*> TypeChecker::TypeCheckerImpl::GetToplevelDecls(const ASTContext& ctx) const
{
    // "scope_level:0 && ast_kind: *decl"
    Query q(Operator::AND);
    q.left = std::make_unique<Query>("scope_level", "0");
    q.right = std::make_unique<Query>("ast_kind", "decl");
    q.right->matchKind = MatchKind::SUFFIX;
    return ctx.searcher->Search(ctx, &q, Sort::posAsc);
}

std::vector<Symbol*> TypeChecker::TypeCheckerImpl::GetAllDecls(const ASTContext& ctx) const
{
    Query q("ast_kind", "decl", MatchKind::SUFFIX);
    return ctx.searcher->Search(ctx, &q, Sort::posAsc);
}

std::vector<Symbol*> TypeChecker::TypeCheckerImpl::GetGenericCandidates(const ASTContext& ctx) const
{
    return ctx.searcher->Search(ctx,
        "ast_kind : class_decl || ast_kind : interface_decl || ast_kind : struct_decl || ast_kind : enum_decl || "
        "ast_kind : func_decl || ast_kind : extend_decl || ast_kind : builtin_decl",
        Sort::posAsc);
}

std::vector<Symbol*> TypeChecker::TypeCheckerImpl::GetAllStructDecls(const ASTContext& ctx) const
{
    return ctx.searcher->Search(ctx,
        "ast_kind : class_decl || ast_kind : interface_decl || ast_kind : struct_decl || ast_kind : enum_decl || "
        "ast_kind : extend_decl",
        Sort::posAsc);
}

std::vector<Symbol*> TypeChecker::TypeCheckerImpl::GetSymsByASTKind(
    const ASTContext& ctx, ASTKind astKind, const Order& order) const
{
    Query q = Query("ast_kind", ASTKIND_TO_STRING_MAP[astKind]);
    return ctx.searcher->Search(ctx, &q, order);
}
