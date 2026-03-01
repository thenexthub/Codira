/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "debugger/object_repository.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "runtime.h"
#include "runtime_options.h"
#include "tooling/debugger.h"

#include "types/numeric_id.h"

#include "common.h"
#include "json_object_matcher.h"

// NOLINTBEGIN

namespace ark::tooling::inspector::test {

static constexpr const char *g_source = R"(
    .record Test {}

    .function i32 Test.foo() {
        ldai 111
        return
    }
)";

class ObjectRepositoryTest : public testing::Test {
protected:
    static void SetUpTestSuite()
    {
        RuntimeOptions options;
        options.SetShouldInitializeIntrinsics(false);
        options.SetShouldLoadBootPandaFiles(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();

        pandasm::Parser p;

        auto res = p.Parse(g_source, "source.pa");
        ASSERT_TRUE(res.HasValue());
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT(pf);
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        classLinker->AddPandaFile(std::move(pf));

        PandaString descriptor;
        auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
        Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("Test"), &descriptor));
        ASSERT_NE(klass, nullptr);

        auto methods = klass->GetMethods();
        ASSERT_EQ(methods.size(), 1);

        clsObject = klass->GetManagedObject();
        methodFoo = &methods[0];
    }

    static void TearDownTestSuite()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    static constexpr uint16_t U16_VALUE = 43602;
    static constexpr int32_t I32_VALUE = -2345678;
    static constexpr int64_t I64_VALUE = 200000000000002;
    static constexpr double F64_VALUE = 6.547;

    static ark::MTManagedThread *thread_;
    static Method *methodFoo;
    static ObjectHeader *clsObject;
};

ark::MTManagedThread *ObjectRepositoryTest::thread_ = nullptr;
Method *ObjectRepositoryTest::methodFoo = nullptr;
ObjectHeader *ObjectRepositoryTest::clsObject = nullptr;

template <typename ValueT, typename V>
static auto GetPrimitiveProperties(const char *type, V value, const char *valueName = "value")
{
    return JsonProperties(JsonProperty<JsonObject::StringT> {"type", type}, JsonProperty<ValueT> {valueName, value});
}

template <typename NameT, typename DescT>
static auto GetObjectProperties(NameT className, DescT description, const char *objectId)
{
    return JsonProperties(JsonProperty<JsonObject::StringT> {"type", "object"},
                          JsonProperty<JsonObject::StringT> {"className", className},
                          JsonProperty<JsonObject::StringT> {"description", description},
                          JsonProperty<JsonObject::StringT> {"objectId", objectId});
}

template <typename NameT, typename V>
static auto GetFrameObjectProperties(NameT name, V valueProperties)
{
    return JsonProperties(JsonProperty<JsonObject::StringT> {"name", name},
                          JsonProperty<JsonObject::JsonObjPointer> {"value", valueProperties},
                          JsonProperty<JsonObject::BoolT> {"writable", testing::_},
                          JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
                          JsonProperty<JsonObject::BoolT> {"enumerable", testing::_});
}

TEST_F(ObjectRepositoryTest, S)
{
    ObjectRepository obj;

    auto clsObj = obj.CreateObject(TypedValue::Reference(clsObject));
    ASSERT_EQ(clsObj.GetObjectId(), RemoteObjectId(1));
    ASSERT_THAT(ToJson(clsObj), GetObjectProperties(testing::_, testing::_, "1"));

    auto nullObj = obj.CreateObject(TypedValue::Reference(nullptr));
    ASSERT_THAT(ToJson(nullObj), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "object"},
                                                JsonProperty<JsonObject::StringT> {"subtype", "null"},
                                                JsonProperty<JsonObject::JsonObjPointer> {"value", testing::IsNull()}));

    auto invObj = obj.CreateObject(TypedValue::Invalid());
    ASSERT_THAT(ToJson(invObj), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "undefined"}));

    auto boolObj = obj.CreateObject(TypedValue::U1(true));
    ASSERT_THAT(ToJson(boolObj), GetPrimitiveProperties<JsonObject::BoolT>("boolean", true));

    auto numObj = obj.CreateObject(TypedValue::U16(U16_VALUE));
    ASSERT_THAT(ToJson(numObj), GetPrimitiveProperties<JsonObject::NumT>("number", U16_VALUE));

    auto negObj = obj.CreateObject(TypedValue::I32(I32_VALUE));
    ASSERT_THAT(ToJson(negObj), GetPrimitiveProperties<JsonObject::NumT>("number", I32_VALUE));

    auto hugeObj = obj.CreateObject(TypedValue::I64(I64_VALUE));
    ASSERT_THAT(ToJson(hugeObj),
                GetPrimitiveProperties<JsonObject::StringT>("bigint", "200000000000002", "unserializableValue"));

    auto doubObj = obj.CreateObject(TypedValue::F64(F64_VALUE));
    ASSERT_THAT(ToJson(doubObj), GetPrimitiveProperties<JsonObject::NumT>("number", testing::DoubleEq(F64_VALUE)));

    auto globObj1 = obj.CreateGlobalObject();
    ASSERT_THAT(ToJson(globObj1), GetObjectProperties("[Global]", "Global object", "0"));

    auto globObj2 = obj.CreateGlobalObject();
    ASSERT_THAT(ToJson(globObj2), GetObjectProperties("[Global]", "Global object", "0"));

    PtDebugFrame frame(methodFoo, nullptr);
    std::map<std::string, TypedValue> locals;
    locals.emplace("a", TypedValue::U16(56U));
    locals.emplace("ref", TypedValue::Reference(clsObject));
    // "this" parameter for static languages. Note that ArkTS uses "=t" instead of "this".
    locals.emplace("this", TypedValue::Reference(clsObject));

    std::optional<RemoteObject> objThis;
    auto frameObj = obj.CreateFrameObject(frame, locals, objThis);
    ASSERT_EQ(frameObj.GetObjectId().value(), RemoteObjectId(2UL));

    auto properties = obj.GetProperties(frameObj.GetObjectId().value(), false);
    ASSERT_EQ(properties.size(), 2UL);
    ASSERT_EQ(properties[0].GetName(), "a");

    ASSERT_THAT(ToJson(frameObj), GetObjectProperties("", "Frame #0", "2"));

    ASSERT_THAT(
        ToJson(properties[0]),
        GetFrameObjectProperties("a", testing::Pointee(GetPrimitiveProperties<JsonObject::NumT>("number", 56U))));

    ASSERT_THAT(ToJson(properties[1]),
                GetFrameObjectProperties("ref", testing::Pointee(GetObjectProperties(testing::_, testing::_, "1"))));

    // Call to "CreateFrameObject" must find and fill "this" parameter.
    ASSERT_TRUE(objThis.has_value());
    auto idThis = objThis->GetObjectId();
    ASSERT_TRUE(idThis.has_value());
    ASSERT_EQ(idThis.value(), 1U);
}

TEST_F(ObjectRepositoryTest, TestFrameObjectNoThis)
{
    ObjectRepository obj;

    PtDebugFrame frame(methodFoo, nullptr);
    std::map<std::string, TypedValue> locals;
    locals.emplace("a", TypedValue::U16(56U));
    locals.emplace("ref", TypedValue::Reference(clsObject));

    std::optional<RemoteObject> objThis;
    auto frameObj = obj.CreateFrameObject(frame, locals, objThis);
    ASSERT_EQ(frameObj.GetObjectId().value(), RemoteObjectId(2UL));

    auto properties = obj.GetProperties(frameObj.GetObjectId().value(), true);
    ASSERT_EQ(properties.size(), 2UL);

    // No "this" parameter was provided.
    ASSERT_FALSE(objThis.has_value());
}
}  // namespace ark::tooling::inspector::test

// NOLINTEND
