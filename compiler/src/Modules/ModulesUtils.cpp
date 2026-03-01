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
 * This file implements utilities for modules.
 */

#include "Codira/Modules/ModulesUtils.h"

namespace Codira::Modules {
PackageRelation GetPackageRelation(const std::string& srcFullPkgName, const std::string& targetFullPkgName)
{
    auto pureSrcFullPackageName = ImportManager::IsTestPackage(srcFullPkgName)
        ? ImportManager::GetMainPartPkgNameForTestPkg(srcFullPkgName)
        : srcFullPkgName;
    auto pureTargetFullPackageName = ImportManager::IsTestPackage(targetFullPkgName)
        ? ImportManager::GetMainPartPkgNameForTestPkg(targetFullPkgName)
        : targetFullPkgName;
    if (pureSrcFullPackageName == pureTargetFullPackageName) {
        return PackageRelation::SAME_PACKAGE;
    }
    if (pureSrcFullPackageName == "" || pureTargetFullPackageName == "") {
        return PackageRelation::NONE;
    }

    auto srcPath = Utils::SplitQualifiedName(pureSrcFullPackageName);
    auto targetPath = Utils::SplitQualifiedName(pureTargetFullPackageName);
    if (targetPath.size() < srcPath.size() &&
        std::equal(targetPath.begin(), targetPath.end(), srcPath.begin(),
            srcPath.begin() + static_cast<ptrdiff_t>(targetPath.size()))) {
        return PackageRelation::CHILD;
    }

    return srcPath.front() == targetPath.front() ? PackageRelation::SAME_MODULE : PackageRelation::NONE;
}
} // namespace Codira::Modules
