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

#include <algorithm>
#include <memory>
#include <ostream>
#include <unordered_set>
#include <vector>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/modifiers.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/core/core_class_linker_extension.h"
#include "runtime/tests/class_linker_test_extension.h"

namespace ark::test {

class ClassLinkerTest : public testing::Test {
public:
    ClassLinkerTest()
    {
        // Just for internal allocator
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetHeapSizeLimit(64_MB);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~ClassLinkerTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(ClassLinkerTest);
    NO_MOVE_SEMANTIC(ClassLinkerTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_;
};

static std::unique_ptr<ClassLinker> CreateClassLinker(ManagedThread *thread)
{
    std::vector<std::unique_ptr<ClassLinkerExtension>> extensions;
    extensions.push_back(std::make_unique<CoreClassLinkerExtension>());
    auto allocator = thread->GetVM()->GetHeapManager()->GetInternalAllocator();
    auto classLinker = std::make_unique<ClassLinker>(allocator, std::move(extensions));
    if (!classLinker->Initialize()) {
        return nullptr;
    }

    return classLinker;
}

TEST_F(ClassLinkerTest, TestGetClass)
{
    pandasm::Parser p;

    auto source = R"(
        .function void main() {
            return.void
        }
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);

    auto *pfPtr = pf.get();
    classLinker->AddPandaFile(std::move(pf));

    Class *klass;

    {
        // Use temporary string to load class. Class loader shouldn't store it.
        auto descriptor = std::make_unique<PandaString>();
        klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), descriptor.get()));
    }

    PandaString descriptor;

    EXPECT_EQ(klass, ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor)));
    EXPECT_EQ(klass->GetBase(), ext->GetClassRoot(ClassRoot::OBJECT));
    EXPECT_EQ(klass->GetPandaFile(), pfPtr);
    EXPECT_EQ(klass->GetMethods().size(), 1U);
    EXPECT_EQ(klass->GetComponentSize(), 0U);
}

std::set<std::string> GetClassesForEnumerateClassesTest()
{
    return {"panda.Object",
            "panda.String",
            "panda.LineString",
            "panda.SlicedString",
            "panda.TreeString",
            "panda.Class",
            "[Lpanda/String;",
            "u1",
            "i8",
            "u8",
            "i16",
            "u16",
            "i32",
            "u32",
            "i64",
            "u64",
            "f32",
            "f64",
            "any",
            "Y",
            "N",
            "[Z",
            "[B",
            "[H",
            "[S",
            "[C",
            "[I",
            "[U",
            "[J",
            "[Q",
            "[F",
            "[D",
            "[A",
            "[Lpanda/Class;",
            "_GLOBAL"};
}

TEST_F(ClassLinkerTest, TestEnumerateClasses)
{
    pandasm::Parser p;

    auto source = R"(
        .function void main() {
            return.void
        }
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    // Load _GLOBAL class
    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));

    std::set<std::string> classes = GetClassesForEnumerateClassesTest();

    std::set<std::string> loadedClasses;

    classLinker->EnumerateClasses([&loadedClasses](Class *k) {
        loadedClasses.emplace(k->GetName());
        return true;
    });

    EXPECT_EQ(loadedClasses, classes);
}

static void TestPrimitiveClassRoot(const ClassLinkerExtension &classLinkerExt, ClassRoot classRoot,
                                   panda_file::Type::TypeId typeId)
{
    std::string msg = "Test with class root ";
    msg += std::to_string(static_cast<int>(classRoot));

    Class *klass = classLinkerExt.GetClassRoot(classRoot);
    ASSERT_NE(klass, nullptr) << msg;
    EXPECT_EQ(klass->GetBase(), nullptr) << msg;
    EXPECT_EQ(klass->GetComponentSize(), 0U) << msg;
    EXPECT_EQ(klass->GetRuntimeFlags(), 0U) << msg;
    EXPECT_EQ(klass->GetAccessFlags(), ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT) << msg;
    EXPECT_EQ(klass->GetType().GetId(), typeId) << msg;
    EXPECT_FALSE(klass->IsArrayClass()) << msg;
    EXPECT_FALSE(klass->IsStringClass()) << msg;
    EXPECT_TRUE(klass->IsPrimitive()) << msg;
    EXPECT_TRUE(klass->IsAbstract()) << msg;
    EXPECT_FALSE(klass->IsClass()) << msg;
    EXPECT_FALSE(klass->IsInterface()) << msg;
    EXPECT_FALSE(klass->IsInstantiable()) << msg;
}

static void TestPrimitiveClassRoots(const ClassLinkerExtension &ext)
{
    TestPrimitiveClassRoot(ext, ClassRoot::U1, panda_file::Type::TypeId::U1);
    TestPrimitiveClassRoot(ext, ClassRoot::I8, panda_file::Type::TypeId::I8);
    TestPrimitiveClassRoot(ext, ClassRoot::U8, panda_file::Type::TypeId::U8);
    TestPrimitiveClassRoot(ext, ClassRoot::I16, panda_file::Type::TypeId::I16);
    TestPrimitiveClassRoot(ext, ClassRoot::U16, panda_file::Type::TypeId::U16);
    TestPrimitiveClassRoot(ext, ClassRoot::I32, panda_file::Type::TypeId::I32);
    TestPrimitiveClassRoot(ext, ClassRoot::U32, panda_file::Type::TypeId::U32);
    TestPrimitiveClassRoot(ext, ClassRoot::I64, panda_file::Type::TypeId::I64);
    TestPrimitiveClassRoot(ext, ClassRoot::U64, panda_file::Type::TypeId::U64);
    TestPrimitiveClassRoot(ext, ClassRoot::F32, panda_file::Type::TypeId::F32);
    TestPrimitiveClassRoot(ext, ClassRoot::F64, panda_file::Type::TypeId::F64);
}

static size_t GetComponentSize(ClassRoot componentRoot)
{
    switch (componentRoot) {
        case ClassRoot::U1:
        case ClassRoot::I8:
        case ClassRoot::U8:
            return sizeof(uint8_t);
        case ClassRoot::I16:
        case ClassRoot::U16:
            return sizeof(uint16_t);
        case ClassRoot::I32:
        case ClassRoot::U32:
        case ClassRoot::F32:
            return sizeof(uint32_t);
        case ClassRoot::I64:
        case ClassRoot::U64:
        case ClassRoot::F64:
            return sizeof(uint64_t);
        default:
            UNREACHABLE();
    }
}

static void TestArrayClassRoot(const ClassLinkerExtension &classLinkerExt, ClassRoot classRoot, ClassRoot componentRoot)
{
    std::string msg = "Test with class root ";
    msg += std::to_string(static_cast<int>(classRoot));

    Class *klass = classLinkerExt.GetClassRoot(classRoot);
    Class *componentClass = classLinkerExt.GetClassRoot(componentRoot);
    ASSERT_NE(klass, nullptr) << msg;
    EXPECT_EQ(klass->GetBase(), classLinkerExt.GetClassRoot(ClassRoot::OBJECT)) << msg;
    EXPECT_EQ(klass->GetComponentType(), componentClass) << msg;
    EXPECT_EQ(klass->GetComponentSize(), GetComponentSize(componentRoot)) << msg;
    EXPECT_EQ(klass->GetRuntimeFlags(), 0U) << msg;
    EXPECT_EQ(klass->GetAccessFlags(), ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT) << msg;
    EXPECT_EQ(klass->GetType().GetId(), panda_file::Type::TypeId::REFERENCE) << msg;
    EXPECT_EQ(klass->IsObjectArrayClass(), !componentClass->IsPrimitive()) << msg;
    EXPECT_TRUE(klass->IsArrayClass()) << msg;
    EXPECT_FALSE(klass->IsStringClass()) << msg;
    EXPECT_FALSE(klass->IsPrimitive()) << msg;
    EXPECT_TRUE(klass->IsAbstract()) << msg;
    EXPECT_TRUE(klass->IsClass()) << msg;
    EXPECT_FALSE(klass->IsInterface()) << msg;
    EXPECT_TRUE(klass->IsInstantiable()) << msg;
}

static void TestArrayClassRoots(const ClassLinkerExtension &ext)
{
    TestArrayClassRoot(ext, ClassRoot::ARRAY_U1, ClassRoot::U1);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_I8, ClassRoot::I8);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_U8, ClassRoot::U8);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_I16, ClassRoot::I16);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_U16, ClassRoot::U16);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_I32, ClassRoot::I32);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_U32, ClassRoot::U32);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_I64, ClassRoot::I64);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_U64, ClassRoot::U64);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_F32, ClassRoot::F32);
    TestArrayClassRoot(ext, ClassRoot::ARRAY_F64, ClassRoot::F64);
}

static void TestClassRoot(Class *classClass, ClassLinkerExtension *ext)
{
    ASSERT_NE(classClass, nullptr);
    EXPECT_EQ(classClass->GetBase(), ext->GetClassRoot(ClassRoot::OBJECT));
    EXPECT_EQ(classClass->GetComponentSize(), 0U);
    EXPECT_EQ(classClass->GetRuntimeFlags(), 0U);
    EXPECT_EQ(classClass->GetType().GetId(), panda_file::Type::TypeId::REFERENCE);
    EXPECT_TRUE(classClass->IsClassClass());
    EXPECT_FALSE(classClass->IsObjectClass());
    EXPECT_FALSE(classClass->IsArrayClass());
    EXPECT_FALSE(classClass->IsObjectArrayClass());
    EXPECT_FALSE(classClass->IsStringClass());
    EXPECT_FALSE(classClass->IsPrimitive());
    EXPECT_TRUE(classClass->IsClass());
    EXPECT_FALSE(classClass->IsInterface());
}

static void TestObjectClassRoot(Class *objectClass)
{
    ASSERT_NE(objectClass, nullptr);
    EXPECT_EQ(objectClass->GetBase(), nullptr);
    EXPECT_EQ(objectClass->GetComponentSize(), 0U);
    EXPECT_EQ(objectClass->GetRuntimeFlags(), 0U);
    EXPECT_EQ(objectClass->GetType().GetId(), panda_file::Type::TypeId::REFERENCE);
    EXPECT_TRUE(objectClass->IsObjectClass());
    EXPECT_FALSE(objectClass->IsArrayClass());
    EXPECT_FALSE(objectClass->IsObjectArrayClass());
    EXPECT_FALSE(objectClass->IsStringClass());
    EXPECT_FALSE(objectClass->IsPrimitive());
    EXPECT_TRUE(objectClass->IsClass());
    EXPECT_FALSE(objectClass->IsInterface());
}

static void TestStringClassRoot(Class *stringClass, Class *objectClass)
{
    ASSERT_NE(stringClass, nullptr);
    EXPECT_EQ(stringClass->GetBase(), objectClass);
    EXPECT_EQ(stringClass->GetComponentSize(), 0U);
    EXPECT_EQ(stringClass->GetRuntimeFlags(), Class::STRING_CLASS);
    EXPECT_EQ(stringClass->GetType().GetId(), panda_file::Type::TypeId::REFERENCE);
    EXPECT_FALSE(stringClass->IsObjectClass());
    EXPECT_FALSE(stringClass->IsArrayClass());
    EXPECT_FALSE(stringClass->IsObjectArrayClass());
    EXPECT_TRUE(stringClass->IsStringClass());
    EXPECT_FALSE(stringClass->IsPrimitive());
    EXPECT_TRUE(stringClass->IsClass());
    EXPECT_FALSE(stringClass->IsInterface());
    EXPECT_FALSE(stringClass->IsFinal());
}

static void TestLineStringClassRoot(Class *lineStringClass, Class *stringClass)
{
    ASSERT_NE(lineStringClass, nullptr);
    EXPECT_EQ(lineStringClass->GetBase(), stringClass);
    EXPECT_EQ(lineStringClass->GetComponentSize(), 0U);
    EXPECT_EQ(lineStringClass->GetRuntimeFlags(), Class::STRING_CLASS);
    EXPECT_EQ(lineStringClass->GetType().GetId(), panda_file::Type::TypeId::REFERENCE);
    EXPECT_FALSE(lineStringClass->IsObjectClass());
    EXPECT_FALSE(lineStringClass->IsArrayClass());
    EXPECT_FALSE(lineStringClass->IsObjectArrayClass());
    EXPECT_TRUE(lineStringClass->IsStringClass());
    EXPECT_FALSE(lineStringClass->IsPrimitive());
    EXPECT_TRUE(lineStringClass->IsClass());
    EXPECT_FALSE(lineStringClass->IsInterface());
    EXPECT_TRUE(lineStringClass->IsFinal());
}

// CC-OFFNXT(G.FUD.05) solid logic
TEST_F(ClassLinkerTest, TestClassRoots)
{
    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);
    Class *classClass = ext->GetClassRoot(ClassRoot::CLASS);
    TestClassRoot(classClass, ext);

    Class *objectClass = ext->GetClassRoot(ClassRoot::OBJECT);
    TestObjectClassRoot(objectClass);

    Class *stringClass = ext->GetClassRoot(ClassRoot::STRING);
    TestStringClassRoot(stringClass, objectClass);

    Class *lineStringClass = ext->GetClassRoot(ClassRoot::LINE_STRING);
    TestLineStringClassRoot(lineStringClass, stringClass);

    TestPrimitiveClassRoots(*ext);
    TestArrayClassRoots(*ext);
}

struct FieldData {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string name;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    size_t size;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    size_t offset;

    bool operator==(const FieldData &other) const
    {
        return name == other.name && size == other.size && offset == other.offset;
    }

    friend std::ostream &operator<<(std::ostream &os, const FieldData &fieldData)
    {
        return os << "{ name: \"" << fieldData.name << "\", size: " << fieldData.size
                  << ", offset: " << fieldData.offset << " }";
    }
};

struct FieldDataHash {
    size_t operator()(const FieldData &fieldData) const
    {
        return std::hash<std::string>()(fieldData.name);
    }
};

size_t GetSize(const Field &field)
{
    size_t size = 0;

    switch (field.GetTypeId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::I8:
        case panda_file::Type::TypeId::U8: {
            size = 1;
            break;
        }
        case panda_file::Type::TypeId::I16:
        case panda_file::Type::TypeId::U16: {
            size = 2L;
            break;
        }
        case panda_file::Type::TypeId::I32:
        case panda_file::Type::TypeId::U32:
        case panda_file::Type::TypeId::F32: {
            size = 4L;
            break;
        }
        case panda_file::Type::TypeId::I64:
        case panda_file::Type::TypeId::U64:
        case panda_file::Type::TypeId::F64: {
            size = 8L;
            break;
        }
        case panda_file::Type::TypeId::REFERENCE: {
            size = ClassHelper::OBJECT_POINTER_SIZE;
            break;
        }
        case panda_file::Type::TypeId::TAGGED: {
            size = coretypes::TaggedValue::TaggedTypeSize();
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return size;
}

void UpdateOffsets(std::vector<FieldData> *fields, size_t offset)
{
    for (auto &field : *fields) {
        offset = AlignUp(offset, field.size);
        field.offset = offset;
        offset += field.size;
    }
}

static std::vector<FieldData> GetFieldLayoutSortedSfields()
{
    return {{"sf_ref", ClassHelper::OBJECT_POINTER_SIZE, 0},
            {"sf_any", coretypes::TaggedValue::TaggedTypeSize(), 0},
            {"sf_f64", sizeof(double), 0},
            {"sf_i64", sizeof(int64_t), 0},
            {"sf_u64", sizeof(uint64_t), 0},
            {"sf_i32", sizeof(int32_t), 0},
            {"sf_u32", sizeof(uint32_t), 0},
            {"sf_f32", sizeof(float), 0},
            {"sf_i16", sizeof(int16_t), 0},
            {"sf_u16", sizeof(uint16_t), 0},
            {"sf_u1", sizeof(uint8_t), 0},
            {"sf_i8", sizeof(int8_t), 0},
            {"sf_u8", sizeof(uint8_t), 0}};
}

static std::vector<FieldData> GetFieldLayoutSortedIfields()
{
    return {{"if_ref", ClassHelper::OBJECT_POINTER_SIZE, 0},
            {"if_any", coretypes::TaggedValue::TaggedTypeSize(), 0},
            {"if_f64", sizeof(double), 0},
            {"if_i64", sizeof(int64_t), 0},
            {"if_u64", sizeof(uint64_t), 0},
            {"if_i32", sizeof(int32_t), 0},
            {"if_u32", sizeof(uint32_t), 0},
            {"if_f32", sizeof(float), 0},
            {"if_i16", sizeof(int16_t), 0},
            {"if_u16", sizeof(uint16_t), 0},
            {"if_u1", sizeof(uint8_t), 0},
            {"if_i8", sizeof(int8_t), 0},
            {"if_u8", sizeof(uint8_t), 0}};
}

static std::string GetFieldLayoutSource()
{
    return R"(
        .record R1 {}

        .record R2 {
            # static fields

            u1  sf_u1  <static>
            i16 sf_i16 <static>
            i8  sf_i8  <static>
            i32 sf_i32 <static>
            u8  sf_u8  <static>
            f64 sf_f64 <static>
            u32 sf_u32 <static>
            u16 sf_u16 <static>
            i64 sf_i64 <static>
            f32 sf_f32 <static>
            u64 sf_u64 <static>
            R1  sf_ref <static>
            any sf_any <static>

            # instance fields

            i16 if_i16
            u1  if_u1
            i8  if_i8
            f64 if_f64
            i32 if_i32
            u8  if_u8
            u32 if_u32
            u16 if_u16
            f32 if_f32
            i64 if_i64
            u64 if_u64
            R2  if_ref
            any if_any
        }
    )";
}

TEST_F(ClassLinkerTest, FieldLayout)
{
    pandasm::Parser p;

    auto source = GetFieldLayoutSource();

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;
    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R2"), &descriptor));
    ASSERT_NE(klass, nullptr);

    std::vector<FieldData> sortedSfields = GetFieldLayoutSortedSfields();

    std::vector<FieldData> sortedIfields = GetFieldLayoutSortedIfields();

    size_t offset = klass->GetStaticFieldsOffset();
    if (!IsAligned<sizeof(double)>(offset + ClassHelper::OBJECT_POINTER_SIZE)) {
        FieldData data {"sf_i32", sizeof(int32_t), 0};
        // NOLINTNEXTLINE(bugprone-inaccurate-erase)
        sortedSfields.erase(std::remove(sortedSfields.begin(), sortedSfields.end(), data));
        sortedSfields.insert(sortedSfields.cbegin() + 1, data);
    }

    UpdateOffsets(&sortedSfields, offset);

    offset = ObjectHeader::ObjectHeaderSize();
    if (!IsAligned<sizeof(double)>(offset + ClassHelper::OBJECT_POINTER_SIZE)) {
        FieldData data {"if_i32", sizeof(int32_t), 0};
        // NOLINTNEXTLINE(bugprone-inaccurate-erase)
        sortedIfields.erase(std::remove(sortedIfields.begin(), sortedIfields.end(), data));
        sortedIfields.insert(sortedIfields.cbegin() + 1, data);
    }

    UpdateOffsets(&sortedIfields, offset);

    auto fieldCmp = [](const FieldData &f1, const FieldData &f2) { return f1.offset < f2.offset; };

    std::vector<FieldData> sfields;
    for (const auto &field : klass->GetStaticFields()) {
        sfields.push_back({utf::Mutf8AsCString(field.GetName().data), GetSize(field), field.GetOffset()});
    }
    std::sort(sfields.begin(), sfields.end(), fieldCmp);
    EXPECT_EQ(sfields, sortedSfields);

    std::unordered_set<FieldData, FieldDataHash> ifields;

    for (const auto &field : klass->GetInstanceFields()) {
        ifields.insert({utf::Mutf8AsCString(field.GetName().data), GetSize(field), field.GetOffset()});
    }

    std::unordered_set<FieldData, FieldDataHash> sortedIfieldsSet(sortedIfields.cbegin(), sortedIfields.cend());
    EXPECT_EQ(ifields, sortedIfieldsSet);
}

TEST_F(ClassLinkerTest, ResolveExternalClass)
{
    uint32_t offset;

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    {
        pandasm::Parser p;

        auto source = R"(
            .record Ext.R <external>

            .function void main() {
                newarr v0, v0, Ext.R[]
                return.void
            }
        )";

        auto res = p.Parse(source);
        ASSERT_TRUE(res);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());

        // 0 - "LExt/R;"
        // 1 - "L_GLOBAL;"
        // 2 - "[LExt/R;"
        offset = pf->GetClasses()[2];

        classLinker->AddPandaFile(std::move(pf));
    }

    PandaString descriptor;

    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_NE(klass, nullptr);

    auto *method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(method, nullptr);

    auto *externalClass = classLinker->GetClass(*method, panda_file::File::EntityId(offset));
    ASSERT_EQ(externalClass, nullptr);

    {
        pandasm::Parser p;

        auto extSource = R"(
            .record Ext {}
            .record Ext.R {}
        )";

        auto res = p.Parse(extSource);
        auto extPf = pandasm::AsmEmitter::Emit(res.Value());

        classLinker->AddPandaFile(std::move(extPf));
    }

    externalClass = classLinker->GetClass(*method, panda_file::File::EntityId(offset));
    ASSERT_NE(externalClass, nullptr);

    EXPECT_STREQ(utf::Mutf8AsCString(externalClass->GetDescriptor()),
                 utf::Mutf8AsCString(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("Ext.R"), 1, &descriptor)));
}

TEST_F(ClassLinkerTest, ArrayClass)
{
    pandasm::Parser p;

    auto source = R"(
        .record R {}
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *klass = ext->GetClass(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("UnknownClass"), 1, &descriptor));
    ASSERT_EQ(klass, nullptr);

    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 0; i < 256U; i++) {
        auto *cls = ext->GetClass(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("R"), i, &descriptor));
        ASSERT_NE(cls, nullptr);
        EXPECT_EQ(utf::Mutf8AsCString(cls->GetDescriptor()), descriptor);
    }
}

static Method *GetMethod(ClassLinker *classLinker, const char *className, const char *methodName,
                         const ark::PandaString &signature)
{
    PandaString descriptor;
    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8(className), &descriptor));
    auto *method = klass->GetDirectMethod(utf::CStringAsMutf8(methodName));
    if (signature != method->GetProto().GetSignature()) {
        return nullptr;
    }
    return method;
}

static std::unordered_set<Method *> GetMethodsSet(Span<Method> methods)
{
    std::unordered_set<Method *> set;
    for (auto &method : methods) {
        set.insert(&method);
    }

    return set;
}

TEST_F(ClassLinkerTest, VTable)
{
    {
        pandasm::Parser p;

        auto source = R"(
            .record A {}

            .function void A.f1() {}
            .function void A.f2(i32 a0) {}

            .function void A.f3(A a0) {}
            .function void A.f4(A a0, i32 a1) {}
        )";

        auto res = p.Parse(source);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());

        auto classLinker = CreateClassLinker(thread_);
        ASSERT_NE(classLinker, nullptr);

        classLinker->AddPandaFile(std::move(pf));

        PandaString descriptor;

        auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *classA = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));
        ASSERT_NE(classA, nullptr);

        auto smethods = classA->GetStaticMethods();
        ASSERT_EQ(smethods.size(), 2U);

        auto vmethods = classA->GetVirtualMethods();
        ASSERT_EQ(vmethods.size(), 2U);

        {
            auto set = GetMethodsSet(smethods);
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "A", "f1", "()V")), set.cend());
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "A", "f2", "(I)V")), set.cend());
        }

        {
            auto set = GetMethodsSet(vmethods);
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "A", "f3", "()V")), set.cend());
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "A", "f4", "(I)V")), set.cend());
        }

        {
            auto vtable = classA->GetVTable();
            ASSERT_EQ(vtable.size(), vmethods.size());

            for (const auto &vmethod : vmethods) {
                ASSERT_EQ(vtable[vmethod.GetVTableIndex()], &vmethod);
            }
        }
    }
}

TEST_F(ClassLinkerTest, VTableInheritance)
{
    {
        pandasm::Parser p;

        auto source = R"(
            .record A {}
            .record B <extends=A> {}
            .record C {}

            .function A A.f1(A a0, i32 a1) {}
            .function A A.f2(A a0, C a1) {}

            .function B B.f1(B a0, i64 a1) {}
            .function B B.f2(B a0, C a1) {}
        )";

        auto res = p.Parse(source);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());

        auto classLinker = CreateClassLinker(thread_);
        ASSERT_NE(classLinker, nullptr);

        classLinker->AddPandaFile(std::move(pf));

        PandaString descriptor;

        auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *classB = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
        ASSERT_NE(classB, nullptr);

        auto *classA = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));
        ASSERT_NE(classA, nullptr);

        auto vtableB = classB->GetVTable();
        ASSERT_EQ(vtableB.size(), 4U);

        auto vtableA = classA->GetVTable();
        ASSERT_EQ(vtableA.size(), 2U);

        {
            auto set = std::unordered_set<Method *> {};
            for (const auto &m : vtableA) {
                set.insert(m);
            }
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "A", "f1", "(I)LA;")), set.cend());
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "A", "f2", "(LC;)LA;")), set.cend());
        }

        {
            auto set = std::unordered_set<Method *> {};
            for (const auto &m : vtableB) {
                set.insert(m);
            }
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "A", "f1", "(I)LA;")), set.cend());
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "B", "f2", "(LC;)LB;")), set.cend());
            ASSERT_NE(set.find(GetMethod(classLinker.get(), "B", "f1", "(J)LB;")), set.cend());
        }
    }
}

TEST_F(ClassLinkerTest, PrimitiveClasses)
{
    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);

    PandaString descriptor;

    auto type = panda_file::Type(panda_file::Type::TypeId::I32);

    auto *primitiveClass = ext->GetClass(ClassHelper::GetPrimitiveDescriptor(type, &descriptor));
    ASSERT_NE(primitiveClass, nullptr);
    EXPECT_STREQ(utf::Mutf8AsCString(primitiveClass->GetDescriptor()),
                 utf::Mutf8AsCString(ClassHelper::GetPrimitiveDescriptor(type, &descriptor)));

    auto *primitiveArrayClass1 = ext->GetClass(ClassHelper::GetPrimitiveArrayDescriptor(type, 1, &descriptor));
    ASSERT_NE(primitiveArrayClass1, nullptr);
    EXPECT_STREQ(utf::Mutf8AsCString(primitiveArrayClass1->GetDescriptor()),
                 utf::Mutf8AsCString(ClassHelper::GetPrimitiveArrayDescriptor(type, 1, &descriptor)));

    auto *primitiveArrayClass2 = ext->GetClass(ClassHelper::GetPrimitiveArrayDescriptor(type, 2, &descriptor));
    ASSERT_NE(primitiveArrayClass2, nullptr);
    EXPECT_STREQ(utf::Mutf8AsCString(primitiveArrayClass2->GetDescriptor()),
                 utf::Mutf8AsCString(ClassHelper::GetPrimitiveArrayDescriptor(type, 2L, &descriptor)));
}

class TestClassLinkerContext : public ClassLinkerContext {
public:
    TestClassLinkerContext(const uint8_t *descriptor, bool needCopyDescriptor, Class *klass,
                           panda_file::SourceLang lang)
        : ClassLinkerContext(lang), descriptor_(descriptor), needCopyDescriptor_(needCopyDescriptor), klass_(klass)
    {
    }

    Class *LoadClass(const uint8_t *descriptor, bool needCopyDescriptor,
                     [[maybe_unused]] ClassLinkerErrorHandler *errorHandler) override
    {
        isSuccess_ = utf::IsEqual(descriptor, descriptor_) && needCopyDescriptor == needCopyDescriptor_;
        InsertClass(klass_);
        return klass_;
    }

    bool IsSuccess() const
    {
        return isSuccess_;
    }

private:
    const uint8_t *descriptor_;
    bool needCopyDescriptor_ {};
    Class *klass_;
    bool isSuccess_ {false};
};

struct LoadContextTestStruct {
    Class *classA;
    Class *classB;
    Class *classArrayB;
};

static void CheckLoadContext(TestClassLinkerContext &ctx, ClassLinker *classLinker,
                             LoadContextTestStruct &loadContextStruct)
{
    auto *classA = loadContextStruct.classA;
    auto *classB = loadContextStruct.classB;
    auto *classArrayB = loadContextStruct.classArrayB;
    {
        PandaUnorderedSet<Class *> expected {classB};
        PandaUnorderedSet<Class *> classes;
        ctx.EnumerateClasses([&](Class *klass) {
            classes.insert(klass);
            return true;
        });

        ASSERT_EQ(classes, expected);
    }

    {
        PandaUnorderedSet<Class *> classes;
        classLinker->EnumerateClasses([&](Class *klass) {
            classes.insert(klass);
            return true;
        });

        ASSERT_NE(classes.find(classA), classes.cend());
        ASSERT_EQ(*classes.find(classA), classA);

        ASSERT_NE(classes.find(classB), classes.cend());
        ASSERT_EQ(*classes.find(classB), classB);

        ASSERT_NE(classes.find(classArrayB), classes.cend());
        ASSERT_EQ(*classes.find(classArrayB), classArrayB);
    }
}

TEST_F(ClassLinkerTest, LoadContext)
{
    pandasm::Parser p;

    auto source = R"(
        .record A {}
        .record B {}
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;
    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classA = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));

    ASSERT_NE(classA, nullptr);
    ASSERT_EQ(classA->GetLoadContext()->IsBootContext(), true);

    auto *classB = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));

    ASSERT_NE(classB, nullptr);
    ASSERT_EQ(classB->GetLoadContext()->IsBootContext(), true);

    auto *desc = ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor);
    TestClassLinkerContext ctx(desc, true, classB, panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classBCtx = ext->GetClass(desc, true, &ctx);

    ASSERT_TRUE(ctx.IsSuccess());
    ASSERT_EQ(classBCtx, classB);

    bool isMatched = false;
    ctx.EnumerateClasses([&isMatched](Class *klass) {
        isMatched = klass->GetName() == "B";
        return true;
    });

    ASSERT_TRUE(isMatched);

    auto *classArrayB =
        classLinker->GetClass(ClassHelper::GetArrayDescriptor(utf::CStringAsMutf8("B"), 1, &descriptor), true, &ctx);

    ASSERT_NE(classArrayB, nullptr);
    ASSERT_EQ(classArrayB->GetLoadContext(), ext->GetBootContext());

    LoadContextTestStruct loadContextStruct {classA, classB, classArrayB};

    CheckLoadContext(ctx, classLinker.get(), loadContextStruct);
}

static void CheckAccesses(ClassLinkerExtension *ext)
{
    // Global
    {
        PandaString descriptor;
        auto klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
        ASSERT_NE(klass, nullptr);
        auto f = klass->GetClassMethod(utf::CStringAsMutf8("f"));
        ASSERT_NE(f, nullptr);
        ASSERT_TRUE(f->IsPublic());
    }

    // record A
    {
        PandaString descriptor;
        auto klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));
        ASSERT_NE(klass, nullptr);
        ASSERT_TRUE(klass->IsPrivate());
        auto f = klass->GetClassMethod(utf::CStringAsMutf8("f"));
        ASSERT_NE(f, nullptr);
        ASSERT_TRUE(f->IsProtected());
    }

    // record B
    {
        PandaString descriptor;
        auto klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
        ASSERT_NE(klass, nullptr);
        ASSERT_TRUE(klass->IsPublic());
        auto f = klass->GetClassMethod(utf::CStringAsMutf8("f"));
        ASSERT_NE(f, nullptr);
        ASSERT_TRUE(f->IsPrivate());

        auto i = 0;
        auto accessPredicates = std::array {&ark::Field::IsPublic, &ark::Field::IsProtected, &ark::Field::IsPrivate};
        for (const auto &field : klass->GetFields()) {
            ASSERT_TRUE((field.*accessPredicates[i])());
            i++;
        }
        ASSERT_EQ(i, klass->GetFields().size());
    }

    // record C
    {
        PandaString descriptor;
        auto klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("C"), &descriptor));
        ASSERT_NE(klass, nullptr);
        ASSERT_TRUE(klass->IsProtected());
    }
}

TEST_F(ClassLinkerTest, Accesses)
{
    auto source = R"(
        .record A <access.record=private> {}

        .record C <access.record=protected> {}

        .record B <access.record=public> {
            i32 pub <access.field=public>
            i32 prt <access.field=protected>
            i32 prv <access.field=private>
        }

        .function void f() <access.function=public> {}
        .function void A.f() <access.function=protected> {}
        .function void B.f() <access.function=private> {}
    )";

    auto res = pandasm::Parser {}.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);

    classLinker->AddPandaFile(std::move(pf));

    CheckAccesses(ext);
}

TEST_F(ClassLinkerTest, Inheritance)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}
    )";

    auto res = pandasm::Parser {}.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);

    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    auto classA = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));
    ASSERT_NE(classA, nullptr);

    auto classB = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
    ASSERT_NE(classB, nullptr);

    ASSERT_EQ(classA->GetBase(), ext->GetClassRoot(ClassRoot::OBJECT));
    ASSERT_EQ(classB->GetBase(), classA);

    ASSERT_TRUE(classB->IsSubClassOf(classA));
}

TEST_F(ClassLinkerTest, IsSubClassOf)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}
        .record C <extends=B> {}
    )";

    auto res = pandasm::Parser {}.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);

    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    auto classA = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));
    ASSERT_NE(classA, nullptr);

    auto classB = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
    ASSERT_NE(classB, nullptr);

    auto classC = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("C"), &descriptor));
    ASSERT_NE(classC, nullptr);

    ASSERT_TRUE(classA->IsSubClassOf(classA));
    ASSERT_FALSE(classA->IsSubClassOf(classB));
    ASSERT_FALSE(classA->IsSubClassOf(classC));

    ASSERT_TRUE(classB->IsSubClassOf(classA));
    ASSERT_TRUE(classB->IsSubClassOf(classB));
    ASSERT_FALSE(classB->IsSubClassOf(classC));

    ASSERT_TRUE(classC->IsSubClassOf(classA));
    ASSERT_TRUE(classC->IsSubClassOf(classB));
    ASSERT_TRUE(classC->IsSubClassOf(classC));
}

TEST_F(ClassLinkerTest, Final)
{
    auto source = R"(
        .record A <final> {}

        .record B {
            i32 fld <final>
        }

        .function void B.f(B a0) <final> {}
    )";

    auto res = pandasm::Parser {}.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);

    classLinker->AddPandaFile(std::move(pf));

    // record A
    {
        PandaString descriptor;
        auto klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));
        ASSERT_NE(klass, nullptr);
        ASSERT_TRUE(klass->IsFinal());
    }

    // record B
    {
        PandaString descriptor;
        auto klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
        ASSERT_NE(klass, nullptr);

        auto f = klass->GetClassMethod(utf::CStringAsMutf8("f"));
        ASSERT_NE(f, nullptr);
        ASSERT_TRUE(f->IsFinal());

        ASSERT_EQ(klass->GetFields().size(), 1);
        for (const auto &field : klass->GetFields()) {
            ASSERT_TRUE(field.IsFinal());
        }
    }
}

}  // namespace ark::test
