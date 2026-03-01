/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <cstdlib>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <vector>
#include "abc2program_driver.h"

namespace ark::abc2program {

constexpr std::string_view FIELD_ABC_TEST_FILE_NAME = "Field.abc";
constexpr std::string_view FUNCTIONS_ABC_TEST_FILE_NAME = "Functions.abc";
constexpr std::string_view FUNCTIONS_CONCAT_ABC_TEST_FILE_NAME = "Functions_concat.abc";
constexpr std::string_view HELLO_WORLD_ABC_TEST_FILE_NAME = "HelloWorld.abc";
constexpr std::string_view INS_ABC_TEST_FILE_NAME = "Ins.abc";
constexpr std::string_view LITERAL_ARRAY_ABC_TEST_FILE_NAME = "LiteralArray.abc";
constexpr std::string_view METADATA_ABC_TEST_FILE_NAME = "Metadata.abc";
constexpr std::string_view RECORD_ABC_TEST_FILE_NAME = "Record.abc";
constexpr std::string_view TYPE_ABC_TEST_FILE_NAME = "Type.abc";
constexpr size_t ANNOTATION_ID_SIZE = 1;
constexpr size_t ANNOTATION_ID_PREFIX_SIZE = 3;

class Abc2ProgramTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void TearDown() override {}

    bool ValidateAnnotationId(const std::vector<std::string> &id)
    {
        if (id.size() != ANNOTATION_ID_SIZE) {
            return false;
        }
        return id[0].size() > ANNOTATION_ID_PREFIX_SIZE && id[0].substr(0, ANNOTATION_ID_PREFIX_SIZE) == "id_";
    }

    bool ValidateAttributes(const std::unordered_map<std::string, std::vector<std::string>> &attributes,
                            const std::unordered_map<std::string, std::vector<std::string>> &expectedAttributes)
    {
        for (const auto &[key, values] : attributes) {
            if (key == "ets.annotation.id") {
                if (!ValidateAnnotationId(values)) {
                    return false;
                }
                continue;
            }
            const auto expectValuesIter = expectedAttributes.find(key);
            if (expectValuesIter == expectedAttributes.end()) {
                return false;
            }
            EXPECT_THAT(values, ::testing::ContainerEq(expectValuesIter->second));
        }
        return true;
    }

    Abc2ProgramDriver driver;                // NOLINT(misc-non-private-member-variables-in-classes)
    const pandasm::Program *prog = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

class Abc2ProgramHelloWorldTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(HELLO_WORLD_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramFunctionsTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(FUNCTIONS_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramFunctionsConcatTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(FUNCTIONS_CONCAT_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramLiteralArrayTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(LITERAL_ARRAY_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramTypeTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(TYPE_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramRecordTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(RECORD_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramInsTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(INS_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramFieldTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(FIELD_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramMetadataTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(METADATA_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

TEST_F(Abc2ProgramHelloWorldTest, Lang)
{
    EXPECT_EQ(prog->lang, panda_file::SourceLang::ETS);
}

TEST_F(Abc2ProgramHelloWorldTest, RecordTable)
{
    using namespace std::literals;
    const std::array expectedRecordNames {
        "HelloWorld.HelloWorld"s,
        "HelloWorld.ETSGLOBAL"s,
    };
    std::set<std::string> recordNames;
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.second.language, panda_file::SourceLang::ETS);
        EXPECT_EQ(it.first, it.second.name);
        recordNames.insert(it.first);
    }
    EXPECT_THAT(expectedRecordNames, ::testing::IsSubsetOf(recordNames));
}

TEST_F(Abc2ProgramHelloWorldTest, Functions)
{
    const std::set<std::string> expectedFunctions = {
        "HelloWorld.ETSGLOBAL._cctor_:void;",
        "HelloWorld.ETSGLOBAL.main:void;",
        "HelloWorld.HelloWorld._ctor_:HelloWorld.HelloWorld;void;",
        "HelloWorld.HelloWorld.bar:HelloWorld.HelloWorld;std.core.Object;std.core.Object;",
        "HelloWorld.HelloWorld.foo:HelloWorld.HelloWorld;i32;i32;",
        "std.core.Console.log:std.core.Console;i32;void;",
        "std.core.Object._ctor_:std.core.Object;void;",
    };
    std::set<std::string> existingFunctions {};
    for (auto &it : prog->functionStaticTable) {
        existingFunctions.insert(it.first);
    }
    for (auto &it : prog->functionInstanceTable) {
        existingFunctions.insert(it.first);
    }
    EXPECT_EQ(existingFunctions, expectedFunctions);
}

TEST_F(Abc2ProgramHelloWorldTest, Fields)
{
    for (const auto &it : prog->recordTable) {
        if (it.first == "HelloWorld") {
            EXPECT_EQ(it.second.fieldList.size(), 2U);
        }
    }
}

TEST_F(Abc2ProgramHelloWorldTest, StringTable)
{
    const std::set<std::string> expectedStrings = {R"(HelloWorld)", R"(NewLine\n)"};
    EXPECT_EQ(prog->strings, expectedStrings);
}

TEST_F(Abc2ProgramFunctionsTest, Lang)
{
    EXPECT_EQ(prog->lang, panda_file::SourceLang::ETS);
}

TEST_F(Abc2ProgramFunctionsTest, RecordTable)
{
    using namespace std::literals;
    const std::array expectedRecordNames = {
        "Functions.ETSGLOBAL"s,
        "Functions.Cls"s,
        "Functions.%%lambda-lambda_invoke-0"s,
        "Functions.%%lambda-lambda_invoke-1"s,
    };
    panda_file::SourceLang expectedLang = panda_file::SourceLang::ETS;
    std::vector<std::string> recordNames;
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.second.language == expectedLang);
        EXPECT_TRUE(it.first == it.second.name);
        recordNames.emplace_back(it.first);
    }
    EXPECT_THAT(recordNames, ::testing::IsSupersetOf(expectedRecordNames));
}

TEST_F(Abc2ProgramFunctionsTest, Functions)
{
    const std::string prefix = "Functions.%%lambda-lambda_invoke";
    const std::set<std::string> expectedFunctions = {
        "Functions.Cls._ctor_:Functions.Cls;void;",
        "Functions.Cls.func_f:Functions.Cls;i32;i32;",
        "Functions.Cls.func_i:Functions.Cls;i32;i32;",
        "Functions.Cls.func_s:i32;i32;",
        "Functions.ETSGLOBAL._cctor_:void;",
        "Functions.ETSGLOBAL.bar:f64;std.core.String;",
        "Functions.ETSGLOBAL.foo:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.lambda_invoke-0:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.lambda_invoke-1:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.main:void;",
        prefix + "-0.$_invoke:Functions.%%lambda-lambda_invoke-0;std.core.String;std.core.String;",
        prefix + "-0._ctor_:Functions.%%lambda-lambda_invoke-0;void;",
        prefix + "-0.invoke1:Functions.%%lambda-lambda_invoke-0;std.core.Object;std.core.Object;",
        prefix + "-1.$_invoke:Functions.%%lambda-lambda_invoke-1;std.core.String;std.core.String;",
        prefix + "-1._ctor_:Functions.%%lambda-lambda_invoke-1;void;",
        prefix + "-1.invoke1:Functions.%%lambda-lambda_invoke-1;std.core.Object;std.core.Object;",
        "std.core.Lambda1._ctor_:std.core.Lambda1;void;",
        "std.core.Object._ctor_:std.core.Object;void;",
        "std.core.Runtime.failedTypeCastExclUndefinedStub:std.core.Object;std.core.Class;std.core.ClassCastError;",
        "std.core.StringBuilder._ctor_:std.core.StringBuilder;std.core.String;void;",
        "std.core.StringBuilder.append:std.core.StringBuilder;f64;std.core.StringBuilder;",
        "std.core.StringBuilder.append:std.core.StringBuilder;std.core.String;std.core.StringBuilder;",
        "std.core.StringBuilder.toString:std.core.StringBuilder;std.core.String;",
    };
    std::set<std::string> existingFunctions {};
    for (auto &it : prog->functionStaticTable) {
        existingFunctions.insert(it.first);
    }
    for (auto &it : prog->functionInstanceTable) {
        existingFunctions.insert(it.first);
    }
    EXPECT_THAT(existingFunctions, ::testing::ContainerEq(expectedFunctions));
}

TEST_F(Abc2ProgramFunctionsConcatTest, Functions)
{
    const std::string prefix = "Functions.%%lambda-lambda_invoke";
    const std::set<std::string> expectedFunctions = {
        "Functions.Cls._ctor_:Functions.Cls;void;",
        "Functions.Cls.func_f:Functions.Cls;i32;i32;",
        "Functions.Cls.func_i:Functions.Cls;i32;i32;",
        "Functions.Cls.func_s:i32;i32;",
        "Functions.ETSGLOBAL._cctor_:void;",
        "Functions.ETSGLOBAL.bar:f64;std.core.String;",
        "Functions.ETSGLOBAL.foo:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.lambda_invoke-0:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.lambda_invoke-1:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.main:void;",
        prefix + "-0.$_invoke:Functions.%%lambda-lambda_invoke-0;std.core.String;std.core.String;",
        prefix + "-0._ctor_:Functions.%%lambda-lambda_invoke-0;void;",
        prefix + "-0.invoke1:Functions.%%lambda-lambda_invoke-0;std.core.Object;std.core.Object;",
        prefix + "-1.$_invoke:Functions.%%lambda-lambda_invoke-1;std.core.String;std.core.String;",
        prefix + "-1._ctor_:Functions.%%lambda-lambda_invoke-1;void;",
        prefix + "-1.invoke1:Functions.%%lambda-lambda_invoke-1;std.core.Object;std.core.Object;",
        "std.core.Lambda1._ctor_:std.core.Lambda1;void;",
        "std.core.Object._ctor_:std.core.Object;void;",
        "std.core.Runtime.failedTypeCastExclUndefinedStub:std.core.Object;std.core.Class;std.core.ClassCastError;",
        "std.core.StringBuilder.concatStrings:std.core.String;std.core.String;std.core.String;",
        "std.core.StringBuilder.toString:f64;std.core.String;",
    };
    std::set<std::string> existingFunctions {};
    for (auto &it : prog->functionStaticTable) {
        existingFunctions.insert(it.first);
    }
    for (auto &it : prog->functionInstanceTable) {
        existingFunctions.insert(it.first);
    }
    EXPECT_THAT(existingFunctions, ::testing::ContainerEq(expectedFunctions));
}

TEST_F(Abc2ProgramFunctionsTest, StringTable)
{
    const std::set<std::string> expectedStrings = {
        "Function foo was called",
        "Function bar was called",
        "Hi,",
    };
    EXPECT_THAT(prog->strings, ::testing::ContainerEq(expectedStrings));
}

TEST_F(Abc2ProgramLiteralArrayTest, LiteralArray)
{
    const auto tagString = std::to_string(static_cast<int>(panda_file::LiteralTag::STRING));
    const std::set<std::string> expectedLiterArray = {
        tagString + " test ",
        tagString + " name01 " + tagString + " name02 ",
    };
    std::set<std::string> existingLiterArray {};
    for (auto &[unused, litArray] : prog->literalarrayTable) {
        std::stringstream content {};
        bool skipTag = false;
        for (const auto &l : litArray.literals) {
            skipTag = !skipTag;
            if (skipTag) {
                continue;
            }
            content << static_cast<int>(l.tag) << " " << std::get<std::string>(l.value) << " ";
        }
        existingLiterArray.insert(content.str());
    }
    EXPECT_EQ(existingLiterArray, expectedLiterArray);
}

TEST_F(Abc2ProgramTypeTest, Type)
{
    const std::set<std::string> expectedType = {
        "Type.%%partial-Cls._ctor_ Type.%%partial-Cls void",
        "Type.Cls._ctor_ Type.Cls void",
        "Type.Cls.add_inner Type.Cls i32 i64 f32 f64 std.core.String",
        "Type.ETSGLOBAL._cctor_ void",
        "Type.ETSGLOBAL.addOuter i32 i64 f32 f64 std.core.String",
        "Type.ETSGLOBAL.main void",
        "std.core.Object._ctor_ std.core.Object void",
    };
    std::set<std::string> existingType {};
    for (auto &it : prog->functionInstanceTable) {
        std::stringstream str;
        str << it.second.name;
        for (auto &param : it.second.params) {
            str << " " << param.type.GetName();
        }
        str << " " << it.second.returnType.GetName();
        existingType.insert(str.str());
    }
    for (auto &it : prog->functionStaticTable) {
        std::stringstream str;
        str << it.second.name;
        for (auto &param : it.second.params) {
            str << " " << param.type.GetName();
        }
        str << " " << it.second.returnType.GetName();
        existingType.insert(str.str());
    }
    EXPECT_EQ(existingType, expectedType);
}

TEST_F(Abc2ProgramRecordTest, Record)
{
    using namespace std::literals;
    const std::array expectedRecord = {
        "Record.Anno"s, "Record.Cls"s, "Record.ETSGLOBAL"s, "Record.Enums"s, "Record.Itf"s, "Record.Ns"s,
    };
    std::set<std::string> existingRecord {};
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.second.language, panda_file::SourceLang::ETS);
        EXPECT_EQ(it.first, it.second.name);
        existingRecord.insert(it.first);
    }
    EXPECT_THAT(existingRecord, ::testing::IsSupersetOf(expectedRecord));
}

TEST_F(Abc2ProgramInsTest, Ins)
{
    const std::string expectedIns =
        "ldai 0x1 sta v0 ldai 0x2 sta v1 ldai 0x3 sta v2 lda v0 sta v4 lda v1 sta v5 lda v4 mov v4, v5 add2 v4 sta v3 "
        "lda v2 sta v4 lda v3 mov v3, v4 add2 v3 return ";
    const auto &it = prog->functionStaticTable.find("Ins.ETSGLOBAL.test:i32;");
    EXPECT_NE(it, prog->functionStaticTable.end());

    std::stringstream str;
    for (auto &i : it->second.ins) {
        str << i.ToString(" ", true, it->second.regsNum);
    }
    EXPECT_EQ(str.str(), expectedIns);
}

TEST_F(Abc2ProgramFieldTest, Field)
{
    const std::set<std::string> expectedField = {
        "anno1",
        "cls1",
        "enums1",
        "ns1",
    };
    std::set<std::string> existingField {};
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.first, it.second.name);
        for (const auto &r : it.second.fieldList) {
            existingField.insert(r.name);
        }
    }
    EXPECT_THAT(existingField, ::testing::IsSupersetOf(expectedField));
}

TEST_F(Abc2ProgramMetadataTest, Record1)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {
        "ets.annotation",
        "ets.abstract",
    };
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"access.record", {"public"}},
    };
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.first, it.second.name);
        if (it.second.name != "Metadata.Anno") {
            continue;
        }
        const auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_EQ(boolAttributes, expectedBoolAttributes);
        const auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Record2)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {
        "ets.abstract",
        "ets.interface",
    };
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.extends", {"std.core.Object"}},
        {"access.record", {"public"}},
    };
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.first, it.second.name);
        if (it.second.name != "Metadata.Iface") {
            continue;
        }
        const auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_EQ(boolAttributes, expectedBoolAttributes);
        const auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Record3)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {};
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.value", {"\"Cls\""}},
        {"ets.annotation.class", {"Metadata.Anno"}},
        {"ets.annotation.type", {"class"}},
        {"ets.implements", {"Metadata.Iface"}},
        {"ets.annotation.element.type", {"string"}},
        {"ets.annotation.element.name", {"name"}},
        {"ets.extends", {"std.core.Object"}},
        {"access.record", {"public"}},
    };
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.first, it.second.name);
        if (it.second.name != "Metadata.Cls") {
            continue;
        }
        const auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_EQ(boolAttributes, expectedBoolAttributes);
        const auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Record4)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {
        "ets.abstract",
    };
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.value", {""}},
        {"ets.annotation.class", {"ets.annotation.Module"}},
        {"ets.annotation.type", {"class"}},
        {"ets.annotation.element.type", {"array"}},
        {"ets.annotation.element.name", {"exported"}},
        {"ets.extends", {"std.core.Object"}},
        {"access.record", {"public"}},
    };
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.first, it.second.name);
        if (it.second.name != "Metadata.Ns") {
            continue;
        }
        const auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_EQ(boolAttributes, expectedBoolAttributes);
        const auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Field)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {};
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.type", {"string"}},
        {"ets.annotation.element.name", {"name"}},
        {"ets.annotation.class", {"Metadata.Anno"}},
        {"ets.annotation.element.value", {"\"foo\""}},
        {"access.field", {"public"}},
    };
    for (const auto &it : prog->recordTable) {
        EXPECT_EQ(it.first, it.second.name);
        if (it.second.name != "Metadata.Cls") {
            continue;
        }
        for (const auto &r : it.second.fieldList) {
            if (r.name != "foo") {
                continue;
            }
            const auto boolAttributes = it.second.metadata->GetBoolAttributes();
            EXPECT_EQ(boolAttributes, expectedBoolAttributes);
            const auto attributes = r.metadata->GetAttributes();
            EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
        }
    }
}

TEST_F(Abc2ProgramMetadataTest, Method1)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {};
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.type", {"string"}},
        {"ets.annotation.element.name", {"name"}},
        {"ets.annotation.class", {"Metadata.Anno"}},
        {"ets.annotation.element.value", {"\"bar\""}},
        {"access.function", {"public"}},
    };
    for (const auto &it : prog->functionInstanceTable) {
        if (it.second.name != "Metadata.Cls.bar") {
            continue;
        }
        const auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_EQ(boolAttributes, expectedBoolAttributes);
        const auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Method2)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {
        "static",
    };
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"access.function", {"public"}},
    };
    for (const auto &it : prog->functionStaticTable) {
        if (it.second.name != "Metadata.Ns.add") {
            continue;
        }
        const auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_EQ(boolAttributes, expectedBoolAttributes);
        const auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Method3)
{
    const std::unordered_set<std::string> expectedBoolAttributes = {
        "ctor",
    };
    const std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"access.function", {"public"}},
    };
    for (const auto &it : prog->functionInstanceTable) {
        if (it.second.name != "Metadata.Cls._ctor_") {
            continue;
        }
        const auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_EQ(boolAttributes, expectedBoolAttributes);
        const auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

}  // namespace ark::abc2program
