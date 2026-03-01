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

#include "LocateDefinition4Import.h"

namespace {
using namespace ark;
std::string GetPackagePrefixWithPaths(const std::vector<std::string> &prefixPaths)
{
    std::stringstream ss;
    for (const auto &prefix : prefixPaths) {
        ss << prefix << ".";
    }
    return ss.str().substr(0, ss.str().size() - 1);
};

Ptr<Decl> ProcSingleImport(
    const ArkAST &ast, File *fileNode,
    ImportContent &importContent,
    std::string packagePrefix = "")
{
    const auto &importManager = ast.packageInstance->importManager;
    const auto srcPkgName = fileNode->curPackage->fullPackageName;
    if (packagePrefix.empty()) {
        packagePrefix = importContent.GetPrefixPath();
    }

    if (const auto targetPkg = importManager.GetPackageDecl(packagePrefix)) {
        const auto members = importManager.GetPackageMembers(srcPkgName, targetPkg->fullPackageName);
        for (const auto &memberDecl : members) {
            if (const auto declPtr = dynamic_cast<const Decl *>(memberDecl.get());
                declPtr == nullptr || declPtr->identifier != importContent.identifier)
                continue;
            return memberDecl;
        }
    }
    return Ptr<Decl>();
};
}

namespace ark {
Ptr<Decl> LocateDefinition4Import::getDecl(const ArkAST &ast, Position pos, File *fileNode)
{
    for (auto &fileImport : fileNode->imports) {
        if (fileImport.get()->IsImportMulti()) {
            continue;
        }
        auto importContent = fileImport.get()->content;
        if (fileImport.get()->IsImportSingle() &&
            pos <= importContent.identifier.End() &&
            pos >= importContent.identifier.Begin()) {
            return ProcSingleImport(ast, fileNode, importContent);
        }
        if (fileImport.get()->IsImportAlias() &&
            (pos <= importContent.identifier.End() && pos >= importContent.identifier.Begin() ||
             pos <= importContent.aliasName.End() && pos >= importContent.aliasName.Begin())) {
            return ProcSingleImport(ast, fileNode, importContent);
        }
    }
    return Ptr<Decl>();
}

void LocateDefinition4Import::getImportDecl(const std::vector<Symbol *> &syms,
                                                  const ArkAST &ast,
                                                  Position pos,
                                                  Ptr<Decl> &decl)
{
    File *fileNode;
    for (const auto sym : syms) {
        if (sym && sym->astKind == ASTKind::FILE) {
            if ((fileNode = dynamic_cast<File *>(sym->node.get())) != nullptr) {
                decl = getDecl(ast, pos, fileNode);
                break;
            }
        }
    }
}
}
