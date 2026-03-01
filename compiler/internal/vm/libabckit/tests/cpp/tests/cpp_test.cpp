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

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"

#include <gtest/gtest.h>
#include <string_view>
#include <unordered_set>

namespace libabckit::test {

class LibAbcKitCppTest : public ::testing::Test {};
// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace abckit;

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Function> GetAllFunctions(const File *file)
{
    std::vector<core::Function> functions;

    for (auto &module : file->GetModules()) {
        for (auto &klass : module.GetClasses()) {
            for (auto &function : klass.GetAllMethods()) {
                functions.push_back(function);
            }
        }
        for (auto &function : module.GetTopLevelFunctions()) {
            functions.push_back(function);
        }
    }

    return functions;
}

// Test: test-kind=api, api=BasicBlock::AddInstFront, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc", "cpp_test_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo logic\nbar logic\n"));

    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    for (const auto &function : GetAllFunctions(&file)) {
        abckit::Graph graph = function.CreateGraph();
        abckit::BasicBlock curBB = graph.GetStartBb().GetSuccByIdx(0);

        // Insert `print("Func start: " + funcName)`
        std::string funcStart = "Func start: " + function.GetName();
        abckit::Instruction loadString = graph.DynIsa().CreateLoadString(funcStart);
        curBB.AddInstFront(loadString);
        abckit::Instruction print = graph.DynIsa().CreateTryldglobalbyname("print").InsertAfter(loadString);
        graph.DynIsa().CreateCallarg1(print, loadString).InsertAfter(print);

        // Find all `return` instructions
        std::string funcEnd = "Func end: " + function.GetName();

        auto instCb = [&](const abckit::Instruction &inst) {
            AbckitIsaApiDynamicOpcode opcode = inst.GetGraph()->DynIsa().GetOpcode(inst);
            if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN ||
                opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED) {
                abckit::Instruction endStr = graph.DynIsa().CreateLoadString(funcEnd).InsertBefore(inst);
                graph.DynIsa().CreateCallarg1(print, endStr).InsertBefore(inst);
            }
        };

        std::vector<abckit::Instruction> instructions;
        for (auto &block : graph.GetBlocksRPO()) {
            for (auto &inst : block.GetInstructions()) {
                instCb(inst);
            }
        }
        function.SetGraph(graph);
    }

    file.WriteAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_modified_1.abc");

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_modified_1.abc", "cpp_test_dynamic");
    EXPECT_TRUE(helpers::Match(output,
                               "Func start: func_main_0\n"
                               "Func start: A\n"
                               "Func end: A\n"
                               "Func start: foo\n"
                               "foo logic\n"
                               "Func start: bar\n"
                               "bar logic\n"
                               "Func end: bar\n"
                               "Func end: foo\n"
                               "Func end: func_main_0\n"));
}

// Test: test-kind=api, api=Function::AddAnnotation, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc", "cpp_test_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo logic\nbar logic\n"));

    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    std::vector<abckit::core::Function> allFunctions = GetAllFunctions(&file);

    auto findFunctionsWithAnnotationInterface = [&funcs = allFunctions](std::vector<abckit::core::Function> &found,
                                                                        const std::string &annoName) {
        for (auto &func : funcs) {
            auto annos = func.GetAnnotations();
            auto foundAnno = std::find_if(begin(annos), end(annos), [&annoName](core::Annotation &anno) {
                return anno.GetInterface().GetName() == annoName;
            });
            if (foundAnno != annos.end()) {
                found.push_back(func);
            }
        }
    };

    std::vector<abckit::core::Function> annotatedBefore;
    findFunctionsWithAnnotationInterface(annotatedBefore, "MyAnnoName");

    std::vector<abckit::core::AnnotationInterface> foundAnnoInterfaces;
    for (auto &fModule : file.GetModules()) {
        for (auto &aiInterface : fModule.GetAnnotationInterfaces()) {
            if (aiInterface.GetName() == "MyAnnoName") {
                foundAnnoInterfaces.emplace_back(aiInterface);
            }
        }
    }

    ASSERT_EQ(foundAnnoInterfaces.size(), 1);

    for (auto &func : allFunctions) {
        abckit::arkts::Function arktsFunc(func);
        arktsFunc.AddAnnotation(abckit::arkts::AnnotationInterface(foundAnnoInterfaces[0]));
    }

    std::vector<abckit::core::Function> annotatedAfter;
    findFunctionsWithAnnotationInterface(annotatedAfter, "MyAnnoName");

    ASSERT_EQ(annotatedBefore.size(), 1);
    ASSERT_EQ(annotatedAfter.size(), 4U);

    int found = 0;
    for (auto &item : annotatedAfter) {
        if (item == annotatedBefore.front()) {
            if (++found > 1) {
                break;
            }
        }
    }
    ASSERT_EQ(found, 1);

    file.WriteAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_modified_2.abc");

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_modified_2.abc", "cpp_test_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo logic\nbar logic\n"));
}

// Test: test-kind=api, api=File::CreateValueU1, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest3)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_js.abc");
    abckit::Value res = file.CreateValueU1(true);
    bool val = res.GetU1();
    ASSERT_TRUE(val);
    file.WriteAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_js_modified_3.abc");
}

// Test: test-kind=api, api=File::CreateLiteralBool, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest4)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_js.abc");
    abckit::Literal res = file.CreateLiteralBool(true);
    bool val = res.GetBool();
    ASSERT_TRUE(val);
    file.WriteAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_js_modified_4.abc");
}

// Test: test-kind=api, api=File::CreateLiteralArray, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest5)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_js.abc");
    const double val1 = 0.7;
    const int val2 = 7U;
    abckit::LiteralArray larr =
        file.CreateLiteralArray(std::vector {file.CreateLiteralDouble(val1), file.CreateLiteralU32(val2)});
    abckit::Literal res = file.CreateLiteralLiteralArray(larr);
    abckit::LiteralArray val = res.GetLiteralArray();

    EXPECT_TRUE(larr == val);
    file.WriteAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_js_modified_5.abc");
}

// Test: test-kind=api, api=Annotation::AddAndGetElement, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest6)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    std::vector<abckit::core::Function> funcs;

    for (auto &func : GetAllFunctions(&file)) {
        if (func.GetName() != "foo") {
            continue;
        }

        funcs.push_back(func);
        break;
    }

    ASSERT_EQ(funcs.size(), 1);
    auto func = funcs[0];
    auto anns = func.GetAnnotations();
    ASSERT_EQ(anns.size(), 1);

    abckit::core::Annotation ann = anns[0];
    abckit::core::AnnotationInterface anni = ann.GetInterface();
    auto name = anni.GetName();
    EXPECT_TRUE(name == "MyAnnoName");

    const double initVal = 0.12;
    constexpr std::string_view NEW_NAME = "newValue";
    abckit::Value value = file.CreateValueDouble(initVal);

    abckit::arkts::AnnotationElement annel = arkts::Annotation(ann).AddAndGetElement(value, NEW_NAME);
    EXPECT_TRUE(annel.GetName() == NEW_NAME);
    EXPECT_TRUE(value.GetDouble() == initVal);

    file.WriteAbc(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic_mofdified_6.abc");
}

// Test: test-kind=api, api=Module::GetNamespaces, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest7)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    std::vector<std::string> nsNames;

    for (auto &mdl : file.GetModules()) {
        for (auto &ns : mdl.GetNamespaces()) {
            nsNames.push_back(ns.GetName());
        }
    }

    ASSERT_TRUE(nsNames.size() == 1);
    ASSERT_EQ(nsNames[0], "MyNamespace");
}

// Test: test-kind=api, api=AnnotationInterface::GetFields, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest8)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    std::vector<std::string> annFieldNames;

    for (auto &mdl : file.GetModules()) {
        for (auto &anni : mdl.GetAnnotationInterfaces()) {
            for (auto &anniField : anni.GetFields()) {
                annFieldNames.push_back(anniField.GetName());
            }
        }
    }

    ASSERT_TRUE(annFieldNames.size() == 4U);
    ASSERT_EQ(annFieldNames[0U], "a");
    ASSERT_EQ(annFieldNames[1U], "b");
    ASSERT_EQ(annFieldNames[2U], "d");
    ASSERT_EQ(annFieldNames[3U], "str");
}

// Test: test-kind=api, api=ImportDescriptor::GetName, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest9)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    std::vector<std::string> importNames;

    for (auto &mdl : file.GetModules()) {
        for (auto &imp : mdl.GetImports()) {
            importNames.push_back(imp.GetName());
        }
    }

    ASSERT_TRUE(importNames.size() == 1);
    ASSERT_EQ(importNames[0], "TsExport");
}

// Test: test-kind=api, api=Module::GetExports, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest10)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    std::vector<std::string> exportNames;

    for (auto &mdl : file.GetModules()) {
        for (auto &exp : mdl.GetExports()) {
            exportNames.push_back(exp.GetName());
        }
    }

    ASSERT_TRUE(exportNames.size() == 1);
    ASSERT_EQ(exportNames[0], "bar");
}

// Test: test-kind=internal, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest11)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    // Test checks that Module is hashable
    std::unordered_set<abckit::core::Module> moduleSet;

    for (auto &mdl : file.GetModules()) {
        moduleSet.insert(mdl);
    }

    const int sizeT = 2;
    ASSERT_EQ(moduleSet.size(), sizeT);
}

// Test: test-kind=internal, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest12)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");
    abckit::core::Function func = GetAllFunctions(&file)[0];
    abckit::Graph graph = func.CreateGraph();
    abckit::BasicBlock bb = graph.GetStartBb();
    abckit::Instruction inst = bb.GetFirstInst();
    while (inst) {
        ASSERT_TRUE(inst);
        inst = inst.GetNext();
    }
    ASSERT_FALSE(inst);
}

// Test: test-kind=api, api=Module::GetModule, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitCppTest, CppTest13)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    std::vector<core::Module> count;

    for (auto &mdl : file.GetModules()) {
        for (auto &ns : mdl.GetNamespaces()) {
            count.push_back(ns.GetModule());
        }
    }

    ASSERT_TRUE(count.size() == 1);
}
}  // namespace libabckit::test
