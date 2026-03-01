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
 * This file declares the Utility functions for Modules.
 */

#ifndef CODIRA_MODULES_UTILS_H
#define CODIRA_MODULES_UTILS_H

#include "Codira/AST/Node.h"
#include "Codira/AST/Utils.h"
#include "Codira/Basic/Utils.h"
#include "Codira/Modules/ImportManager.h"

namespace Codira::Modules {
template <typename T> bool IsAllFuncDecl(T& results)
{
    for (auto it : results) {
        if (it && it->astKind != AST::ASTKind::FUNC_DECL && it->astKind != AST::ASTKind::MACRO_DECL) {
            return false;
        }
    }
    return true;
}

enum class PackageRelation { NONE, CHILD, SAME_MODULE, SAME_PACKAGE };
/**
 * Get what relation 'targetFullPackageName' is to 'srcFullPackageName'.
 * Eg: 1. srcFullPackageName: a, targetFullPackageName: b, return NONE.
 *     2. srcFullPackageName: a, targetFullPackageName: a.b, return CHILD.
 *     3. srcFullPackageName: a.b, targetFullPackageName: a.c, return SAME_MODULE.
 *     4. srcFullPackageName: a.b, targetFullPackageName: a.b, return SAME_PACKAGE.
 * Note: If one package is a testcase, we'll remove its '$test' suffix first.
 */
PackageRelation GetPackageRelation(const std::string& srcFullPkgName, const std::string& targetFullPkgName);

/**
 * Return true if 'srcFullPackageName' is direct parent package of 'targetFullPackageName'.
 * Eg: 1. srcFullPackageName: a, targetFullPackageName: a.b, return true.
 *     2. srcFullPackageName: a, targetFullPackageName: a.b.c, return false.
 */
inline bool IsSuperPackage(const std::string& srcFullPackageName, const std::string& targetFullPackageName)
{
    auto pureSrcFullPackageName = ImportManager::IsTestPackage(srcFullPackageName) ?
        ImportManager::GetMainPartPkgNameForTestPkg(srcFullPackageName) : srcFullPackageName;
    auto pureTargetFullPackageName = ImportManager::IsTestPackage(targetFullPackageName) ?
        ImportManager::GetMainPartPkgNameForTestPkg(targetFullPackageName) : targetFullPackageName;
    if (pureTargetFullPackageName.rfind(pureSrcFullPackageName, 0) == 0) {
        // don't split org name, make it part of root package name.
        auto srcNames = Utils::SplitQualifiedName(pureSrcFullPackageName);
        auto targetNames = Utils::SplitQualifiedName(pureTargetFullPackageName);
        return srcNames.size() + 1 == targetNames.size();
    }
    return false;
}

inline std::string RelationToString(PackageRelation relation)
{
    return relation == PackageRelation::NONE       ? "irrelevant"
        : relation == PackageRelation::CHILD       ? "child"
        : relation == PackageRelation::SAME_MODULE ? "same module"
                                                   : "same package";
}

inline bool IsVisible(const AST::Node& node, PackageRelation relation)
{
    return node.TestAnyAttr(AST::Attribute::PUBLIC) || relation == PackageRelation::SAME_PACKAGE ||
        (node.TestAttr(AST::Attribute::PROTECTED) && relation != PackageRelation::NONE) ||
        (node.TestAttr(AST::Attribute::INTERNAL) &&
            (relation == PackageRelation::CHILD || relation == PackageRelation::SAME_PACKAGE));
}

inline void AddImportedDeclToMap(
    const AST::OrderedDeclSet& decls, AST::OrderedDeclSet& targetSet, AST::AccessLevel importLevel)
{
    for (auto decl : decls) {
        if (IsCompatibleAccessLevel(importLevel, GetAccessLevel(*decl))) {
            targetSet.emplace(decl);
        }
    }
}

inline AST::OrderedDeclSet GetVisibleDeclToMap(
    const AST::OrderedDeclSet& decls, AST::AccessLevel importLevel, PackageRelation relation)
{
    AST::OrderedDeclSet ret;
    std::copy_if(decls.cbegin(), decls.cend(), std::inserter(ret, ret.end()), [importLevel, relation](auto decl) {
        return IsCompatibleAccessLevel(importLevel, GetAccessLevel(*decl)) && IsVisible(*decl, relation);
    });
    return ret;
}

const std::string NO_CODEO_HELP_INFO =
    "check if the .codeo file of the package exists in CODIRA_PATH or CODIRA_HOME, or use "
    "'--import-path' to specify the .codeo file path";
} // namespace Codira::Modules

#endif
