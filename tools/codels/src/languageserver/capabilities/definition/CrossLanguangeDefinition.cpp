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

#include "CrossLanguangeDefinition.h"

bool ark::CrossLanguangeDefinition::getCrossSymbols(const CrossLanguageJumpParams &params, CrossSymbolsResult &result)
{
    const auto index = CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return false;
    }
    std::string outerName = params.outerName;
    index->FindCrossSymbolByName(params.packageName, params.name, params.isCombined,
        [outerName, &result](const lsp::CrossSymbol &crs) {
        if (crs.location.IsZeroLoc()) {
            return;
        }
        const auto realPath = crs.location.fileUri;
        if (EndsWith(realPath, ".macrocall")) {
            return;
        }
        if (!outerName.empty() && outerName != crs.containerName) {
            return;
        }
        Location loc{URI::URIFromAbsolutePath(realPath).ToString(),
                     TransformFromChar2IDE({crs.location.begin, crs.location.end})};
        result.locations.emplace(loc);
    });
    return true;
}

bool ark::CrossLanguangeDefinition::GetExportSID(IDArray id, ExportIDItem &exportIdItem)
{
    const auto index = CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return false;
    }
    index->GetExportSID(id, [&exportIdItem](const lsp::CrossSymbol &crs) {
        exportIdItem.exportName = crs.name;
        exportIdItem.containerName = crs.containerName;
    });
    return true;
}

bool ark::CrossLanguangeDefinition::getRegisterCrossSymbols(
        const CrossLanguageJumpParams &params, RegisterCrossSymbolsResult &result)
{
    const auto index = CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return false;
    }
    std::string outerName = params.outerName;
    index->FindCrossSymbolByName(
            params.packageName, params.name, params.isCombined, [outerName, &result](const lsp::CrossSymbol &crs) {
                if (crs.location.IsZeroLoc()) {
                    return;
                }
                const auto realPath = crs.location.fileUri;
                if (EndsWith(realPath, ".macrocall")) {
                    return;
                }
                if (!outerName.empty() && outerName != crs.containerName) {
                    return;
                }
                Location definitionLoc{URI::URIFromAbsolutePath(realPath).ToString(),
                                       TransformFromChar2IDE({crs.location.begin, crs.location.end})};
                Location declarationLoc{URI::URIFromAbsolutePath(realPath).ToString(),
                                        TransformFromChar2IDE({crs.declaration.begin, crs.declaration.end})};
                std::string registerName = crs.name;
                int registerType = static_cast<int>(crs.crossType);
                RegisterItem item{definitionLoc, declarationLoc, registerName, registerType};
                result.registerItems.push_back(item);
            });
    return true;
}
