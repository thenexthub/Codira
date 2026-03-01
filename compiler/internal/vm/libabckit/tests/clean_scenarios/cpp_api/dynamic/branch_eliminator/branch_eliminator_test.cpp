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
#include "libabckit/src/logger.h"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <optional>
#include <unordered_map>

namespace {

class GTestAssertErrorHandler final : public abckit::IErrorHandler {
public:
    void HandleError(abckit::Exception &&err) override
    {
        EXPECT_TRUE(false) << "Abckit expection raised: " << err.what();
    }
};

struct ConstantMeta {
    std::string_view modulePath;
    std::string_view objName;
    std::string_view fieldName;
    bool fieldValue;
};

using ConstantMetaCollection = std::vector<ConstantMeta>;
using ConstantMetaIterator = typename ConstantMetaCollection::const_iterator;
using SuspectsType = std::unordered_multimap<abckit::core::ImportDescriptor, ConstantMetaIterator>;

using InstructionCallback = std::function<bool(abckit::Instruction)>;
using FunctionCallback = std::function<bool(abckit::core::Function)>;
using ClassCallback = std::function<bool(abckit::core::Class)>;
using NamespaceCallback = std::function<bool(abckit::core::Namespace)>;

void EnumerateAllMethods(const abckit::core::Module &mod, const FunctionCallback &fnUserCallback)
{
    ClassCallback clsCallback;

    FunctionCallback fnCallback = [&](const abckit::core::Function &fun) -> bool {
        if (!fnUserCallback(fun)) {
            return false;
        }
        fun.EnumerateNestedFunctions(fnCallback);
        fun.EnumerateNestedClasses(clsCallback);
        return true;
    };

    clsCallback = [&](const abckit::core::Class &cls) -> bool {
        cls.EnumerateMethods(fnCallback);
        return true;
    };

    NamespaceCallback nsCallback = [&](const abckit::core::Namespace &ns) -> bool {
        ns.EnumerateNamespaces(nsCallback);
        ns.EnumerateClasses(clsCallback);
        ns.EnumerateTopLevelFunctions(fnCallback);
        return true;
    };

    mod.EnumerateNamespaces(nsCallback);
    mod.EnumerateClasses(clsCallback);
    mod.EnumerateTopLevelFunctions(fnCallback);
}

inline void EnumerateGraphInsts(const abckit::Graph &graph, const InstructionCallback &cb)
{
    bool askedContinue = true;
    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) -> bool {
        for (auto inst = bb.GetFirstInst(); !!inst && askedContinue; inst = inst.GetNext()) {
            askedContinue = cb(inst);
        }
        return true;
    });
}

std::optional<abckit::Instruction> GraphInstsFindIf(const abckit::Graph &graph, const InstructionCallback &cb)
{
    std::optional<abckit::Instruction> maybeInst;
    EnumerateGraphInsts(graph, [&](const abckit::Instruction &inst) -> bool {
        if (!maybeInst && cb(inst)) {
            maybeInst = inst;
        }
        return !maybeInst;
    });
    return maybeInst;
}

SuspectsType GetSuspects(const abckit::core::Module &mod, const ConstantMetaCollection &constants)
{
    SuspectsType suspects;
    mod.EnumerateImports([&](const abckit::core::ImportDescriptor &idesc) {
        auto importName = idesc.GetName();
        auto modulePath = idesc.GetImportedModule().GetName();

        for (auto itConst = constants.begin(); itConst != constants.end(); ++itConst) {
            if (std::tie(itConst->modulePath, itConst->objName) != std::tie(modulePath, importName)) {
                continue;
            }
            suspects.emplace(idesc, itConst);
        }
        return true;
    });
    return suspects;
}

ConstantMetaIterator GetConstantMetaIterator(const abckit::Instruction &inst, const ConstantMetaCollection &constants,
                                             const SuspectsType &suspects)
{
    auto end = constants.end();

    if (inst.GetGraph()->DynIsa().GetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME) {
        return end;
    }
    auto ldExternalModuleVar = inst.GetInput(0);
    if (ldExternalModuleVar.GetGraph()->DynIsa().GetOpcode(ldExternalModuleVar) !=
        ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return end;
    }

    auto idesc = ldExternalModuleVar.GetGraph()->DynIsa().GetImportDescriptor(ldExternalModuleVar);
    auto range = suspects.equal_range(idesc);
    if (range.first == range.second) {
        return end;
    }

    auto propName = inst.GetString();  // "isDebug"
    for (auto it = range.first; it != range.second; ++it) {
        const ConstantMetaIterator &itCM = it->second;
        if (itCM->fieldName == propName) {
            return itCM;
        }
    }

    return end;
}

void ReplaceUsers(const abckit::Instruction &oldInst, const abckit::Instruction &newInst)
{
    oldInst.VisitUsers([&](const abckit::Instruction &user) -> bool {
        size_t inputCount = user.GetInputCount();
        for (size_t idx = 0; idx < inputCount; ++idx) {
            if (user.GetInput(idx) == oldInst) {
                user.SetInput(idx, newInst);
            }
        }
        return true;
    });
}

void ReplaceLdObjByNameWithBoolean(abckit::Graph &graph, abckit::Instruction ldObjByName, const ConstantMeta &constMeta)
{
    const bool fieldVal = constMeta.fieldValue;
    abckit::BasicBlock bb = ldObjByName.GetBasicBlock();
    // abckit::Graph& graph = *const_cast<abckit::Graph*>(bb.GetGraph());
    auto value = fieldVal ? graph.DynIsa().CreateLdtrue() : graph.DynIsa().CreateLdfalse();
    value.InsertAfter(ldObjByName);
    ReplaceUsers(ldObjByName, value);

    auto ldExternalModuleVar = ldObjByName.GetInput(0);
    bool isRemovable = true;
    std::vector<abckit::Instruction> removableChecks;
    ldExternalModuleVar.VisitUsers([&](abckit::Instruction user) -> bool {
        if (user != ldObjByName) {
            if (user.GetGraph()->DynIsa().GetOpcode(user) !=
                ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME) {
                isRemovable = false;
            } else {
                removableChecks.push_back(std::move(user));
            }
        }
        return true;
    });

    if (isRemovable) {
        for (auto &inst : removableChecks) {
            inst.Remove();
        }
        ldExternalModuleVar.Remove();
    }

    ldObjByName.Remove();
}

bool ReplaceModuleVarByConstant(abckit::Graph &graph, const ConstantMetaCollection &constants,
                                const SuspectsType &suspects)
{
    // find the following pattern:
    // ...
    // 1. ldExternalModuleVar (importdescriptor)
    // 2. ldObjByName v1, "isDebug"
    //
    // and replace it by
    // 3. ldtrue

    std::unordered_map<abckit::Instruction, ConstantMetaIterator> moduleVars;
    EnumerateGraphInsts(graph, [&](const abckit::Instruction &inst) {
        if (auto itCM = GetConstantMetaIterator(inst, constants, suspects); itCM != constants.end()) {
            moduleVars.emplace(inst, itCM);
        }
        return true;
    });

    for (auto &[ldObjByName, itCM] : moduleVars) {
        EXPECT_TRUE(!!ldObjByName);
        EXPECT_TRUE(itCM != constants.end());
        ReplaceLdObjByNameWithBoolean(graph, ldObjByName, *itCM);
    }

    return !moduleVars.empty();
}

bool GetInstAsBool(const abckit::Instruction &inst)
{
    switch (inst.GetGraph()->DynIsa().GetOpcode(inst)) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT:
            return inst.GetConstantValueI64() != 0;
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

bool CanBeRemoved(const abckit::Instruction &inst)
{
    switch (inst.GetGraph()->DynIsa().GetOpcode(inst)) {
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

bool RemoveUnusedInsts(abckit::Graph &graph)
{
    bool graphUpdated = false;
    bool hasUnprocessedInsts = true;

    while (hasUnprocessedInsts) {
        std::vector<abckit::Instruction> removables;
        EnumerateGraphInsts(graph, [&](abckit::Instruction inst) {
            if (inst.GetUserCount() == 0 && CanBeRemoved(inst)) {
                removables.push_back(std::move(inst));
            }
            return true;
        });

        if (removables.empty()) {
            hasUnprocessedInsts = false;
            continue;
        }

        EXPECT_TRUE(!removables.empty());
        for (auto &inst : removables) {
            inst.Remove();
        }
        graphUpdated = true;
    }

    return graphUpdated;
}

bool LoweringConstantsSinglePass(abckit::Graph &graph)
{
    std::vector<abckit::Instruction> boolUsers;
    auto fnInstCollectBoolUsers = [&boolUsers](const abckit::Instruction &user) -> bool {
        auto userOp = user.GetGraph()->DynIsa().GetOpcode(user);
        if (userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE || userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE ||
            userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISTRUE ||
            userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISFALSE) {
            boolUsers.emplace_back(user);
        }
        return true;
    };

    std::optional<abckit::Instruction> maybeLdBool = GraphInstsFindIf(graph, [&](const abckit::Instruction &inst) {
        auto op = inst.GetGraph()->DynIsa().GetOpcode(inst);
        if (op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE && op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE) {
            return false;
        }

        inst.VisitUsers(fnInstCollectBoolUsers);
        return !boolUsers.empty();
    });

    if (!maybeLdBool) {
        return false;
    }

    auto ldBoolVal = GetInstAsBool(*maybeLdBool);

    for (abckit::Instruction &isBool : boolUsers) {
        auto isBoolVal = GetInstAsBool(isBool);
        auto ldBool = ldBoolVal == isBoolVal ? graph.DynIsa().CreateLdtrue() : graph.DynIsa().CreateLdfalse();
        ldBool.InsertAfter(isBool);
        ReplaceUsers(isBool, ldBool);
        isBool.Remove();
    }

    return true;
}

bool LoweringConstants(abckit::Graph &graph)
{
    bool graphUpdated = false;

    while (bool hasUpdates = LoweringConstantsSinglePass(graph)) {
        graphUpdated |= hasUpdates;
    };

    graphUpdated |= RemoveUnusedInsts(graph);
    return graphUpdated;
}

bool IsEliminatableIfInst(const abckit::Instruction &inst)
{
    if (inst.GetGraph()->DynIsa().GetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_IF || inst.GetInputCount() != 2U) {
        return false;
    }
    auto ldBool = inst.GetInput(0);
    auto op = ldBool.GetGraph()->DynIsa().GetOpcode(ldBool);
    if (op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE && op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE) {
        return false;
    }

    auto constInst = inst.GetInput(1);
    return constInst.GetGraph()->DynIsa().GetOpcode(constInst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT;
}

void DeleteUnreachableBranch(abckit::Graph &graph, abckit::Instruction &ifInst)
{
    auto ldBool = ifInst.GetInput(0);     // 0: ldtrue or ldfalse
    auto constInst = ifInst.GetInput(1);  // 1: ConstantInt
    const bool valLhs = GetInstAsBool(ldBool);
    const bool valRhs = GetInstAsBool(constInst);

    auto conditionCode = ifInst.GetGraph()->DynIsa().GetConditionCode(ifInst);
    EXPECT_TRUE(conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ ||
                conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    bool result = (conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ && valLhs == valRhs) ||
                  (conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE && valLhs != valRhs);

    auto bb = ifInst.GetBasicBlock();
    // if true ==> delete false branch
    ifInst.Remove();

    bb.EraseSuccBlock(result ? 1 : 0);
    graph.RunPassRemoveUnreachableBlocks();
}

void EliminateBranchWithSuspect(abckit::core::Function &method, const ConstantMetaCollection &constants,
                                const SuspectsType &suspects)
{
    auto graph = method.CreateGraph();

    bool graphUpdated = ReplaceModuleVarByConstant(graph, constants, suspects);
    std::optional<abckit::Instruction> maybeRemovable;
    do {
        if (maybeRemovable) {
            DeleteUnreachableBranch(graph, *maybeRemovable);
            graphUpdated = true;
        }

        graphUpdated |= LoweringConstants(graph);

        maybeRemovable =
            GraphInstsFindIf(graph, [&](const abckit::Instruction &inst) { return IsEliminatableIfInst(inst); });
    } while (maybeRemovable);

    if (graphUpdated) {
        method.SetGraph(graph);
    }
}

void Run(abckit::File &file, const ConstantMetaCollection &constants)
{
    if (constants.empty()) {
        return;
    }

    file.EnumerateModules([&](const abckit::core::Module &mod) {
        const auto suspects = GetSuspects(mod, constants);
        if (!suspects.empty()) {
            EnumerateAllMethods(mod, [&](abckit::core::Function method) {
                EliminateBranchWithSuspect(method, constants, suspects);
                return true;
            });
        }
        return true;
    });
}

bool MethodHasBranch(const abckit::File &file, const std::string &moduleName, const std::string &methodName)
{
    std::optional<abckit::core::Function> foundMethod;

    file.EnumerateModules([&](const abckit::core::Module &mod) {
        if (foundMethod || mod.GetName() != moduleName) {
            return !foundMethod;
        }

        EnumerateAllMethods(mod, [&](const abckit::core::Function &method) {
            if (foundMethod || method.GetName() != methodName) {
                return !foundMethod;
            }
            foundMethod = method;
            return !foundMethod;
        });

        return !foundMethod;
    });
    EXPECT_TRUE(foundMethod.has_value());

    auto graph = foundMethod->CreateGraph();
    auto maybeIfInst = GraphInstsFindIf(graph, [&](const abckit::Instruction &inst) {
        return inst.GetGraph()->DynIsa().GetOpcode(inst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_IF;
    });
    return maybeIfInst.has_value();
}

}  // namespace

namespace libabckit::test {

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
    const std::string sandboxPath = ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/branch_eliminator/";
    const std::string inputPath = sandboxPath + "branch_eliminator.abc";
    const std::string outputPath = sandboxPath + "branch_eliminator_modified.abc";

    abckit::File file(inputPath, std::make_unique<GTestAssertErrorHandler>());

    const auto origOutput = helpers::ExecuteDynamicAbc(inputPath, "branch_eliminator");
    auto expectedOrigOutput = GetExpectOutput(CONFIG_IS_DEBUG_ORIGIN_VALUE);

    EXPECT_EQ(origOutput, expectedOrigOutput);
    ASSERT_TRUE(MethodHasBranch(file, "modules/myfoo", "myfoo"));
    ASSERT_TRUE(MethodHasBranch(file, "modules/mybar", "test1"));
    ASSERT_TRUE(MethodHasBranch(file, "modules/mybar", "test2"));

    // Delete true branch
    const ConstantMetaCollection constants = {{"modules/config", "Config", "isDebug", configIsDebugFinal}};
    Run(file, constants);

    ASSERT_FALSE(MethodHasBranch(file, "modules/myfoo", "myfoo"));
    ASSERT_FALSE(MethodHasBranch(file, "modules/mybar", "test1"));
    ASSERT_FALSE(MethodHasBranch(file, "modules/mybar", "test2"));

    file.WriteAbc(outputPath);

    auto modOutput = helpers::ExecuteDynamicAbc(outputPath, "branch_eliminator");
    auto expectedModOutput = GetExpectOutput(configIsDebugFinal);
    EXPECT_EQ(modOutput, expectedModOutput);
}

class AbckitScenarioCppTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestDynamicBranchEliminatorClean1)
{
    GeneralBranchEliminatorTest(false);
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestDynamicBranchEliminatorClean2)
{
    GeneralBranchEliminatorTest(true);
}

}  // namespace libabckit::test
