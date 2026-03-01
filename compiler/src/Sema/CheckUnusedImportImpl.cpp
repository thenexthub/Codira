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
 * This file defines functions for check unused import.
 */

#include "TypeCheckerImpl.h"
#include "Codira/Utils/Utils.h"

using namespace Codira;
using namespace AST;

namespace {
using UsedPackageMap = std::unordered_map<std::string, std::unordered_set<std::pair<std::string, Ptr<Decl>>, HashPair>>;

class CheckUnusedImportImpl {
public:
    CheckUnusedImportImpl(Package& pkg, DiagnosticEngine& diag, ImportManager& importManager)
        : pkg(pkg), diag(diag), importManager(importManager)
    {
    }

    ~CheckUnusedImportImpl() = default;
    void Check();

private:
    void CollectUsedPackages(Node& node);
    void CollectNeedCheckImports();
    bool IsImportContentUsed(AST::ImportSpec& importSpec);
    bool IsImportContentUsedInMacro(AST::ImportSpec& importSpec);
    void AddUsedExtendDeclTarget(const Ptr<AST::ExtendDecl> ed, UsedPackageMap& usedMap) const;
    void AddUsedTargetWithIdentifier(Node& node, const std::string& identifier, Ptr<Decl> target);
    void AddUsedTarget(Node& node, Ptr<Decl> target);
    void AddUsedPackage(Node& node);
    void ReportUnusedImports();
    UsedPackageMap usedPackageInAST;
    std::unordered_map<Ptr<File>, UsedPackageMap> usedPackageInFile;
    UsedPackageMap cacheUsedPackageInAST;
    std::unordered_map<Ptr<File>, UsedPackageMap> cacheUsedPackageInFile;
    std::vector<Ptr<ImportSpec>> needCheckImport;
    Package& pkg;
    DiagnosticEngine& diag;
    ImportManager& importManager;
};

std::string GetRefIdentifier(Node& node)
{
    if (node.astKind == ASTKind::REF_TYPE) {
        return StaticCast<RefType>(&node)->ref.identifier.Val();
    }
    if (node.astKind == ASTKind::REF_EXPR) {
        return StaticCast<RefExpr>(&node)->ref.identifier.Val();
    }
    return "";
}
} // namespace

void CheckUnusedImportImpl::AddUsedExtendDeclTarget(
    const Ptr<AST::ExtendDecl> ed, UsedPackageMap& usedMap) const
{
    Ptr<Decl> target = nullptr;
    for (auto& type : ed->inheritedTypes) {
        if (target = type->GetTarget(); target) {
            usedMap[target->GetFullPackageName()].emplace(std::make_pair(target->identifier.Val(), target));
        }
    }
    if (ed->extendedType && ed->extendedType->GetTarget()) {
        target = ed->extendedType->GetTarget();
        usedMap[target->GetFullPackageName()].emplace(std::make_pair(target->identifier.Val(), target));
    }

    if (!ed->generic) {
        return;
    }

    for (auto& gc : ed->generic->genericConstraints) {
        for (auto& ub : gc->upperBounds) {
            if (target = ub->GetTarget(); target) {
                usedMap[target->GetFullPackageName()].emplace(std::make_pair(target->identifier.Val(), target));
            }
        }
    }
}

void CheckUnusedImportImpl::AddUsedTargetWithIdentifier(Node& node, const std::string& identifier, Ptr<Decl> target)
{
    auto& foundInAST = usedPackageInAST[target->fullPackageName];
    foundInAST.emplace(std::make_pair(identifier, target));
    if (target->outerDecl != nullptr) {
        foundInAST.emplace(std::make_pair(target->outerDecl->identifier.Val(), target->outerDecl));
        if (auto ed = DynamicCast<ExtendDecl>(target->outerDecl); ed != nullptr) {
            AddUsedExtendDeclTarget(ed, usedPackageInAST);
        }
    }

    if (!node.curFile) {
        return;
    }

    auto& fileUsed = usedPackageInFile[node.curFile];
    auto& foundInFile = fileUsed[target->fullPackageName];

    foundInFile.emplace(std::make_pair(identifier, target));
    if (target->outerDecl != nullptr) {
        foundInFile.emplace(std::make_pair(target->outerDecl->identifier.Val(), target->outerDecl));
        if (auto ed = DynamicCast<ExtendDecl>(target->outerDecl); ed != nullptr) {
            AddUsedExtendDeclTarget(ed, fileUsed);
        }
    }
}

void CheckUnusedImportImpl::AddUsedTarget(Node& node, Ptr<Decl> target)
{
    const std::string& identifier = GetRefIdentifier(node);
    if (!identifier.empty()) {
        AddUsedTargetWithIdentifier(node, identifier, target);
    } else {
        AddUsedTargetWithIdentifier(node, target->identifier.Val(), target);
    }
}

void CheckUnusedImportImpl::AddUsedPackage(Node& node)
{
    auto target = node.GetTarget();
    if (target == nullptr) {
        return;
    }

    AddUsedTarget(node, target);

    auto targets = node.GetTargets();
    for (auto decl : targets) {
        AddUsedTarget(node, decl);
    }
}

void CheckUnusedImportImpl::CollectNeedCheckImports()
{
    for (auto& file : pkg.files) {
        for (auto& import : file->imports) {
            // IMPLICIT_ADD(std.core) or codemp scenes does not need to be checked.
            if (import->TestAttr(AST::Attribute::IMPLICIT_ADD) || import->begin.IsZero() || import->end.IsZero()) {
                continue;
            }
            // multi-import or reExport scenes does not need to be checked.
            auto modifier = import->modifier ? import->modifier->modifier : TokenKind::PRIVATE;
            if (modifier == TokenKind::PUBLIC || modifier == TokenKind::PROTECTED || import->IsImportMulti()) {
                continue;
            }
            bool isInternalNeedExport =
                import->curFile && import->curFile->curPackage && !import->curFile->curPackage->noSubPkg;
            if (modifier == TokenKind::INTERNAL && isInternalNeedExport) {
                continue;
            }
            needCheckImport.emplace_back(import.get());
        }
    }
}

void CheckUnusedImportImpl::CollectUsedPackages(Node& node)
{
    Walker walker(&node, nullptr, [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::IMPORT_SPEC) {
            return VisitAction::SKIP_CHILDREN;
        }
        AddUsedPackage(*node);
        return VisitAction::WALK_CHILDREN;
    });

    walker.Walk();
}

bool CheckUnusedImportImpl::IsImportContentUsedInMacro(AST::ImportSpec& importSpec)
{
    CODEC_ASSERT(importSpec.curFile);
    auto codeoManager = importManager.GetCodeoManager();
    const std::string& packageName = codeoManager->GetPackageNameByImport(importSpec);

    auto usedMacroInFile = importManager.GetUsedMacroDecls(*importSpec.curFile);
    auto declsMap = codeoManager->GetPackageMembers(packageName);
    if (importSpec.IsImportAll() || !importSpec.content.isDecl) {
        if (!usedMacroInFile[packageName].empty()) {
            return true;
        }
        for (auto [_, decls] : declsMap) {
            for (auto decl : decls) {
                if (usedMacroInFile[decl->fullPackageName].count(decl) > 0) {
                    cacheUsedPackageInFile[importSpec.curFile][packageName].emplace(
                        std::make_pair(decl->identifier.Val(), decl));
                    return true;
                }
            }
        }
    } else {
        auto decls = declsMap[importSpec.content.identifier];
        for (auto decl : decls) {
            if (usedMacroInFile[decl->fullPackageName].count(decl) > 0) {
                cacheUsedPackageInFile[importSpec.curFile][packageName].emplace(
                    std::make_pair(decl->identifier.Val(), decl));
                return true;
            }
        }
    }

    return false;
}

bool CheckUnusedImportImpl::IsImportContentUsed(ImportSpec& importSpec)
{
    auto codeoManager = importManager.GetCodeoManager();
    const std::string& packageName = codeoManager->GetPackageNameByImport(importSpec);

    UsedPackageMap& usedPackage =
        (importSpec.IsPrivateImport() && importSpec.curFile) ? usedPackageInFile[importSpec.curFile] : usedPackageInAST;

    const auto& usedDecls = usedPackage[packageName];
    if (!usedDecls.empty() && importSpec.IsImportAll()) {
        return true;
    }

    UsedPackageMap& cacheUsedPackage =
        (importSpec.IsPrivateImport() && importSpec.curFile) ? cacheUsedPackageInFile[importSpec.curFile]
                                                             : cacheUsedPackageInAST;

    auto declsMap = codeoManager->GetPackageMembers(packageName);
    if (importSpec.IsImportAll()) {
        if (!cacheUsedPackage[packageName].empty()) {
            return true;
        }
        for (auto [_, decls] : declsMap) {
            for (auto decl : decls) {
                const auto& usedDeclsInPkg = usedPackage[decl->fullPackageName];
                if (usedDeclsInPkg.find(std::make_pair(decl->identifier.Val(), decl)) != usedDeclsInPkg.end()) {
                    cacheUsedPackageInAST[packageName].emplace(std::make_pair(decl->identifier.Val(), decl));
                    cacheUsedPackageInFile[importSpec.curFile][packageName].emplace(
                        std::make_pair(decl->identifier.Val(), decl));
                    return true;
                }
            }
        }
    } else if (importSpec.content.isDecl) {
        const std::string& identifier =
            importSpec.IsImportAlias() ? importSpec.content.aliasName.Val() : importSpec.content.identifier.Val();
        const auto& decls = declsMap[importSpec.content.identifier];
        for (auto decl : decls) {
            if (cacheUsedPackage[packageName].find(std::make_pair(identifier, decl)) !=
                cacheUsedPackage[packageName].end()) {
                return true;
            }
            const auto& usedDeclsInPkg = usedPackage[decl->fullPackageName];
            if (usedDeclsInPkg.find(std::make_pair(identifier, decl)) != usedDeclsInPkg.end()) {
                cacheUsedPackageInAST[packageName].emplace(std::make_pair(identifier, decl));
                cacheUsedPackageInFile[importSpec.curFile][packageName].emplace(std::make_pair(identifier, decl));
                return true;
            }
        }
    } else {
        const std::string& identifier =
            importSpec.IsImportAlias() ? importSpec.content.aliasName.Val() : importSpec.content.identifier.Val();
        auto packageDecl = importManager.GetPackageDecl(packageName);
        if (usedDecls.find(std::make_pair(identifier, packageDecl)) != usedDecls.end()) {
            return true;
        }
    }

    auto package = codeoManager->GetPackage(packageName);
    if (package && package->isMacroPackage) {
        return IsImportContentUsedInMacro(importSpec);
    }
    return false;
}

void CheckUnusedImportImpl::ReportUnusedImports()
{
    for (auto& importSpec : needCheckImport) {
        if (importSpec->TestAttr(Attribute::IMPORTED) && importSpec->TestAttr(Attribute::FROM_COMMON_PART)) {
            continue;
        }
        if (IsImportContentUsed(*importSpec)) {
            continue;
        }
        diag.DiagnoseRefactor(DiagKindRefactor::sema_unused_import, MakeRange(importSpec->begin, importSpec->end),
            importSpec->content.ToString());
    }
}

void CheckUnusedImportImpl::Check()
{
    CollectUsedPackages(pkg);
    CollectNeedCheckImports();
    ReportUnusedImports();
}

void TypeChecker::TypeCheckerImpl::CheckUnusedImportSpec(Package& pkg)
{
    CheckUnusedImportImpl(pkg, diag, importManager).Check();
}
