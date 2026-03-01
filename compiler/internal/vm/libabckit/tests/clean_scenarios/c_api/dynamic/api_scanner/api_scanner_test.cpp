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

#include <gtest/gtest.h>

#include "libabckit/src/logger.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/isa/isa_dynamic.h"

namespace {

constexpr AbckitApiVersion VERSION = ABCKIT_VERSION_RELEASE_1_0_0;
auto *g_impl = AbckitGetApiImpl(VERSION);
const AbckitInspectApi *g_implI = AbckitGetInspectApiImpl(VERSION);
const AbckitGraphApi *g_implG = AbckitGetGraphApiImpl(VERSION);
const AbckitIsaApiDynamic *g_dynG = AbckitGetIsaApiDynamicImpl(VERSION);
const AbckitModifyApi *g_implM = AbckitGetModifyApiImpl(VERSION);

template <class FunctionCallBack>
inline void EnumerateModuleTopLevelFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->moduleEnumerateTopLevelFunctions(mod, (void *)(&cb), [](AbckitCoreFunction *method, void *data) {
        const auto &cb = *((FunctionCallBack *)data);
        cb(method);
        return true;
    });
}

template <class ClassCallBack>
inline void EnumerateModuleClasses(AbckitCoreModule *mod, const ClassCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->moduleEnumerateClasses(mod, (void *)(&cb), [](AbckitCoreClass *klass, void *data) {
        const auto &cb = *((ClassCallBack *)data);
        cb(klass);
        return true;
    });
}

template <class MethodCallBack>
inline void EnumerateClassMethods(AbckitCoreClass *klass, const MethodCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;
    g_implI->classEnumerateMethods(klass, (void *)(&cb), [](AbckitCoreFunction *method, void *data) {
        const auto &cb = *((MethodCallBack *)data);
        cb(method);
        return true;
    });
}

template <class FunctionCallBack>
inline void EnumerateModuleFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;
    // NOTE: currently we can only enumerate class methods and top level functions. need to update.
    EnumerateModuleTopLevelFunctions(mod, cb);
    EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) { EnumerateClassMethods(klass, cb); });
}

template <class UserCallBack>
inline void EnumerateInstUsers(AbckitInst *inst, const UserCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implG->iVisitUsers(inst, (void *)(&cb), [](AbckitInst *user, void *data) {
        const auto &cb = *((UserCallBack *)data);
        cb(user);
        return true;
    });
}

struct CapturedData {
    void *callback = nullptr;
    const AbckitGraphApi *gImplG = nullptr;
};

template <class InstCallBack>
inline bool VisitBlock(AbckitBasicBlock *bb, void *data)
{
    auto *captured = reinterpret_cast<CapturedData *>(data);
    const auto &cb = *reinterpret_cast<InstCallBack *>(captured->callback);
    auto *implG = captured->gImplG;
    for (auto *inst = implG->bbGetFirstInst(bb); inst != nullptr; inst = implG->iGetNext(inst)) {
        cb(inst);
    }
    return true;
}

template <class InstCallBack>
inline void EnumerateGraphInsts(AbckitGraph *graph, const InstCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    CapturedData captured {(void *)(&cb), g_implG};
    g_implG->gVisitBlocksRpo(graph, &captured,
                             [](AbckitBasicBlock *bb, void *data) { return VisitBlock<InstCallBack>(bb, data); });
}

template <class InstCallBack>
inline void EnumerateFunctionInsts(AbckitCoreFunction *func, const InstCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    AbckitGraph *graph = g_implI->createGraphFromFunction(func);
    EnumerateGraphInsts(graph, cb);
    g_impl->destroyGraph(graph);
}

template <class ImportCallBack>
inline void EnumerateModuleImports(AbckitCoreModule *mod, const ImportCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->moduleEnumerateImports(mod, (void *)(&cb), [](AbckitCoreImportDescriptor *i, void *data) {
        const auto &cb = *((ImportCallBack *)(data));
        cb(i);
        return true;
    });
}

template <class ModuleCallBack>
inline void EnumerateModules(const ModuleCallBack &cb, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->fileEnumerateModules(file, (void *)(&cb), [](AbckitCoreModule *mod, void *data) {
        const auto &cb = *((ModuleCallBack *)(data));
        cb(mod);
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
using SuspectsMap = std::unordered_map<AbckitCoreImportDescriptor *, size_t>;

std::string GetMethodName(AbckitCoreFunction *method)
{
    auto mname = g_implI->functionGetName(method);
    auto methodName = g_implI->abckitStringToString(mname);
    auto *cls = g_implI->functionGetParentClass(method);
    if (cls == nullptr) {
        return methodName;
    }
    auto cname = g_implI->classGetName(cls);
    auto className = (std::string)g_implI->abckitStringToString(cname);
    return className + '.' + methodName;
}

bool IsLoadApi(AbckitCoreImportDescriptor *id, size_t apiIndex, AbckitInst *inst, std::vector<ApiInfo> &apiList)
{
    // find the following pattern:
    // 1. ldExternalModuleVar (import {ApiNamespace} from "./modules/myApi")
    // 2. ldObjByName v1, "foo" (api.propName)
    // or
    // 1. ldExternalModuleVar (import {bar} from "./modules/myApi")

    if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return false;
    }

    if (g_dynG->iGetImportDescriptor(inst) != id) {
        return false;
    }

    const auto &prop = apiList[apiIndex].propName;
    if (prop.empty()) {
        return true;
    }

    bool found = false;
    EnumerateInstUsers(inst, [&](AbckitInst *user) {
        if (!found && g_dynG->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME) {
            auto str = g_implI->abckitStringToString(g_implG->iGetString(user));
            found = str == prop;
        }
    });

    return found;
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

void CollectUsageInMethod(AbckitCoreFunction *method, SuspectsMap &suspects, ApiUsageMap *apiUsages,
                          std::vector<ApiInfo> &apiList)
{
    auto methodName = GetMethodName(method);
    auto *sourceRaw = g_implI->moduleGetName(g_implI->functionGetModule(method));
    auto source = g_implI->abckitStringToString(sourceRaw);
    UsageInfo usage = {source, methodName, 0};
    EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        for (const auto &[id, apiIndex] : suspects) {
            if (IsLoadApi(id, apiIndex, inst, apiList)) {
                usage.idx = apiIndex;
                AddApiUsage(apiIndex, usage, apiUsages);
            }
        }
    });
}

bool GetSuspects(AbckitCoreModule *mod, SuspectsMap *suspects, std::vector<ApiInfo> &apiList)
{
    EnumerateModuleImports(mod, [&](AbckitCoreImportDescriptor *id) {
        auto importName = g_implI->abckitStringToString(g_implI->importDescriptorGetName(id));
        auto *importedModule = g_implI->importDescriptorGetImportedModule(id);
        auto source = g_implI->abckitStringToString(g_implI->moduleGetName(importedModule));
        for (size_t i = 0; i < apiList.size(); ++i) {
            const auto api = apiList[i];
            if (source != api.source) {
                continue;
            }
            if (importName == api.objName) {
                suspects->emplace(id, i);
            }
        }
    });
    return !suspects->empty();
}

void CollectApiUsages(ApiUsageMap *apiUsages, std::vector<ApiInfo> &apiList, AbckitFile *file)
{
    EnumerateModules(
        [&](AbckitCoreModule *mod) {
            SuspectsMap suspects = {};
            GetSuspects(mod, &suspects, apiList);
            if (suspects.empty()) {
                return;
            }
            EnumerateModuleFunctions(
                mod, [&](AbckitCoreFunction *method) { CollectUsageInMethod(method, suspects, apiUsages, apiList); });
        },
        file);
}

ApiUsageMap GetApiUsages(std::vector<ApiInfo> &apiList, AbckitFile *file)
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

class AbckitScenarioCTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestDynamicApiScannerClean)
{
    // CC-OFFNXT(G.NAM.03) project code style

    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/api_scanner/api_scanner.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_NE(file, nullptr);

    std::vector<ApiInfo> apiList = {{"modules/apiNamespace", "ApiNamespace", "foo"},
                                    {"modules/toplevelApi", "bar", ""},
                                    {"@ohos.geolocation", "geolocation", "getCurrentLocation"}};

    const ApiUsageMap usages = GetApiUsages(apiList, file);
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

    g_impl->closeFile(file);
}

}  // namespace libabckit::test
