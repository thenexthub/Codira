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

#include "libabckit/cpp/abckit_cpp.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

#include <optional>
#include <string_view>

namespace {

class ErrorHandler final : public abckit::IErrorHandler {
    void HandleError(abckit::Exception &&e) override
    {
        EXPECT_TRUE(false) << "Abckit exception raised: " << e.what();
    }
};

inline void EnumerateModuleFunctions(const abckit::core::Module &mod,
                                     const std::function<bool(abckit::core::Function)> &cb)
{
    // NOTE: currently we can only enumerate class methods and top level functions. need to update.
    mod.EnumerateTopLevelFunctions(cb);
    mod.EnumerateClasses([&](const abckit::core::Class &klass) -> bool {
        klass.EnumerateMethods(cb);
        return true;
    });
}

inline void EnumerateFunctionInsts(const abckit::core::Function &func,
                                   const std::function<bool(abckit::Instruction)> &cb)
{
    abckit::Graph graph = func.CreateGraph();
    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) {
        for (auto inst = bb.GetFirstInst(); inst; inst = inst.GetNext()) {
            cb(inst);
        }
        return true;
    });
}

struct ApiInfo {
    std::string source;
    std::string objName;
    std::string propName;
};

struct UsageInfo {
    std::string source;
    std::string funcName;
    size_t idx;
};

using ApiUsageMap = std::unordered_map<size_t, std::vector<UsageInfo>>;
using SuspectsMap = std::unordered_map<abckit::core::ImportDescriptor, size_t>;

std::string GetMethodName(const abckit::core::Function &method)
{
    auto methodName = method.GetName();
    auto cls = method.GetParentClass();
    if (cls) {
        auto className = cls.GetName();
        return className + '.' + methodName;
    };
    return methodName;
}

bool IsLoadApi(const abckit::core::ImportDescriptor &id, size_t apiIndex, const abckit::Instruction &inst,
               std::vector<ApiInfo> &apiList)
{
    // find the following pattern:
    // 1. ldExternalModuleVar (import {ApiNamespace} from "./modules/myApi")
    // 2. ldObjByName v1, "foo" (api.propName)
    // or
    // 1. ldExternalModuleVar (import {bar} from "./modules/myApi")

    if (inst.GetGraph()->DynIsa().GetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return false;
    }

    abckit::core::ImportDescriptor instId = inst.GetGraph()->DynIsa().GetImportDescriptor(inst);
    if (instId != id) {
        return false;
    }

    const auto &prop = apiList[apiIndex].propName;
    if (prop.empty()) {
        return true;
    }

    return !inst.VisitUsers([&](const abckit::Instruction &user) {
        if (user.GetGraph()->DynIsa().GetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME) {
            if (user.GetString() == prop) {
                return false;
            }
        }
        return true;
    });
}

void AddApiUsage(size_t apiIndex, const UsageInfo &usage, ApiUsageMap *apiUsages)
{
    auto iter = apiUsages->find(apiIndex);
    if (iter == apiUsages->end()) {
        std::vector<UsageInfo> usages;
        usages.emplace_back(usage);
        apiUsages->emplace(apiIndex, usages);
    } else {
        iter->second.emplace_back(usage);
    }
}

void CollectUsageInMethod(const abckit::core::Function &method, SuspectsMap &suspects, ApiUsageMap *apiUsages,
                          std::vector<ApiInfo> &apiList)
{
    auto methodName = GetMethodName(method);
    auto mod = method.GetModule();
    auto source = mod.GetName();
    UsageInfo usage = {source, methodName, 0};
    EnumerateFunctionInsts(method, [&](const abckit::Instruction &inst) {
        for (const auto &[id, apiIndex] : suspects) {
            if (IsLoadApi(id, apiIndex, inst, apiList)) {
                usage.idx = apiIndex;
                AddApiUsage(apiIndex, usage, apiUsages);
            }
        }
        return true;
    });
}

bool GetSuspects(const abckit::core::Module &mod, SuspectsMap *suspects, std::vector<ApiInfo> &apiList)
{
    mod.EnumerateImports([&](const abckit::core::ImportDescriptor &id) -> bool {
        auto importName = id.GetName();
        auto importedModule = id.GetImportedModule();
        auto source = importedModule.GetName();
        for (size_t i = 0; i < apiList.size(); ++i) {
            const auto api = apiList[i];
            if (source != api.source) {
                continue;
            }
            if (importName == api.objName) {
                suspects->emplace(id, i);
            }
        }
        return true;
    });
    return !suspects->empty();
}

void CollectApiUsages(ApiUsageMap *apiUsages, std::vector<ApiInfo> &apiList, abckit::File *file)
{
    file->EnumerateModules([&](const abckit::core::Module &mod) -> bool {
        SuspectsMap suspects = {};
        GetSuspects(mod, &suspects, apiList);
        if (suspects.empty()) {
            return true;
        }
        EnumerateModuleFunctions(mod, [&](const abckit::core::Function &method) -> bool {
            CollectUsageInMethod(method, suspects, apiUsages, apiList);
            return true;
        });
        return true;
    });
}

ApiUsageMap GetApiUsages(std::vector<ApiInfo> &apiList, abckit::File *file)
{
    auto apiUsages = ApiUsageMap({});
    CollectApiUsages(&apiUsages, apiList, file);
    return apiUsages;
}

std::string GetQuatatedStr(const std::string &str)
{
    const static std::string QUOTATION = "\"";
    return QUOTATION + str + QUOTATION;
}

std::string ApiUsageMapGetString(const ApiUsageMap &apiUsages, std::vector<ApiInfo> &apiList)
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

    ss << LEFT_BRACKET << std::endl;
    for (const auto &[index, usageInfos] : apiUsages) {
        ss << INDENT_1 << LEFT_BRACKET << std::endl;

        if (usageInfos.empty()) {
            continue;
        }
        const auto &apiInfo = apiList[index];
        ss << INDENT_2 << APIINFO << COLON << LEFT_BRACKET << std::endl;
        ss << INDENT_3 << APIINFO_SOURCE << COLON << GetQuatatedStr(apiInfo.source) << COMMA << std::endl;
        ss << INDENT_3 << APIINFO_OBJNAME << COLON << GetQuatatedStr(apiInfo.objName) << COMMA << std::endl;
        ss << INDENT_3 << APIINFO_PROPNAME << COLON << GetQuatatedStr(apiInfo.propName) << COMMA << std::endl;
        ss << INDENT_2 << RIGHT_BRACKET << COMMA << std::endl;

        ss << INDENT_2 << USAGES << COLON << LEFT_BRACKET << std::endl;
        for (const auto &usage : usageInfos) {
            ss << INDENT_3 << LEFT_BRACKET << std::endl;
            ss << INDENT_4 << USAGES_SOURCE << COLON << GetQuatatedStr(usage.source) << COMMA << std::endl;
            ss << INDENT_4 << USAGES_FUNCNAME << COLON << GetQuatatedStr(usage.funcName) << COMMA << std::endl;
            ss << INDENT_4 << API_IDX << COLON << usage.idx << COMMA << std::endl;
            ss << INDENT_3 << RIGHT_BRACKET << COMMA << std::endl;
        }
        ss << INDENT_2 << RIGHT_BRACKET << std::endl;

        ss << INDENT_1 << RIGHT_BRACKET << COMMA << std::endl;
    }
    ss << RIGHT_BRACKET << std::endl;

    return ss.str();
}

bool IsUsagesEqual(const std::vector<UsageInfo> &otherUsages, const ApiUsageMap &apiUsages)
{
    for (auto &otherUsage : otherUsages) {
        auto usages = apiUsages.at(otherUsage.idx);
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

}  // namespace

namespace libabckit::test {

class AbckitScenarioCppTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestDynamicApiScannerClean)
{
    // CC-OFFNXT(G.NAM.03) project code style

    const std::string testSandboxPath = ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/api_scanner/";
    const std::string inputAbcPath = testSandboxPath + "api_scanner.abc";

    abckit::File file(inputAbcPath, std::make_unique<ErrorHandler>());

    std::vector<ApiInfo> apiList = {{"modules/apiNamespace", "ApiNamespace", "foo"},
                                    {"modules/toplevelApi", "bar", ""},
                                    {"@ohos.geolocation", "geolocation", "getCurrentLocation"}};

    const ApiUsageMap usages = GetApiUsages(apiList, &file);
    EXPECT_FALSE(usages.empty());

    auto usagesStr = ApiUsageMapGetString(usages, apiList);
    std::vector<UsageInfo> expectedUsages = {
        {"api_scanner", "useAll", 0},
        {"api_scanner", "useFoo", 0},
        {"api_scanner", "Person.personUseAll", 0},
        {"api_scanner", "Person.personUseFoo", 0},
        {"modules/src2", "Animal.animalUseFoo", 0},
        {"modules/src2", "Animal.animalUseAll", 0},
        {"modules/src2", "src2UseAll", 0},
        {"modules/src2", "src2UseFoo", 0},
        {"api_scanner", "useAll", 1},
        {"api_scanner", "useBar", 1},
        {"api_scanner", "Person.personUseAll", 1},
        {"api_scanner", "Person.personUseBar", 1},
        {"api_scanner", "useAll", 2},
        {"api_scanner", "useLocation", 2},
        {"api_scanner", "Person.personUseLocation", 2},
        {"modules/src3", "src3UseLocation", 2},
        {"modules/src3", "Cat.catUseLocation", 2},
    };
    EXPECT_TRUE(IsUsagesEqual(expectedUsages, usages));
    LIBABCKIT_LOG(DEBUG) << "====================================\n";
    LIBABCKIT_LOG(DEBUG) << usagesStr;
}

}  // namespace libabckit::test
