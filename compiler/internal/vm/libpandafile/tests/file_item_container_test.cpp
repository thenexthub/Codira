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

#include "annotation_data_accessor.h"
#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "debug_data_accessor-inl.h"
#include "debug_info_extractor.h"
#include "field_data_accessor-inl.h"
#include "file.h"
#include "file_item_container.h"
#include "file_writer.h"
#include "helpers.h"
#include "libpandabase/utils/logger.h"
#include "method_data_accessor-inl.h"
#include "method_handle_data_accessor.h"
#include "modifiers.h"
#include "os/file.h"
#include "proto_data_accessor-inl.h"
#include "pgo.h"
#include "value.h"

#include "zlib.h"

#include <cstddef>

#include <memory>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace panda::panda_file::test {
constexpr uint8_t ISA_VERSION_FIRST_NUMBER = panda::panda_file::version[0];
constexpr uint8_t ISA_VERSION_SECOND_NUMBER = panda::panda_file::version[1];
constexpr uint8_t ISA_VERSION_THIRD_NUMBER = panda::panda_file::version[2]; // 2: the third number of ISA version
constexpr uint8_t ISA_VERSION_FOURTH_NUMBER = panda::panda_file::version[3]; // 3: the fourth number of ISA version
constexpr size_t ELEM_WIDTH = 4;
constexpr size_t ELEM_PER16 = 16 / ELEM_WIDTH;
constexpr uint8_t API_VERSION = 11;

HWTEST(ItemContainer, DeduplicationTest, testing::ext::TestSize.Level0)
{
    ItemContainer container;

    StringItem *string_item = container.GetOrCreateStringItem("1");
    EXPECT_EQ(string_item, container.GetOrCreateStringItem("1"));

    ClassItem *class_item = container.GetOrCreateClassItem("1");
    EXPECT_EQ(class_item, container.GetOrCreateClassItem("1"));

    ValueItem *int_item = container.GetOrCreateIntegerValueItem(1);
    EXPECT_EQ(int_item, container.GetOrCreateIntegerValueItem(1));

    ValueItem *long_item = container.GetOrCreateLongValueItem(1);
    EXPECT_EQ(long_item, container.GetOrCreateLongValueItem(1));
    EXPECT_NE(long_item, int_item);

    ValueItem *float_item = container.GetOrCreateFloatValueItem(1.0);
    EXPECT_EQ(float_item, container.GetOrCreateFloatValueItem(1.0));
    EXPECT_NE(float_item, int_item);
    EXPECT_NE(float_item, long_item);

    ValueItem *double_item = container.GetOrCreateDoubleValueItem(1.0);
    EXPECT_EQ(double_item, container.GetOrCreateDoubleValueItem(1.0));
    EXPECT_NE(double_item, int_item);
    EXPECT_NE(double_item, long_item);
    EXPECT_NE(double_item, float_item);
}

HWTEST(ItemContainer, TestFileOpen, testing::ext::TestSize.Level0)
{
    using panda::os::file::Mode;
    using panda::os::file::Open;

    // Write panda file to disk
    ItemContainer container;

    const std::string file_name = "test_file_open.panda";
    auto writer = FileWriter(file_name);

    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    EXPECT_NE(File::Open(file_name), nullptr);
}

HWTEST(ItemContainer, TestFileFormatVersionTooOldDeathTest, testing::ext::TestSize.Level0)
{
    const std::string file_name = "test_file_format_version_too_old.abc";
    {
        ItemContainer container;
        auto writer = FileWriter(file_name);

        File::Header header;
        errno_t res = memset_s(&header, sizeof(header), 0, sizeof(header));
        if (res != EOK) {
            UNREACHABLE();
        }
        header.magic = File::MAGIC;

        auto old = std::array<uint8_t, File::VERSION_SIZE>(minVersion);
        --old[3];

        header.version = old;
        header.file_size = sizeof(File::Header);

        for (uint8_t b : Span<uint8_t>(reinterpret_cast<uint8_t *>(&header), sizeof(header))) {
            writer.WriteByte(b);
        }
        EXPECT_TRUE(writer.FinishWrite());
    }

#ifdef HOST_UT
    EXPECT_DEATH(File::Open(file_name), ".*");
#endif
}

HWTEST(ItemContainer, TestRewriteChecksum, testing::ext::TestSize.Level0)
{
    const std::string file_name = "test_rewrite_checksum.abc";
    {
        ItemContainer container;
        auto writer = FileWriter(file_name);
        File::Header header;
        errno_t res = memset_s(&header, sizeof(header), 0, sizeof(header));
        EXPECT_EQ(res, EOK);
        header.magic = File::MAGIC;
        header.version = std::array<uint8_t, File::VERSION_SIZE>(minVersion);
        header.file_size = sizeof(File::Header);
        EXPECT_EQ(header.checksum, 0u);
        writer.CountChecksum(true);
        for (uint8_t b : Span<uint8_t>(reinterpret_cast<uint8_t *>(&header), sizeof(header))) {
            writer.WriteByte(b);
        }
        EXPECT_NE(writer.GetChecksum(), 0u);
        writer.CountChecksum(false);
        size_t offset = static_cast<size_t>(reinterpret_cast<uint8_t *>(&(header.checksum)) -
            reinterpret_cast<uint8_t *>(&header));
        EXPECT_TRUE(writer.RewriteChecksum(offset));
        EXPECT_TRUE(writer.FinishWrite());
    }
    const auto &file = File::Open(file_name);
    EXPECT_NE(file, nullptr);
    EXPECT_NE(file->GetHeader()->checksum, 0u);
}

HWTEST(ItemContainer, TestReserveBufferCapacity, testing::ext::TestSize.Level0)
{
    const std::string file_name = "test_reserve_buffer_capacity.abc";
    {
        auto writer = FileWriter(file_name);
        static constexpr size_t CAPACITY = 2000u;
        EXPECT_LT(writer.GetBuffer().capacity(), CAPACITY);
        writer.ReserveBufferCapacity(CAPACITY);
        EXPECT_GE(writer.GetBuffer().capacity(), CAPACITY);
    }
}

HWTEST(ItemContainer, TestFileFormatVersionTooNewDeathTest, testing::ext::TestSize.Level0)
{
    const std::string file_name = "test_file_format_version_too_new.abc";
    {
        ItemContainer container;
        auto writer = FileWriter(file_name);

        File::Header header;
        errno_t res = memset_s(&header, sizeof(header), 0, sizeof(header));
        if (res != EOK) {
            UNREACHABLE();
        }
        header.magic = File::MAGIC;
        if (ISA_VERSION_FIRST_NUMBER + 1 < 256 && ISA_VERSION_SECOND_NUMBER + 1 < 256) {
            header.version = {ISA_VERSION_FIRST_NUMBER + 1, ISA_VERSION_SECOND_NUMBER + 1,
                ISA_VERSION_THIRD_NUMBER, ISA_VERSION_FOURTH_NUMBER};
        } else {
            header.version = {ISA_VERSION_FIRST_NUMBER, ISA_VERSION_SECOND_NUMBER,
                ISA_VERSION_THIRD_NUMBER, ISA_VERSION_FOURTH_NUMBER};
        }
        header.file_size = sizeof(File::Header);

        for (uint8_t b : Span<uint8_t>(reinterpret_cast<uint8_t *>(&header), sizeof(header))) {
            writer.WriteByte(b);
        }
        EXPECT_TRUE(writer.FinishWrite());
    }

#ifdef HOST_UT
    EXPECT_DEATH(File::Open(file_name), ".*");
#endif
}

HWTEST(ItemContainer, TestFileFormatVersionValid, testing::ext::TestSize.Level0)
{
    const std::string file_name = "test_file_format_version_valid.abc";
    {
        ItemContainer container;
        auto writer = FileWriter(file_name);

        File::Header header;
        errno_t res = memset_s(&header, sizeof(header), 0, sizeof(header));
        if (res != EOK) {
            UNREACHABLE();
        }
        header.magic = File::MAGIC;
        header.version = {0, 0, 0, 2};
        header.file_size = sizeof(File::Header);

        for (uint8_t b : Span<uint8_t>(reinterpret_cast<uint8_t *>(&header), sizeof(header))) {
            writer.WriteByte(b);
        }
        EXPECT_TRUE(writer.FinishWrite());
    }

    EXPECT_NE(File::Open(file_name), nullptr);
}

static std::unique_ptr<const File> GetPandaFile(std::vector<uint8_t> &data)
{
    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data.data()), data.size(),
                              [](std::byte *, size_t) noexcept {});
    return File::OpenFromMemory(std::move(ptr));
}

void TestPandaFile(MemoryWriter& mem_writer, std::unique_ptr<const File>& panda_file,
                   ClassItem* class_item, ClassItem* empty_class_item, StringItem* source_file,
                   ClassDataAccessor class_data_accessor)
{
    EXPECT_THAT(panda_file->GetHeader()->version, ::testing::ElementsAre(ISA_VERSION_FIRST_NUMBER,
        ISA_VERSION_SECOND_NUMBER, ISA_VERSION_THIRD_NUMBER, ISA_VERSION_FOURTH_NUMBER));
    EXPECT_EQ(panda_file->GetHeader()->file_size, mem_writer.GetData().size());
    EXPECT_EQ(panda_file->GetHeader()->foreign_off, 0U);
    EXPECT_EQ(panda_file->GetHeader()->foreign_size, 0U);
    EXPECT_EQ(panda_file->GetHeader()->num_classes, 3U);
    EXPECT_EQ(panda_file->GetHeader()->class_idx_off, sizeof(File::Header));

    const uint32_t *class_index =
        reinterpret_cast<const uint32_t *>(panda_file->GetBase() + panda_file->GetHeader()->class_idx_off);
    EXPECT_EQ(class_index[0], class_item->GetOffset());
    EXPECT_EQ(class_index[1], empty_class_item->GetOffset());

    EXPECT_EQ(class_data_accessor.GetSuperClassId().GetOffset(), empty_class_item->GetOffset());
    EXPECT_EQ(class_data_accessor.GetAccessFlags(), ACC_PUBLIC);
    EXPECT_EQ(class_data_accessor.GetFieldsNumber(), 1U);
    EXPECT_EQ(class_data_accessor.GetMethodsNumber(), 1U);
    EXPECT_EQ(class_data_accessor.GetIfacesNumber(), 1U);
    EXPECT_TRUE(class_data_accessor.GetSourceFileId().has_value());
    EXPECT_EQ(class_data_accessor.GetSourceFileId().value().GetOffset(), source_file->GetOffset());
    EXPECT_EQ(class_data_accessor.GetSize(), class_item->GetSize());
}

void TestClassDataAccessor(std::unique_ptr<const File>& panda_file, ClassItem* class_item, ClassItem* iface_item,
                           AnnotationItem* runtime_annotation_item, ClassDataAccessor class_data_accessor)
{
    class_data_accessor.EnumerateInterfaces([&](File::EntityId id) {
        EXPECT_EQ(id.GetOffset(), iface_item->GetOffset());

        ClassDataAccessor iface_class_data_accessor(*panda_file, id);
        EXPECT_EQ(iface_class_data_accessor.GetSuperClassId().GetOffset(), 0U);
        EXPECT_EQ(iface_class_data_accessor.GetAccessFlags(), ACC_PUBLIC);
        EXPECT_EQ(iface_class_data_accessor.GetFieldsNumber(), 0U);
        EXPECT_EQ(iface_class_data_accessor.GetMethodsNumber(), 0U);
        EXPECT_EQ(iface_class_data_accessor.GetIfacesNumber(), 0U);
        EXPECT_FALSE(iface_class_data_accessor.GetSourceFileId().has_value());
        EXPECT_EQ(iface_class_data_accessor.GetSize(), iface_item->GetSize());
    });

    class_data_accessor.EnumerateRuntimeAnnotations([&](File::EntityId id) {
        EXPECT_EQ(id.GetOffset(), runtime_annotation_item->GetOffset());

        AnnotationDataAccessor data_accessor(*panda_file, id);
        EXPECT_EQ(data_accessor.GetAnnotationId().GetOffset(), runtime_annotation_item->GetOffset());
        EXPECT_EQ(data_accessor.GetClassId().GetOffset(), class_item->GetOffset());
        EXPECT_EQ(data_accessor.GetCount(), 0U);
    });
}

void TestAnnotationsAndFields(ClassDataAccessor class_data_accessor, AnnotationItem* annotation_item,
                              std::unique_ptr<const File>& panda_file, ClassItem* class_item,
                              FieldItem* field_item, StringItem* field_name)
{
    class_data_accessor.EnumerateAnnotations([&](File::EntityId id) {
        EXPECT_EQ(id.GetOffset(), annotation_item->GetOffset());

        AnnotationDataAccessor data_accessor(*panda_file, id);
        EXPECT_EQ(data_accessor.GetAnnotationId().GetOffset(), annotation_item->GetOffset());
        EXPECT_EQ(data_accessor.GetClassId().GetOffset(), class_item->GetOffset());
        EXPECT_EQ(data_accessor.GetCount(), 0U);
    });

    class_data_accessor.EnumerateFields([&](FieldDataAccessor &data_accessor) {
        EXPECT_EQ(data_accessor.GetFieldId().GetOffset(), field_item->GetOffset());
        EXPECT_EQ(data_accessor.GetClassId().GetOffset(), class_item->GetOffset());
        EXPECT_EQ(data_accessor.GetNameId().GetOffset(), field_name->GetOffset());
        EXPECT_EQ(data_accessor.GetType(), Type(Type::TypeId::I32).GetFieldEncoding());
        EXPECT_EQ(data_accessor.GetAccessFlags(), ACC_PUBLIC);
        EXPECT_FALSE(data_accessor.GetValue<int32_t>().has_value());
        EXPECT_EQ(data_accessor.GetSize(), field_item->GetSize());

        data_accessor.EnumerateRuntimeAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
        data_accessor.EnumerateAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
    });
}

void TestMethods(ClassDataAccessor class_data_accessor, MethodItem* method_item, ClassItem* class_item,
                 StringItem* method_name, ProtoItem* proto_item)
{
    class_data_accessor.EnumerateMethods([&](MethodDataAccessor &data_accessor) {
        EXPECT_FALSE(data_accessor.IsExternal());
        EXPECT_EQ(data_accessor.GetMethodId().GetOffset(), method_item->GetOffset());
        EXPECT_EQ(data_accessor.GetClassId().GetOffset(), class_item->GetOffset());
        EXPECT_EQ(data_accessor.GetNameId().GetOffset(), method_name->GetOffset());
        EXPECT_EQ(data_accessor.GetProtoId().GetOffset(), proto_item->GetOffset());
        EXPECT_EQ(data_accessor.GetAccessFlags(), ACC_PUBLIC | ACC_STATIC);
        EXPECT_FALSE(data_accessor.GetCodeId().has_value());
        EXPECT_EQ(data_accessor.GetSize(), method_item->GetSize());
        EXPECT_FALSE(data_accessor.GetRuntimeParamAnnotationId().has_value());
        EXPECT_FALSE(data_accessor.GetParamAnnotationId().has_value());
        EXPECT_FALSE(data_accessor.GetDebugInfoId().has_value());

        data_accessor.EnumerateRuntimeAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
        data_accessor.EnumerateAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
    });
}

void TestEmptyClassDataAccessor(std::unique_ptr<const File> &panda_file, ClassItem *empty_class_item)
{
    ClassDataAccessor empty_class_data_accessor(*panda_file, File::EntityId(empty_class_item->GetOffset()));
    EXPECT_EQ(empty_class_data_accessor.GetSuperClassId().GetOffset(), 0U);
    EXPECT_EQ(empty_class_data_accessor.GetAccessFlags(), 0U);
    EXPECT_EQ(empty_class_data_accessor.GetFieldsNumber(), 0U);
    EXPECT_EQ(empty_class_data_accessor.GetMethodsNumber(), 0U);
    EXPECT_EQ(empty_class_data_accessor.GetIfacesNumber(), 0U);
    EXPECT_FALSE(empty_class_data_accessor.GetSourceFileId().has_value());
    EXPECT_EQ(empty_class_data_accessor.GetSize(), empty_class_item->GetSize());
}

HWTEST(ItemContainer, TestClasses, testing::ext::TestSize.Level0)
{
    // Write panda file to memory

    ItemContainer container;

    ClassItem *empty_class_item = container.GetOrCreateClassItem("Foo");

    ClassItem *class_item = container.GetOrCreateClassItem("Bar");
    class_item->SetAccessFlags(ACC_PUBLIC);
    class_item->SetSuperClass(empty_class_item);

    // Add interface

    ClassItem *iface_item = container.GetOrCreateClassItem("Iface");
    iface_item->SetAccessFlags(ACC_PUBLIC);

    class_item->AddInterface(iface_item);

    // Add method

    StringItem *method_name = container.GetOrCreateStringItem("foo");

    PrimitiveTypeItem *ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    ProtoItem *proto_item = container.GetOrCreateProtoItem(ret_type, params);

    MethodItem *method_item = class_item->AddMethod(method_name, proto_item, ACC_PUBLIC | ACC_STATIC, params);

    // Add field

    StringItem *field_name = container.GetOrCreateStringItem("field");
    PrimitiveTypeItem *field_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);

    FieldItem *field_item = class_item->AddField(field_name, field_type, ACC_PUBLIC);

    // Add runtime annotation

    std::vector<AnnotationItem::Elem> runtime_elems;
    std::vector<AnnotationItem::Tag> runtime_tags;
    AnnotationItem *runtime_annotation_item =
        container.CreateItem<AnnotationItem>(class_item, runtime_elems, runtime_tags);

    class_item->AddRuntimeAnnotation(runtime_annotation_item);

    // Add annotation

    std::vector<AnnotationItem::Elem> elems;
    std::vector<AnnotationItem::Tag> tags;
    AnnotationItem *annotation_item = container.CreateItem<AnnotationItem>(class_item, elems, tags);

    class_item->AddAnnotation(annotation_item);

    // Add source file

    StringItem *source_file = container.GetOrCreateStringItem("source_file");

    class_item->SetSourceFile(source_file);

    MemoryWriter mem_writer;
    container.DeduplicateItems();
    ASSERT_TRUE(container.Write(&mem_writer));

    // Read panda file from memory

    auto data = mem_writer.GetData();
    auto panda_file = GetPandaFile(data);

    ASSERT_NE(panda_file, nullptr);

    std::vector<uint8_t> class_name {'B', 'a', 'r', '\0'};
    auto class_id = panda_file->GetClassId(class_name.data());
    EXPECT_EQ(class_id.GetOffset(), class_item->GetOffset());

    ClassDataAccessor class_data_accessor(*panda_file, class_id);

    TestPandaFile(mem_writer, panda_file, class_item, empty_class_item, source_file, class_data_accessor);
    TestClassDataAccessor(panda_file, class_item, iface_item, runtime_annotation_item, class_data_accessor);

    // Annotation is the same as the runtime one, so we deduplicate it
    EXPECT_FALSE(annotation_item->NeedsEmit());
    annotation_item = runtime_annotation_item;

    TestAnnotationsAndFields(class_data_accessor, annotation_item, panda_file, class_item, field_item, field_name);

    TestMethods(class_data_accessor, method_item, class_item, method_name, proto_item);

    TestEmptyClassDataAccessor(panda_file, empty_class_item);
}

HWTEST(ItemContainer, TestMethods, testing::ext::TestSize.Level0)
{
    // Write panda file to memory

    ItemContainer container;

    ClassItem *class_item = container.GetOrCreateClassItem("A");
    class_item->SetAccessFlags(ACC_PUBLIC);

    StringItem *method_name = container.GetOrCreateStringItem("foo");

    PrimitiveTypeItem *ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    ProtoItem *proto_item = container.GetOrCreateProtoItem(ret_type, params);

    MethodItem *method_item = class_item->AddMethod(method_name, proto_item, ACC_PUBLIC | ACC_STATIC, params);

    std::vector<uint8_t> instructions {1, 2, 3, 4};
    CodeItem *code_item = container.CreateItem<CodeItem>(0, 2, instructions);

    method_item->SetCode(code_item);

    MemoryWriter mem_writer;

    ASSERT_TRUE(container.Write(&mem_writer));

    // Read panda file from memory

    auto data = mem_writer.GetData();
    auto panda_file = GetPandaFile(data);

    ASSERT_NE(panda_file, nullptr);

    ClassDataAccessor class_data_accessor(*panda_file, File::EntityId(class_item->GetOffset()));

    class_data_accessor.EnumerateMethods([&](MethodDataAccessor &data_accessor) {
        EXPECT_FALSE(data_accessor.IsExternal());
        EXPECT_EQ(data_accessor.GetMethodId().GetOffset(), method_item->GetOffset());
        EXPECT_EQ(data_accessor.GetClassId().GetOffset(), class_item->GetOffset());
        EXPECT_EQ(data_accessor.GetNameId().GetOffset(), method_name->GetOffset());
        EXPECT_EQ(data_accessor.GetProtoId().GetOffset(), proto_item->GetOffset());
        EXPECT_EQ(data_accessor.GetAccessFlags(), ACC_PUBLIC | ACC_STATIC);
        EXPECT_EQ(data_accessor.GetSize(), method_item->GetSize());

        auto code_id = data_accessor.GetCodeId();
        EXPECT_TRUE(code_id.has_value());
        EXPECT_EQ(code_id.value().GetOffset(), code_item->GetOffset());

        CodeDataAccessor code_data_accessor(*panda_file, code_id.value());
        EXPECT_EQ(code_data_accessor.GetNumVregs(), 0U);
        EXPECT_EQ(code_data_accessor.GetNumArgs(), 2U);
        EXPECT_EQ(code_data_accessor.GetCodeSize(), instructions.size());
        EXPECT_THAT(instructions, ::testing::ElementsAreArray(code_data_accessor.GetInstructions(),
                                                              code_data_accessor.GetCodeSize()));

        EXPECT_EQ(code_data_accessor.GetTriesSize(), 0U);
        EXPECT_EQ(code_data_accessor.GetSize(), code_item->GetSize());

        code_data_accessor.EnumerateTryBlocks([](const CodeDataAccessor::TryBlock &) {
            EXPECT_TRUE(false);
            return false;
        });

        EXPECT_FALSE(data_accessor.GetDebugInfoId().has_value());

        EXPECT_FALSE(data_accessor.GetRuntimeParamAnnotationId().has_value());

        EXPECT_FALSE(data_accessor.GetParamAnnotationId().has_value());

        data_accessor.EnumerateRuntimeAnnotations([](File::EntityId) { EXPECT_TRUE(false); });

        data_accessor.EnumerateAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
    });
}

void TestProtoDataAccessor(MemoryWriter& mem_writer, ClassItem* class_item, MethodItem* method_item,
                           ProtoItem* proto_item, std::vector<Type::TypeId> types, std::vector<ClassItem *> ref_types)
{
    auto data = mem_writer.GetData();
    auto panda_file = GetPandaFile(data);

    ASSERT_NE(panda_file, nullptr);

    ClassDataAccessor class_data_accessor(*panda_file, File::EntityId(class_item->GetOffset()));

    class_data_accessor.EnumerateMethods([&](MethodDataAccessor &data_accessor) {
        EXPECT_EQ(data_accessor.GetMethodId().GetOffset(), method_item->GetOffset());
        EXPECT_EQ(data_accessor.GetProtoId().GetOffset(), proto_item->GetOffset());

        ProtoDataAccessor proto_data_accessor(*panda_file, data_accessor.GetProtoId());
        EXPECT_EQ(proto_data_accessor.GetProtoId().GetOffset(), proto_item->GetOffset());

        size_t num = 0;
        size_t nref = 0;
        proto_data_accessor.EnumerateTypes([&](Type t) {
            EXPECT_EQ(t.GetEncoding(), Type(types[num]).GetEncoding());
            ++num;

            if (!t.IsPrimitive()) {
                ++nref;
            }
        });

        EXPECT_EQ(num, types.size());

        for (size_t i = 0; i < num - 1; i++) {
            EXPECT_EQ(proto_data_accessor.GetArgType(i).GetEncoding(), Type(types[i + 1]).GetEncoding());
        }

        EXPECT_EQ(proto_data_accessor.GetReturnType().GetEncoding(), Type(types[0]).GetEncoding());

        EXPECT_EQ(nref, ref_types.size());

        for (size_t i = 0; i < nref; i++) {
            EXPECT_EQ(proto_data_accessor.GetReferenceType(0).GetOffset(), ref_types[i]->GetOffset());
        }

        size_t size = ((num + ELEM_PER16) / ELEM_PER16 + nref) * sizeof(uint16_t);

        EXPECT_EQ(proto_data_accessor.GetSize(), size);
        EXPECT_EQ(proto_data_accessor.GetSize(), proto_item->GetSize());
    });
}

void TestProtos(size_t n)
{
    // Write panda file to memory
    ItemContainer::SetApi(API_VERSION);
    ItemContainer container;

    ClassItem *class_item = container.GetOrCreateClassItem("A");
    class_item->SetAccessFlags(ACC_PUBLIC);

    StringItem *method_name = container.GetOrCreateStringItem("foo");

    std::vector<Type::TypeId> types {Type::TypeId::VOID, Type::TypeId::I32};
    std::vector<ClassItem *> ref_types;

    PrimitiveTypeItem *ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;

    params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));

    for (size_t i = 0; i < ELEM_PER16 * 2 - 2; i++) {
        params.emplace_back(container.GetOrCreateClassItem("B"));
        types.push_back(Type::TypeId::REFERENCE);
        ref_types.push_back(container.GetOrCreateClassItem("B"));
        params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::F64));
        types.push_back(Type::TypeId::F64);
    }

    for (size_t i = 0; i < n; i++) {
        params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::F32));
        types.push_back(Type::TypeId::F32);
    }

    ProtoItem *proto_item = container.GetOrCreateProtoItem(ret_type, params);

    MethodItem *method_item = class_item->AddMethod(method_name, proto_item, ACC_PUBLIC | ACC_STATIC, params);

    MemoryWriter mem_writer;

    ASSERT_TRUE(container.Write(&mem_writer));

    // Read panda file from memory

    TestProtoDataAccessor(mem_writer, class_item, method_item, proto_item, types, ref_types);
}

HWTEST(ItemContainer, TestProtos, testing::ext::TestSize.Level0)
{
    TestProtos(0);
    TestProtos(1);
    TestProtos(2);
    TestProtos(7);
}

HWTEST(ItemContainer, TestDebugInfo, testing::ext::TestSize.Level0)
{
    // Write panda file to memory

    ItemContainer container;

    ClassItem *class_item = container.GetOrCreateClassItem("A");
    class_item->SetAccessFlags(ACC_PUBLIC);

    StringItem *method_name = container.GetOrCreateStringItem("foo");

    PrimitiveTypeItem *ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *proto_item = container.GetOrCreateProtoItem(ret_type, params);
    MethodItem *method_item = class_item->AddMethod(method_name, proto_item, ACC_PUBLIC | ACC_STATIC, params);

    StringItem *source_file_item = container.GetOrCreateStringItem("<source>");
    StringItem *source_code_item = container.GetOrCreateStringItem("let a = 1;");
    StringItem *param_string_item = container.GetOrCreateStringItem("a0");

    LineNumberProgramItem *line_number_program_item = container.CreateLineNumberProgramItem();
    DebugInfoItem *debug_info_item = container.CreateItem<DebugInfoItem>(line_number_program_item);
    method_item->SetDebugInfo(debug_info_item);

    // Create foreign class
    ForeignClassItem *class_item_foreign = container.GetOrCreateForeignClassItem("ForeignClass");

    // Create foreign method
    StringItem *method_name_foreign = container.GetOrCreateStringItem("ForeignMethod");
    PrimitiveTypeItem *ret_type_foreign = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params_foreign;
    params_foreign.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *proto_item_foreign = container.GetOrCreateProtoItem(ret_type_foreign, params_foreign);
    container.CreateItem<ForeignMethodItem>(class_item_foreign,
        method_name_foreign, proto_item_foreign, 0);

    // Add debug info

    container.ComputeLayout();

    std::vector<uint8_t> opcodes {
        static_cast<uint8_t>(LineNumberProgramItem::Opcode::SET_SOURCE_CODE),
        static_cast<uint8_t>(LineNumberProgramItem::Opcode::SET_FILE),
        static_cast<uint8_t>(LineNumberProgramItem::Opcode::SET_PROLOGUE_END),
        static_cast<uint8_t>(LineNumberProgramItem::Opcode::ADVANCE_PC),
        static_cast<uint8_t>(LineNumberProgramItem::Opcode::ADVANCE_LINE),
        static_cast<uint8_t>(LineNumberProgramItem::Opcode::SET_EPILOGUE_BEGIN),
        static_cast<uint8_t>(LineNumberProgramItem::Opcode::END_SEQUENCE),
    };

    auto *constant_pool = debug_info_item->GetConstantPool();
    debug_info_item->SetLineNumber(5);
    line_number_program_item->EmitSetSourceCode(constant_pool, source_code_item);
    line_number_program_item->EmitSetFile(constant_pool, source_file_item);
    line_number_program_item->EmitPrologEnd();
    line_number_program_item->EmitAdvancePc(constant_pool, 10);
    line_number_program_item->EmitAdvanceLine(constant_pool, -5);
    line_number_program_item->EmitEpilogBegin();
    line_number_program_item->EmitEnd();

    debug_info_item->AddParameter(param_string_item);

    method_item->SetDebugInfo(debug_info_item);

    MemoryWriter mem_writer;

    ASSERT_TRUE(container.Write(&mem_writer));

    // Read panda file from memory

    auto data = mem_writer.GetData();
    auto panda_file = GetPandaFile(data);

    ASSERT_NE(panda_file, nullptr);

    ClassDataAccessor class_data_accessor(*panda_file, File::EntityId(class_item->GetOffset()));

    class_data_accessor.EnumerateMethods([&](MethodDataAccessor &data_accessor) {
        EXPECT_EQ(data_accessor.GetMethodId().GetOffset(), method_item->GetOffset());
        EXPECT_EQ(data_accessor.GetSize(), method_item->GetSize());

        auto debug_info_id = data_accessor.GetDebugInfoId();
        EXPECT_TRUE(debug_info_id.has_value());

        EXPECT_EQ(debug_info_id.value().GetOffset(), debug_info_item->GetOffset());

        DebugInfoDataAccessor dda(*panda_file, debug_info_id.value());
        EXPECT_EQ(dda.GetDebugInfoId().GetOffset(), debug_info_item->GetOffset());
        EXPECT_EQ(dda.GetLineStart(), 5U);
        EXPECT_EQ(dda.GetNumParams(), params.size());

        dda.EnumerateParameters([&](File::EntityId id) { EXPECT_EQ(id.GetOffset(), param_string_item->GetOffset()); });

        auto cp = dda.GetConstantPool();
        EXPECT_EQ(cp.size(), constant_pool->size());
        EXPECT_THAT(*constant_pool, ::testing::ElementsAreArray(cp.data(), cp.Size()));

        EXPECT_EQ(helpers::ReadULeb128(&cp), source_code_item->GetOffset());
        EXPECT_EQ(helpers::ReadULeb128(&cp), source_file_item->GetOffset());
        EXPECT_EQ(helpers::ReadULeb128(&cp), 10U);
        EXPECT_EQ(helpers::ReadLeb128(&cp), -5);

        const uint8_t *line_number_program = dda.GetLineNumberProgram();
        EXPECT_EQ(panda_file->GetIdFromPointer(line_number_program).GetOffset(), line_number_program_item->GetOffset());
        EXPECT_EQ(line_number_program_item->GetSize(), opcodes.size());

        EXPECT_THAT(opcodes, ::testing::ElementsAreArray(line_number_program, opcodes.size()));

        EXPECT_EQ(dda.GetSize(), debug_info_item->GetSize());
    });

    DebugInfoExtractor extractor(panda_file.get());
    const auto &methods = extractor.GetMethodIdList();
    for (const auto &method_id : methods) {
        for (const auto &info : extractor.GetParameterInfo(method_id)) {
            EXPECT_EQ(info.name, "a0");
        }
    }
}

HWTEST(ItemContainer, ForeignItems, testing::ext::TestSize.Level0)
{
    ItemContainer container;

    // Create foreign class
    ForeignClassItem *class_item = container.GetOrCreateForeignClassItem("ForeignClass");

    // Create foreign field
    StringItem *field_name = container.GetOrCreateStringItem("foreign_field");
    PrimitiveTypeItem *field_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    ForeignFieldItem *field_item = container.CreateItem<ForeignFieldItem>(class_item, field_name, field_type);

    // Create foreign method
    StringItem *method_name = container.GetOrCreateStringItem("ForeignMethod");
    PrimitiveTypeItem *ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *proto_item = container.GetOrCreateProtoItem(ret_type, params);
    ForeignMethodItem *method_item = container.CreateItem<ForeignMethodItem>(class_item, method_name, proto_item, 0);

    MemoryWriter mem_writer;

    ASSERT_TRUE(container.Write(&mem_writer));

    // Read panda file from memory

    auto data = mem_writer.GetData();
    auto panda_file = GetPandaFile(data);

    ASSERT_NE(panda_file, nullptr);

    EXPECT_EQ(panda_file->GetHeader()->foreign_off, class_item->GetOffset());

    size_t foreign_size = class_item->GetSize() + field_item->GetSize() + method_item->GetSize();
    EXPECT_EQ(panda_file->GetHeader()->foreign_size, foreign_size);

    ASSERT_TRUE(panda_file->IsExternal(class_item->GetFileId()));

    MethodDataAccessor method_data_accessor(*panda_file, method_item->GetFileId());
    EXPECT_EQ(method_data_accessor.GetMethodId().GetOffset(), method_item->GetOffset());
    EXPECT_EQ(method_data_accessor.GetSize(), method_item->GetSize());
    EXPECT_EQ(method_data_accessor.GetClassId().GetOffset(), class_item->GetOffset());
    EXPECT_EQ(method_data_accessor.GetNameId().GetOffset(), method_name->GetOffset());
    EXPECT_EQ(method_data_accessor.GetProtoId().GetOffset(), proto_item->GetOffset());
    EXPECT_TRUE(method_data_accessor.IsExternal());

    FieldDataAccessor field_data_accessor(*panda_file, field_item->GetFileId());
    EXPECT_EQ(field_data_accessor.GetFieldId().GetOffset(), field_item->GetOffset());
    EXPECT_EQ(field_data_accessor.GetSize(), field_item->GetSize());
    EXPECT_EQ(field_data_accessor.GetClassId().GetOffset(), class_item->GetOffset());
    EXPECT_EQ(field_data_accessor.GetNameId().GetOffset(), field_name->GetOffset());
    EXPECT_EQ(field_data_accessor.GetType(), field_type->GetType().GetFieldEncoding());
    EXPECT_TRUE(field_data_accessor.IsExternal());
}

HWTEST(ItemContainer, EmptyContainerChecksum, testing::ext::TestSize.Level0)
{
    using panda::os::file::Mode;
    using panda::os::file::Open;

    // Write panda file to disk
    ItemContainer container;

    const std::string file_name = "test_empty_checksum.ark";
    auto writer = FileWriter(file_name);

    // Initial value of adler32
    EXPECT_EQ(writer.GetChecksum(), 1U);
    ASSERT_TRUE(container.Write(&writer));

    // At least header was written so the checksum should be changed
    auto container_checksum = writer.GetChecksum();
    EXPECT_NE(container_checksum, 1U);

    // Read panda file from disk
    auto file = File::Open(file_name);
    EXPECT_NE(file, nullptr);
    EXPECT_EQ(file->GetHeader()->checksum, container_checksum);

    constexpr size_t DATA_OFFSET = 12U;
    auto checksum = adler32(1, file->GetBase() + DATA_OFFSET, file->GetHeader()->file_size - DATA_OFFSET);
    EXPECT_EQ(file->GetHeader()->checksum, checksum);
}

HWTEST(ItemContainer, ContainerChecksum, testing::ext::TestSize.Level0)
{
    using panda::os::file::Mode;
    using panda::os::file::Open;

    uint32_t empty_checksum = 0;
    {
        ItemContainer container;
        const std::string file_name = "test_checksum_empty.ark";
        auto writer = FileWriter(file_name);
        ASSERT_TRUE(container.Write(&writer));
        empty_checksum = writer.GetChecksum();
    }
    ASSERT(empty_checksum != 0);

    // Create not empty container
    ItemContainer container;
    container.GetOrCreateClassItem("C");

    const std::string file_name = "test_checksum.ark";
    auto writer = FileWriter(file_name);

    ASSERT_TRUE(container.Write(&writer));

    // This checksum must be different from the empty one (collision may happen though)
    auto container_checksum = writer.GetChecksum();
    EXPECT_NE(empty_checksum, container_checksum);

    // Read panda file from disk
    auto file = File::Open(file_name);
    EXPECT_NE(file, nullptr);
    EXPECT_EQ(file->GetHeader()->checksum, container_checksum);

    constexpr size_t DATA_OFFSET = 12U;
    auto checksum = adler32(1, file->GetBase() + DATA_OFFSET, file->GetHeader()->file_size - DATA_OFFSET);
    EXPECT_EQ(file->GetHeader()->checksum, checksum);
}

void CreateItems(ItemContainer& container)
{
    // Add classes
    ClassItem *empty_class_item = container.GetOrCreateClassItem("LTest;");
    ClassItem *class_item_a = container.GetOrCreateClassItem("LAA;");
    class_item_a->SetSuperClass(empty_class_item);
    ClassItem *class_item_b = container.GetOrCreateClassItem("LBB;");

    // Add method1
    StringItem *method_name_1 = container.GetOrCreateStringItem("foo1");
    PrimitiveTypeItem *ret_type_1 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params_1;
    ProtoItem *proto_item_1 = container.GetOrCreateProtoItem(ret_type_1, params_1);
    MethodItem *method_item_1 = class_item_a->AddMethod(method_name_1, proto_item_1, ACC_PUBLIC | ACC_STATIC, params_1);
    // Set code_1
    std::vector<uint8_t> instructions_1 {1, 2, 3, 4};
    CodeItem *code_item_1 = container.CreateItem<CodeItem>(0, 2, instructions_1);
    method_item_1->SetCode(code_item_1);
    code_item_1->AddMethod(method_item_1);

    // Add method2
    StringItem *method_name_2 = container.GetOrCreateStringItem("foo2");
    PrimitiveTypeItem *ret_type_2 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    std::vector<MethodParamItem> params_2;
    ProtoItem *proto_item_2 = container.GetOrCreateProtoItem(ret_type_2, params_2);
    MethodItem *method_item_2 = class_item_b->AddMethod(method_name_2, proto_item_2, ACC_PUBLIC | ACC_STATIC, params_2);
    // Set code_2
    std::vector<uint8_t> instructions_2 {5, 6, 7, 8};
    CodeItem *code_item_2 = container.CreateItem<CodeItem>(0, 2, instructions_2);
    method_item_2->SetCode(code_item_2);
    code_item_2->AddMethod(method_item_2);

    // Add method_3
    StringItem *method_name_3 = container.GetOrCreateStringItem("foo3");
    auto *method_item_3 = empty_class_item->AddMethod(method_name_3, proto_item_1, ACC_PUBLIC | ACC_STATIC, params_1);
    // Set code_3
    std::vector<uint8_t> instructions_3 {3, 4, 5, 6};
    CodeItem *code_item_3 = container.CreateItem<CodeItem>(0, 2, instructions_3);
    method_item_3->SetCode(code_item_3);
    code_item_3->AddMethod(method_item_3);

    // Add method_4
    StringItem *method_name_4 = container.GetOrCreateStringItem("foo4");
    auto *method_item_4 = empty_class_item->AddMethod(method_name_4, proto_item_1, ACC_PUBLIC | ACC_STATIC, params_1);
    // Set code. method_4 and method_3 share code_item_3
    method_item_4->SetCode(code_item_3);
    code_item_3->AddMethod(method_item_4);

    // Add field
    StringItem *field_name = container.GetOrCreateStringItem("test_field");
    PrimitiveTypeItem *field_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    class_item_a->AddField(field_name, field_type, ACC_PUBLIC);

    // Add source file
    StringItem *source_file = container.GetOrCreateStringItem("source_file");
    class_item_a->SetSourceFile(source_file);
}

void CheckItemBeforePGO1(std::list<std::unique_ptr<BaseItem>>::const_iterator& item,
                         std::string_view PRIMITIVE_TYPE_ITEM,
                         std::string_view PROTO_ITEM, std::string_view END_ITEM)
{
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "Test");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "AA");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "BB");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo1");
    item++;
    EXPECT_EQ((*item)->GetName(), PRIMITIVE_TYPE_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), PROTO_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo2");
    item++;
    EXPECT_EQ((*item)->GetName(), PRIMITIVE_TYPE_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), PROTO_ITEM);
    item++;
}

void CheckItemBeforePGO2(std::list<std::unique_ptr<BaseItem>>::const_iterator& item,
                         std::string_view PRIMITIVE_TYPE_ITEM,
                         std::string_view PROTO_ITEM, std::string_view END_ITEM)
{
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo3");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo4");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "test_field");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "source_file");
    item++;
    EXPECT_EQ((*item)->GetName(), END_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), CODE_ITEM);
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[0], "AA::foo1");
    item++;
    EXPECT_EQ((*item)->GetName(), CODE_ITEM);
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[0], "BB::foo2");
    item++;
    EXPECT_EQ((*item)->GetName(), CODE_ITEM);
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[0], "Test::foo3");
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[1], "Test::foo4");
    item++;
    EXPECT_EQ((*item)->GetName(), END_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), END_ITEM);
    item++;
}

void CheckItemAfterPGO1(std::list<std::unique_ptr<BaseItem>>::const_iterator& item,
                        std::string_view PRIMITIVE_TYPE_ITEM,
                        std::string_view PROTO_ITEM, std::string_view END_ITEM)
{
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "test_field");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo1");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo2");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo3");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo4");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "source_file");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "BB");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "Test");
    item++;
}

void CheckItemAfterPGO2(std::list<std::unique_ptr<BaseItem>>::const_iterator& item,
                        std::string_view PRIMITIVE_TYPE_ITEM,
                        std::string_view PROTO_ITEM, std::string_view END_ITEM)
{
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(panda::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "AA");
    item++;
    EXPECT_EQ((*item)->GetName(), PRIMITIVE_TYPE_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), PROTO_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), PRIMITIVE_TYPE_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), PROTO_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), END_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), END_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), END_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), CODE_ITEM);
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[0], "BB::foo2");
    item++;
    EXPECT_EQ((*item)->GetName(), CODE_ITEM);
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[0], "Test::foo3");
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[1], "Test::foo4");
    item++;
    EXPECT_EQ((*item)->GetName(), CODE_ITEM);
    EXPECT_EQ(static_cast<CodeItem *>((*item).get())->GetMethodNames()[0], "AA::foo1");
    item++;
}

HWTEST(ItemContainer, TestProfileGuidedRelayout, testing::ext::TestSize.Level0)
{
    ItemContainer container;

    // Add
    CreateItems(container);

    constexpr std::string_view PRIMITIVE_TYPE_ITEM = "primitive_type_item";
    constexpr std::string_view PROTO_ITEM = "proto_item";
    constexpr std::string_view END_ITEM = "end_item";

    // Items before PGO
    const auto &items = container.GetItems();
    auto item = items.begin();
    CheckItemBeforePGO1(item, PRIMITIVE_TYPE_ITEM, PROTO_ITEM, END_ITEM);
    CheckItemBeforePGO2(item, PRIMITIVE_TYPE_ITEM, PROTO_ITEM, END_ITEM);
    EXPECT_EQ(item, items.end());

    // Prepare profile data
    std::string profile_path = "TestProfileGuidedRelayout_profile_test_data.txt";
    std::ofstream test_file;
    test_file.open(profile_path);
    test_file << "string_item:test_field" << std::endl;
    test_file << "class_item:BB" << std::endl;
    test_file << "code_item:BB::foo2" << std::endl;
    test_file << "code_item:Test::foo4" << std::endl;
    test_file.close();

    // Run PGO
    panda::panda_file::pgo::ProfileOptimizer profile_opt;
    profile_opt.SetProfilePath(profile_path);
    container.ReorderItems(&profile_opt);

    // Items after PGO
    item = items.begin();
    CheckItemAfterPGO1(item, PRIMITIVE_TYPE_ITEM, PROTO_ITEM, END_ITEM);
    CheckItemAfterPGO2(item, PRIMITIVE_TYPE_ITEM, PROTO_ITEM, END_ITEM);
    EXPECT_EQ(item, items.end());
}

void PerformTests(ItemContainer& container)
{
    ASSERT_TRUE(container.GetItems().size() == 14);

    std::map<std::string, panda_file::BaseClassItem *> *class_map = container.GetClassMap();
    ASSERT_TRUE(class_map != nullptr && class_map->size() == 2);
    auto it = class_map->find("Bar");
    ASSERT_TRUE(it != class_map->end());

    std::unordered_map<std::string, StringItem *> *string_map = container.GetStringMap();
    ASSERT_TRUE(string_map != nullptr && string_map->size() == 4);
    auto sit0 = string_map->find("field");
    auto sit1 = string_map->find("source_file");
    auto sit2 = string_map->find("foo1");
    auto sit3 = string_map->find("foo2");
    ASSERT_TRUE(sit0 != string_map->end() && sit1 != string_map->end() && sit2 != string_map->end() &&
                sit3 != string_map->end());

    std::unordered_map<Type::TypeId, PrimitiveTypeItem *> *primitive_type_map = container.GetPrimitiveTypeMap();
    ASSERT_TRUE(primitive_type_map != nullptr && primitive_type_map->size() == 3);
    auto pit0 = primitive_type_map->find(Type::TypeId::F32);
    auto pit1 = primitive_type_map->find(Type::TypeId::I32);
    auto pit2 = primitive_type_map->find(Type::TypeId::VOID);
    ASSERT_TRUE(pit0 != primitive_type_map->end() && pit1 != primitive_type_map->end() &&
                pit2 != primitive_type_map->end());

    auto *rclass_item = static_cast<panda_file::ClassItem *>(it->second);
    std::string method_name;
    std::function<bool(BaseItem *)> TestMethod = [&](BaseItem *method) {
        auto *method_item = static_cast<panda_file::MethodItem *>(method);
        ASSERT(method_item != nullptr && method_item->GetItemType() == ItemTypes::METHOD_ITEM);
        method_name = method_item->GetNameItem()->GetData();
        method_name.pop_back();  // remove '\0'
        ASSERT(method_name == "foo1" || method_name == "foo2");
        return true;
    };

    using std::placeholders::_1;
    panda_file::BaseItem::VisitorCallBack cb_method = TestMethod;
    rclass_item->VisitMethods(cb_method);

    std::string f_name;
    std::function<bool(BaseItem *)> TestField = [&](BaseItem *field) {
        auto *field_item = static_cast<panda_file::FieldItem *>(field);
        ASSERT(field_item != nullptr && field_item->GetItemType() == ItemTypes::FIELD_ITEM);
        f_name = field_item->GetNameItem()->GetData();
        f_name.pop_back();  // remove '\0'
        ASSERT(f_name == "field");
        return true;
    };

    panda_file::BaseItem::VisitorCallBack cb_field = TestField;
    rclass_item->VisitFields(cb_field);
}

HWTEST(ItemContainer, GettersTest, testing::ext::TestSize.Level0)
{
    ItemContainer container;

    ClassItem *empty_class_item = container.GetOrCreateClassItem("Foo");

    ClassItem *class_item = container.GetOrCreateClassItem("Bar");
    class_item->SetAccessFlags(ACC_PUBLIC);
    class_item->SetSuperClass(empty_class_item);

    // Add methods

    StringItem *method_name1 = container.GetOrCreateStringItem("foo1");

    PrimitiveTypeItem *ret_type1 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params1;
    params1.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *proto_item1 = container.GetOrCreateProtoItem(ret_type1, params1);

    class_item->AddMethod(method_name1, proto_item1, ACC_PUBLIC | ACC_STATIC, params1);

    StringItem *method_name2 = container.GetOrCreateStringItem("foo2");

    PrimitiveTypeItem *ret_type2 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    std::vector<MethodParamItem> params2;
    params2.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::F32));
    ProtoItem *proto_item2 = container.GetOrCreateProtoItem(ret_type2, params2);

    class_item->AddMethod(method_name2, proto_item2, ACC_PUBLIC | ACC_STATIC, params2);

    // Add field

    StringItem *field_name = container.GetOrCreateStringItem("field");
    PrimitiveTypeItem *field_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);

    class_item->AddField(field_name, field_type, ACC_PUBLIC);

    // Add source file

    StringItem *source_file = container.GetOrCreateStringItem("source_file");

    class_item->SetSourceFile(source_file);

    // Read items from container

    PerformTests(container);
}

HWTEST(ItemContainer, IndexedItemGlobalIndexTest, testing::ext::TestSize.Level0)
{
    ItemContainer container;
    EXPECT_EQ(container.GetIndexedItemCount(), 0U);

    // Create foreign class
    ForeignClassItem *foreign_class_item = container.GetOrCreateForeignClassItem("foreign_class");
    EXPECT_EQ(foreign_class_item->GetIndexedItemCount(), 0U);
    // BaseClassItem will initialize one StringItem member, which will increase the count by 1.
    EXPECT_EQ(container.GetIndexedItemCount(), foreign_class_item->GetIndexedItemCount() + 2);

    // Create foreign field
    StringItem *foreign_field_name = container.GetOrCreateStringItem("foreign_field");
    PrimitiveTypeItem *foreign_field_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    ForeignFieldItem *foreign_field_item = container.CreateItem<ForeignFieldItem>(foreign_class_item,
        foreign_field_name, foreign_field_type);
    EXPECT_EQ(foreign_field_item->GetIndexedItemCount(), 4U);
    EXPECT_EQ(container.GetIndexedItemCount(), foreign_field_item->GetIndexedItemCount() + 1);

    // Create foreign method
    StringItem *foreign_method_name = container.GetOrCreateStringItem("foreign_method");
    PrimitiveTypeItem *foreign_ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> foreign_params;
    foreign_params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I64));
    ProtoItem *foreign_proto_item = container.GetOrCreateProtoItem(foreign_ret_type, foreign_params);
    ForeignMethodItem *foreign_method_item = container.CreateItem<ForeignMethodItem>(foreign_class_item,
        foreign_method_name, foreign_proto_item, 0);
    EXPECT_EQ(foreign_method_item->GetIndexedItemCount(), 9U);
    EXPECT_EQ(container.GetIndexedItemCount(), foreign_method_item->GetIndexedItemCount() + 1);

    // Create class
    ClassItem *class_item = container.GetOrCreateClassItem("classA");
    EXPECT_EQ(class_item->GetIndexedItemCount(), 10U);
    EXPECT_EQ(container.GetIndexedItemCount(), class_item->GetIndexedItemCount() + 2);

    // Create method
    StringItem *method_name = container.GetOrCreateStringItem("a");
    // TypeId::VOID is repeated, count won't increase
    PrimitiveTypeItem *ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    ProtoItem *proto_item = container.GetOrCreateProtoItem(ret_type, params);
    MethodItem *method_item = class_item->AddMethod(method_name, proto_item, ACC_PUBLIC | ACC_STATIC, params);
    EXPECT_EQ(method_item->GetIndexedItemCount(), 14U);
    EXPECT_EQ(container.GetIndexedItemCount(), method_item->GetIndexedItemCount() + 1);

    // Create field
    StringItem *field_name = container.GetOrCreateStringItem("field");
    PrimitiveTypeItem *field_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    FieldItem *field_item = class_item->AddField(field_name, field_type, ACC_PUBLIC);
    EXPECT_EQ(field_item->GetIndexedItemCount(), 16U);
    EXPECT_EQ(container.GetIndexedItemCount(), field_item->GetIndexedItemCount() + 1);

    // Create code, item count is not expected to increase
    std::vector<uint8_t> instructions {1, 2, 3, 4};
    CodeItem *code_item = container.CreateItem<CodeItem>(0, 2, instructions);
    method_item->SetCode(code_item);
    EXPECT_EQ(container.GetIndexedItemCount(), field_item->GetIndexedItemCount() + 1);

    // Create line number program
    LineNumberProgramItem *line_number_program_item = container.CreateLineNumberProgramItem();
    EXPECT_EQ(line_number_program_item->GetIndexedItemCount(), 17U);
    EXPECT_EQ(container.GetIndexedItemCount(), line_number_program_item->GetIndexedItemCount() + 1);

    // Create value items
    ScalarValueItem *scalarValueItem = container.CreateItem<ScalarValueItem>(1.0);
    EXPECT_EQ(scalarValueItem->GetIndexedItemCount(), 18U);
    EXPECT_EQ(container.GetIndexedItemCount(), scalarValueItem->GetIndexedItemCount() + 1);
}

void GenerateModifiedAbc(const std::vector<unsigned char> &buffer, const std::string &filename)
{
    std::ofstream abc_file(filename, std::ios::out | std::ios::binary);
    abc_file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    abc_file.close();
}

HWTEST(ItemContainer, ValidateChecksumTest, testing::ext::TestSize.Level0)
{
    using panda::os::file::Mode;
    using panda::os::file::Open;

    ItemContainer container;
    container.GetOrCreateClassItem("A");

    const std::string file_name = "test_validate_checksum.abc";
    auto writer = FileWriter(file_name);

    ASSERT_TRUE(container.Write(&writer));

    auto file = File::Open(file_name);
    EXPECT_TRUE(file->ValidateChecksum());

    std::ifstream base_file(file_name, std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<uint8_t> new_content = {0x01, 0x01};

    // The checksum calculation starts with the 12th element
    buffer[12] = new_content[0];
    buffer[13] = new_content[1];

    GenerateModifiedAbc(buffer, file_name);

    EXPECT_FALSE(file->ValidateChecksum());
}

HWTEST(ItemContainer, ThrowIfWithCheckTest, testing::ext::TestSize.Level0)
{
    Logger::InitializeStdLogging(Logger::Level::FATAL, Logger::Component::PANDAFILE);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::PANDAFILE));

    using panda::os::file::Mode;
    using panda::os::file::Open;

    ItemContainer container;
    container.GetOrCreateClassItem("B");

    const std::string file_name = "test_throw_if_with_check.abc";
    auto writer = FileWriter(file_name);

    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    auto file = File::Open(file_name);
    EXPECT_NE(file, nullptr);

    file->ThrowIfWithCheck(0U > 1U, File::INVALID_FILE_OFFSET);

#ifdef HOST_UT
    EXPECT_DEATH(file->ThrowIfWithCheck(1U > 0U, File::INVALID_FILE_OFFSET),
                 "F/pandafile: Invalid file offset");
#endif

    std::ifstream base_file(file_name, std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(base_file), {});

    std::vector<uint8_t> new_content = {0x01, 0x01};

    // The checksum calculation starts with the 12th element
    buffer[12] = new_content[0];
    buffer[13] = new_content[1];

    GenerateModifiedAbc(buffer, file_name);

#ifdef HOST_UT
    EXPECT_DEATH(file->ThrowIfWithCheck(1U > 0U, File::INVALID_FILE_OFFSET),
                 "F/pandafile: Invalid file offset, checksum mismatch. The abc file has been corrupted.");
#endif

}

HWTEST(ItemContainer, GetStatEmpty, testing::ext::TestSize.Level0)
{
    ItemContainer container;
    auto stat = container.GetStat();

    EXPECT_EQ(stat.at("header_item"), sizeof(File::Header));
    EXPECT_EQ(stat.at("class_idx_item"), 0u);
    EXPECT_EQ(stat.at("instructions_number"), 0u);
    EXPECT_EQ(stat.at("codesize"), 0u);
}

HWTEST(ItemContainer, GetStatClassIdxIncrease, testing::ext::TestSize.Level0)
{
    ItemContainer container;
    auto stat0 = container.GetStat();

    // add one class
    container.GetOrCreateClassItem("LTest;");
    auto stat1 = container.GetStat();

    EXPECT_EQ(stat1.at("class_idx_item"), stat0.at("class_idx_item") + ID_SIZE);
}

HWTEST(ItemContainer, GetStatCodeMetrics, testing::ext::TestSize.Level0)
{
    ItemContainer container;

    // create a class and a method with code
    ClassItem *class_item = container.GetOrCreateClassItem("LCodeTest;");
    StringItem *method_name = container.GetOrCreateStringItem("m");
    PrimitiveTypeItem *ret_type = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    ProtoItem *proto_item = container.GetOrCreateProtoItem(ret_type, params);
    MethodItem *method_item = class_item->AddMethod(method_name, proto_item, ACC_PUBLIC | ACC_STATIC, params);

    std::vector<uint8_t> instructions {1, 2, 3, 4};
    CodeItem *code_item = container.CreateItem<CodeItem>(0, 0, instructions);
    code_item->SetNumInstructions(instructions.size());
    method_item->SetCode(code_item);
    code_item->AddMethod(method_item);

    auto stat = container.GetStat();

    EXPECT_EQ(stat.at("instructions_number"), instructions.size());
    EXPECT_EQ(stat.at("codesize"), code_item->GetCodeSize());
}
}  // namespace panda::panda_file::test
