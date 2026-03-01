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

#include "InheritDeclUtil.h"
#include "InheritImpl.h"
#include "../Utils.h"
#include "../../CompilerCodiraProject.h"

using namespace Codira::AST;
using namespace Codira;

namespace ark {
template<class T>
void InheritDeclUtil::HandleDeclBody(T *decl)
{
    if (!decl) { return; }
    if (inDecl->astKind == Codira::AST::ASTKind::PROP_DECL) {
        HandleDeclBodyForProp(decl);
        return;
    }
    for (auto &declItem: decl->GetMemberDecls()) {
        if (!ark::Is<FuncDecl>(declItem.get().get())) {
            continue;
        }
        auto funcDecl = dynamic_cast<FuncDecl*>(declItem.get().get());
        if (!funcDecl || !defaultFuncDecl) { continue; }
        if (IsFuncSignatureIdentical(*defaultFuncDecl, *funcDecl)) {
            (void) funcDecls.insert(funcDecl);
            return;
        }
    }

    auto inheritableDecl = dynamic_cast<InheritableDecl*>(decl);
    if (!inheritableDecl) { return; }
    auto extendDecls = CompilerCodiraProject::GetInstance()->GetExtendDecls(inheritableDecl, pkgName);
    for (auto extendDecl : extendDecls) {
        for (auto &member : extendDecl->members) {
            if (!ark::Is<FuncDecl>(member.get().get())) {
                continue;
            }
            auto funcDecl = dynamic_cast<FuncDecl*>(member.get().get());
            if (!funcDecl || !defaultFuncDecl) { continue; }
            if (IsFuncSignatureIdentical(*defaultFuncDecl, *funcDecl)) {
                (void) funcDecls.insert(funcDecl);
                return;
            }
        }
    }
}

template<class T>
void InheritDeclUtil::HandleDeclBodyForProp(T *decl)
{
    if (!decl) { return; }
    for (auto &declItem : decl->GetMemberDecls()) {
        if (!ark::Is<PropDecl>(declItem.get().get())) { continue; }
        auto pPropDecl = dynamic_cast<PropDecl*>(declItem.get().get());
        if (!pPropDecl || !defaultPropDecl) { continue; }
        if (pPropDecl->identifier == defaultPropDecl->identifier) {
            (void) funcDecls.insert(pPropDecl);
            return;
        }
    }

    auto inheritableDecl = dynamic_cast<InheritableDecl*>(decl);
    if (!inheritableDecl) { return; }
    auto extendDecls = CompilerCodiraProject::GetInstance()->GetExtendDecls(inheritableDecl, pkgName);
    for (auto extendDecl : extendDecls) {
        for (auto &declItem : extendDecl->members) {
            if (!ark::Is<PropDecl>(declItem.get().get())) {
                continue;
            }
            auto pPropDecl = dynamic_cast<PropDecl*>(declItem.get().get());
            if (!pPropDecl || !defaultPropDecl) { continue; }
            if (pPropDecl->identifier == defaultPropDecl->identifier) {
                (void) funcDecls.insert(pPropDecl);
                return;
            }
        }
    }
}

void InheritDeclUtil::HandleRelatedFuncDeclsFromTopLevel(Ptr<Decl> topLevel, bool needSub)
{
    // if not in super push it in
    if (!topLevel || topLevel->identifier.Empty()) {
        return;
    }
    // Only exported classes and interfaces are recorded.
    if (superDecls.find(topLevel->identifier) == superDecls.end()) {
        superDecls[topLevel->identifier] = topLevel->TestAttr(Attribute::PUBLIC);
    }
    if (ark::Is<ClassDecl>(topLevel.get())) {
        auto classDecl = dynamic_cast<ClassDecl*>(topLevel.get());
        HandleDeclBody(classDecl);
        if (!needSub) { return; }
        for (auto &item : classDecl->subDecls) {
            HandleRelatedFuncDeclsFromTopLevel(item);
        }
    } else if (ark::Is<StructDecl>(topLevel.get())) {
        auto structDecl = dynamic_cast<StructDecl*>(topLevel.get());
        HandleDeclBody(structDecl);
    } else if (ark::Is<EnumDecl>(topLevel.get())) {
        auto enumDecl = dynamic_cast<EnumDecl*>(topLevel.get());
        HandleDeclBody(enumDecl);
    } else {
        auto interfaceDecl = dynamic_cast<InterfaceDecl*>(topLevel.get());
        if (!interfaceDecl || !interfaceDecl->body) {
            return;
        }
        HandleDeclBody(interfaceDecl);
        if (!needSub) { return; }
        for (auto &item : interfaceDecl->subDecls) {
            HandleRelatedFuncDeclsFromTopLevel(item);
        }
    }
}

void InheritDeclUtil::HandleFuncDecl(bool isDocumentHighlight)
{
    bool isInvalid = !inDecl || !inDecl->outerDecl || inDecl->astKind != Codira::AST::ASTKind::FUNC_DECL &&
                                                      inDecl->astKind != Codira::AST::ASTKind::PROP_DECL;
    if (isInvalid) { return; }
    Ptr<InheritableDecl> classLikeOrStructDecl{nullptr};
    pkgName = inDecl->fullPackageName;
    if (inDecl->astKind == Codira::AST::ASTKind::FUNC_DECL) {
        defaultFuncDecl = dynamic_cast<const FuncDecl*>(inDecl.get());
        // Make sure that the funcDefinition is in class/enum/interface/struct
        if (!defaultFuncDecl || !defaultFuncDecl->funcBody ||
            (!defaultFuncDecl->funcBody->parentClassLike && !defaultFuncDecl->funcBody->parentStruct
             && !defaultFuncDecl->funcBody->parentEnum)) {
            return;
        }
    }
    if (inDecl->astKind == Codira::AST::ASTKind::PROP_DECL) {
        defaultPropDecl = dynamic_cast<const PropDecl*>(inDecl.get());
    }
    classLikeOrStructDecl = dynamic_cast<InheritableDecl*>(inDecl->outerDecl.get());
    if (!classLikeOrStructDecl) { return; }
    // deal funcDecl in Extend
    if (inDecl->outerDecl->astKind == Codira::AST::ASTKind::EXTEND_DECL) {
        for (auto &item: classLikeOrStructDecl->inheritedTypes) {
            if (item->ty->kind == TypeKind::TYPE_CLASS) {
                auto superDecl = dynamic_cast<ClassTy *>(item->ty.get())->decl;
                auto realDecl = CompilerCodiraProject::GetInstance()->GetDeclInPkgByNode(superDecl, editPkgPath);
                HandleRelatedFuncDeclsFromTopLevel(realDecl, !isDocumentHighlight);
            } else if (item->ty->kind == TypeKind::TYPE_INTERFACE) {
                auto superDecl = dynamic_cast<InterfaceTy *>(item->ty.get())->decl;
                auto realDecl = CompilerCodiraProject::GetInstance()->GetDeclInPkgByNode(superDecl, editPkgPath);
                HandleRelatedFuncDeclsFromTopLevel(realDecl, !isDocumentHighlight);
            }
        }
        if (isDocumentHighlight) { return; }
    }

    // DocumentHighlight just find parentDecl
    if (isDocumentHighlight) {
        HandleRelatedFuncDeclsFromTopLevel(classLikeOrStructDecl, false);
        return;
    }
    std::vector<Ptr<InheritableDecl> > topClasses = GetTopClassDecl(*classLikeOrStructDecl);
    DealTopClass(topClasses);
}

void InheritDeclUtil::DealTopClass(std::vector<Ptr<InheritableDecl> > &topClasses)
{
    for (auto item: topClasses) {
        // realDecl has more info about subDecl
        auto realDecl = CompilerCodiraProject::GetInstance()->GetDeclInPkgByNode(item, editPkgPath);
        if (!realDecl) { continue; }
        // pre deal in topClass
        HandleRelatedFuncDeclsFromTopLevel(realDecl);
        // deal upDown subClass
        auto sortResult = CompilerCodiraProject::GetInstance()->GetIncTopologySort(realDecl->fullPackageName);
        for (auto &sortPkgName: sortResult) {
            pkgName = sortPkgName;
            auto users = GetAimSubDecl(sortPkgName, superDecls, editPkgPath);
            for (auto &user: users) {
                HandleRelatedFuncDeclsFromTopLevel(user);
            }
            if (!isRename) {
                GetRefInfoFromFuncDecl();
            } else {
                GetReNameInfoFromFuncDecl();
            }
            auto tempSuperDecls = superDecls;
            for (auto &superDecl: tempSuperDecls) {
                if (!superDecl.second) { (void) superDecls.erase(superDecl.first); }
            }
        }
    }
}

void InheritDeclUtil::GetRefInfoFromFuncDecl()
{
    for (auto &decl: funcDecls) {
        // add def to result
        if (IsFromSrcOrNoSrc(decl) && !IsZeroPosition(decl)) {
            // primary constructor has COMPILER_ADD attribute
            if (decl->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR)) {
                auto name = decl->outerDecl ? decl->outerDecl->identifier : decl->identifierForLsp;
                auto length = static_cast<int>(CountUnicodeCharacters(name));
                addDeclToRef(decl, length);
                // push the decl only if it's not add by the compiler
            } else if ((!decl->TestAttr(Codira::AST::Attribute::COMPILER_ADD) ||
                        decl->TestAttr(Codira::AST::Attribute::IS_CLONED_SOURCE_CODE) ||
                        decl->TestAttr(Codira::AST::Attribute::MACRO_FUNC))) {
                auto length = static_cast<int>(CountUnicodeCharacters(decl->identifier));
                addDeclToRef(decl, length);
            }
        }
        auto declPkgName = GetPkgNameFromNode(decl);
        // not in CiMAP and CIMapNotInSrc
        if (!IsFromCIMap(declPkgName) && !IsFromCIMapNotInSrc(declPkgName)) {
            auto ciMapList = CompilerCodiraProject::GetInstance()->GetCiMapList();
            // search users in CiMaP
            for (auto &item : ciMapList) {
                auto users = GetOnePkgUsers(*decl, item, editPkgPath);
                InsertRefUsers(users);
            }
            // search users in CIMapNotInSrc
            auto ciMapNotInSrcList = CompilerCodiraProject::GetInstance()->GetCIMapNotInSrcList();
            // search in CiMap
            for (auto &item: ciMapNotInSrcList) {
                auto users = GetOnePkgUsers(*decl, item, editPkgPath);
                InsertRefUsers(users);
            }
            continue;
        }

        // in CiMaP or NotInCiMap
        auto sortResult = CompilerCodiraProject::GetInstance()->GetIncTopologySort(declPkgName);
        for (const auto& item: sortResult) {
            if (!decl->curFile) { continue; }
            auto users = GetOnePkgUsers(*decl, item, decl->curFile->filePath, false, editPkgPath);
            InsertRefUsers(users);
        }
    }
    funcDecls = {};
}

void InheritDeclUtil::InsertRefUsers(std::unordered_set<Ptr<Codira::AST::Node> > &users)
{
    for (auto &user: users) {
        bool isInvalid = !user || !IsFromSrcOrNoSrc(user) || IsZeroPosition(user) ||
                         user->TestAttr(Attribute::COMPILER_ADD) &&
                         !user->TestAttr(Attribute::IS_CLONED_SOURCE_CODE);
        if (isInvalid) {
            continue;
        }
        const auto fileID = user->begin.fileID;
        std::string path = CompilerCodiraProject::GetInstance()->GetFilePathByID(*user, fileID);
        if (path.empty()) { continue; }
        ArkAST *arkAst = CompilerCodiraProject::GetInstance()->GetArkAST(path);
        const auto currentFileID = user->GetBegin().fileID;
        std::string currentFilePath =
                CompilerCodiraProject::GetInstance()->GetFilePathByID(*user, currentFileID);
        Range range;
        if (arkAst != nullptr) {
            range = GetRangeFromNode(user, arkAst->tokens);
        }
        Location location = {{URI::URIFromAbsolutePath(currentFilePath).ToString()},
                             TransformFromChar2IDE(range)};
        (void) References.insert(location);
    }
}

void InheritDeclUtil::GetReNameInfoFromFuncDecl()
{
    for (auto &decl: funcDecls) {
        // get definition
        if (decl->identifier != "init" && !IsZeroPosition(decl)) {
            unsigned int fileID = decl->identifier.Begin().fileID;
            int length = static_cast<int>(CountUnicodeCharacters(decl->identifier));
            // for primary constructor
            if (decl->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR)) {
                auto name = decl->outerDecl ? decl->outerDecl->identifier : decl->identifierForLsp;
                length = static_cast<int>(CountUnicodeCharacters(name));
            }
            Range range = GetDeclRange(*decl, length);
            std::string path = CompilerCodiraProject::GetInstance()->GetFilePathByID(decl->curFile->filePath, fileID);
            ArkAST *arkAst = CompilerCodiraProject::GetInstance()->GetArkAST(path);
            if (arkAst) {
                UpdateRange(arkAst->tokens, range, *decl);
            }
            TextEdit declEdit = {TransformFromChar2IDE(range), newName};
            auto result = defineEditMap.find(path);
            if (result == defineEditMap.end()) {
                defineEditMap[path] = std::set<TextEdit>{declEdit};
            } else {
                (void) result->second.insert(declEdit);
            }
        }
        // the fileID of defineEdit
        if (!decl->curFile) { continue; }
        auto definedPath = CompilerCodiraProject::GetInstance()->GetFilePathByID(
            decl->curFile->filePath, decl->identifier.Begin().fileID);
        // sort TextEdit by fileID
        auto DeclPkgName = GetPkgNameFromNode(decl);
        auto sortResult = CompilerCodiraProject::GetInstance()->GetIncTopologySort(DeclPkgName);
        for (const auto& item: sortResult) {
            if (!decl->curFile || decl->curFile->filePath.empty()) { continue; }
            auto users = GetOnePkgUsers(*decl, item, decl->curFile->filePath, true, editPkgPath);
            InsertRenameUsers(definedPath, users);
        }
    }
    funcDecls = {};
}

void InheritDeclUtil::InsertRenameUsers(const std::string &definedPath,
                                        std::unordered_set<Ptr<Codira::AST::Node> > &users)
{
    for (auto user: users) {
        if (!user || IsZeroPosition(user)) { continue; }
        if (user->TestAttr(Attribute::COMPILER_ADD) &&
            !user->TestAttr(Attribute::IS_CLONED_SOURCE_CODE)) {
            continue;
        }
        const auto fileID = user->begin.fileID;
        bool isInvalid = Codira::Is<RefExpr>(user) &&
                (dynamic_cast<RefExpr*>(user.get())->isThis || dynamic_cast<RefExpr*>(user.get())->isSuper);
        if (isInvalid) {
            continue;
        }
        std::string path = CompilerCodiraProject::GetInstance()->GetFilePathByID(*user, fileID);
        if (path.empty()) { continue; }
        ArkAST *arkAst = CompilerCodiraProject::GetInstance()->GetArkAST(path);
        if (!arkAst) { continue; }
        if (definedPath == path) {
            TextEdit textEdit{};
            textEdit.range = TransformFromChar2IDE(GetRangeFromNode(user, arkAst->tokens));
            textEdit.newText = newName;
            (void) defineEditMap[path].insert(textEdit);
            continue;
        }
        TextEdit textEdit{};
        textEdit.range = TransformFromChar2IDE(GetRangeFromNode(user, arkAst->tokens));
        textEdit.newText = newName;
        (void) usersEditMap[path].insert(textEdit);
    }
}

void InheritDeclUtil::addDeclToRef(Ptr<Decl> const &decl, int length)
{
    if (!decl->curFile || decl->curFile->filePath.empty()) { return; }
    Range range = GetDeclRange(*decl, length);
    auto fileID = range.start.fileID;
    auto instance = CompilerCodiraProject::GetInstance();
    std::string path = instance->GetFilePathByID(decl->curFile->filePath, fileID);
    if (path.empty()) { return; }
    ArkAST *arkAst = instance->GetArkAST(path);
    if (arkAst) {
        UpdateRange(arkAst->tokens, range, *decl);
    }
    Location location = {{URI::URIFromAbsolutePath(path).ToString()}, TransformFromChar2IDE(range)};
    (void) References.insert(location);
}
} // namespace ark
