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

#ifndef LSPSERVER_OVERRIDECOMPLETER_H
#define LSPSERVER_OVERRIDECOMPLETER_H

#include <vector>
#include "Codira/AST/Node.h"
#include "../../common/ItemResolverUtil.h"
#include "../overrideMethods/FindOverrideMethodsUtils.h"
#include "CompletionImpl.h"
#include "Codira/Basic/Position.h"

namespace ark {
using namespace Codira::AST;

class OverrideCompleter {
public:
    OverrideCompleter() = default;

    OverrideCompleter(Ptr<Codira::AST::Decl> decl, const std::string& prefix)
        : topLevelDecl(decl), prefix(prefix) {}

    void FindOvrrideFunction();

    std::vector<CodeCompletion> ExportItems();

    /**
     * Check function start postion, modifer.
     */
    bool SetCompletionConfig(Ptr<Decl> decl, const Position& pos);

private:

    void ResolveDeclModifiers(const Ptr<Decl> &decl);

    void CompleteFuncDecl(Ptr<FuncDecl> decl,
        const std::optional<std::unordered_map<std::string, std::string>>& replace);

    std::string GetTextString(const std::vector<std::string>& strs);

    std::string GetModifierString(const std::vector<std::string>& strs,
                                  const std::unordered_set<std::string>& filter);

    void FilterModifiers();

    std::vector<Ptr<ClassLikeDecl>> GetSuperCallDecls(Ptr<Decl> decl);

    void ExtractReplace(Ptr<Decl> decl);

    bool CheckConfilctModifer(Ptr<Decl> decl);

    bool CheckRepeated(Ptr<FuncDecl> decl);

    std::string GetFuncLabel(const FuncDetail& funcDetail);

    Ptr<Codira::AST::Decl> topLevelDecl = nullptr;

    std::map<Ptr<InheritableDecl>, std::vector<Ptr<FuncDecl>>> funcMap{};

    std::string prefix{};

    std::vector<FuncDetail> functionDetails;

    Position additionalPos;

    std::optional<Attribute> modifier;

    std::unordered_set<std::string> modifierSet;

    std::unordered_set<std::string> curModifierSet;

    bool hasFuncWord = true; // if decl has a func keyword

    std::vector<Ptr<FuncDecl>> implementedMethods;

    std::unordered_map<Ptr<InheritableDecl>, std::unordered_map<std::string, std::string>> genericTypeMap;
};

} // namespace ark

#endif // LSPSERVER_OVERRIDECOMPLETER_H
