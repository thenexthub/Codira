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

#include "libarkfile/debug_info_extractor.h"
#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/file.h"
#include "libarkfile/file_item_container.h"
#include "libarkfile/file_writer.h"
#include "libarkfile/helpers.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/modifiers.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkbase/utils/utils.h"

#include <cstdio>

#include <memory>
#include <vector>

#include <gtest/gtest.h>

namespace ark::panda_file::test {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static const char SOURCE_FILE[] = "asm.pa";

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
void PreparePandaFile(ItemContainer *container)
{
    ClassItem *classItem = container->GetOrCreateClassItem("A");
    classItem->SetAccessFlags(ACC_PUBLIC);

    StringItem *methodName = container->GetOrCreateStringItem("foo");

    PrimitiveTypeItem *retType = container->GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params;
    params.emplace_back(container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    ProtoItem *protoItem = container->GetOrCreateProtoItem(retType, params);
    MethodItem *methodItem = classItem->AddMethod(methodName, protoItem, ACC_PUBLIC | ACC_STATIC, params);

    std::vector<uint8_t> instructions {1U, 2U, 3U, 4U};
    auto *codeItem = container->CreateItem<CodeItem>(4U, 1U, instructions);

    methodItem->SetCode(codeItem);

    StringItem *sourceFileItem = container->GetOrCreateStringItem(SOURCE_FILE);
    StringItem *paramStringItem = container->GetOrCreateStringItem("arg0");
    StringItem *localVariableName0 = container->GetOrCreateStringItem("local_0");
    StringItem *localVariableName1 = container->GetOrCreateStringItem("local_1");
    StringItem *localVariableName2 = container->GetOrCreateStringItem("local_2");
    StringItem *localVariableTypeI32 = container->GetOrCreateStringItem("I");
    StringItem *localVariableSigTypeI32 = container->GetOrCreateStringItem("type_i32");

    LineNumberProgramItem *lineNumberProgramItem = container->CreateLineNumberProgramItem();
    auto *debugInfoItem = container->CreateItem<DebugInfoItem>(lineNumberProgramItem);
    methodItem->SetDebugInfo(debugInfoItem);

    // Add static method with ref arg

    StringItem *methodNameBar = container->GetOrCreateStringItem("bar");

    PrimitiveTypeItem *retTypeBar = container->GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> paramsBar;
    paramsBar.emplace_back(container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I32));
    paramsBar.emplace_back(container->GetOrCreateClassItem("RefArg"));
    ProtoItem *protoItemBar = container->GetOrCreateProtoItem(retTypeBar, paramsBar);
    MethodItem *methodItemBar = classItem->AddMethod(methodNameBar, protoItemBar, ACC_PUBLIC | ACC_STATIC, paramsBar);

    auto *codeItemBar = container->CreateItem<CodeItem>(0U, 2U, instructions);

    methodItemBar->SetCode(codeItemBar);

    StringItem *paramStringItemBar1 = container->GetOrCreateStringItem("arg0");
    StringItem *paramStringItemBar2 = container->GetOrCreateStringItem("arg1");

    LineNumberProgramItem *lineNumberProgramItemBar = container->CreateLineNumberProgramItem();
    auto *debugInfoItemBar = container->CreateItem<DebugInfoItem>(lineNumberProgramItemBar);
    methodItemBar->SetDebugInfo(debugInfoItemBar);

    // Add non static method with ref arg

    StringItem *methodNameBaz = container->GetOrCreateStringItem("baz");

    PrimitiveTypeItem *retTypeBaz = container->GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> paramsBaz;
    paramsBaz.emplace_back(container->GetOrCreateClassItem("RefArg"));
    paramsBaz.emplace_back(container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U1));
    ProtoItem *protoItemBaz = container->GetOrCreateProtoItem(retTypeBaz, paramsBaz);
    MethodItem *methodItemBaz = classItem->AddMethod(methodNameBaz, protoItemBaz, ACC_PUBLIC, paramsBaz);

    auto *codeItemBaz = container->CreateItem<CodeItem>(0U, 2U, instructions);

    methodItemBaz->SetCode(codeItemBaz);

    StringItem *paramStringItemBaz1 = container->GetOrCreateStringItem("arg0");
    StringItem *paramStringItemBaz2 = container->GetOrCreateStringItem("arg1");

    LineNumberProgramItem *lineNumberProgramItemBaz = container->CreateLineNumberProgramItem();
    auto *debugInfoItemBaz = container->CreateItem<DebugInfoItem>(lineNumberProgramItemBaz);
    methodItemBaz->SetDebugInfo(debugInfoItemBaz);

    // Add debug info for the following source file;

    //  1 # file: asm.pa
    //  2 .function foo(i32 arg0) {
    //  3   ldai arg0
    //  4   stai v1     // START_LOCAL: reg=1, name="local_0", type="i32"
    //  5   ldai 2
    //  6   stai v2     // START_LOCAL_EXTENDED: reg=2, name="local_1",
    //  type="i32", type_signature="type_i32" 7               // END_LOCAL: reg=1
    //  8   stai v3     // START_LOCAL: reg=3, name="local_2", type="i32"
    //  9
    // 10   return.void
    // 11 }
    // 12 .function bar(i32 arg0, B arg1) { // static
    // 13   ldai arg0
    // 13   return.void
    // 14 }
    // 15 .function baz(B arg0, u1 arg1) { // non static
    // 17   ldai arg0
    // 16   return.void
    // 17 }

    container->ComputeLayout();

    // foo line number program
    auto *constantPool = debugInfoItem->GetConstantPool();
    // Line 3
    debugInfoItem->SetLineNumber(3U);
    lineNumberProgramItem->EmitSetFile(constantPool, sourceFileItem);
    lineNumberProgramItem->EmitAdvancePc(constantPool, 1);
    lineNumberProgramItem->EmitAdvanceLine(constantPool, 1);
    lineNumberProgramItem->EmitSpecialOpcode(0, 0);
    lineNumberProgramItem->EmitColumn(constantPool, 0, 7U);
    // Line 4
    lineNumberProgramItem->EmitStartLocal(constantPool, 1, localVariableName0, localVariableTypeI32);
    lineNumberProgramItem->EmitSpecialOpcode(1, 1);
    lineNumberProgramItem->EmitColumn(constantPool, 0, 8U);
    // Line 5
    lineNumberProgramItem->EmitSpecialOpcode(1, 1);
    // NOLINTNEXTLINE(readability-magic-numbers)
    lineNumberProgramItem->EmitColumn(constantPool, 0, 9U);
    // Line 6
    lineNumberProgramItem->EmitStartLocalExtended(constantPool, 2_I, localVariableName1, localVariableTypeI32,
                                                  localVariableSigTypeI32);
    lineNumberProgramItem->EmitEndLocal(1);
    lineNumberProgramItem->EmitSpecialOpcode(1, 2_I);
    // NOLINTNEXTLINE(readability-magic-numbers)
    lineNumberProgramItem->EmitColumn(constantPool, 0, 10U);
    // Line 8
    lineNumberProgramItem->EmitStartLocal(constantPool, 3_I, localVariableName2, localVariableTypeI32);
    lineNumberProgramItem->EmitAdvanceLine(constantPool, 2_I);
    lineNumberProgramItem->EmitSpecialOpcode(0, 0);
    // NOLINTNEXTLINE(readability-magic-numbers)
    lineNumberProgramItem->EmitColumn(constantPool, 0, 11U);
    // Line 10
    lineNumberProgramItem->EmitEnd();

    debugInfoItem->AddParameter(paramStringItem);

    methodItem->SetDebugInfo(debugInfoItem);

    // bar line number program
    auto *constantPoolBar = debugInfoItemBar->GetConstantPool();
    // NOLINTNEXTLINE(readability-magic-numbers)
    debugInfoItemBar->SetLineNumber(13U);
    lineNumberProgramItemBar->EmitSetFile(constantPoolBar, sourceFileItem);
    lineNumberProgramItemBar->EmitAdvancePc(constantPoolBar, 1);
    lineNumberProgramItemBar->EmitAdvanceLine(constantPoolBar, 1);
    lineNumberProgramItemBar->EmitSpecialOpcode(0, 0);
    lineNumberProgramItemBar->EmitColumn(constantPoolBar, 0, 5U);
    lineNumberProgramItemBar->EmitEnd();

    debugInfoItemBar->AddParameter(paramStringItemBar1);
    debugInfoItemBar->AddParameter(paramStringItemBar2);

    methodItemBar->SetDebugInfo(debugInfoItemBar);

    // baz line number program
    auto *constantPoolBaz = debugInfoItemBaz->GetConstantPool();
    // NOLINTNEXTLINE(readability-magic-numbers)
    debugInfoItemBaz->SetLineNumber(15U);
    lineNumberProgramItemBaz->EmitSetFile(constantPoolBaz, sourceFileItem);
    lineNumberProgramItemBaz->EmitAdvancePc(constantPoolBaz, 1);
    lineNumberProgramItemBaz->EmitAdvanceLine(constantPoolBaz, 1);
    lineNumberProgramItemBaz->EmitSpecialOpcode(0, 0);
    lineNumberProgramItemBaz->EmitColumn(constantPoolBaz, 0, 6U);
    lineNumberProgramItemBaz->EmitEnd();

    debugInfoItemBaz->AddParameter(paramStringItemBaz1);
    debugInfoItemBaz->AddParameter(paramStringItemBaz2);

    methodItemBaz->SetDebugInfo(debugInfoItemBaz);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct SourcePairLocation {
    std::string path;  // NOLINT(misc-non-private-member-variables-in-classes)
    size_t line;       // NOLINT(misc-non-private-member-variables-in-classes)

    bool operator==(const SourcePairLocation &other) const
    {
        return path == other.path && line == other.line;
    }

    bool IsValid() const
    {
        return !path.empty();
    }
};

static std::optional<size_t> GetLineNumberByTableOffsetWrapper(const panda_file::LineNumberTable &table,
                                                               uint32_t offset)
{
    for (const auto &value : table) {
        if (value.offset == offset) {
            return value.line;
        }
    }
    return std::nullopt;
}

static std::optional<uint32_t> GetOffsetByTableLineNumberWrapper(const panda_file::LineNumberTable &table, size_t line)
{
    for (const auto &value : table) {
        if (value.line == line) {
            return value.offset;
        }
    }
    return std::nullopt;
}

static std::pair<File::EntityId, uint32_t> GetBreakpointAddressWrapper(const DebugInfoExtractor &extractor,
                                                                       const SourcePairLocation &sourceLocation)
{
    auto pos = sourceLocation.path.find_last_of("/\\");
    auto name = sourceLocation.path;

    if (pos != std::string::npos) {
        name = name.substr(pos + 1);
    }

    std::vector<panda_file::File::EntityId> methods = extractor.GetMethodIdList();
    for (const auto &method : methods) {
        if (extractor.GetSourceFile(method) == sourceLocation.path || extractor.GetSourceFile(method) == name) {
            const panda_file::LineNumberTable &lineTable = extractor.GetLineNumberTable(method);
            if (lineTable.empty()) {
                continue;
            }

            std::optional<size_t> offset = GetOffsetByTableLineNumberWrapper(lineTable, sourceLocation.line);
            if (offset == std::nullopt) {
                continue;
            }
            return {method, offset.value()};
        }
    }
    return {File::EntityId(), 0};
}

static std::vector<panda_file::LocalVariableInfo> GetLocalVariableInfoWrapper(const DebugInfoExtractor &extractor,
                                                                              File::EntityId methodId, size_t offset)
{
    std::vector<panda_file::LocalVariableInfo> variables = extractor.GetLocalVariableTable(methodId);
    std::vector<panda_file::LocalVariableInfo> result;

    for (const auto &variable : variables) {
        if (variable.startOffset <= offset && offset <= variable.endOffset) {
            result.push_back(variable);
        }
    }
    return result;
}

static SourcePairLocation GetSourcePairLocationWrapper(const DebugInfoExtractor &extractor, File::EntityId methodId,
                                                       uint32_t bytecodeOffset)
{
    const panda_file::LineNumberTable &lineTable = extractor.GetLineNumberTable(methodId);
    if (lineTable.empty()) {
        return SourcePairLocation();
    }

    std::optional<size_t> line = GetLineNumberByTableOffsetWrapper(lineTable, bytecodeOffset);
    if (line == std::nullopt) {
        return SourcePairLocation();
    }

    return SourcePairLocation {extractor.GetSourceFile(methodId), line.value()};
}

static std::unique_ptr<const File> GetPandaFile(std::vector<uint8_t> &data)
{
    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data.data()), data.size(),
                              [](std::byte *, size_t) noexcept {});
    return File::OpenFromMemory(std::move(ptr));
}

class ExtractorTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        ItemContainer container;
        PreparePandaFile(&container);
        MemoryWriter writer;
        ASSERT_TRUE(container.Write(&writer));

        fileData_ = writer.GetData();
        uFile_ = GetPandaFile(fileData_);
    }

    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    static std::unique_ptr<const panda_file::File> uFile_;
    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    static std::vector<uint8_t> fileData_;
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::unique_ptr<const panda_file::File> ExtractorTest::uFile_ {nullptr};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::vector<uint8_t> ExtractorTest::fileData_;

TEST_F(ExtractorTest, DebugInfoTest)
{
    const panda_file::File *pf = uFile_.get();
    ASSERT_TRUE(pf != nullptr);
    DebugInfoExtractor extractor(pf);

    auto breakpoint1Address = GetBreakpointAddressWrapper(extractor, SourcePairLocation {SOURCE_FILE, 1});
    ASSERT_FALSE(breakpoint1Address.first.IsValid());
    auto [method_id, bytecode_offset] = GetBreakpointAddressWrapper(extractor, SourcePairLocation {SOURCE_FILE, 6U});
    ASSERT_TRUE(method_id.IsValid());
    ASSERT_EQ(bytecode_offset, 3U);

    auto sourceLocation = GetSourcePairLocationWrapper(extractor, method_id, 2U);
    ASSERT_EQ(sourceLocation.path, SOURCE_FILE);
    ASSERT_EQ(sourceLocation.line, 5U);

    auto vars = GetLocalVariableInfoWrapper(extractor, method_id, 4U);
    EXPECT_EQ(vars.size(), 2U);
    ASSERT_EQ(vars[0].name, "local_1");
    ASSERT_EQ(vars[0].type, "I");
    ASSERT_EQ(vars[1].name, "local_2");
    ASSERT_EQ(vars[1].type, "I");
}

TEST_F(ExtractorTest, DebugInfoTestStaticWithRefArg)
{
    const panda_file::File *pf = uFile_.get();
    ASSERT_TRUE(pf != nullptr);
    DebugInfoExtractor extractor(pf);

    auto breakpoint1Address = GetBreakpointAddressWrapper(extractor, SourcePairLocation {SOURCE_FILE, 1});
    ASSERT_FALSE(breakpoint1Address.first.IsValid());
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto methodId = GetBreakpointAddressWrapper(extractor, SourcePairLocation {SOURCE_FILE, 14U}).first;
    ASSERT_TRUE(methodId.IsValid());

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto vars = GetLocalVariableInfoWrapper(extractor, methodId, 14U);
    EXPECT_EQ(vars.size(), 0);
}

TEST_F(ExtractorTest, DebugInfoTestNonStaticWithRefArg)
{
    const panda_file::File *pf = uFile_.get();
    ASSERT_TRUE(pf != nullptr);
    DebugInfoExtractor extractor(pf);

    auto breakpoint1Address = GetBreakpointAddressWrapper(extractor, SourcePairLocation {SOURCE_FILE, 1});
    ASSERT_FALSE(breakpoint1Address.first.IsValid());
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto methodId = GetBreakpointAddressWrapper(extractor, SourcePairLocation {SOURCE_FILE, 16U}).first;
    ASSERT_TRUE(methodId.IsValid());

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto vars = GetLocalVariableInfoWrapper(extractor, methodId, 16U);
    EXPECT_EQ(vars.size(), 0);
}

TEST_F(ExtractorTest, DebugInfoTestColumnNumber)
{
    const panda_file::File *pf = uFile_.get();
    ASSERT_TRUE(pf != nullptr);
    DebugInfoExtractor extractor(pf);

    auto methods = extractor.GetMethodIdList();
    for (auto const &methodId : methods) {
        auto &cnt = extractor.GetColumnNumberTable(methodId);

        ASSERT(!cnt.empty());
        if (cnt[0].column == 5U || cnt[0].column == 6U) {
            ASSERT(cnt.size() == 1);
        } else {
            ASSERT(cnt[0].column == 7U);
            ASSERT(cnt.size() == 5U);

            auto i = 7;
            for (auto const &col : cnt) {
                EXPECT_EQ(col.column, i++);
            }
        }
    }
}
}  // namespace ark::panda_file::test
