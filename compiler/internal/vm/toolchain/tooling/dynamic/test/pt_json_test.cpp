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

#include "ecmascript/tests/test_helper.h"
#include "tooling/dynamic/base/pt_json.h"

using namespace panda::ecmascript::tooling;

namespace panda::test {
class PtJsonTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

HWTEST_F_L0(PtJsonTest, FalseTest)
{
    std::string str = "false";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsBool());
    EXPECT_FALSE(json->GetBool());
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, TrueTest)
{
    std::string str = "true";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsBool());
    EXPECT_TRUE(json->GetBool());
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, IntTest)
{
    std::string str = "100";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsNumber());
    EXPECT_EQ(json->GetInt(), 100);
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, Int64Test)
{
    std::string str = "123456789012345";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsNumber());
    EXPECT_EQ(json->GetInt64(), 123456789012345);
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, DoubleTest)
{
    std::string str = "12345.6789";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsNumber());
    EXPECT_EQ(json->GetDouble(), 12345.6789);
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, StringTest)
{
    std::string str = "\"abcdefg\"";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsString());
    EXPECT_EQ(json->GetString(), "abcdefg");
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, ArrayTest1)
{
    std::string str = "[\"a\",\"b\",200]";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsArray());
    EXPECT_EQ(json->GetSize(), 3);
    EXPECT_EQ(json->Get(0)->GetString(), "a");
    EXPECT_EQ(json->Get(1)->GetString(), "b");
    EXPECT_EQ(json->Get(2)->GetInt(), 200);
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, ArrayTest2)
{
    std::string str = "[\"a\",\"b\",200,10.5,{}]";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    ASSERT_TRUE(json->IsArray());
    EXPECT_EQ(json->GetSize(), 5);
    EXPECT_EQ(json->Get(0)->GetString(), "a");
    EXPECT_EQ(json->Get(1)->GetString(), "b");
    EXPECT_EQ(json->Get(2)->GetInt(), 200);
    EXPECT_EQ(json->Get(3)->GetDouble(), 10.5);
    EXPECT_TRUE(json->Get(4)->IsObject());
    EXPECT_EQ(json->Stringify(), str);
    json->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, ObjectTest)
{
    auto child1 = PtJson::CreateObject();
    child1->Add("ch", "child_1");
    ASSERT_TRUE(child1->Contains("ch"));

    auto child2 = PtJson::CreateObject();
    child2->Add("ch", "child_2");
    ASSERT_TRUE(child2->Contains("ch"));

    auto arr = PtJson::CreateArray();
    arr->Push(100);
    EXPECT_EQ(arr->GetSize(), 1);

    auto root = PtJson::CreateObject();
    root->Add("a", false);
    root->Add("b", 100);
    root->Add("c", 100.2);
    root->Add("d", static_cast<int64_t>(200));
    root->Add("e", "abc");
    root->Add("f", child2);
    root->Add("g", arr);

    bool b;
    int32_t i32;
    int64_t i64;
    double d;
    std::string str;
    std::unique_ptr<PtJson> json;
    ASSERT_EQ(root->GetBool("a", &b), Result::SUCCESS);
    EXPECT_FALSE(b);
    ASSERT_EQ(root->GetInt("b", &i32), Result::SUCCESS);
    EXPECT_EQ(i32, 100);
    ASSERT_EQ(root->GetDouble("c", &d), Result::SUCCESS);
    EXPECT_EQ(d, 100.2);
    ASSERT_EQ(root->GetInt64("d", &i64), Result::SUCCESS);
    EXPECT_EQ(i64, static_cast<int64_t>(200));
    ASSERT_EQ(root->GetString("e", &str), Result::SUCCESS);
    EXPECT_EQ(str, "abc");
    ASSERT_EQ(root->GetObject("f", &json), Result::SUCCESS);
    ASSERT_EQ(json->GetString("ch", &str), Result::SUCCESS);
    EXPECT_EQ(str, "child_2");
    ASSERT_EQ(root->GetArray("g", &json), Result::SUCCESS);
    ASSERT_TRUE(json->IsArray());
    EXPECT_EQ(json->Get(0)->GetInt(), 100);

    EXPECT_EQ(root->Stringify(),
        "{\"a\":false,\"b\":100,\"c\":100.2,\"d\":200,\"e\":\"abc\",\"f\":{\"ch\":\"child_2\"},\"g\":[100]}");
    root->ReleaseRoot();
}

HWTEST_F_L0(PtJsonTest, StringifyTest)
{
    PtJson ptJson;
    std::string result = ptJson.Stringify();
    ASSERT_EQ(result, "");
}

HWTEST_F_L0(PtJsonTest, GetKeyTest)
{
    PtJson ptJson;
    std::string result = ptJson.GetKey();
    ASSERT_EQ(result, "");
    cJSON temp;
    char test[] = "test";
    temp.string = test;
    PtJson ptJson1(&temp);
    result = ptJson1.GetKey();
    ASSERT_EQ(result, "test");
    cJSON temp1;
    temp1.string = nullptr;
    PtJson ptJson2(&temp1);
    result = ptJson2.GetKey();
    ASSERT_EQ(result, "");
    std::unique_ptr<PtJson> ptr = std::make_unique<PtJson>();
    Result result1 = ptJson.GetAny(test, &ptr);
    ASSERT_EQ(result1, Result::NOT_EXIST);
    cJSON json;
    cJSON son;
    son.string = test;
    json.string = test;
    json.child = &son;
    PtJson ptJson3(&json);
    std::unique_ptr<PtJson> ptr1 = std::make_unique<PtJson>();
    result1 = ptJson3.GetAny(test, &ptr1);
    ASSERT_EQ(result1, Result::SUCCESS);
}

HWTEST_F_L0(PtJsonTest, GetTypeTest)
{
    cJSON *node = cJSON_CreateNumber(0);
    PtJson num(node);
    cJSON *node1 = cJSON_CreateString("test");
    PtJson str(node1);
    bool result = str.GetBool(false);
    ASSERT_EQ(result, false);
    int32_t result1 = str.GetInt(int32_t(0));
    ASSERT_EQ(result1, 0);
    int64_t result2 = str.GetInt64(int64_t(0));
    ASSERT_EQ(result2, 0);
    double result3 = str.GetDouble(0.1); // 0.1:num
    ASSERT_EQ(result3, 0.1); // 0.1:num
    std::string result4 = num.GetString();
    ASSERT_EQ(result4, "");
}

HWTEST_F_L0(PtJsonTest, PushTest)
{
    char *ptr = nullptr;
    PtJson ptJson;
    bool result = ptJson.Push(ptr);
    ASSERT_EQ(result, false);
    char cPtr[] = "value";
    result = ptJson.Push(cPtr);
    ASSERT_EQ(result, false);
    bool value = true;
    result = ptJson.Push(value);
    ASSERT_EQ(result, false);
    double value1 = 0.1; // 0.1:value
    result = ptJson.Push(value1);
    ASSERT_EQ(result, false);
    std::unique_ptr<PtJson> ptr1;
    result = ptJson.Push(std::move(ptr1));
    ASSERT_EQ(result, false);
    std::unique_ptr<PtJson> ptr2 = std::make_unique<PtJson>();
    result = ptJson.Push(std::move(ptr2));
    ASSERT_EQ(result, false);
    cJSON json;
    std::unique_ptr<PtJson> ptr3 = std::make_unique<PtJson>(&json);
    result = ptJson.Push(std::move(ptr3));
    ASSERT_EQ(result, false);
}

HWTEST_F_L0(PtJsonTest, AddTest)
{
    char key[] = "key";
    double value = 0.1; // 0.1:value
    PtJson ptJson;
    bool result = ptJson.Add(key, value);
    ASSERT_EQ(result, false);
    char value1[] = "value";
    result = ptJson.Add(key, value1);
    ASSERT_EQ(result, false);
    std::unique_ptr<PtJson> ptr = std::make_unique<PtJson>();
    result = ptJson.Add(key, std::move(ptr));
    ASSERT_EQ(result, false);
    cJSON json;
    std::unique_ptr<PtJson> ptr1 = std::make_unique<PtJson>(&json);
    result = ptJson.Add(key, std::move(ptr1));
    ASSERT_EQ(result, false);
    bool value2 = true;
    result = ptJson.Add(key, value2);
    ASSERT_EQ(result, false);
}

HWTEST_F_L0(PtJsonTest, UIntTest)
{
    std::string str = "UIntTest";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    EXPECT_EQ(json->GetUInt(), 0);
    str = "12345";
    json = PtJson::Parse(str.c_str());
    EXPECT_EQ(json->GetUInt(), 12345);
}

HWTEST_F_L0(PtJsonTest, UInt64Test)
{
    std::string str = "UInt64Test";
    std::unique_ptr<PtJson> json = PtJson::Parse(str.c_str());
    EXPECT_EQ(json->GetUInt64(), 0);
    str = "123456789012345";
    json = PtJson::Parse(str.c_str());
    EXPECT_EQ(json->GetUInt64(), 123456789012345);
}

HWTEST_F_L0(PtJsonTest, ResultUInt64Test)
{
    auto test = PtJson::CreateObject();
    test->Add("a", "ResultUInt64Test");
    test->Add("b", 100);
    uint64_t ui64;
    ASSERT_EQ(test->GetUInt64("a", &ui64), Result::TYPE_ERROR);
    ASSERT_EQ(test->GetUInt64("b", &ui64), Result::SUCCESS);
    EXPECT_EQ(ui64, static_cast<uint64_t>(100));
}

HWTEST_F_L0(PtJsonTest, GetKeysArraytest)
{
    auto test = PtJson::CreateObject();
    test->Add("a", "ResultUInt64Test");
    test->Add("b", 100);
    auto keys = test->GetKeysArray();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(keys.at(0) == "a");
    EXPECT_TRUE(keys.at(1) == "b");
}
}