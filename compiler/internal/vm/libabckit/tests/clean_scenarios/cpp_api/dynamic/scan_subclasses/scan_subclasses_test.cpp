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
#include <optional>
#include <string>
#include <set>

namespace {

class GTestAssertErrorHandler final : public abckit::IErrorHandler {
public:
    void HandleError(abckit::Exception &&err) override
    {
        EXPECT_TRUE(false) << "Abckit expection raised: " << err.what();
    }
};

using FunctionCallback = std::function<bool(abckit::core::Function)>;
using ClassCallback = std::function<bool(abckit::core::Class)>;
using NamespaceCallback = std::function<bool(abckit::core::Namespace)>;

struct ClassMeta {
    std::string modulePath;
    std::string className;

    friend bool operator==(const ClassMeta &lhs, const ClassMeta &rhs)
    {
        return std::tie(lhs.modulePath, lhs.className) == std::tie(rhs.modulePath, rhs.className);
    }

    friend bool operator<(const ClassMeta &lhs, const ClassMeta &rhs)
    {
        return std::tie(lhs.modulePath, lhs.className) < std::tie(rhs.modulePath, rhs.className);
    }
};

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

std::vector<abckit::core::ImportDescriptor> GetImportDescriptors(const abckit::core::Module &mod,
                                                                 const std::set<ClassMeta> &baseClasses)
{
    std::vector<abckit::core::ImportDescriptor> idescs;
    mod.EnumerateImports([&](abckit::core::ImportDescriptor idesc) -> bool {
        const ClassMeta idescMeta {idesc.GetImportedModule().GetName(), idesc.GetName()};
        for (const auto &baseClass : baseClasses) {
            if (idescMeta == baseClass) {
                idescs.push_back(std::move(idesc));
                return true;
            }
        }
        return true;
    });
    return idescs;
}

std::optional<ClassMeta> FindClassMetaFromLoadApi(const abckit::core::ImportDescriptor &idesc,
                                                  const abckit::Instruction &inst)
{
    if (inst.GetGraph()->DynIsa().GetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR ||
        inst.GetGraph()->DynIsa().GetImportDescriptor(inst) != idesc) {
        return std::nullopt;
    }

    std::optional<ClassMeta> definedClass;
    inst.VisitUsers([&](const abckit::Instruction &user) -> bool {
        if (!definedClass &&
            user.GetGraph()->DynIsa().GetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER) {
            auto klass = user.GetFunction().GetParentClass();
            auto mod = klass.GetModule();
            definedClass = {mod.GetName(), klass.GetName()};
            return false;
        }
        return true;
    });

    return definedClass;
}

void CollectSubClasses(const abckit::Instruction &inst,
                       const std::vector<abckit::core::ImportDescriptor> &importDescriptors,
                       std::set<ClassMeta> &subClasses)
{
    for (auto &idesc : importDescriptors) {
        if (auto maybeMeta = FindClassMetaFromLoadApi(idesc, inst)) {
            subClasses.insert(*maybeMeta);
        }
    }
}

void CollectSubClasses(const abckit::core::Function &method,
                       const std::vector<abckit::core::ImportDescriptor> &importDescriptors,
                       std::set<ClassMeta> &subClasses)
{
    auto graph = method.CreateGraph();
    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) -> bool {
        for (auto inst = bb.GetFirstInst(); !!inst; inst = inst.GetNext()) {
            CollectSubClasses(inst, importDescriptors, subClasses);
        }
        return true;
    });
}

}  // namespace

namespace libabckit::test {

class AbckitScenarioCppTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestScanSubclassesClean)
{
    const std::string inputPath = ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/scan_subclasses/scan_subclasses.abc";
    abckit::File file(inputPath, std::make_unique<GTestAssertErrorHandler>());

    const std::set<ClassMeta> baseClasses = {{"modules/base", "Base"}};

    std::set<ClassMeta> subClasses;
    file.EnumerateModules([&](const abckit::core::Module &mod) {
        auto importDescriptors = GetImportDescriptors(mod, baseClasses);
        if (importDescriptors.empty()) {
            return true;
        }

        EnumerateAllMethods(mod, [&](const abckit::core::Function &method) {
            CollectSubClasses(method, importDescriptors, subClasses);
            return true;
        });
        return true;
    });

    const std::set<ClassMeta> expectedSubClasses = {{"scan_subclasses", "Child1"}, {"scan_subclasses", "Child2"}};

    EXPECT_FALSE(subClasses.empty());
    EXPECT_TRUE(subClasses == expectedSubClasses);
}

}  // namespace libabckit::test
