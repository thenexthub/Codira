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

#include <algorithm>
#include <cassert>
#include <sstream>
#include "libabckit/c/abckit.h"
#include "api_scanner.h"
#include "helpers/helpers.h"

ApiScanner::ApiScanner(enum AbckitApiVersion version, AbckitFile *file, std::vector<ApiInfo> apiList)
    : file_(file), apiList_(std::move(apiList))
{
    impl_ = AbckitGetApiImpl(version);
    implI_ = AbckitGetInspectApiImpl(version);
    implG_ = AbckitGetGraphApiImpl(version);
    dynG_ = AbckitGetIsaApiDynamicImpl(version);
    vh_ = VisitHelper(file_, impl_, implI_, implG_, dynG_);
}

void ApiScanner::CollectApiUsages()
{
    vh_.EnumerateModules([&](AbckitCoreModule *mod) {
        SuspectsMap suspects;
        GetSuspects(mod, suspects);
        if (suspects.empty()) {
            return;
        }
        vh_.EnumerateModuleFunctions(mod, [&](AbckitCoreFunction *method) { CollectUsageInMethod(method, suspects); });
    });
}

bool ApiScanner::GetSuspects(AbckitCoreModule *mod, SuspectsMap &suspects)
{
    vh_.EnumerateModuleImports(mod, [&](AbckitCoreImportDescriptor *id) {
        auto importName = vh_.GetString(implI_->importDescriptorGetName(id));
        auto *importedModule = implI_->importDescriptorGetImportedModule(id);
        auto source = vh_.GetString(implI_->moduleGetName(importedModule));
        for (size_t i = 0; i < apiList_.size(); ++i) {
            const auto api = apiList_[i];
            if (source != api.source) {
                continue;
            }
            if (importName == api.objName) {
                suspects.emplace(id, i);
            }
        }
    });
    return !suspects.empty();
}

bool ApiScanner::IsLoadApi(AbckitCoreImportDescriptor *id, size_t apiIndex, AbckitInst *inst)
{
    // find the following pattern:
    // 1. ldExternalModuleVar (import {ApiNamespace} from "./modules/myApi")
    // 2. ldObjByName v1, "foo" (api.propName)
    // or
    // 1. ldExternalModuleVar (import {bar} from "./modules/myApi")

    if (dynG_->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return false;
    }

    if (dynG_->iGetImportDescriptor(inst) != id) {
        return false;
    }

    const auto &prop = apiList_[apiIndex].propName;
    if (prop.empty()) {
        return true;
    }

    bool found = false;
    vh_.EnumerateInstUsers(inst, [&](AbckitInst *user) {
        if (!found && dynG_->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME) {
            auto str = vh_.GetString(implG_->iGetString(user));
            found = str == prop;
        }
    });

    return found;
}

std::string ApiScanner::GetMethodName(AbckitCoreFunction *method)
{
    auto mname = implI_->functionGetName(method);
    auto methodName = std::string(libabckit::test::helpers::AbckitStringToString(mname));
    auto *cls = implI_->functionGetParentClass(method);
    if (cls == nullptr) {
        return methodName;
    }
    auto cname = implI_->classGetName(cls);
    auto className = std::string(libabckit::test::helpers::AbckitStringToString(cname));
    return className + '.' + methodName;
}

void ApiScanner::CollectUsageInMethod(AbckitCoreFunction *method, SuspectsMap &suspects)
{
    auto methodName = GetMethodName(method);
    auto *sourceRaw = implI_->moduleGetName(implI_->functionGetModule(method));
    auto source = vh_.GetString(sourceRaw);
    UsageInfo usage = {source.data(), methodName, 0};
    vh_.EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        for (const auto &[id, apiIndex] : suspects) {
            if (IsLoadApi(id, apiIndex, inst)) {
                usage.idx = apiIndex;
                AddApiUsage(apiIndex, usage);
            }
        }
    });
}

static std::string GetQuatatedStr(const std::string &str)
{
    const static std::string QUOTATION = "\"";
    return QUOTATION + str + QUOTATION;
}

std::string ApiScanner::ApiUsageMapGetString() const
{
    std::stringstream ss;
    const static std::string INDENT_1 = "  ";
    const static std::string INDENT_2 = INDENT_1 + INDENT_1;
    const static std::string INDENT_3 = INDENT_1 + INDENT_2;
    const static std::string INDENT_4 = INDENT_1 + INDENT_3;
    const static std::string LEFT_BRACKET = "{";
    const static std::string RIGHT_BRACKET = "}";
    const static std::string COLON = ": ";
    const static std::string COMMA = ",";
    const static std::string APIINFO = "APIInfo";
    const static std::string APIINFO_SOURCE = "source";
    const static std::string APIINFO_OBJNAME = "objName";
    const static std::string APIINFO_PROPNAME = "propName";
    const static std::string USAGES = "Usages";
    const static std::string USAGES_SOURCE = "source";
    const static std::string USAGES_FUNCNAME = "funcName";
    const static std::string API_IDX = "indexOfAPI";

    LIBABCKIT_LOG(DEBUG) << LEFT_BRACKET << std::endl;
    for (const auto &[index, usageInfos] : apiUsages_) {
        LIBABCKIT_LOG(DEBUG) << INDENT_1 << LEFT_BRACKET << std::endl;

        if (usageInfos.empty()) {
            continue;
        }
        const auto &apiInfo = apiList_[index];
        LIBABCKIT_LOG(DEBUG) << INDENT_2 << APIINFO << COLON << LEFT_BRACKET << std::endl;
        LIBABCKIT_LOG(DEBUG) << INDENT_3 << APIINFO_SOURCE << COLON << GetQuatatedStr(apiInfo.source) << COMMA
                             << std::endl;
        LIBABCKIT_LOG(DEBUG) << INDENT_3 << APIINFO_OBJNAME << COLON << GetQuatatedStr(apiInfo.objName) << COMMA
                             << std::endl;
        LIBABCKIT_LOG(DEBUG) << INDENT_3 << APIINFO_PROPNAME << COLON << GetQuatatedStr(apiInfo.propName) << COMMA
                             << std::endl;
        LIBABCKIT_LOG(DEBUG) << INDENT_2 << RIGHT_BRACKET << COMMA << std::endl;

        LIBABCKIT_LOG(DEBUG) << INDENT_2 << USAGES << COLON << LEFT_BRACKET << std::endl;
        for (const auto &usage : usageInfos) {
            LIBABCKIT_LOG(DEBUG) << INDENT_3 << LEFT_BRACKET << std::endl;
            LIBABCKIT_LOG(DEBUG) << INDENT_4 << USAGES_SOURCE << COLON << GetQuatatedStr(usage.source) << COMMA
                                 << std::endl;
            LIBABCKIT_LOG(DEBUG) << INDENT_4 << USAGES_FUNCNAME << COLON << GetQuatatedStr(usage.funcName) << COMMA
                                 << std::endl;
            LIBABCKIT_LOG(DEBUG) << INDENT_4 << API_IDX << COLON << usage.idx << COMMA << std::endl;
            LIBABCKIT_LOG(DEBUG) << INDENT_3 << RIGHT_BRACKET << COMMA << std::endl;
        }
        LIBABCKIT_LOG(DEBUG) << INDENT_2 << RIGHT_BRACKET << std::endl;

        LIBABCKIT_LOG(DEBUG) << INDENT_1 << RIGHT_BRACKET << COMMA << std::endl;
    }
    LIBABCKIT_LOG(DEBUG) << RIGHT_BRACKET << std::endl;

    return ss.str();
}

bool ApiScanner::IsUsagesEqual(const std::vector<UsageInfo> &otherUsages) const
{
    for (auto &otherUsage : otherUsages) {
        auto usages = apiUsages_.at(otherUsage.idx);
        auto iter = std::find_if(usages.begin(), usages.end(), [&otherUsage](const UsageInfo &usage) {
            return (otherUsage.funcName == usage.funcName) && (otherUsage.source == usage.source) &&
                   (otherUsage.idx == usage.idx);
        });
        if (iter == usages.end()) {
            return false;
        }
    }
    return true;
}
