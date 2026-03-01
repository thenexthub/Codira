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

#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/file.h"
#include "libarkfile/file_item_container.h"
#include "libarkfile/file_reader.h"
#include "libarkfile/file_writer.h"
#include "libarkfile/helpers.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/method_handle_data_accessor.h"
#include "libarkfile/modifiers.h"
#include "libarkbase/os/file.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/pgo.h"
#include "libarkfile/value.h"

#include "zlib.h"

#include <cstddef>

#include <memory>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace ark::panda_file::test {

TEST(ItemContainer, DeduplicationTest)
{
    ItemContainer container;

    StringItem *stringItem = container.GetOrCreateStringItem("1");
    EXPECT_EQ(stringItem, container.GetOrCreateStringItem("1"));

    ClassItem *classItem = container.GetOrCreateClassItem("1");
    EXPECT_EQ(classItem, container.GetOrCreateClassItem("1"));

    ValueItem *intItem = container.GetOrCreateIntegerValueItem(1);
    EXPECT_EQ(intItem, container.GetOrCreateIntegerValueItem(1));

    ValueItem *longItem = container.GetOrCreateLongValueItem(1);
    EXPECT_EQ(longItem, container.GetOrCreateLongValueItem(1));
    EXPECT_NE(longItem, intItem);

    ValueItem *floatItem = container.GetOrCreateFloatValueItem(1.0);
    EXPECT_EQ(floatItem, container.GetOrCreateFloatValueItem(1.0));
    EXPECT_NE(floatItem, intItem);
    EXPECT_NE(floatItem, longItem);

    ValueItem *doubleItem = container.GetOrCreateDoubleValueItem(1.0);
    EXPECT_EQ(doubleItem, container.GetOrCreateDoubleValueItem(1.0));
    EXPECT_NE(doubleItem, intItem);
    EXPECT_NE(doubleItem, longItem);
    EXPECT_NE(doubleItem, floatItem);
}

TEST(ItemContainer, TestFileOpen)
{
    using ark::os::file::Mode;
    using ark::os::file::Open;

    // Write panda file to disk
    ItemContainer container;

    const std::string fileName = "test_file_open.panda";
    auto writer = FileWriter(fileName);

    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    EXPECT_NE(File::Open(fileName), nullptr);
}

TEST(ItemContainer, TestFileFormatVersionTooOld)
{
    const std::string fileName = "test_file_format_version_too_old.abc";
    {
        ItemContainer container;
        auto writer = FileWriter(fileName);

        File::Header header {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::fill(reinterpret_cast<uint8_t *>(&header), reinterpret_cast<uint8_t *>(&header) + sizeof(header), 0);
        header.magic = File::MAGIC;

        auto old = std::array<uint8_t, File::VERSION_SIZE>(MIN_VERSION);
        --old[3];

        header.version = old;
        header.fileSize = sizeof(File::Header);

        for (uint8_t b : Span<uint8_t>(reinterpret_cast<uint8_t *>(&header), sizeof(header))) {
            writer.WriteByte(b);
        }
        EXPECT_TRUE(writer.FinishWrite());
    }

    EXPECT_EQ(File::Open(fileName), nullptr);
}

TEST(ItemContainer, TestFileFormatVersionTooNew)
{
    const std::string fileName = "test_file_format_version_too_new.abc";
    {
        ItemContainer container;
        auto writer = FileWriter(fileName);

        File::Header header {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::fill(reinterpret_cast<uint8_t *>(&header), reinterpret_cast<uint8_t *>(&header) + sizeof(header), 0);
        header.magic = File::MAGIC;

        auto newArr = std::array<uint8_t, File::VERSION_SIZE>(VERSION);
        ++newArr[3];

        header.version = newArr;
        header.fileSize = sizeof(File::Header);

        for (uint8_t b : Span<uint8_t>(reinterpret_cast<uint8_t *>(&header), sizeof(header))) {
            writer.WriteByte(b);
        }
        EXPECT_TRUE(writer.FinishWrite());
    }

    EXPECT_EQ(File::Open(fileName), nullptr);
}

TEST(ItemContainer, TestFileFormatVersionValid)
{
    const std::string fileName = "test_file_format_version_valid.abc";
    {
        ItemContainer container;
        auto writer = FileWriter(fileName);
        File::Header header {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::fill(reinterpret_cast<uint8_t *>(&header), reinterpret_cast<uint8_t *>(&header) + sizeof(header), 0);
        header.magic = File::MAGIC;
        header.version = {0, 0, 0, 6};
        header.fileSize = sizeof(File::Header);
        auto pChecksumData = reinterpret_cast<uint8_t *>(&header.version);
        auto checksum = adler32(1, pChecksumData, sizeof(header) - offsetof(File::Header, version));
        header.checksum = checksum;
        for (uint8_t b : Span<uint8_t>(reinterpret_cast<uint8_t *>(&header), sizeof(header))) {
            writer.WriteByte(b);
        }
        EXPECT_TRUE(writer.FinishWrite());
    }

    EXPECT_NE(File::Open(fileName), nullptr);
}

static std::unique_ptr<const File> GetPandaFile(std::vector<uint8_t> &data)
{
    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data.data()), data.size(),
                              [](std::byte *, size_t) noexcept {});
    return File::OpenFromMemory(std::move(ptr));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(ItemContainer, TestClasses)
{
    // Write panda file to memory

    ItemContainer container;

    ClassItem *emptyClassItem = container.GetOrCreateClassItem("Foo");

    ClassItem *classItem = container.GetOrCreateClassItem("Bar");
    classItem->SetAccessFlags(ACC_PUBLIC);
    classItem->SetSuperClass(emptyClassItem);

    // Add interface

    ClassItem *ifaceItem = container.GetOrCreateClassItem("Iface");
    ifaceItem->SetAccessFlags(ACC_PUBLIC);

    classItem->AddInterface(ifaceItem);

    // Add method

    StringItem *methodName = container.GetOrCreateStringItem("foo");

    PrimitiveTypeItem *retType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);

    MethodItem *methodItem = classItem->AddMethod(methodName, protoItem, ACC_PUBLIC | ACC_STATIC, params);

    // Add field

    StringItem *fieldName = container.GetOrCreateStringItem("field");
    PrimitiveTypeItem *fieldType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);

    FieldItem *fieldItem = classItem->AddField(fieldName, fieldType, ACC_PUBLIC);

    // Add runtime annotation

    std::vector<AnnotationItem::Elem> runtimeElems;
    std::vector<AnnotationItem::Tag> runtimeTags;
    auto *runtimeAnnotationItem = container.CreateItem<AnnotationItem>(classItem, runtimeElems, runtimeTags);

    classItem->AddRuntimeAnnotation(runtimeAnnotationItem);

    // Add annotation

    std::vector<AnnotationItem::Elem> elems;
    std::vector<AnnotationItem::Tag> tags;
    auto *annotationItem = container.CreateItem<AnnotationItem>(classItem, elems, tags);

    classItem->AddAnnotation(annotationItem);

    // Add source file

    StringItem *sourceFile = container.GetOrCreateStringItem("source_file");

    classItem->SetSourceFile(sourceFile);

    MemoryWriter memWriter;

    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory

    auto data = memWriter.GetData();
    auto pandaFile = GetPandaFile(data);

    ASSERT_NE(pandaFile, nullptr);

    EXPECT_THAT(pandaFile->GetHeader()->version, ::testing::ElementsAre(0, 0, 0, 6));
    EXPECT_EQ(pandaFile->GetHeader()->fileSize, memWriter.GetData().size());
    EXPECT_EQ(pandaFile->GetHeader()->foreignOff, 0U);
    EXPECT_EQ(pandaFile->GetHeader()->foreignSize, 0U);
    EXPECT_EQ(pandaFile->GetHeader()->numClasses, 3U);
    EXPECT_EQ(pandaFile->GetHeader()->classIdxOff, sizeof(File::Header));
    EXPECT_EQ(pandaFile->GetHeader()->numExportTable, 0U);
    EXPECT_EQ(pandaFile->GetHeader()->exportTableOff,
              sizeof(File::Header) + (pandaFile->GetHeader()->numClasses * File::EntityId::GetSize()));

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto *classIndex =
        reinterpret_cast<const uint32_t *>(pandaFile->GetBase() + pandaFile->GetHeader()->classIdxOff);
    EXPECT_EQ(classIndex[0], classItem->GetOffset());
    EXPECT_EQ(classIndex[1], emptyClassItem->GetOffset());
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    std::vector<uint8_t> className {'B', 'a', 'r', '\0'};
    auto classId = pandaFile->GetClassId(className.data());
    EXPECT_EQ(classId.GetOffset(), classItem->GetOffset());

    ClassDataAccessor classDataAccessor(*pandaFile, classId);
    EXPECT_EQ(classDataAccessor.GetSuperClassId().GetOffset(), emptyClassItem->GetOffset());
    EXPECT_EQ(classDataAccessor.GetAccessFlags(), ACC_PUBLIC);
    EXPECT_EQ(classDataAccessor.GetFieldsNumber(), 1U);
    EXPECT_EQ(classDataAccessor.GetMethodsNumber(), 1U);
    EXPECT_EQ(classDataAccessor.GetIfacesNumber(), 1U);
    EXPECT_TRUE(classDataAccessor.GetSourceFileId().has_value());
    EXPECT_EQ(classDataAccessor.GetSourceFileId().value().GetOffset(), sourceFile->GetOffset());
    EXPECT_EQ(classDataAccessor.GetSize(), classItem->GetSize());

    classDataAccessor.EnumerateInterfaces([&](File::EntityId id) {
        EXPECT_EQ(id.GetOffset(), ifaceItem->GetOffset());

        ClassDataAccessor ifaceClassDataAccessor(*pandaFile, id);
        EXPECT_EQ(ifaceClassDataAccessor.GetSuperClassId().GetOffset(), 0U);
        EXPECT_EQ(ifaceClassDataAccessor.GetAccessFlags(), ACC_PUBLIC);
        EXPECT_EQ(ifaceClassDataAccessor.GetFieldsNumber(), 0U);
        EXPECT_EQ(ifaceClassDataAccessor.GetMethodsNumber(), 0U);
        EXPECT_EQ(ifaceClassDataAccessor.GetIfacesNumber(), 0U);
        EXPECT_FALSE(ifaceClassDataAccessor.GetSourceFileId().has_value());
        EXPECT_EQ(ifaceClassDataAccessor.GetSize(), ifaceItem->GetSize());
    });

    classDataAccessor.EnumerateRuntimeAnnotations([&](File::EntityId id) {
        EXPECT_EQ(id.GetOffset(), runtimeAnnotationItem->GetOffset());

        AnnotationDataAccessor dataAccessor(*pandaFile, id);
        EXPECT_EQ(dataAccessor.GetAnnotationId().GetOffset(), runtimeAnnotationItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetClassId().GetOffset(), classItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetCount(), 0U);
    });

    // Annotation is the same as the runtime one, so we deduplicate it
    EXPECT_FALSE(annotationItem->NeedsEmit());
    annotationItem = runtimeAnnotationItem;

    classDataAccessor.EnumerateAnnotations([&](File::EntityId id) {
        EXPECT_EQ(id.GetOffset(), annotationItem->GetOffset());

        AnnotationDataAccessor dataAccessor(*pandaFile, id);
        EXPECT_EQ(dataAccessor.GetAnnotationId().GetOffset(), annotationItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetClassId().GetOffset(), classItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetCount(), 0U);
    });

    classDataAccessor.EnumerateFields([&](FieldDataAccessor &dataAccessor) {
        EXPECT_EQ(dataAccessor.GetFieldId().GetOffset(), fieldItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetClassId().GetOffset(), classItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetNameId().GetOffset(), fieldName->GetOffset());
        EXPECT_EQ(dataAccessor.GetType(), Type(Type::TypeId::I32).GetFieldEncoding());
        EXPECT_EQ(dataAccessor.GetAccessFlags(), ACC_PUBLIC);
        EXPECT_FALSE(dataAccessor.GetValue<int32_t>().has_value());
        EXPECT_EQ(dataAccessor.GetSize(), fieldItem->GetSize());

        dataAccessor.EnumerateRuntimeAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
        dataAccessor.EnumerateAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
    });

    classDataAccessor.EnumerateMethods([&](MethodDataAccessor &dataAccessor) {
        EXPECT_FALSE(dataAccessor.IsExternal());
        EXPECT_EQ(dataAccessor.GetMethodId().GetOffset(), methodItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetClassId().GetOffset(), classItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetNameId().GetOffset(), methodName->GetOffset());
        EXPECT_EQ(dataAccessor.GetProtoId().GetOffset(), protoItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetAccessFlags(), ACC_PUBLIC | ACC_STATIC);
        EXPECT_FALSE(dataAccessor.GetCodeId().has_value());
        EXPECT_EQ(dataAccessor.GetSize(), methodItem->GetSize());
        EXPECT_FALSE(dataAccessor.GetRuntimeParamAnnotationId().has_value());
        EXPECT_FALSE(dataAccessor.GetParamAnnotationId().has_value());
        EXPECT_FALSE(dataAccessor.GetDebugInfoId().has_value());

        dataAccessor.EnumerateRuntimeAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
        dataAccessor.EnumerateAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
    });

    ClassDataAccessor emptyClassDataAccessor(*pandaFile, File::EntityId(emptyClassItem->GetOffset()));
    EXPECT_EQ(emptyClassDataAccessor.GetSuperClassId().GetOffset(), 0U);
    EXPECT_EQ(emptyClassDataAccessor.GetAccessFlags(), 0U);
    EXPECT_EQ(emptyClassDataAccessor.GetFieldsNumber(), 0U);
    EXPECT_EQ(emptyClassDataAccessor.GetMethodsNumber(), 0U);
    EXPECT_EQ(emptyClassDataAccessor.GetIfacesNumber(), 0U);
    EXPECT_FALSE(emptyClassDataAccessor.GetSourceFileId().has_value());
    EXPECT_EQ(emptyClassDataAccessor.GetSize(), emptyClassItem->GetSize());
}

TEST(ItemContainer, TestMethods)
{
    // Write panda file to memory

    ItemContainer container;

    ClassItem *classItem = container.GetOrCreateClassItem("A");
    classItem->SetAccessFlags(ACC_PUBLIC);

    StringItem *methodName = container.GetOrCreateStringItem("foo");

    PrimitiveTypeItem *retType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);

    MethodItem *methodItem = classItem->AddMethod(methodName, protoItem, ACC_PUBLIC | ACC_STATIC, params);

    std::vector<uint8_t> instructions {1, 2, 3, 4};
    auto *codeItem = container.CreateItem<CodeItem>(0, 2, instructions);

    methodItem->SetCode(codeItem);

    MemoryWriter memWriter;

    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory

    auto data = memWriter.GetData();
    auto pandaFile = GetPandaFile(data);

    ASSERT_NE(pandaFile, nullptr);

    ClassDataAccessor classDataAccessor(*pandaFile, File::EntityId(classItem->GetOffset()));

    classDataAccessor.EnumerateMethods([&](MethodDataAccessor &dataAccessor) {
        EXPECT_FALSE(dataAccessor.IsExternal());
        EXPECT_EQ(dataAccessor.GetMethodId().GetOffset(), methodItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetClassId().GetOffset(), classItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetNameId().GetOffset(), methodName->GetOffset());
        EXPECT_EQ(dataAccessor.GetProtoId().GetOffset(), protoItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetAccessFlags(), ACC_PUBLIC | ACC_STATIC);
        EXPECT_EQ(dataAccessor.GetSize(), methodItem->GetSize());

        auto codeId = dataAccessor.GetCodeId();
        EXPECT_TRUE(codeId.has_value());
        EXPECT_EQ(codeId.value().GetOffset(), codeItem->GetOffset());

        CodeDataAccessor codeDataAccessor(*pandaFile, codeId.value());
        EXPECT_EQ(codeDataAccessor.GetNumVregs(), 0U);
        EXPECT_EQ(codeDataAccessor.GetNumArgs(), 2U);
        EXPECT_EQ(codeDataAccessor.GetCodeSize(), instructions.size());
        EXPECT_THAT(instructions,
                    ::testing::ElementsAreArray(codeDataAccessor.GetInstructions(), codeDataAccessor.GetCodeSize()));

        EXPECT_EQ(codeDataAccessor.GetTriesSize(), 0U);
        EXPECT_EQ(codeDataAccessor.GetSize(), codeItem->GetSize());

        codeDataAccessor.EnumerateTryBlocks([](const CodeDataAccessor::TryBlock &) {
            EXPECT_TRUE(false);
            return false;
        });

        EXPECT_FALSE(dataAccessor.GetDebugInfoId().has_value());

        EXPECT_FALSE(dataAccessor.GetRuntimeParamAnnotationId().has_value());

        EXPECT_FALSE(dataAccessor.GetParamAnnotationId().has_value());

        dataAccessor.EnumerateRuntimeAnnotations([](File::EntityId) { EXPECT_TRUE(false); });

        dataAccessor.EnumerateAnnotations([](File::EntityId) { EXPECT_TRUE(false); });
    });
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
void TestProtos(size_t n)
{
    constexpr size_t ELEM_WIDTH = 4;
    constexpr size_t ELEM_PER16 = 16 / ELEM_WIDTH;

    // Write panda file to memory

    ItemContainer container;

    ClassItem *classItem = container.GetOrCreateClassItem("A");
    classItem->SetAccessFlags(ACC_PUBLIC);

    StringItem *methodName = container.GetOrCreateStringItem("foo");

    std::vector<Type::TypeId> types {Type::TypeId::VOID, Type::TypeId::I32};
    std::vector<ClassItem *> refTypes;

    PrimitiveTypeItem *retType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;

    params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));

    for (size_t i = 0; i < ELEM_PER16 * 2U - 2U; i++) {
        params.emplace_back(container.GetOrCreateClassItem("B"));
        types.push_back(Type::TypeId::REFERENCE);
        refTypes.push_back(container.GetOrCreateClassItem("B"));
        params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::F64));
        types.push_back(Type::TypeId::F64);
    }

    for (size_t i = 0; i < n; i++) {
        params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::F32));
        types.push_back(Type::TypeId::F32);
    }

    ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);

    MethodItem *methodItem = classItem->AddMethod(methodName, protoItem, ACC_PUBLIC | ACC_STATIC, params);

    MemoryWriter memWriter;

    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory

    auto data = memWriter.GetData();
    auto pandaFile = GetPandaFile(data);

    ASSERT_NE(pandaFile, nullptr);

    ClassDataAccessor classDataAccessor(*pandaFile, File::EntityId(classItem->GetOffset()));

    classDataAccessor.EnumerateMethods([&](MethodDataAccessor &dataAccessor) {
        EXPECT_EQ(dataAccessor.GetMethodId().GetOffset(), methodItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetProtoId().GetOffset(), protoItem->GetOffset());

        ProtoDataAccessor protoDataAccessor(*pandaFile, dataAccessor.GetProtoId());
        EXPECT_EQ(protoDataAccessor.GetProtoId().GetOffset(), protoItem->GetOffset());

        size_t num = 0;
        size_t nref = 0;
        protoDataAccessor.EnumerateTypes([&](Type t) {
            EXPECT_EQ(t.GetEncoding(), Type(types[num]).GetEncoding());
            ++num;

            if (!t.IsPrimitive()) {
                ++nref;
            }
        });

        EXPECT_EQ(num, types.size());

        for (size_t i = 0; i < num - 1; i++) {
            EXPECT_EQ(protoDataAccessor.GetArgType(i).GetEncoding(), Type(types[i + 1]).GetEncoding());
        }

        EXPECT_EQ(protoDataAccessor.GetReturnType().GetEncoding(), Type(types[0]).GetEncoding());

        EXPECT_EQ(nref, refTypes.size());

        for (size_t i = 0; i < nref; i++) {
            EXPECT_EQ(protoDataAccessor.GetReferenceType(0).GetOffset(), refTypes[i]->GetOffset());
        }

        size_t size = ((num + ELEM_PER16) / ELEM_PER16 + nref) * sizeof(uint16_t);

        EXPECT_EQ(protoDataAccessor.GetSize(), size);
        EXPECT_EQ(protoDataAccessor.GetSize(), protoItem->GetSize());
    });
}

TEST(ItemContainer, TestProtos)
{
    TestProtos(0);
    TestProtos(1);
    TestProtos(2);
    TestProtos(7);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(ItemContainer, TestDebugInfo)
{
    // Write panda file to memory

    ItemContainer container;

    ClassItem *classItem = container.GetOrCreateClassItem("A");
    classItem->SetAccessFlags(ACC_PUBLIC);

    StringItem *methodName = container.GetOrCreateStringItem("foo");

    PrimitiveTypeItem *retType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);
    MethodItem *methodItem = classItem->AddMethod(methodName, protoItem, ACC_PUBLIC | ACC_STATIC, params);

    StringItem *sourceFileItem = container.GetOrCreateStringItem("<source>");
    StringItem *sourceCodeItem = container.GetOrCreateStringItem("let a = 1;");
    StringItem *paramStringItem = container.GetOrCreateStringItem("a0");

    LineNumberProgramItem *lineNumberProgramItem = container.CreateLineNumberProgramItem();
    auto *debugInfoItem = container.CreateItem<DebugInfoItem>(lineNumberProgramItem);
    methodItem->SetDebugInfo(debugInfoItem);

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

    auto *constantPool = debugInfoItem->GetConstantPool();
    debugInfoItem->SetLineNumber(5);
    lineNumberProgramItem->EmitSetSourceCode(constantPool, sourceCodeItem);
    lineNumberProgramItem->EmitSetFile(constantPool, sourceFileItem);
    lineNumberProgramItem->EmitPrologueEnd();
    // NOLINTNEXTLINE(readability-magic-numbers)
    lineNumberProgramItem->EmitAdvancePc(constantPool, 10);
    lineNumberProgramItem->EmitAdvanceLine(constantPool, -5);
    lineNumberProgramItem->EmitEpilogueBegin();
    lineNumberProgramItem->EmitEnd();

    debugInfoItem->AddParameter(paramStringItem);

    methodItem->SetDebugInfo(debugInfoItem);

    MemoryWriter memWriter;

    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory

    auto data = memWriter.GetData();
    auto pandaFile = GetPandaFile(data);

    ASSERT_NE(pandaFile, nullptr);

    ClassDataAccessor classDataAccessor(*pandaFile, File::EntityId(classItem->GetOffset()));

    classDataAccessor.EnumerateMethods([&](MethodDataAccessor &dataAccessor) {
        EXPECT_EQ(dataAccessor.GetMethodId().GetOffset(), methodItem->GetOffset());
        EXPECT_EQ(dataAccessor.GetSize(), methodItem->GetSize());

        auto debugInfoId = dataAccessor.GetDebugInfoId();
        EXPECT_TRUE(debugInfoId.has_value());

        EXPECT_EQ(debugInfoId.value().GetOffset(), debugInfoItem->GetOffset());

        DebugInfoDataAccessor dda(*pandaFile, debugInfoId.value());
        EXPECT_EQ(dda.GetDebugInfoId().GetOffset(), debugInfoItem->GetOffset());
        EXPECT_EQ(dda.GetLineStart(), 5U);
        EXPECT_EQ(dda.GetNumParams(), params.size());

        dda.EnumerateParameters([&](File::EntityId id) { EXPECT_EQ(id.GetOffset(), paramStringItem->GetOffset()); });

        auto cp = dda.GetConstantPool();
        EXPECT_EQ(cp.size(), constantPool->size());
        EXPECT_THAT(*constantPool, ::testing::ElementsAreArray(cp.data(), cp.Size()));

        EXPECT_EQ(helpers::ReadULeb128(&cp), sourceCodeItem->GetOffset());
        EXPECT_EQ(helpers::ReadULeb128(&cp), sourceFileItem->GetOffset());
        EXPECT_EQ(helpers::ReadULeb128(&cp), 10U);
        EXPECT_EQ(helpers::ReadLeb128(&cp), -5);

        const uint8_t *lineNumberProgram = dda.GetLineNumberProgram();
        EXPECT_EQ(pandaFile->GetIdFromPointer(lineNumberProgram).GetOffset(), lineNumberProgramItem->GetOffset());
        EXPECT_EQ(lineNumberProgramItem->GetSize(), opcodes.size());

        EXPECT_THAT(opcodes, ::testing::ElementsAreArray(lineNumberProgram, opcodes.size()));

        EXPECT_EQ(dda.GetSize(), debugInfoItem->GetSize());
    });
}

TEST(ItemContainer, ForeignItems)
{
    ItemContainer container;

    // Create foreign class
    ForeignClassItem *classItem = container.GetOrCreateForeignClassItem("ForeignClass");

    // Create foreign field
    StringItem *fieldName = container.GetOrCreateStringItem("foreign_field");
    PrimitiveTypeItem *fieldType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    auto *fieldItem = container.CreateItem<ForeignFieldItem>(classItem, fieldName, fieldType, 0);

    // Create foreign method
    StringItem *methodName = container.GetOrCreateStringItem("ForeignMethod");
    PrimitiveTypeItem *retType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    params.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);
    auto *methodItem = container.CreateItem<ForeignMethodItem>(classItem, methodName, protoItem, 0);

    MemoryWriter memWriter;

    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory

    auto data = memWriter.GetData();
    auto pandaFile = GetPandaFile(data);

    ASSERT_NE(pandaFile, nullptr);

    EXPECT_EQ(pandaFile->GetHeader()->foreignOff, classItem->GetOffset());

    size_t foreignSize = classItem->GetSize() + fieldItem->GetSize() + methodItem->GetSize();
    EXPECT_EQ(pandaFile->GetHeader()->foreignSize, foreignSize);

    ASSERT_TRUE(pandaFile->IsExternal(classItem->GetFileId()));

    MethodDataAccessor methodDataAccessor(*pandaFile, methodItem->GetFileId());
    EXPECT_EQ(methodDataAccessor.GetMethodId().GetOffset(), methodItem->GetOffset());
    EXPECT_EQ(methodDataAccessor.GetSize(), methodItem->GetSize());
    EXPECT_EQ(methodDataAccessor.GetClassId().GetOffset(), classItem->GetOffset());
    EXPECT_EQ(methodDataAccessor.GetNameId().GetOffset(), methodName->GetOffset());
    EXPECT_EQ(methodDataAccessor.GetProtoId().GetOffset(), protoItem->GetOffset());
    EXPECT_TRUE(methodDataAccessor.IsExternal());

    FieldDataAccessor fieldDataAccessor(*pandaFile, fieldItem->GetFileId());
    EXPECT_EQ(fieldDataAccessor.GetFieldId().GetOffset(), fieldItem->GetOffset());
    EXPECT_EQ(fieldDataAccessor.GetSize(), fieldItem->GetSize());
    EXPECT_EQ(fieldDataAccessor.GetClassId().GetOffset(), classItem->GetOffset());
    EXPECT_EQ(fieldDataAccessor.GetNameId().GetOffset(), fieldName->GetOffset());
    EXPECT_EQ(fieldDataAccessor.GetType(), fieldType->GetType().GetFieldEncoding());
    EXPECT_TRUE(fieldDataAccessor.IsExternal());
}

TEST(ItemContainer, EmptyContainerChecksum)
{
    using ark::os::file::Mode;
    using ark::os::file::Open;

    // Write panda file to disk
    ItemContainer container;

    const std::string fileName = "test_empty_checksum.ark";
    auto writer = FileWriter(fileName);

    // Initial value of adler32
    EXPECT_EQ(writer.GetChecksum(), 1);
    ASSERT_TRUE(container.Write(&writer));

    // At least header was written so the checksum should be changed
    auto containerChecksum = writer.GetChecksum();
    EXPECT_NE(containerChecksum, 1);

    // Read panda file from disk
    auto file = File::Open(fileName);
    EXPECT_NE(file, nullptr);
    EXPECT_EQ(file->GetHeader()->checksum, containerChecksum);

    constexpr size_t DATA_OFFSET = 12U;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto checksum = adler32(1, file->GetBase() + DATA_OFFSET, file->GetHeader()->fileSize - DATA_OFFSET);
    EXPECT_EQ(file->GetHeader()->checksum, checksum);
}

TEST(ItemContainer, ContainerChecksum)
{
    using ark::os::file::Mode;
    using ark::os::file::Open;

    uint32_t emptyChecksum = 0;
    {
        ItemContainer container;
        const std::string fileName = "test_checksum_empty.ark";
        auto writer = FileWriter(fileName);
        ASSERT_TRUE(container.Write(&writer));
        emptyChecksum = writer.GetChecksum();
    }
    ASSERT(emptyChecksum != 0);

    // Create not empty container
    ItemContainer container;
    container.GetOrCreateClassItem("C");

    const std::string fileName = "test_checksum.ark";
    auto writer = FileWriter(fileName);

    ASSERT_TRUE(container.Write(&writer));

    // This checksum must be different from the empty one (collision may happen though)
    auto containerChecksum = writer.GetChecksum();
    EXPECT_NE(emptyChecksum, containerChecksum);

    // Read panda file from disk
    auto file = File::Open(fileName);
    EXPECT_NE(file, nullptr);
    EXPECT_EQ(file->GetHeader()->checksum, containerChecksum);

    constexpr size_t DATA_OFFSET = 12U;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto checksum = adler32(1, file->GetBase() + DATA_OFFSET, file->GetHeader()->fileSize - DATA_OFFSET);
    EXPECT_EQ(file->GetHeader()->checksum, checksum);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(ItemContainer, TestProfileGuidedRelayout)
{
    ItemContainer container;

    // Add classes
    ClassItem *emptyClassItem = container.GetOrCreateClassItem("LTest;");
    ClassItem *classItemA = container.GetOrCreateClassItem("LAA;");
    classItemA->SetSuperClass(emptyClassItem);
    ClassItem *classItemB = container.GetOrCreateClassItem("LBB;");

    // Add method1
    StringItem *methodName1 = container.GetOrCreateStringItem("foo1");
    PrimitiveTypeItem *retType1 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params1;
    ProtoItem *protoItem1 = container.GetOrCreateProtoItem(retType1, params1);
    MethodItem *methodItem1 = classItemA->AddMethod(methodName1, protoItem1, ACC_PUBLIC | ACC_STATIC, params1);
    // Set code_1
    std::vector<uint8_t> instructions1 {1, 2, 3, 4};
    auto *codeItem1 = container.CreateItem<CodeItem>(0, 2, instructions1);
    methodItem1->SetCode(codeItem1);
    codeItem1->AddMethod(methodItem1);

    // Add method2
    StringItem *methodName2 = container.GetOrCreateStringItem("foo2");
    PrimitiveTypeItem *retType2 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    std::vector<MethodParamItem> params2;
    ProtoItem *protoItem2 = container.GetOrCreateProtoItem(retType2, params2);
    MethodItem *methodItem2 = classItemB->AddMethod(methodName2, protoItem2, ACC_PUBLIC | ACC_STATIC, params2);
    // Set code_2
    std::vector<uint8_t> instructions2 {5, 6, 7, 8};
    auto *codeItem2 = container.CreateItem<CodeItem>(0, 2, instructions2);
    methodItem2->SetCode(codeItem2);
    codeItem2->AddMethod(methodItem2);

    // Add method_3
    StringItem *methodName3 = container.GetOrCreateStringItem("foo3");
    auto *methodItem3 = emptyClassItem->AddMethod(methodName3, protoItem1, ACC_PUBLIC | ACC_STATIC, params1);
    // Set code_3
    std::vector<uint8_t> instructions3 {3, 4, 5, 6};
    auto *codeItem3 = container.CreateItem<CodeItem>(0, 2, instructions3);
    methodItem3->SetCode(codeItem3);
    codeItem3->AddMethod(methodItem3);

    // Add method_4
    StringItem *methodName4 = container.GetOrCreateStringItem("foo4");
    auto *methodItem4 = emptyClassItem->AddMethod(methodName4, protoItem1, ACC_PUBLIC | ACC_STATIC, params1);
    // Set code. method_4 and method_3 share code_item_3
    methodItem4->SetCode(codeItem3);
    codeItem3->AddMethod(methodItem4);

    // Add field
    StringItem *fieldName = container.GetOrCreateStringItem("test_field");
    PrimitiveTypeItem *fieldType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    classItemA->AddField(fieldName, fieldType, ACC_PUBLIC);

    // Add source file
    StringItem *sourceFile = container.GetOrCreateStringItem("source_file");
    classItemA->SetSourceFile(sourceFile);

    constexpr std::string_view PRIMITIVE_TYPE_ITEM = "primitive_type_item";
    constexpr std::string_view PROTO_ITEM = "proto_item";
    constexpr std::string_view END_ITEM = "end_item";

    // Items before PGO
    const auto &items = container.GetItems();
    auto item = items.begin();
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "Test");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "AA");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "BB");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo1");
    item++;
    EXPECT_EQ((*item)->GetName(), PRIMITIVE_TYPE_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), PROTO_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo2");
    item++;
    EXPECT_EQ((*item)->GetName(), PRIMITIVE_TYPE_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), PROTO_ITEM);
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo3");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo4");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "test_field");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "source_file");
    item++;
    EXPECT_EQ((*item)->GetName(), END_ITEM);
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
    EXPECT_EQ(item, items.end());

    // Prepare profile data
    std::string profilePath = "TestProfileGuidedRelayout_profile_test_data.txt";
    std::ofstream testFile;
    testFile.open(profilePath);
    testFile << "string_item:test_field" << std::endl;
    testFile << "class_item:BB" << std::endl;
    testFile << "code_item:BB::foo2" << std::endl;
    testFile << "code_item:Test::foo4" << std::endl;
    testFile.close();

    // Run PGO
    ark::panda_file::pgo::ProfileOptimizer profileOpt;
    profileOpt.SetProfilePath(profilePath);
    container.ReorderItems(&profileOpt);

    // Items after PGO
    item = items.begin();
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "test_field");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo1");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo2");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo3");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "foo4");
    item++;
    EXPECT_EQ((*item)->GetName(), STRING_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "source_file");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "BB");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "Test");
    item++;
    EXPECT_EQ((*item)->GetName(), CLASS_ITEM);
    EXPECT_EQ(ark::panda_file::pgo::ProfileOptimizer::GetNameInfo(*item), "AA");
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
    EXPECT_EQ(item, items.end());
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(ItemContainer, GettersTest)
{
    ItemContainer container;

    ClassItem *emptyClassItem = container.GetOrCreateClassItem("Foo");

    ClassItem *classItem = container.GetOrCreateClassItem("Bar");
    classItem->SetAccessFlags(ACC_PUBLIC);
    classItem->SetSuperClass(emptyClassItem);

    // Add methods

    StringItem *methodName1 = container.GetOrCreateStringItem("foo1");

    PrimitiveTypeItem *retType1 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params1;
    params1.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *protoItem1 = container.GetOrCreateProtoItem(retType1, params1);

    classItem->AddMethod(methodName1, protoItem1, ACC_PUBLIC | ACC_STATIC, params1);

    StringItem *methodName2 = container.GetOrCreateStringItem("foo2");

    PrimitiveTypeItem *retType2 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);
    std::vector<MethodParamItem> params2;
    params2.emplace_back(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::F32));
    ProtoItem *protoItem2 = container.GetOrCreateProtoItem(retType2, params2);

    classItem->AddMethod(methodName2, protoItem2, ACC_PUBLIC | ACC_STATIC, params2);

    // Add field

    StringItem *fieldName = container.GetOrCreateStringItem("field");
    PrimitiveTypeItem *fieldType = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::I32);

    classItem->AddField(fieldName, fieldType, ACC_PUBLIC);

    // Add source file

    StringItem *sourceFile = container.GetOrCreateStringItem("source_file");

    classItem->SetSourceFile(sourceFile);

    // Read items from container

    ASSERT_TRUE(container.GetItems().size() == 15);

    std::map<std::string, panda_file::BaseClassItem *> *classMap = container.GetClassMap();
    ASSERT_TRUE(classMap != nullptr && classMap->size() == 2);
    auto it = classMap->find("Bar");
    ASSERT_TRUE(it != classMap->end());

    std::map<std::string, panda_file::BaseClassItem *> *exportMap = container.GetExportMap();
    ASSERT_TRUE(exportMap != nullptr && exportMap->empty());

    std::unordered_map<std::string, StringItem *> *stringMap = container.GetStringMap();
    ASSERT_TRUE(stringMap != nullptr && stringMap->size() == 4);
    auto sit0 = stringMap->find("field");
    auto sit1 = stringMap->find("source_file");
    auto sit2 = stringMap->find("foo1");
    auto sit3 = stringMap->find("foo2");
    ASSERT_TRUE(sit0 != stringMap->end() && sit1 != stringMap->end() && sit2 != stringMap->end() &&
                sit3 != stringMap->end());

    std::unordered_map<Type::TypeId, PrimitiveTypeItem *> *primitiveTypeMap = container.GetPrimitiveTypeMap();
    ASSERT_TRUE(primitiveTypeMap != nullptr && primitiveTypeMap->size() == 3);
    auto pit0 = primitiveTypeMap->find(Type::TypeId::F32);
    auto pit1 = primitiveTypeMap->find(Type::TypeId::I32);
    auto pit2 = primitiveTypeMap->find(Type::TypeId::VOID);
    ASSERT_TRUE(pit0 != primitiveTypeMap->end() && pit1 != primitiveTypeMap->end() && pit2 != primitiveTypeMap->end());

    auto *rclassItem = static_cast<panda_file::ClassItem *>(it->second);
    std::string methodName;
    std::function<bool(BaseItem *)> testMethod = [&](BaseItem *method) {
        auto *methodItem = static_cast<panda_file::MethodItem *>(method);
        ASSERT(methodItem != nullptr && methodItem->GetItemType() == ItemTypes::METHOD_ITEM);
        methodName = methodItem->GetNameItem()->GetData();
        methodName.pop_back();  // remove '\0'
        ASSERT(methodName == "foo1" || methodName == "foo2");
        return true;
    };

    using std::placeholders::_1;
    panda_file::BaseItem::VisitorCallBack cbMethod = testMethod;
    rclassItem->VisitMethods(cbMethod);

    std::string fName;
    std::function<bool(BaseItem *)> testField = [&](BaseItem *field) {
        auto *fieldItem = static_cast<panda_file::FieldItem *>(field);
        ASSERT(fieldItem != nullptr && fieldItem->GetItemType() == ItemTypes::FIELD_ITEM);
        ASSERT(fieldItem->GetBaseItemType() == ItemTypes::ANNOTATION_ITEM);
        fieldItem->SetBaseItemType(ItemTypes::FIELD_ITEM);
        ASSERT(fieldItem->GetBaseItemType() == ItemTypes::FIELD_ITEM);
        fName = fieldItem->GetNameItem()->GetData();
        fName.pop_back();  // remove '\0'
        ASSERT(fName == "field");
        return true;
    };

    panda_file::BaseItem::VisitorCallBack cbField = testField;
    rclassItem->VisitFields(cbField);
}

TEST(ItemContainer, AnnotationDeduplication)
{
    ItemContainer container;

    auto annot = container.GetOrCreateClassItem("Annot");
    annot->SetAccessFlags(ACC_PUBLIC | ACC_ANNOTATION);
    auto annot1 = container.CreateItem<AnnotationItem>(annot, std::vector<AnnotationItem::Elem> {},
                                                       std::vector<AnnotationItem::Tag> {});
    auto annot2 = container.CreateItem<AnnotationItem>(annot, std::vector<AnnotationItem::Elem> {},
                                                       std::vector<AnnotationItem::Tag> {});

    auto str = container.GetOrCreateStringItem("Abc");

    auto glb = container.GetOrCreateGlobalClassItem();
    auto proto = container.GetOrCreateProtoItem(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID), {});
    auto meth = glb->AddMethod(container.GetOrCreateStringItem("foo"), proto, ACC_STATIC | ACC_PUBLIC,
                               std::vector<MethodParamItem> {});
    meth->AddAnnotation(annot1);
    meth->AddAnnotation(annot2);

    std::vector<uint8_t> ins = {(uint8_t)BytecodeInstruction::Opcode::LDA_STR_ID32, 0, 0, 0, 0,
                                (uint8_t)BytecodeInstruction::Opcode::RETURN_VOID};
    meth->SetCode(container.CreateItem<CodeItem>(0, 0, ins));

    container.ComputeLayout();
    for (size_t i = 0; i < 4; i++) {
        // NOLINTBEGIN(readability-magic-numbers)
        meth->GetCode()->GetInstructions()->at(1 + i) =
            static_cast<uint8_t>((str->GetFileId().GetOffset() >> (i * 8)) & 0xffU);
        // NOLINTEND(readability-magic-numbers)
    }
    container.DeduplicateItems(false);

    ASSERT_TRUE(!annot1->NeedsEmit() || !annot2->NeedsEmit());

    container.ComputeLayout();
    for (size_t i = 0; i < 4; i++) {
        // NOLINTBEGIN(readability-magic-numbers)
        ASSERT_EQ(meth->GetCode()->GetInstructions()->at(1 + i),
                  static_cast<uint8_t>((str->GetFileId().GetOffset() >> (i * 8)) & 0xffU));
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST(ItemContainer, AnnotationDeduplicationReading)
{
    ItemContainer container;

    auto annot = container.GetOrCreateClassItem("Annot");
    annot->SetAccessFlags(ACC_PUBLIC | ACC_ANNOTATION);
    auto annot1 = container.CreateItem<AnnotationItem>(annot, std::vector<AnnotationItem::Elem> {},
                                                       std::vector<AnnotationItem::Tag> {});
    auto annot2 = container.CreateItem<AnnotationItem>(annot, std::vector<AnnotationItem::Elem> {},
                                                       std::vector<AnnotationItem::Tag> {});

    auto glb = container.GetOrCreateGlobalClassItem();

    glb->AddAnnotation(annot1);
    glb->AddAnnotation(annot2);

    size_t size = container.ComputeLayout();

    container.DeduplicateAnnotations();
    ASSERT_FALSE(annot1->NeedsEmit() && annot2->NeedsEmit());
    ASSERT_EQ(glb->GetAnnotations()->at(0), glb->GetAnnotations()->at(1));
    container.DeduplicateAnnotations();

    std::vector<uint8_t> memBuf(size);
    MemoryBufferWriter writer {memBuf.data(), memBuf.size()};
    ASSERT_EQ(glb->GetAnnotations()->at(0), glb->GetAnnotations()->at(1));
    container.Write(&writer);

    auto emptyDeleter = +[](std::byte *, size_t) noexcept {};
    auto reader = FileReader(File::OpenFromMemory(
        os::mem::ConstBytePtr(reinterpret_cast<std::byte *>(memBuf.data()), memBuf.size(), emptyDeleter)));
    ASSERT_TRUE(reader.ReadContainer());

    auto annots = reader.GetContainerPtr()->GetOrCreateGlobalClassItem()->GetAnnotations();
    ASSERT_EQ(annots->size(), 2);
    ASSERT_EQ(annots->at(0), annots->at(1));
    auto annot3 = reader.GetContainerPtr()->CreateItem<AnnotationItem>(
        reader.GetContainerPtr()->GetOrCreateClassItem("Annot"), std::vector<AnnotationItem::Elem> {},
        std::vector<AnnotationItem::Tag> {});
    annots->insert(annots->begin(), annot3);
    reader.GetContainerPtr()->DeduplicateItems(true);
    reader.GetContainerPtr()->DeduplicateItems(true);
}

TEST(ItemContainer, CodeSizeZeroShouldFatal)
{
    EXPECT_DEATH(
        {
            ItemContainer container;
            auto *klass = container.GetOrCreateClassItem("DeathTest_ZeroCodeSize");
            auto *methodName = container.GetOrCreateStringItem("test_method_zero");
            auto *proto =
                container.GetOrCreateProtoItem(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID), {});
            auto *method =
                klass->AddMethod(methodName, proto, ACC_PUBLIC | ACC_STATIC, std::vector<MethodParamItem> {});

            std::vector<uint8_t> code {};
            auto *codeItem = container.CreateItem<CodeItem>(0, 0, code);
            method->SetCode(codeItem);

            CodeItem::CatchBlock cb(method, nullptr, 0);
            CodeItem::TryBlock tryBlock(0, 1, {cb});
            codeItem->AddTryBlock(tryBlock);

            MemoryWriter writer;
            ASSERT_TRUE(container.Write(&writer));
            auto data = writer.GetData();

            auto pandaFile = GetPandaFile(data);
            ASSERT_NE(pandaFile, nullptr);

            ClassDataAccessor cda(*pandaFile, File::EntityId(klass->GetOffset()));
            cda.EnumerateMethods([&](MethodDataAccessor &mda) {
                CodeDataAccessor codeData(*pandaFile, mda.GetCodeId().value());
                codeData.EnumerateTryBlocks([](const CodeDataAccessor::TryBlock &) { return true; });
            });
        },
        ".*");
}

TEST(ItemContainer, StartPcTooBigShouldFatal)
{
    EXPECT_DEATH(
        {
            ItemContainer container;
            auto *klass = container.GetOrCreateClassItem("DeathTestKlass");
            auto *methodName = container.GetOrCreateStringItem("test_method");
            auto *proto =
                container.GetOrCreateProtoItem(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID), {});
            auto *method =
                klass->AddMethod(methodName, proto, ACC_PUBLIC | ACC_STATIC, std::vector<MethodParamItem> {});

            std::vector<uint8_t> instructions(4, 0);
            auto *codeItem = container.CreateItem<CodeItem>(0, 0, instructions);
            method->SetCode(codeItem);

            CodeItem::CatchBlock cb(method, nullptr, 0);
            CodeItem::TryBlock tryBlock(4, 1, std::vector {cb});
            codeItem->AddTryBlock(tryBlock);

            MemoryWriter writer;
            ASSERT_TRUE(container.Write(&writer));
            auto data = writer.GetData();

            auto pandaFile = GetPandaFile(data);
            ASSERT_NE(pandaFile, nullptr);

            ClassDataAccessor cda(*pandaFile, File::EntityId(klass->GetOffset()));
            cda.EnumerateMethods([&](MethodDataAccessor &mda) {
                CodeDataAccessor codeData(*pandaFile, mda.GetCodeId().value());
                codeData.EnumerateTryBlocks([&](const CodeDataAccessor::TryBlock &) { return true; });
            });
        },
        ".*");
}

TEST(ItemContainer, LengthTooBigShouldFatal)
{
    EXPECT_DEATH(
        {
            ItemContainer container;
            auto *klass = container.GetOrCreateClassItem("DeathTest_LengthTooBig");
            auto *methodName = container.GetOrCreateStringItem("test_method_len");
            auto *proto =
                container.GetOrCreateProtoItem(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID), {});
            auto *method =
                klass->AddMethod(methodName, proto, ACC_PUBLIC | ACC_STATIC, std::vector<MethodParamItem> {});

            std::vector<uint8_t> code(4, 0);
            auto *codeItem = container.CreateItem<CodeItem>(0, 0, code);
            method->SetCode(codeItem);

            CodeItem::CatchBlock cb(method, nullptr, 0);
            CodeItem::TryBlock tryBlock(0, 5, {cb});
            codeItem->AddTryBlock(tryBlock);

            MemoryWriter writer;
            ASSERT_TRUE(container.Write(&writer));
            auto data = writer.GetData();

            auto pandaFile = GetPandaFile(data);
            ASSERT_NE(pandaFile, nullptr);

            ClassDataAccessor cda(*pandaFile, File::EntityId(klass->GetOffset()));
            cda.EnumerateMethods([&](MethodDataAccessor &mda) {
                CodeDataAccessor codeData(*pandaFile, mda.GetCodeId().value());
                codeData.EnumerateTryBlocks([](const CodeDataAccessor::TryBlock &) { return true; });
            });
        },
        ".*");
}

class FakeCatchBlock : public ark::panda_file::CodeItem::CatchBlock {
public:
    using CodeItem::CatchBlock::CatchBlock;

    size_t CalculateSize() const override
    {
        return std::numeric_limits<size_t>::max();
    }

    bool Write(Writer *writer) override
    {
        writer->Write<uint32_t>(0);
        return true;
    }

    ItemTypes GetItemType() const override
    {
        return ItemTypes::CATCH_BLOCK_ITEM;
    }

    bool IsValid() const
    {
        return true;
    }
};

void TriggerFatalTryBlock()
{
    ItemContainer container;
    auto *klass = container.GetOrCreateClassItem("DeathTestFatal");
    auto *methodName = container.GetOrCreateStringItem("test_fatal");
    auto *proto = container.GetOrCreateProtoItem(container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID), {});
    auto *method = klass->AddMethod(methodName, proto, ACC_PUBLIC | ACC_STATIC, std::vector<MethodParamItem> {});

    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr uint32_t kCodeSize = 2;
    std::vector<uint8_t> code(kCodeSize, 0);
    auto *codeItem = container.CreateItem<CodeItem>(0, 0, code);
    method->SetCode(codeItem);

    CodeItem::CatchBlock cb(method, nullptr, 0);
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr uint32_t kTryBlockLength = 10;
    CodeItem::TryBlock tryBlock(0, kTryBlockLength, {cb});
    codeItem->AddTryBlock(tryBlock);

    MemoryWriter writer;
    ASSERT_TRUE(container.Write(&writer));
    auto buf = writer.GetData();

    auto pandaFile = GetPandaFile(buf);
    ASSERT_NE(pandaFile, nullptr);

    ClassDataAccessor cda(*pandaFile, File::EntityId(klass->GetOffset()));
    cda.EnumerateMethods([&](MethodDataAccessor &mda) {
        CodeDataAccessor codeData(*pandaFile, mda.GetCodeId().value());
        codeData.EnumerateTryBlocks([](const CodeDataAccessor::TryBlock &) { return true; });
    });
}

TEST(ItemContainer, TryBlockDeclaredSizeOverflowShouldFatal)
{
    EXPECT_DEATH(TriggerFatalTryBlock(), ".*");
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(ItemContainer, TestErrorItemType)
{
    // Write panda file to memory
    ItemContainer container;

    ClassItem *emptyClassItem = container.GetOrCreateClassItem("Foo");

    ClassItem *classItem = container.GetOrCreateClassItem("Bar");
    classItem->SetAccessFlags(ACC_PUBLIC);
    classItem->SetSuperClass(emptyClassItem);

    // Add interface
    ClassItem *ifaceItem = container.GetOrCreateClassItem("Iface");
    ifaceItem->SetAccessFlags(ACC_PUBLIC);
    classItem->AddInterface(ifaceItem);

    // Add source file
    StringItem *sourceFile = container.GetOrCreateStringItem("source_file");
    classItem->SetSourceFile(sourceFile);

    MemoryWriter memWriter;
    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory
    auto data = memWriter.GetData();

    auto emptyDeleter = +[](std::byte *, size_t) noexcept {};
    auto reader = FileReader(File::OpenFromMemory(
        os::mem::ConstBytePtr(reinterpret_cast<std::byte *>(data.data()), data.size(), emptyDeleter)));

    const auto *items = reader.GetItems();
    File::EntityId classId(emptyClassItem->GetOffset());
    StringItem *errorClassItem = container.GetOrCreateStringItem("errorClassItem");
    auto *nonConstItems = const_cast<std::map<File::EntityId, BaseItem *> *>(items);
    nonConstItems->insert({classId, static_cast<BaseItem *>(errorClassItem)});

    EXPECT_DEATH({ reader.ReadContainer(); }, ".*");
}

}  // namespace ark::panda_file::test
