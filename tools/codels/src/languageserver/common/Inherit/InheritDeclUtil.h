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

#ifndef LSPSERVER_INHERITDECLUTIL_H
#define LSPSERVER_INHERITDECLUTIL_H

#include "Codira/AST/Node.h"
#include "../../../json-rpc/Common.h"
#include "../../../json-rpc/CompletionType.h"

namespace ark {
class InheritDeclUtil {
public:
    explicit InheritDeclUtil(Ptr<const Codira::AST::Decl> iDecl) : inDecl(iDecl) {};

    void HandleFuncDecl(bool isDocumentHighlight = false);

    std::set<Ptr<Codira::AST::Decl> > GetRelatedFuncDecls() const
    {
        return funcDecls;
    };
private:
    template<class T>
    void HandleDeclBody(T *decl);

    template<class T>
    void HandleDeclBodyForProp(T *decl);

    void HandleRelatedFuncDeclsFromTopLevel(Ptr<Codira::AST::Decl> topLevel, bool needSub = true);

    Ptr<const Codira::AST::Decl> inDecl{nullptr};
    Ptr<const Codira::AST::FuncDecl> defaultFuncDecl{nullptr};
    Ptr<const Codira::AST::PropDecl> defaultPropDecl{nullptr};
    std::set<Ptr<Codira::AST::Decl> > funcDecls{};
    bool isRename = false;
    std::string pkgName = "";
    std::string newName = "";
    std::string editPkgPath = "";
    std::map<std::string, bool> superDecls = {};
    std::set<Location> References{};
    std::unordered_map<std::string, std::set<TextEdit>> defineEditMap{};
    std::unordered_map<std::string, std::set<TextEdit>> usersEditMap{};

    void GetRefInfoFromFuncDecl();

    void GetReNameInfoFromFuncDecl();

    void addDeclToRef(Ptr<Codira::AST::Decl> const &decl, int length);

    void InsertRefUsers(std::unordered_set<Ptr<Codira::AST::Node> > &users);

    void InsertRenameUsers(const std::string &definedPath, std::unordered_set<Ptr<Codira::AST::Node> > &users);

    void DealTopClass(std::vector<Ptr<Codira::AST::InheritableDecl> > &topClasses);
};
} // namespace ark

#endif // LSPSERVER_INHERITDECLUTIL_H
