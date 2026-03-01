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

#ifndef LSPSERVER_FINDOVERRIDEMETHODSIMPL_H
#define LSPSERVER_FINDOVERRIDEMETHODSIMPL_H

#include "../../ArkAST.h"

namespace ark {

struct OverrideMethodInfo {
    bool deprecated{false};
    bool isProp{false};
    std::string signatureWithRet;
    std::string insertText;
};

struct OverrideMethodsItem {
    std::string package;
    std::string kind;
    std::string identifier;
    std::vector<OverrideMethodInfo> overrideMethodInfos;
};

struct FindOverrideMethodResult {
    std::vector<OverrideMethodsItem> overrideMethods;
};

using OverridableFuncAndPropMap =
    std::unordered_map<Ptr<InheritableDecl>, std::pair<std::vector<FuncDecl*>, std::vector<PropDecl*>>>;

class FindOverrideMethodsImpl {
    static std::string fullPkgName;
    static std::string curFilePath;
    static std::unordered_map<Ptr<InheritableDecl>, std::unordered_map<std::string, std::string>> genericReplaceMap;

public:
    static void FindOverrideMethods(const ArkAST &ast, FindOverrideMethodResult &result, Codira::Position pos,
                                    bool isExtend);
    static void GetImplementedMethodsAndProps(const ArkAST &ast, Ptr<InheritableDecl> decl,
                                              std::vector<FuncDecl*>& implementedMethods,
                                              std::vector<PropDecl*>& implementedDecls);
    static void GetOverridableMethodsAndPropsMap(Ptr<InheritableDecl> decl,
                                                 OverridableFuncAndPropMap& overrideableMethodsAndPropMap,
                                                 const std::vector<FuncDecl*>& implementedMethods,
                                                 const std::vector<PropDecl*>& implementedProps);
    static void AddFuncItemsToResult(const Ptr<Decl>& decl, const Ptr<InheritableDecl>& owner,
                                     OverrideMethodsItem& item, const std::vector<FuncDecl*>& overridableMethods,
                                     const std::vector<Ptr<ClassLikeDecl>>& canSuperCall);

    static void AddPropItemsToResult(const Ptr<Decl> decl, const Ptr<InheritableDecl> owner,
                                     OverrideMethodsItem& item, const std::vector<PropDecl*>& overridableProps,
                                     const std::vector<Ptr<ClassLikeDecl>>& canSuperCall);
};
} // namespace ark
#endif // LSPSERVER_FINDOVERRIDEMETHODSIMPL_H
