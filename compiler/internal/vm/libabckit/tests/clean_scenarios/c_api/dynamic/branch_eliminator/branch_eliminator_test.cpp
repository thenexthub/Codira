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

#include <sstream>

#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "helpers/helpers_runtime.h"

#include "helpers/helpers.h"
#include "libabckit/src/logger.h"

namespace {

constexpr AbckitApiVersion VERSION = ABCKIT_VERSION_RELEASE_1_0_0;
auto *g_impl = AbckitGetApiImpl(VERSION);
const AbckitInspectApi *g_implI = AbckitGetInspectApiImpl(VERSION);
const AbckitGraphApi *g_implG = AbckitGetGraphApiImpl(VERSION);
const AbckitIsaApiDynamic *g_dynG = AbckitGetIsaApiDynamicImpl(VERSION);
const AbckitModifyApi *g_implM = AbckitGetModifyApiImpl(VERSION);

struct ConstantInfo {
    std::string path;
    std::string objName;    // Config
    std::string fieldName;  // isDebug
    bool fieldValue;        // field is constant true or false
};

using ConstantInfoIndexType = uint32_t;
using SuspectsType = std::unordered_map<AbckitCoreImportDescriptor *, std::vector<ConstantInfoIndexType>>;
constexpr ConstantInfoIndexType INVALID_INDEX = std::numeric_limits<ConstantInfoIndexType>::max();

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
    const AbckitGraphApi *implG = nullptr;
};

template <class InstCallBack>
inline void EnumerateGraphInsts(AbckitGraph *graph, const InstCallBack &cb)
{
    CapturedData captured {(void *)(&cb), g_implG};

    g_implG->gVisitBlocksRpo(graph, &captured, [](AbckitBasicBlock *bb, void *data) {
        auto *captured = reinterpret_cast<CapturedData *>(data);
        const auto &cb = *((InstCallBack *)(captured->callback));
        auto *implG = captured->implG;
        for (auto *inst = implG->bbGetFirstInst(bb); inst != nullptr; inst = implG->iGetNext(inst)) {
            cb(inst);
        }
        return true;
    });
}

template <class InstCallBack>
inline AbckitInst *GraphInstsFindIf(AbckitGraph *graph, const InstCallBack &cb)
{
    AbckitInst *ret = nullptr;
    bool found = false;
    EnumerateGraphInsts(graph, [&](AbckitInst *inst) {
        if (!found && cb(inst)) {
            found = true;
            ret = inst;
        }
    });

    return ret;
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

bool GetSuspects(AbckitCoreModule *mod, const std::vector<ConstantInfo> &constants, SuspectsType &suspects)
{
    EnumerateModuleImports(mod, [&](AbckitCoreImportDescriptor *id) {
        auto importName = g_implI->abckitStringToString(g_implI->importDescriptorGetName(id));
        auto *importedModule = g_implI->importDescriptorGetImportedModule(id);
        auto path = g_implI->abckitStringToString(g_implI->moduleGetName(importedModule));
        for (ConstantInfoIndexType i = 0; i < constants.size(); ++i) {
            const auto &constInfo = constants[i];
            if (constInfo.path != path || constInfo.objName != importName) {
                continue;
            }
            auto iter = suspects.find(id);
            if (iter == suspects.end()) {
                std::vector<ConstantInfoIndexType> vec = {i};
                suspects.emplace(id, vec);
            } else {
                iter->second.push_back(i);
            }
        }
    });
    return !suspects.empty();
}

ConstantInfoIndexType GetConstantInfoIndex(AbckitInst *inst, const std::vector<ConstantInfo> &constants,
                                           const SuspectsType &suspects)
{
    if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME) {
        return INVALID_INDEX;
    }
    auto *ldExternalModuleVar = g_implG->iGetInput(inst, 0);
    if (g_dynG->iGetOpcode(ldExternalModuleVar) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return INVALID_INDEX;
    }
    auto *id = g_dynG->iGetImportDescriptor(ldExternalModuleVar);
    auto iter = suspects.find(id);
    if (iter == suspects.end()) {
        return INVALID_INDEX;
    }
    const auto &constInfoIndexes = iter->second;
    const auto propName = g_implI->abckitStringToString(g_implG->iGetString(inst));  // "isDebug"
    for (auto index : constInfoIndexes) {
        EXPECT_TRUE(index >= 0);
        EXPECT_TRUE(index < constants.size());
        if (constants[index].fieldName == propName) {
            return index;
        }
    }

    return INVALID_INDEX;
}

void ReplaceUsers(AbckitInst *oldInst, AbckitInst *newInst)
{
    EnumerateInstUsers(oldInst, [&](AbckitInst *user) {
        auto inputCount = g_implG->iGetInputCount(user);
        for (uint64_t i = 0; i < inputCount; ++i) {
            if (g_implG->iGetInput(user, i) == oldInst) {
                g_implG->iSetInput(user, newInst, i);
            }
        }
    });
}

void ReplaceLdObjByNameWithBoolean(AbckitInst *ldObjByName, ConstantInfoIndexType constInfoIndex,
                                   const std::vector<ConstantInfo> &constants)
{
    bool fieldVal = constants[constInfoIndex].fieldValue;
    auto *bb = g_implG->iGetBasicBlock(ldObjByName);
    auto *graph = g_implG->bbGetGraph(bb);
    auto *value = fieldVal ? g_dynG->iCreateLdtrue(graph) : g_dynG->iCreateLdfalse(graph);
    g_implG->iInsertAfter(value, ldObjByName);
    ReplaceUsers(ldObjByName, value);

    auto *ldExternalModuleVar = g_implG->iGetInput(ldObjByName, 0);
    bool isRemovable = true;
    std::vector<AbckitInst *> checks;
    EnumerateInstUsers(ldExternalModuleVar, [&](AbckitInst *inst) {
        if (inst != ldObjByName) {
            if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME) {
                isRemovable = false;
            } else {
                checks.emplace_back(inst);
            }
        }
    });

    if (isRemovable) {
        for (auto *check : checks) {
            g_implG->iRemove(check);
        }
        g_implG->iRemove(ldExternalModuleVar);
    }
    g_implG->iRemove(ldObjByName);
}

bool ReplaceModuleVarByConstant(AbckitGraph *graph, const std::vector<ConstantInfo> &constants,
                                const SuspectsType &suspects)
{
    // find the following patter:
    // ...
    // 1. ldExternalModuleVar (importdescriptor)
    // 2. ldObjByName v1, "isDebug"
    //
    // and replace it by
    // 3. ldtrue

    std::unordered_map<AbckitInst *, ConstantInfoIndexType> moduleVars;
    EnumerateGraphInsts(graph, [&](AbckitInst *inst) {
        auto constInfoIndex = GetConstantInfoIndex(inst, constants, suspects);
        if (constInfoIndex != INVALID_INDEX) {
            moduleVars.emplace(inst, constInfoIndex);
        }
    });
    for (auto &[ldObjByName, constInfoIndex] : moduleVars) {
        EXPECT_TRUE(ldObjByName != nullptr);
        EXPECT_TRUE(constInfoIndex != INVALID_INDEX);
        ReplaceLdObjByNameWithBoolean(ldObjByName, constInfoIndex, constants);
    }

    return !moduleVars.empty();
}

bool GetInstAsBool(AbckitInst *inst)
{
    switch (g_dynG->iGetOpcode(inst)) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT:
            return g_implG->iGetConstantValueI64(inst) != 0;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISTRUE:
            return true;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISFALSE:
            return false;
        default:
            EXPECT_TRUE(false);  // UNREACHABLE
    }
    return false;
}

bool CanBeRemoved(AbckitInst *inst)
{
    switch (g_dynG->iGetOpcode(inst)) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE:
            return true;
        default:
            return false;
    }
}

bool RemoveUnusedInsts(AbckitGraph *graph)
{
    bool hasGraphChanged = false;
    bool continueLoop = true;

    while (continueLoop) {
        std::vector<AbckitInst *> removableInsts;
        EnumerateGraphInsts(graph, [&](AbckitInst *inst) {
            if (g_implG->iGetUserCount(inst) == 0 && CanBeRemoved(inst)) {
                removableInsts.emplace_back(inst);
            }
        });

        if (removableInsts.empty()) {
            continueLoop = false;
        } else {
            EXPECT_TRUE(!removableInsts.empty());
            for (auto *inst : removableInsts) {
                g_implG->iRemove(inst);
            }
            hasGraphChanged = true;
        }
    }

    return hasGraphChanged;
}

bool LoweringConstants(AbckitGraph *graph)
{
    bool hasGraphChanged = false;
    auto doCb = [graph, &hasGraphChanged]() -> bool {
        std::vector<AbckitInst *> users;

        auto instFindUsers = [&users](AbckitInst *user) {
            auto userOp = g_dynG->iGetOpcode(user);
            if (userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE || userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE ||
                userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISTRUE ||
                userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISFALSE) {
                users.emplace_back(user);
            }
        };

        AbckitInst *ldBool = GraphInstsFindIf(graph, [&](AbckitInst *inst) {
            auto op = g_dynG->iGetOpcode(inst);
            if (op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE && op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE) {
                return false;
            }

            EnumerateInstUsers(inst, instFindUsers);

            return !users.empty();
        });

        if (ldBool == nullptr) {
            return false;
        }

        auto ldBoolVal = GetInstAsBool(ldBool);
        for (auto *isBool : users) {
            auto isBoolVal = GetInstAsBool(isBool);
            auto *ldBool = ldBoolVal == isBoolVal ? g_dynG->iCreateLdtrue(graph) : g_dynG->iCreateLdfalse(graph);
            g_implG->iInsertAfter(ldBool, isBool);
            ReplaceUsers(isBool, ldBool);
            g_implG->iRemove(isBool);
        }

        hasGraphChanged = true;

        return true;
    };

    while (doCb()) {
    }

    hasGraphChanged |= RemoveUnusedInsts(graph);

    return hasGraphChanged;
}

bool IsEliminatableIfInst(AbckitInst *inst)
{
    if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_IF || g_implG->iGetInputCount(inst) != 2U) {
        return false;
    }
    auto *ldBool = g_implG->iGetInput(inst, 0);
    auto op = g_dynG->iGetOpcode(ldBool);
    if (op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE && op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE) {
        return false;
    }
    auto *constInst = g_implG->iGetInput(inst, 1);
    return g_dynG->iGetOpcode(constInst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT;
}

void DeleteUnreachableBranch(AbckitInst *ifInst)
{
    // compute result of If compare
    auto *ldBool = g_implG->iGetInput(ifInst, 0);     // 0: ldtrue or ldfalse
    auto *constInst = g_implG->iGetInput(ifInst, 1);  // 1: ConstantInst
    auto valLeft = GetInstAsBool(ldBool);
    auto valRight = GetInstAsBool(constInst);
    auto conditionCode = g_dynG->iGetConditionCode(ifInst);

    EXPECT_TRUE(conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ ||
                conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    bool result = (conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ && valLeft == valRight) ||
                  (conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE && valLeft != valRight);

    auto *bb = g_implG->iGetBasicBlock(ifInst);
    // if true ==> delete false branch
    g_implG->iRemove(ifInst);

    g_implG->bbDisconnectSuccBlock(bb, result ? 1 : 0);
    g_implG->gRunPassRemoveUnreachableBlocks(g_implG->bbGetGraph(bb));
}

template <class FunctionCallBack>
inline void TransformMethod(AbckitCoreFunction *f, const FunctionCallBack &cb)
{
    cb(g_implI->functionGetFile(f), f);
}

void EliminateBranchWithSuspect(AbckitCoreFunction *method, const std::vector<ConstantInfo> &constants,
                                const SuspectsType &suspects)
{
    TransformMethod(method, [&](AbckitFile *, AbckitCoreFunction *method) {
        AbckitGraph *graph = g_implI->createGraphFromFunction(method);
        bool hasGraphChanged = ReplaceModuleVarByConstant(graph, constants, suspects);
        bool continueLoop = true;
        while (continueLoop) {
            hasGraphChanged |= LoweringConstants(graph);

            auto *ifInst = GraphInstsFindIf(graph, [&](AbckitInst *inst) { return IsEliminatableIfInst(inst); });

            if (ifInst == nullptr) {
                continueLoop = false;
            } else {
                DeleteUnreachableBranch(ifInst);
                hasGraphChanged = true;
            }
        }

        if (hasGraphChanged) {
            g_implM->functionSetGraph(method, graph);
        }
        g_impl->destroyGraph(graph);
    });
}

void Run(const std::vector<ConstantInfo> &constants, AbckitFile *file)
{
    if (constants.empty()) {
        return;
    }
    EnumerateModules(
        [&](AbckitCoreModule *mod) {
            SuspectsType suspects;
            if (!GetSuspects(mod, constants, suspects)) {
                return;
            }
            EnumerateModuleFunctions(
                mod, [&](AbckitCoreFunction *func) { EliminateBranchWithSuspect(func, constants, suspects); });
        },
        file);
}
}  // namespace

namespace libabckit::test {

static bool MethodHasBranch(const std::string &moduleName, const std::string &methodName, AbckitFile *file)
{
    bool found = false;
    bool ret = false;
    EnumerateModules(
        [&](AbckitCoreModule *mod) {
            if (g_implI->abckitStringToString(g_implI->moduleGetName(mod)) != moduleName) {
                return;
            }
            EnumerateModuleFunctions(mod, [&](AbckitCoreFunction *func) {
                if (found || g_implI->abckitStringToString(g_implI->functionGetName(func)) != methodName) {
                    return;
                }
                found = true;
                auto *graph = g_implI->createGraphFromFunction(func);
                auto *ifInst = GraphInstsFindIf(graph, [&](AbckitInst *inst) {
                    return g_dynG->iGetOpcode(inst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_IF;
                });
                ret = ifInst != nullptr;
                g_impl->destroyGraph(graph);
            });
        },
        file);

    EXPECT_TRUE(found);
    return ret;
}

class AbckitScenarioCTestClean : public ::testing::Test {};

static constexpr bool CONFIG_IS_DEBUG_ORIGIN_VALUE = false;

/*
 * @param value: the value of isDebug when we handle, it can be different from configIsDebugOriginValue
 */
static std::string GetExpectOutput(bool value)
{
    std::stringstream expectOutput;
    expectOutput << std::boolalpha;
    expectOutput << "Config.isDebug = " << value << std::endl;
    expectOutput << "myfoo: Config.isDebug is " << value << std::endl;
    expectOutput << "Mybar.test1: Config.isDebug is " << value << std::endl;
    expectOutput << "Mybar.test2: Config.isDebug is " << value << std::endl;
    return expectOutput.str();
}

static void GeneralBranchEliminatorTest(bool configIsDebugFinal)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/branch_eliminator/branch_eliminator.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_NE(file, nullptr);

    const auto actualOutput = helpers::ExecuteDynamicAbc(INPUT_PATH, "branch_eliminator");
    auto expectedOutput = GetExpectOutput(CONFIG_IS_DEBUG_ORIGIN_VALUE);
    EXPECT_TRUE(helpers::Match(actualOutput, expectedOutput));

    ASSERT_TRUE(MethodHasBranch("modules/myfoo", "myfoo", file));
    ASSERT_TRUE(MethodHasBranch("modules/mybar", "test1", file));
    ASSERT_TRUE(MethodHasBranch("modules/mybar", "test2", file));
    // Delete true branch
    const std::vector<ConstantInfo> infos = {{"modules/config", "Config", "isDebug", configIsDebugFinal}};
    Run(infos, file);
    ASSERT_FALSE(MethodHasBranch("modules/myfoo", "myfoo", file));
    ASSERT_FALSE(MethodHasBranch("modules/mybar", "test1", file));
    ASSERT_FALSE(MethodHasBranch("modules/mybar", "test2", file));

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/branch_eliminator/branch_eliminator_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);
    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, "branch_eliminator");
    auto expectOutput2 = GetExpectOutput(configIsDebugFinal);
    ASSERT_EQ(output, expectOutput2);
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestDynamicBranchEliminatorClean1)
{
    GeneralBranchEliminatorTest(false);
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestDynamicBranchEliminatorClean2)
{
    GeneralBranchEliminatorTest(true);
}

}  // namespace libabckit::test
