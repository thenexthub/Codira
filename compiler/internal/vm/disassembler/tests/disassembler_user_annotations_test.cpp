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

#include "disassembler.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

using namespace testing::ext;
namespace panda::disasm {

static const std::string FILE_DECLARATION_3D_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-boolean.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-enum-number.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-enum-string.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-number.abc";
static const std::string FILE_DECLARATION_3D_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-3d-array-string.abc";
static const std::string FILE_DECLARATION_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-array-boolean.abc";
static const std::string FILE_DECLARATION_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-array-enum-number.abc";
static const std::string FILE_DECLARATION_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-array-enum-string.abc";
static const std::string FILE_DECLARATION_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-array-number.abc";
static const std::string FILE_DECLARATION_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-array-string.abc";
static const std::string FILE_DECLARATION_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-boolean.abc";
static const std::string FILE_DECLARATION_COMBINATION =
    GRAPH_TEST_ABC_DIR "declaration-combination.abc";
static const std::string FILE_DECLARATION_EMPTY =
    GRAPH_TEST_ABC_DIR "declaration-empty.abc";
static const std::string FILE_DECLARATION_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-enum-number.abc";
static const std::string FILE_DECLARATION_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-enum-string.abc";
static const std::string FILE_DECLARATION_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-number.abc";
static const std::string FILE_DECLARATION_STRING =
    GRAPH_TEST_ABC_DIR "declaration-string.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-boolean.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-enum-number.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-enum-string.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-number.abc";
static const std::string FILE_DECLARATION_USAGE_3D_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-3d-array-string.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-boolean.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-enum-number.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-enum-string.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-number.abc";
static const std::string FILE_DECLARATION_USAGE_ARRAY_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-array-string.abc";
static const std::string FILE_DECLARATION_USAGE_BOOLEAN =
    GRAPH_TEST_ABC_DIR "declaration-usage-boolean.abc";
static const std::string FILE_DECLARATION_USAGE_COMBINATION =
    GRAPH_TEST_ABC_DIR "declaration-usage-combination.abc";
static const std::string FILE_DECLARATION_USAGE_EMPTY =
    GRAPH_TEST_ABC_DIR "declaration-usage-empty.abc";
static const std::string FILE_DECLARATION_USAGE_ENUM_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-enum-number.abc";
static const std::string FILE_DECLARATION_USAGE_ENUM_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-enum-string.abc";
static const std::string FILE_DECLARATION_USAGE_NUMBER =
    GRAPH_TEST_ABC_DIR "declaration-usage-number.abc";
static const std::string FILE_DECLARATION_USAGE_STRING =
    GRAPH_TEST_ABC_DIR "declaration-usage-string.abc";
static const std::string FILE_EXPORT =
    GRAPH_TEST_ABC_DIR "export.abc";
static const std::string FILE_IMPORT_QUALIFIED =
    GRAPH_TEST_ABC_DIR "import-qualified.abc";
static const std::string FILE_IMPORT_UNQUALIFIED =
    GRAPH_TEST_ABC_DIR "import-unqualified.abc";
static const std::string FILE_MULTIPLE_ANNOTATIONS =
    GRAPH_TEST_ABC_DIR "multiple-annotations.abc";

class DisassemblerUserAnnotationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        disasm = new panda::disasm::Disassembler{};
    };

    void TearDown()
    {
        delete disasm;
        disasm = nullptr;
    };

    void ValidateMethodAnnotation(const std::string &method_name, const std::string &anno_name,
                                  std::string anno_expected)
    {
        auto anno = disasm->GetSerializedMethodAnnotation(method_name, anno_name);
        ASSERT_NE(anno, std::nullopt);
        std::string anno_str = anno.value();
        // remove whitespaces from both strings
        auto anno_str_rem_iter = std::remove_if(anno_str.begin(), anno_str.end(),
                                                [](unsigned char x) { return std::isspace(x); });
        anno_str.erase(anno_str_rem_iter, anno_str.end());
        auto anno_expected_rem_iter = std::remove_if(anno_expected.begin(), anno_expected.end(),
                                                     [](unsigned char x) { return std::isspace(x); });
        anno_expected.erase(anno_expected_rem_iter, anno_expected.end());
        ASSERT_EQ(anno_str, anno_expected);
    }

    void ValidateRecord(const std::string &record_name, std::string record_expected)
    {
        auto record = disasm->GetSerializedRecord(record_name);
        ASSERT_NE(record, std::nullopt);
        std::string record_str = record.value();
        // remove whitespaces from both strings
        auto record_str_rem_iter = std::remove_if(record_str.begin(), record_str.end(),
                                                  [](unsigned char x) { return std::isspace(x); });
        record_str.erase(record_str_rem_iter, record_str.end());
        auto record_expected_rem_iter = std::remove_if(record_expected.begin(), record_expected.end(),
                                                       [](unsigned char x) { return std::isspace(x); });
        record_expected.erase(record_expected_rem_iter, record_expected.end());
        ASSERT_EQ(record_str, record_expected);
    }

    void InitDisasm(std::string file_path)
    {
        disasm->Disassemble(file_path, false, false);
    }

private:
    panda::disasm::Disassembler *disasm = nullptr;
};

/**
* @tc.name: test_declaration_3d_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_BOOLEAN);

    ValidateRecord("declaration-3d-array-boolean.Anno1", R"(
        .language ECMAScript.record declaration-3d-array-boolean.Anno1 {
            u1[][][] a
        }
    )");

    ValidateRecord("declaration-3d-array-boolean.Anno2", R"(
        .language ECMAScript.record declaration-3d-array-boolean.Anno2 {
            u1[][][] a = [[[]]]
        }
    )");

    ValidateRecord("declaration-3d-array-boolean.Anno3", R"(
        .language ECMAScript.record declaration-3d-array-boolean.Anno3 {
            u1[][][] a = [[[1, 0], [1, 0]]]
        }
    )");
}

/**
* @tc.name: test_declaration_3d_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_ENUM_NUMBER);

    ValidateRecord("declaration-3d-array-enum-number.Anno1", R"(
        .language ECMAScript.record declaration-3d-array-enum-number.Anno1 {
            f64[][][] a
        }
    )");

    ValidateRecord("declaration-3d-array-enum-number.Anno2", R"(
        .language ECMAScript.record declaration-3d-array-enum-number.Anno2 {
            f64[][][] a = [[[]]]
        }
    )");

    ValidateRecord("declaration-3d-array-enum-number.Anno3", R"(
        .language ECMAScript.record declaration-3d-array-enum-number.Anno3 {
            f64[][][] a = [[[42, -314, 42]]]
        }
    )");
}

/**
* @tc.name: test_declaration_3d_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_ENUM_STRING);

    ValidateRecord("declaration-3d-array-enum-string.Anno1", R"(
        .language ECMAScript.record declaration-3d-array-enum-string.Anno1 {
            panda.String[][][] a
        }
    )");

    ValidateRecord("declaration-3d-array-enum-string.Anno2", R"(
        .language ECMAScript.record declaration-3d-array-enum-string.Anno2 {
            panda.String[][][] a = [[[]]]
        }
    )");

    ValidateRecord("declaration-3d-array-enum-string.Anno3", R"(
        .language ECMAScript.record declaration-3d-array-enum-string.Anno3 {
            panda.String[][][] a = [[["Hello", "world!", "Hello"]]]
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_3d_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_NUMBER);

    ValidateRecord("declaration-3d-array-number.Anno1", R"(
        .language ECMAScript.record declaration-3d-array-number.Anno1 {
            f64[][][] a
        }
    )");

    ValidateRecord("declaration-3d-array-number.Anno2", R"(
        .language ECMAScript.record declaration-3d-array-number.Anno2 {
            f64[][][] a = [[[]]]
        }
    )");

    ValidateRecord("declaration-3d-array-number.Anno3", R"(
        .language ECMAScript.record declaration-3d-array-number.Anno3 {
            f64[][][] a = [[[1, -2, 3], [4, -5, 6]]]
        }
    )");
}

/**
* @tc.name: test_declaration_3d_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_3d_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_3D_ARRAY_STRING);

    ValidateRecord("declaration-3d-array-string.Anno1", R"(
        .language ECMAScript.record declaration-3d-array-string.Anno1 {
            panda.String[][][] a
        }
    )");

    ValidateRecord("declaration-3d-array-string.Anno2", R"(
        .language ECMAScript.record declaration-3d-array-string.Anno2 {
            panda.String[][][] a = [[[]]]
        }
    )");

    ValidateRecord("declaration-3d-array-string.Anno3", R"(
        .language ECMAScript.record declaration-3d-array-string.Anno3 {
            panda.String[][][] a = [[["hello", "world"], ["hello", "world"]]]
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_BOOLEAN);

    ValidateRecord("declaration-array-boolean.Anno1", R"(
        .language ECMAScript.record declaration-array-boolean.Anno1 {
            u1[] a
        }
    )");

    ValidateRecord("declaration-array-boolean.Anno2", R"(
        .language ECMAScript.record declaration-array-boolean.Anno2 {
            u1[] a = []
        }
    )");

    ValidateRecord("declaration-array-boolean.Anno3", R"(
        .language ECMAScript.record declaration-array-boolean.Anno3 {
            u1[] a = [1, 0, 1]
        }
    )");
}

/**
* @tc.name: test_declaration_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_ENUM_NUMBER);

    ValidateRecord("declaration-array-enum-number.Anno1", R"(
        .language ECMAScript.record declaration-array-enum-number.Anno1 {
            f64[] a
        }
    )");

    ValidateRecord("declaration-array-enum-number.Anno2", R"(
        .language ECMAScript.record declaration-array-enum-number.Anno2 {
            f64[] a = []
        }
    )");

    ValidateRecord("declaration-array-enum-number.Anno3", R"(
        .language ECMAScript.record declaration-array-enum-number.Anno3 {
            f64[] a = [42, -314, 42]
        }
    )");
}

/**
* @tc.name: test_declaration_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_ENUM_STRING);

    ValidateRecord("declaration-array-enum-string.Anno1", R"(
        .language ECMAScript.record declaration-array-enum-string.Anno1 {
            panda.String[] a
        }
    )");

    ValidateRecord("declaration-array-enum-string.Anno2", R"(
        .language ECMAScript.record declaration-array-enum-string.Anno2 {
            panda.String[] a = []
        }
    )");

    ValidateRecord("declaration-array-enum-string.Anno3", R"(
        .language ECMAScript.record declaration-array-enum-string.Anno3 {
            panda.String[] a = ["Hello", "world!", "Hello"]
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_NUMBER);

    ValidateRecord("declaration-array-number.Anno1", R"(
        .language ECMAScript.record declaration-array-number.Anno1 {
            f64[] a
        }
    )");

    ValidateRecord("declaration-array-number.Anno2", R"(
        .language ECMAScript.record declaration-array-number.Anno2 {
            f64[] a = []
        }
    )");

    ValidateRecord("declaration-array-number.Anno3", R"(
        .language ECMAScript.record declaration-array-number.Anno3 {
            f64[] a = [1, -2, 3]
        }
    )");
}

/**
* @tc.name: test_declaration_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ARRAY_STRING);

    ValidateRecord("declaration-array-string.Anno1", R"(
        .language ECMAScript.record declaration-array-string.Anno1 {
            panda.String[] a
        }
    )");

    ValidateRecord("declaration-array-string.Anno2", R"(
        .language ECMAScript.record declaration-array-string.Anno2 {
            panda.String[] a = []
        }
    )");

    ValidateRecord("declaration-array-string.Anno3", R"(
        .language ECMAScript.record declaration-array-string.Anno3 {
            panda.String[] a = ["Hello", "world", "!"]
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_BOOLEAN);

    ValidateRecord("declaration-boolean.Anno1", R"(
        .language ECMAScript.record declaration-boolean.Anno1 {
            u1 a
        }
    )");

    ValidateRecord("declaration-boolean.Anno2", R"(
        .language ECMAScript.record declaration-boolean.Anno2 {
            u1 a = 1
        }
    )");
}

/**
* @tc.name: test_declaration_combination
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_combination, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_COMBINATION);

    ValidateRecord("declaration-combination.Anno", R"(
        .language ECMAScript.record declaration-combination.Anno {
            f64 a
            f64[] b = [13, -10]
            panda.String c
            u1 d
            f64[] e = [1, -2, 3]
            f64[] f
            f64 h
            f64[][][] i
            panda.String j
            panda.String[][][] k
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_empty
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_empty, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_EMPTY);

    ValidateRecord("declaration-empty.Anno", R"(
        .language ECMAScript.record declaration-empty.Anno {
        }
    )");
}

/**
* @tc.name: test_declaration_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ENUM_NUMBER);

    ValidateRecord("declaration-enum-number.Anno1", R"(
        .language ECMAScript.record declaration-enum-number.Anno1 {
            f64 a
        }
    )");

    ValidateRecord("declaration-enum-number.Anno2", R"(
        .language ECMAScript.record declaration-enum-number.Anno2 {
            f64 a = 42
        }
    )");
}

/**
* @tc.name: test_declaration_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_ENUM_STRING);

    ValidateRecord("declaration-enum-string.Anno1", R"(
        .language ECMAScript.record declaration-enum-string.Anno1 {
            panda.String a
        }
    )");

    ValidateRecord("declaration-enum-string.Anno2", R"(
        .language ECMAScript.record declaration-enum-string.Anno2 {
            panda.String a = "Hello"
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_NUMBER);

    ValidateRecord("declaration-number.Anno1", R"(
        .language ECMAScript.record declaration-number.Anno1 {
            f64 a
        }
    )");

    ValidateRecord("declaration-number.Anno2", R"(
        .language ECMAScript.record declaration-number.Anno2 {
            f64 a = 42
        }
    )");

    ValidateRecord("declaration-number.Anno3", R"(
        .language ECMAScript.record declaration-number.Anno3 {
            f64 a = -314
        }
    )");
}

/**
* @tc.name: test_declaration_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_STRING);

    ValidateRecord("declaration-string.Anno1", R"(
        .language ECMAScript.record declaration-string.Anno1 {
            panda.String a
        }
    )");

    ValidateRecord("declaration-string.Anno2", R"(
        .language ECMAScript.record declaration-string.Anno2 {
            panda.String a = "Hello world!"
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_BOOLEAN);

    ValidateRecord("declaration-usage-3d-array-boolean.Anno", R"(
        .language ECMAScript.record declaration-usage-3d-array-boolean.Anno {
            u1[][][] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-boolean.#~A=#A", "Ldeclaration-usage-3d-array-boolean.Anno",
                             R"(
        Ldeclaration-usage-3d-array-boolean.Anno:
            u1[][][] a { [[[1, 0, 1]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-boolean.#~A>#foo",
                             "Ldeclaration-usage-3d-array-boolean.Anno", R"(
        Ldeclaration-usage-3d-array-boolean.Anno:
            u1[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_NUMBER);

    ValidateRecord("declaration-usage-3d-array-enum-number.Anno", R"(
        .language ECMAScript.record declaration-usage-3d-array-enum-number.Anno {
            f64[][][] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-number.#~A=#A",
                             "Ldeclaration-usage-3d-array-enum-number.Anno", R"(
        Ldeclaration-usage-3d-array-enum-number.Anno:
            f64[][][] a { [[[1, -2, 1]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-number.#~A>#foo",
                             "Ldeclaration-usage-3d-array-enum-number.Anno", R"(
        Ldeclaration-usage-3d-array-enum-number.Anno:
            f64[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_ENUM_STRING);

    ValidateRecord("declaration-usage-3d-array-enum-string.Anno", R"(
        .language ECMAScript.record declaration-usage-3d-array-enum-string.Anno {
            panda.String[][][] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-string.#~A=#A",
                             "Ldeclaration-usage-3d-array-enum-string.Anno", R"(
        Ldeclaration-usage-3d-array-enum-string.Anno:
            panda.String[][][] a { [[["Hello", "world", "Hello"]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-enum-string.#~A>#foo",
                             "Ldeclaration-usage-3d-array-enum-string.Anno", R"(
        Ldeclaration-usage-3d-array-enum-string.Anno:
            panda.String[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_NUMBER);

    ValidateRecord("declaration-usage-3d-array-number.Anno", R"(
        .language ECMAScript.record declaration-usage-3d-array-number.Anno {
            f64[][][] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-number.#~A=#A",
                             "Ldeclaration-usage-3d-array-number.Anno", R"(
        Ldeclaration-usage-3d-array-number.Anno:
            f64[][][] a { [[[1, -2, 3]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-number.#~A>#foo",
                             "Ldeclaration-usage-3d-array-number.Anno", R"(
        Ldeclaration-usage-3d-array-number.Anno:
            f64[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_3d_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_3d_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_3D_ARRAY_STRING);

    ValidateRecord("declaration-usage-3d-array-string.Anno", R"(
        .language ECMAScript.record declaration-usage-3d-array-string.Anno {
            panda.String[][][] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-string.#~A=#A",
                             "Ldeclaration-usage-3d-array-string.Anno", R"(
        Ldeclaration-usage-3d-array-string.Anno:
            panda.String[][][] a { [[["Hello", "world", "!"]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-3d-array-string.#~A>#foo",
                             "Ldeclaration-usage-3d-array-string.Anno", R"(
        Ldeclaration-usage-3d-array-string.Anno:
            panda.String[][][] a { [[[]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_BOOLEAN);

    ValidateRecord("declaration-usage-array-boolean.Anno", R"(
        .language ECMAScript.record declaration-usage-array-boolean.Anno {
            u1[] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-array-boolean.#~A=#A",
                             "Ldeclaration-usage-array-boolean.Anno", R"(
        Ldeclaration-usage-array-boolean.Anno:
            u1[] a { [1, 0, 1] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-boolean.#~A>#foo",
                             "Ldeclaration-usage-array-boolean.Anno", R"(
        Ldeclaration-usage-array-boolean.Anno:
            u1[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_ENUM_NUMBER);

    ValidateRecord("declaration-usage-array-enum-number.Anno", R"(
        .language ECMAScript.record declaration-usage-array-enum-number.Anno {
            f64[] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-number.#~A=#A",
                             "Ldeclaration-usage-array-enum-number.Anno", R"(
        Ldeclaration-usage-array-enum-number.Anno:
            f64[] a { [1, -2, 1] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-number.#~A>#foo",
                             "Ldeclaration-usage-array-enum-number.Anno", R"(
        Ldeclaration-usage-array-enum-number.Anno:
            f64[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_ENUM_STRING);

    ValidateRecord("declaration-usage-array-enum-string.Anno", R"(
        .language ECMAScript.record declaration-usage-array-enum-string.Anno {
            panda.String[] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-string.#~A=#A",
                             "Ldeclaration-usage-array-enum-string.Anno", R"(
        Ldeclaration-usage-array-enum-string.Anno:
            panda.String[] a { ["Hello", "world", "Hello"] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-enum-string.#~A>#foo",
                             "Ldeclaration-usage-array-enum-string.Anno", R"(
        Ldeclaration-usage-array-enum-string.Anno:
            panda.String[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_NUMBER);

    ValidateRecord("declaration-usage-array-number.Anno", R"(
        .language ECMAScript.record declaration-usage-array-number.Anno {
            f64[] a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-array-number.#~A=#A", "Ldeclaration-usage-array-number.Anno", R"(
        Ldeclaration-usage-array-number.Anno:
            f64[] a { [1, -2, 3] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-number.#~A>#foo", "Ldeclaration-usage-array-number.Anno", R"(
        Ldeclaration-usage-array-number.Anno:
            f64[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_array_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_array_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ARRAY_STRING);

    ValidateRecord("declaration-usage-array-string.Anno", R"(
        .language ECMAScript.record declaration-usage-array-string.Anno {
            panda.String[] a
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-array-string.#~A=#A", "Ldeclaration-usage-array-string.Anno", R"(
        Ldeclaration-usage-array-string.Anno:
            panda.String[] a { ["hello", "world", "!"] }
    )");

    ValidateMethodAnnotation("declaration-usage-array-string.#~A>#foo", "Ldeclaration-usage-array-string.Anno", R"(
        Ldeclaration-usage-array-string.Anno:
            panda.String[] a { [] }
    )");
}

/**
* @tc.name: test_declaration_usage_boolean
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_boolean, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_BOOLEAN);

    ValidateRecord("declaration-usage-boolean.Anno", R"(
        .language ECMAScript.record declaration-usage-boolean.Anno {
            u1 a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-boolean.#~A=#A", "Ldeclaration-usage-boolean.Anno", R"(
        Ldeclaration-usage-boolean.Anno:
            u1 a { 1 }
    )");

    ValidateMethodAnnotation("declaration-usage-boolean.#~A>#foo", "Ldeclaration-usage-boolean.Anno", R"(
        Ldeclaration-usage-boolean.Anno:
            u1 a { 0 }
    )");
}

/**
* @tc.name: test_declaration_usage_combination
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_combination, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_COMBINATION);

    ValidateRecord("declaration-usage-combination.Anno", R"(
        .language ECMAScript.record declaration-usage-combination.Anno {
            f64 a
            f64[] b = [13, -10]
            panda.String c
            u1 d
            f64[] e = [1, -2, 3]
            f64[] f
            f64 h
            f64[][][] i
            panda.String j
            panda.String[][][] k
        }
    )");

    ValidateMethodAnnotation("declaration-usage-combination.#~A=#A", "Ldeclaration-usage-combination.Anno", R"(
        Ldeclaration-usage-combination.Anno:
            f64 a { 20 }
            f64[] b { [-13, 10] }
            panda.String c { "ab" }
            u1 d { 1 }
            f64[] e { [-1, 2, 3] }
            f64[] f { [] }
            f64[][][] g { [[[0]]] }
            f64 h { 10 }
            f64[][][] i { [[[]]] }
            panda.String j { "B" }
            panda.String[][][] k { [[[]]] }
    )");

    ValidateMethodAnnotation("declaration-usage-combination.#~A>#foo", "Ldeclaration-usage-combination.Anno", R"(
        Ldeclaration-usage-combination.Anno:
            f64 a { -10 }
            f64[] b { [1, 2, -3] }
            panda.String c { "cde" }
            u1 d { 1 }
            f64[] e { [1, -2, 3] }
            f64[] f { [1] }
            f64[][][] g { [[[0], [1]]] }
            f64 h { -10 }
            f64[][][] i { [[[10], [20]]] }
            panda.String j { "B" }
            panda.String[][][] k { [[["A"], ["B"]]] }
    )");
}

/**
* @tc.name: test_declaration_usage_empty
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_empty, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_EMPTY);

    ValidateRecord("declaration-usage-empty.Anno", R"(
        .language ECMAScript.record declaration-usage-empty.Anno {
        }
    )");

    ValidateMethodAnnotation("declaration-usage-empty.#~A=#A", "Ldeclaration-usage-empty.Anno", R"(
        Ldeclaration-usage-empty.Anno:
    )");

    ValidateMethodAnnotation("declaration-usage-empty.#~A>#foo", "Ldeclaration-usage-empty.Anno", R"(
        Ldeclaration-usage-empty.Anno:
    )");

    ValidateMethodAnnotation("declaration-usage-empty.#~A>#bar", "Ldeclaration-usage-empty.Anno", R"(
        Ldeclaration-usage-empty.Anno:
    )");
}

/**
* @tc.name: test_declaration_usage_enum_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_enum_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ENUM_NUMBER);

    ValidateRecord("declaration-usage-enum-number.Anno", R"(
        .language ECMAScript.record declaration-usage-enum-number.Anno {
            f64 a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-enum-number.#~A=#A", "Ldeclaration-usage-enum-number.Anno", R"(
        Ldeclaration-usage-enum-number.Anno:
            f64 a { 1 }
    )");

    ValidateMethodAnnotation("declaration-usage-enum-number.#~A>#foo", "Ldeclaration-usage-enum-number.Anno", R"(
        Ldeclaration-usage-enum-number.Anno:
            f64 a { -2 }
    )");
}

/**
* @tc.name: test_declaration_usage_enum_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_enum_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_ENUM_STRING);

    ValidateRecord("declaration-usage-enum-string.Anno", R"(
        .language ECMAScript.record declaration-usage-enum-string.Anno {
            panda.String a
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-enum-string.#~A=#A", "Ldeclaration-usage-enum-string.Anno", R"(
        Ldeclaration-usage-enum-string.Anno:
            panda.String a { "Hello" }
    )");

    ValidateMethodAnnotation("declaration-usage-enum-string.#~A>#foo", "Ldeclaration-usage-enum-string.Anno", R"(
        Ldeclaration-usage-enum-string.Anno:
            panda.String a { "world" }
    )");
}

/**
* @tc.name: test_declaration_usage_number
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_number, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_NUMBER);

    ValidateRecord("declaration-usage-number.Anno", R"(
        .language ECMAScript.record declaration-usage-number.Anno {
            f64 a
        }
    )");

    ValidateMethodAnnotation("declaration-usage-number.#~A=#A", "Ldeclaration-usage-number.Anno", R"(
        Ldeclaration-usage-number.Anno:
            f64 a { 42 }
    )");

    ValidateMethodAnnotation("declaration-usage-number.#~A>#foo", "Ldeclaration-usage-number.Anno", R"(
        Ldeclaration-usage-number.Anno:
            f64 a { -314 }
    )");
}

/**
* @tc.name: test_declaration_usage_string
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_declaration_usage_string, TestSize.Level1)
{
    InitDisasm(FILE_DECLARATION_USAGE_STRING);

    ValidateRecord("declaration-usage-string.Anno", R"(
        .language ECMAScript.record declaration-usage-string.Anno {
            panda.String a
        }
    )");

    ValidateRecord("panda.String", R"(
        .language ECMAScript.record panda.String <external>
    )");

    ValidateMethodAnnotation("declaration-usage-string.#~A=#A", "Ldeclaration-usage-string.Anno", R"(
        Ldeclaration-usage-string.Anno:
            panda.String a { "Hello" }
    )");

    ValidateMethodAnnotation("declaration-usage-string.#~A>#foo", "Ldeclaration-usage-string.Anno", R"(
        Ldeclaration-usage-string.Anno:
            panda.String a { "world" }
    )");
}

/**
* @tc.name: test_export
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_export, TestSize.Level1)
{
    InitDisasm(FILE_EXPORT);

    ValidateRecord("export.Anno1", R"(
        .language ECMAScript.record export.Anno1 {
        }
    )");

    ValidateRecord("export.Anno2", R"(
        .language ECMAScript.record export.Anno2 {
            f64 a = 0
        }
    )");

    ValidateMethodAnnotation("export.#~A=#A", "Lexport.Anno1", R"(
        Lexport.Anno1:
    )");

    ValidateMethodAnnotation("export.#~B=#B", "Lexport.Anno1", R"(
        Lexport.Anno1:
    )");
}

/**
* @tc.name: test_import_qualified
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_import_qualified, TestSize.Level1)
{
    InitDisasm(FILE_IMPORT_QUALIFIED);

    ValidateRecord("import-qualified.Namespace1.Anno", R"(
        .language ECMAScript.record import-qualified.Namespace1.Anno <external>
    )");

    ValidateRecord("import-qualified.Namespace1.Anno1", R"(
        .language ECMAScript.record import-qualified.Namespace1.Anno1 <external>
    )");

    ValidateRecord("import-qualified.Namespace1.Namespace2.Namespace3.Anno2", R"(
        .language ECMAScript.record import-qualified.Namespace1.Namespace2.Namespace3.Anno2 <external>
    )");

    ValidateRecord("import-qualified.Namespace1.Namespace2.Namespace3.Anno3", R"(
        .language ECMAScript.record import-qualified.Namespace1.Namespace2.Namespace3.Anno3 <external>
    )");

    ValidateMethodAnnotation("import-qualified.#~A=#A", "Limport-qualified.Namespace1.Anno", R"(
        Limport-qualified.Namespace1.Anno:
    )");

    ValidateMethodAnnotation("import-qualified.#~B=#B", "Limport-qualified.Namespace1.Anno", R"(
        Limport-qualified.Namespace1.Anno:
    )");

    ValidateMethodAnnotation("import-qualified.#~C=#C", "Limport-qualified.Namespace1.Anno1", R"(
        Limport-qualified.Namespace1.Anno1:
            f64 a { 1 }
            panda.String b { "string" }
    )");

    ValidateMethodAnnotation("import-qualified.#~D=#D", "Limport-qualified.Namespace1.Namespace2.Namespace3.Anno2",
                             R"(
        Limport-qualified.Namespace1.Namespace2.Namespace3.Anno2:
    )");

    ValidateMethodAnnotation("import-qualified.#~E=#E", "Limport-qualified.Namespace1.Namespace2.Namespace3.Anno2",
                             R"(
        Limport-qualified.Namespace1.Namespace2.Namespace3.Anno2:
    )");
}

/**
* @tc.name: test_import_unqualified
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_import_unqualified, TestSize.Level1)
{
    InitDisasm(FILE_IMPORT_UNQUALIFIED);

    ValidateRecord("import-unqualified.Anno1", R"(
        .language ECMAScript.record import-unqualified.Anno1 <external>
    )");

    ValidateRecord("import-unqualified.Anno2", R"(
        .language ECMAScript.record import-unqualified.Anno2 <external>
    )");

    ValidateMethodAnnotation("import-unqualified.#~A=#A", "Limport-unqualified.Anno1", R"(
        Limport-unqualified.Anno1:
    )");

    ValidateMethodAnnotation("import-unqualified.#~B=#B", "Limport-unqualified.Anno1", R"(
        Limport-unqualified.Anno1:
    )");

    ValidateMethodAnnotation("import-unqualified.#~C=#C", "Limport-unqualified.Anno1", R"(
        Limport-unqualified.Anno1:
    )");

    ValidateMethodAnnotation("import-unqualified.#~D=#D", "Limport-unqualified.Anno2", R"(
        Limport-unqualified.Anno2:
            f64 a { 1 }
            panda.String b { "string" }
    )");
}

/**
* @tc.name: test_multiple_annotations
* @tc.desc: validate annotation usage
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerUserAnnotationTest, test_multiple_annotations, TestSize.Level1)
{
    InitDisasm(FILE_MULTIPLE_ANNOTATIONS);

    ValidateRecord("multiple-annotations.Anno1", R"(
        .language ECMAScript.record multiple-annotations.Anno1 {
            f64 a = 1
        }
    )");

    ValidateRecord("multiple-annotations.Anno2", R"(
        .language ECMAScript.record multiple-annotations.Anno2 {
            panda.String b = "string"
        }
    )");

    ValidateRecord("multiple-annotations.Anno3", R"(
        .language ECMAScript.record multiple-annotations.Anno3 {
            u1[] c = [1, 0]
        }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A=#A", "Lmultiple-annotations.Anno1", R"(
        Lmultiple-annotations.Anno1:
            f64 a { 42 }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A=#A", "Lmultiple-annotations.Anno2", R"(
        Lmultiple-annotations.Anno2:
            panda.String b { "abc" }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A=#A", "Lmultiple-annotations.Anno3", R"(
        Lmultiple-annotations.Anno3:
            u1[] c { [0, 1] }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A>#foo", "Lmultiple-annotations.Anno1", R"(
        Lmultiple-annotations.Anno1:
            f64 a { 42 }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A>#foo", "Lmultiple-annotations.Anno2", R"(
        Lmultiple-annotations.Anno2:
            panda.String b { "abc" }
    )");

    ValidateMethodAnnotation("multiple-annotations.#~A>#foo", "Lmultiple-annotations.Anno3", R"(
        Lmultiple-annotations.Anno3:
            u1[] c { [0, 1] }
    )");
}

};
