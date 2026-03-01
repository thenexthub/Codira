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

#include <iomanip>
#include <tuple>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "libarkfile/annotation_data_accessor.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/file_items.h"
#include "lexer.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/param_annotations_data_accessor.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/leb128.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/bytecode_instruction-inl.h"

namespace ark::test {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark::pandasm;

static const uint8_t *GetTypeDescriptor(const std::string &name, std::string *storage)
{
    *storage = "L" + name + ";";
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(emittertests, test)
{
    Parser p;

    auto source = R"(            # 1
        .record R {              # 2
            i32 sf <static>      # 3
            i8  if               # 4
        }                        # 5
        #                        # 6
        .function void main() {  # 7
            return.void          # 8
        }                        # 9
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    // Check _GLOBAL class

    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        ASSERT_TRUE(classId.IsValid());
        ASSERT_FALSE(pf->IsExternal(classId));

        panda_file::ClassDataAccessor cda(*pf, classId);
        ASSERT_EQ(cda.GetSuperClassId().GetOffset(), 0U);
        ASSERT_EQ(cda.GetAccessFlags(), ACC_PUBLIC);
        ASSERT_EQ(cda.GetFieldsNumber(), 0U);
        ASSERT_EQ(cda.GetMethodsNumber(), 1U);
        ASSERT_EQ(cda.GetIfacesNumber(), 0U);

        ASSERT_FALSE(cda.GetSourceFileId().has_value());

        cda.EnumerateRuntimeAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });

        cda.EnumerateAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });

        cda.EnumerateFields([](panda_file::FieldDataAccessor &) { ASSERT_TRUE(false); });

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            ASSERT_FALSE(mda.IsExternal());
            ASSERT_EQ(mda.GetClassId(), classId);
            ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("main")),
                      0);

            panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
            ASSERT_EQ(pda.GetNumArgs(), 0U);
            ASSERT_EQ(pda.GetReturnType().GetId(), panda_file::Type::TypeId::VOID);

            ASSERT_EQ(mda.GetAccessFlags(), ACC_STATIC);
            ASSERT_TRUE(mda.GetCodeId().has_value());

            panda_file::CodeDataAccessor cdacc(*pf, mda.GetCodeId().value());
            ASSERT_EQ(cdacc.GetNumVregs(), 0U);
            ASSERT_EQ(cdacc.GetNumArgs(), 0U);
            ASSERT_EQ(cdacc.GetCodeSize(), 1U);
            ASSERT_EQ(cdacc.GetTriesSize(), 0U);

            ASSERT_FALSE(mda.GetRuntimeParamAnnotationId().has_value());
            ASSERT_FALSE(mda.GetParamAnnotationId().has_value());
            ASSERT_TRUE(mda.GetDebugInfoId().has_value());

            panda_file::DebugInfoDataAccessor dda(*pf, mda.GetDebugInfoId().value());
            ASSERT_EQ(dda.GetLineStart(), 8U);
            ASSERT_EQ(dda.GetNumParams(), 0U);

            mda.EnumerateRuntimeAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });
            mda.EnumerateAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });
        });
    }

    // Check R class

    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
        ASSERT_TRUE(classId.IsValid());
        ASSERT_FALSE(pf->IsExternal(classId));

        panda_file::ClassDataAccessor cda(*pf, classId);
        ASSERT_EQ(cda.GetSuperClassId().GetOffset(), 0U);
        ASSERT_EQ(cda.GetAccessFlags(), 0);
        ASSERT_EQ(cda.GetFieldsNumber(), 2U);
        ASSERT_EQ(cda.GetMethodsNumber(), 0U);
        ASSERT_EQ(cda.GetIfacesNumber(), 0U);

        // We emit SET_FILE in debuginfo
        ASSERT_TRUE(cda.GetSourceFileId().has_value());

        EXPECT_EQ(std::string(reinterpret_cast<const char *>(pf->GetStringData(cda.GetSourceFileId().value()).data)),
                  sourceFilename);

        cda.EnumerateRuntimeAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });

        cda.EnumerateAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });

        struct FieldData {
            std::string name;
            panda_file::Type::TypeId typeId;
            uint32_t accessFlags;
        };

        std::vector<FieldData> fields {{"sf", panda_file::Type::TypeId::I32, ACC_STATIC},
                                       {"if", panda_file::Type::TypeId::I8, 0}};

        size_t i = 0;
        cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
            ASSERT_FALSE(fda.IsExternal());
            ASSERT_EQ(fda.GetClassId(), classId);
            ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(fda.GetNameId()).data,
                                               utf::CStringAsMutf8(fields[i].name.c_str())),
                      0);

            ASSERT_EQ(fda.GetType(), panda_file::Type(fields[i].typeId).GetFieldEncoding());
            ASSERT_EQ(fda.GetAccessFlags(), fields[i].accessFlags);

            fda.EnumerateRuntimeAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });
            fda.EnumerateAnnotations([](panda_file::File::EntityId) { ASSERT_TRUE(false); });

            ++i;
        });

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &) { ASSERT_TRUE(false); });
    }
}

uint8_t GetSpecialOpcode(uint32_t pcInc, int32_t lineInc)
{
    return (lineInc - panda_file::LineNumberProgramItem::LINE_BASE) +
           (pcInc * panda_file::LineNumberProgramItem::LINE_RANGE) + panda_file::LineNumberProgramItem::OPCODE_BASE;
}

static auto g_debuginfo = R"(
        .function void main() {
            ldai.64 0   # line 3, pc 0
                        # line 4
                        # line 5
                        # line 6
                        # line 7
                        # line 8
                        # line 9
                        # line 10
                        # line 11
                        # line 12
                        # line 13
                        # line 14
            ldai.64 1   # line 15, pc 9
            return.void # line 16, pc 18
        }
    )";

TEST(emittertests, debuginfo)
{
    Parser p;

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(g_debuginfo, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;
    auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    ASSERT_TRUE(classId.IsValid());

    panda_file::ClassDataAccessor cda(*pf, classId);

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        panda_file::CodeDataAccessor cdacc(*pf, mda.GetCodeId().value());
        ASSERT_TRUE(mda.GetDebugInfoId().has_value());

        panda_file::DebugInfoDataAccessor dda(*pf, mda.GetDebugInfoId().value());
        ASSERT_EQ(dda.GetLineStart(), 3U);
        ASSERT_EQ(dda.GetNumParams(), 0U);

        const uint8_t *program = dda.GetLineNumberProgram();
        Span<const uint8_t> constantPool = dda.GetConstantPool();

        std::vector<uint8_t> opcodes {
            static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::SET_FILE),
            static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::ADVANCE_PC),
            static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::ADVANCE_LINE), GetSpecialOpcode(0, 0),
            // NOLINTNEXTLINE(readability-magic-numbers)
            GetSpecialOpcode(9, 1), static_cast<uint8_t>(panda_file::LineNumberProgramItem::Opcode::END_SEQUENCE)};

        EXPECT_THAT(opcodes, ::testing::ElementsAreArray(program, opcodes.size()));

        size_t size {};
        bool isFull {};
        size_t constantPoolOffset = 0;

        uint32_t offset {};

        std::tie(offset, size, isFull) = leb128::DecodeUnsigned<uint32_t>(&constantPool[constantPoolOffset]);
        constantPoolOffset += size;
        ASSERT_TRUE(isFull);
        EXPECT_EQ(
            std::string(reinterpret_cast<const char *>(pf->GetStringData(panda_file::File::EntityId(offset)).data)),
            sourceFilename);

        uint32_t pcInc;
        std::tie(pcInc, size, isFull) = leb128::DecodeUnsigned<uint32_t>(&constantPool[constantPoolOffset]);
        constantPoolOffset += size;
        ASSERT_TRUE(isFull);
        EXPECT_EQ(pcInc, 9U);

        int32_t lineInc;
        std::tie(lineInc, size, isFull) = leb128::DecodeSigned<int32_t>(&constantPool[constantPoolOffset]);
        constantPoolOffset += size;
        ASSERT_TRUE(isFull);
        EXPECT_EQ(lineInc, 12);

        EXPECT_EQ(constantPoolOffset, constantPool.size());
    });
}

static auto g_exceptions = R"(
        .record Exception1 {}
        .record Exception2 {}

        .function void main() {
            ldai.64 0
        try_begin:
            ldai.64 1
            ldai.64 2
        try_end:
            ldai.64 3
        catch_begin1:
            ldai.64 4
        catch_begin2:
            ldai.64 5
        catchall_begin:
            ldai.64 6

        .catch Exception1, try_begin, try_end, catch_begin1
        .catch Exception2, try_begin, try_end, catch_begin2
        .catchall try_begin, try_end, catchall_begin
        }
    )";

TEST(emittertests, exceptions)
{
    Parser p;

    auto res = p.Parse(g_exceptions);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;

    auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    ASSERT_TRUE(classId.IsValid());

    panda_file::ClassDataAccessor cda(*pf, classId);

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        panda_file::CodeDataAccessor cdacc(*pf, mda.GetCodeId().value());
        ASSERT_EQ(cdacc.GetNumVregs(), 0U);
        ASSERT_EQ(cdacc.GetNumArgs(), 0U);
        ASSERT_EQ(cdacc.GetTriesSize(), 1);

        cdacc.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
            EXPECT_EQ(tryBlock.GetStartPc(), 9);
            EXPECT_EQ(tryBlock.GetLength(), 18);
            EXPECT_EQ(tryBlock.GetNumCatches(), 3);

            struct CatchInfo {
                panda_file::File::EntityId typeId;
                uint32_t handlerPc;
            };

            // NOLINTBEGIN(readability-magic-numbers)
            std::vector<CatchInfo> catchInfos {{pf->GetClassId(GetTypeDescriptor("Exception1", &descriptor)), 4 * 9},
                                               {pf->GetClassId(GetTypeDescriptor("Exception2", &descriptor)), 5 * 9},
                                               {panda_file::File::EntityId(), 6 * 9}};
            // NOLINTEND(readability-magic-numbers)

            size_t i = 0;
            tryBlock.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
                auto idx = catchBlock.GetTypeIdx();
                auto id = idx != panda_file::INVALID_INDEX ? pf->ResolveClassIndex(mda.GetMethodId(), idx)
                                                           : panda_file::File::EntityId();
                EXPECT_EQ(id, catchInfos[i].typeId);
                EXPECT_EQ(catchBlock.GetHandlerPc(), catchInfos[i].handlerPc);
                ++i;

                return true;
            });

            return true;
        });
    });
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(emittertests, errors)
{
    {
        Parser p;
        auto source = R"(
            .record A {
                B b
            }
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_EQ(pf, nullptr);
        ASSERT_EQ(AsmEmitter::GetLastError(), "Field A.b has undefined type");
    }

    {
        Parser p;
        auto source = R"(
            .function void A.b() {}
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_EQ(pf, nullptr);
        ASSERT_EQ(AsmEmitter::GetLastError(), "Function A.b is bound to undefined record A");
    }

    {
        Parser p;
        auto source = R"(
            .function A b() {}
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_EQ(pf, nullptr);
        ASSERT_EQ(AsmEmitter::GetLastError(), "Function b has undefined return type");
    }

    {
        Parser p;
        auto source = R"(
            .function void a(b a0) {}
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_EQ(pf, nullptr);
        ASSERT_EQ(AsmEmitter::GetLastError(), "Argument 0 of function a has undefined type");
    }

    {
        Parser p;
        auto source = R"(
            .record A <external>
            .function void A.x() {}
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_EQ(pf, nullptr);
        ASSERT_EQ(AsmEmitter::GetLastError(), "Non-external function A.x is bound to external record");
    }
}

TEST(emittertests, language)
{
    {
        Parser p;
        auto source = R"(
            .function void foo() {}
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        std::string descriptor;

        auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
        ASSERT_TRUE(classId.IsValid());

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_FALSE(cda.GetSourceLang());

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) { ASSERT_FALSE(mda.GetSourceLang()); });
    }
}

TEST(emittertests, constructors)
{
    {
        Parser p;
        auto source = R"(
            .record R {}
            .function void R.foo(R a0) <ctor> {}
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        std::string descriptor;

        auto classId = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
        ASSERT_TRUE(classId.IsValid());

        panda_file::ClassDataAccessor cda(*pf, classId);

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            auto *name = utf::Mutf8AsCString(pf->GetStringData(mda.GetNameId()).data);
            ASSERT_STREQ(name, ".ctor");
        });
    }

    {
        Parser p;
        auto source = R"(
            .record R {}
            .function void R.foo(R a0) <cctor> {}
        )";

        auto res = p.Parse(source);
        ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

        auto pf = AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        std::string descriptor;

        auto classId = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
        ASSERT_TRUE(classId.IsValid());

        panda_file::ClassDataAccessor cda(*pf, classId);

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            auto *name = utf::Mutf8AsCString(pf->GetStringData(mda.GetNameId()).data);
            ASSERT_STREQ(name, ".cctor");
        });
    }
}

static auto g_fieldValue = R"(
        .record panda.String <external>

        .record R {
            u1 f_u1 <value=1>
            i8 f_i8 <value=2>
            u8 f_u8 <value=128>
            i16 f_i16 <value=256>
            u16 f_u16 <value=32768>
            i32 f_i32 <value=65536>
            u32 f_u32 <value=2147483648>
            i64 f_i64 <value=4294967296>
            u64 f_u64 <value=9223372036854775808>
            f32 f_f32 <value=1.0>
            f64 f_f64 <value=2.0>
            panda.String f_str <value="str">
        }
    )";

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(emittertests, field_value)
{
    Parser p;

    struct FieldData {
        std::string name;
        panda_file::Type::TypeId typeId;
        std::variant<int32_t, uint32_t, int64_t, uint64_t, float, double, std::string> value;
    };

    // NOLINTBEGIN(readability-magic-numbers)
    std::vector<FieldData> data {
        {"f_u1", panda_file::Type::TypeId::U1, static_cast<uint32_t>(1)},
        {"f_i8", panda_file::Type::TypeId::I8, static_cast<int32_t>(2)},
        {"f_u8", panda_file::Type::TypeId::U8, static_cast<uint32_t>(128)},
        {"f_i16", panda_file::Type::TypeId::I16, static_cast<int32_t>(256)},
        {"f_u16", panda_file::Type::TypeId::U16, static_cast<uint32_t>(32768)},
        {"f_i32", panda_file::Type::TypeId::I32, static_cast<int32_t>(65536)},
        {"f_u32", panda_file::Type::TypeId::U32, static_cast<uint32_t>(2147483648)},
        {"f_i64", panda_file::Type::TypeId::I64, static_cast<int64_t>(4294967296)},
        {"f_u64", panda_file::Type::TypeId::U64, static_cast<uint64_t>(9223372036854775808ULL)},
        {"f_f32", panda_file::Type::TypeId::F32, static_cast<float>(1.0)},
        {"f_f64", panda_file::Type::TypeId::F64, static_cast<double>(2.0)},
        {"f_str", panda_file::Type::TypeId::REFERENCE, "str"}};
    // NOLINTEND(readability-magic-numbers)

    auto res = p.Parse(g_fieldValue);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;
    auto classId = pf->GetClassId(GetTypeDescriptor("R", &descriptor));
    ASSERT_TRUE(classId.IsValid());
    ASSERT_FALSE(pf->IsExternal(classId));

    panda_file::ClassDataAccessor cda(*pf, classId);
    ASSERT_EQ(cda.GetFieldsNumber(), data.size());

    auto pandaStringId = pf->GetClassId(GetTypeDescriptor("panda.String", &descriptor));

    size_t idx = 0;
    cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
        const FieldData &fieldData = data[idx];

        ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(fda.GetNameId()).data,
                                           utf::CStringAsMutf8(fieldData.name.c_str())),
                  0);

        panda_file::Type type(fieldData.typeId);
        uint32_t typeValue;
        if (type.IsReference()) {
            typeValue = pandaStringId.GetOffset();
        } else {
            typeValue = type.GetFieldEncoding();
        }

        ASSERT_EQ(fda.GetType(), typeValue);

        switch (fieldData.typeId) {
            case panda_file::Type::TypeId::U1: {
                auto result = fda.GetValue<uint8_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<uint32_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::I8: {
                auto result = fda.GetValue<int8_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<int32_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::U8: {
                auto result = fda.GetValue<uint8_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<uint32_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::I16: {
                auto result = fda.GetValue<int16_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<int32_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::U16: {
                auto result = fda.GetValue<uint16_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<uint32_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::I32: {
                auto result = fda.GetValue<int32_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<int32_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::U32: {
                auto result = fda.GetValue<uint32_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<uint32_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::I64: {
                auto result = fda.GetValue<int64_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<int64_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::U64: {
                auto result = fda.GetValue<uint64_t>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<uint64_t>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::F32: {
                auto result = fda.GetValue<float>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<float>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::F64: {
                auto result = fda.GetValue<double>();
                ASSERT_TRUE(result);
                ASSERT_EQ(result.value(), std::get<double>(fieldData.value));
                break;
            }
            case panda_file::Type::TypeId::REFERENCE: {
                auto result = fda.GetValue<uint32_t>();
                ASSERT_TRUE(result);

                panda_file::File::EntityId stringId(result.value());
                auto val = std::get<std::string>(fieldData.value);

                ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(stringId).data, utf::CStringAsMutf8(val.c_str())),
                          0);
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }

        ++idx;
    });
}

TEST(emittertests, tagged_in_func_decl)
{
    Parser p;
    auto source = R"(
        .function any foo(any a0) <noimpl>
    )";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;

    auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    ASSERT_TRUE(classId.IsValid());

    panda_file::ClassDataAccessor cda(*pf, classId);

    size_t numMethods = 0;
    const auto tagged = panda_file::Type(panda_file::Type::TypeId::TAGGED);
    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        ASSERT_EQ(tagged, pda.GetReturnType());
        ASSERT_EQ(1, pda.GetNumArgs());
        ASSERT_EQ(tagged, pda.GetArgType(0));

        ++numMethods;
    });
    ASSERT_EQ(1, numMethods);
}

TEST(emittertests, tagged_in_field_decl)
{
    Parser p;
    auto source = R"(
        .record Test {
            any foo
        }
    )";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;

    auto classId = pf->GetClassId(GetTypeDescriptor("Test", &descriptor));
    ASSERT_TRUE(classId.IsValid());

    panda_file::ClassDataAccessor cda(*pf, classId);

    size_t numFields = 0;
    const auto tagged = panda_file::Type(panda_file::Type::TypeId::TAGGED);
    cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
        uint32_t type = fda.GetType();
        ASSERT_EQ(tagged.GetFieldEncoding(), type);

        ++numFields;
    });
    ASSERT_EQ(1, numFields);
}

TEST(emittertests, function_overloading_1)
{
    Parser p;
    auto source = R"(
        .function void foo(i8 a0) {}
        .function u1 foo(u1 a0) {}

        .function i8 f(i32 a0) {
            call foo:(i8), v0
            call foo:(u1), v1
        }
    )";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor {};

    auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    ASSERT_TRUE(classId.IsValid());

    panda_file::ClassDataAccessor cda(*pf, classId);

    size_t numMethods = 0;

    std::unordered_map<panda_file::File::EntityId, panda_file::Type> idToArgType {};
    panda_file::File::EntityId idF {};

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        ++numMethods;

        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());

        if (pda.GetArgType(0) == panda_file::Type(panda_file::Type::TypeId::I32)) {
            idF = mda.GetMethodId();

            return;
        }

        idToArgType.emplace(mda.GetMethodId(), pda.GetArgType(0));
    });

    panda_file::MethodDataAccessor mdaF(*pf, idF);
    panda_file::CodeDataAccessor cdaF(*pf, mdaF.GetCodeId().value());

    const auto insSz = cdaF.GetCodeSize();
    const auto insArr = cdaF.GetInstructions();

    auto bcIns = BytecodeInstruction(insArr);
    const auto bcInsLast = bcIns.JumpTo(insSz);

    while (bcIns.GetAddress() != bcInsLast.GetAddress()) {
        const auto argMethodIdx = bcIns.GetId().AsIndex();
        const auto argMethodId = pf->ResolveMethodIndex(idF, argMethodIdx);

        panda_file::MethodDataAccessor methodAccessor(*pf, argMethodId);
        panda_file::ProtoDataAccessor protoAccessor(*pf, methodAccessor.GetProtoId());

        ASSERT_EQ(protoAccessor.GetArgType(0), idToArgType.at(argMethodId));
        ASSERT_EQ(std::string(utf::Mutf8AsCString(pf->GetStringData(methodAccessor.GetNameId()).data)), "foo");

        bcIns = bcIns.GetNext();
    }

    ASSERT_EQ(3, numMethods);
}

static auto g_testAccessModifiers = R"(
        .record A <access.record=private> {}

        .record B <access.record=public> {
            i32 pub <access.field=public>
            i32 prt <access.field=protected>
            i32 prv <access.field=private>
        }

        .record C <access.record=protected> {}

        .function void f() <access.function=public> {}
        .function void A.f() <access.function=protected> {}
        .function void B.f() <access.function=private> {}
        .function void C.f() <access.function=public> {}
)";

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(emittertests, access_modifiers)
{
    Parser p;

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(g_testAccessModifiers, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    // Global
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.GetMethodsNumber() == 1);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("f")), 0);

            ASSERT_TRUE(mda.IsPublic());
        });
    }

    // record A
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("A", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.IsPrivate());
        ASSERT_TRUE(cda.GetMethodsNumber() == 1);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("f")), 0);

            ASSERT_TRUE(mda.IsProtected());
        });
    }

    // record B
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("B", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.IsPublic());
        ASSERT_TRUE(cda.GetFieldsNumber() == 3);
        auto i = 0;
        auto accessPredicates =
            std::array {&panda_file::FieldDataAccessor::IsPublic, &panda_file::FieldDataAccessor::IsProtected,
                        &panda_file::FieldDataAccessor::IsPrivate};
        cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
            EXPECT_TRUE((fda.*accessPredicates[i])());

            ++i;
        });

        ASSERT_TRUE(cda.GetMethodsNumber() == 1);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("f")), 0);

            ASSERT_TRUE(mda.IsPrivate());
        });
    }

    // record C
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("C", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.IsProtected());
        ASSERT_TRUE(cda.GetMethodsNumber() == 1);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("f")), 0);

            ASSERT_TRUE(mda.IsPublic());
        });
    }
}

TEST(emittertests, valid_inheritance)
{
    Parser p;

    auto source = R"(
        .record A {}
        .record B <extends=A> {}
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);
}

TEST(emittertests, inheritance_undefined_record)
{
    Parser p;

    auto source = R"(
        .record A <extends=B> {}
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_EQ(pf, nullptr);
    ASSERT_EQ(AsmEmitter::GetLastError(), "Base record B is not defined for record A");
}

TEST(emittertests, primitive_inheritance)
{
    Parser p;

    auto source = R"(
        .record A <extends=u8> {}
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_EQ(pf, nullptr);
}

TEST(emittertests, final_modifier)
{
    Parser p;

    auto source = R"(
        .record A <final> {}

        .record B {
            i32 fld <final>
        }

        .record C {}

        .function void f0() <final> {}
        .function void C.f1() <final> {}
        .function void C.f2(C a0) <final> {}
        .function void C.f3(i32 a0) <final> {}
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    // Global
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.GetMethodsNumber() == 1);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("f0")), 0);

            ASSERT_TRUE(mda.IsFinal());
        });
    }

    // record A
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("A", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.IsFinal());
    }

    // record B
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("B", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.GetFieldsNumber() == 1);
        cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) { ASSERT_TRUE(fda.IsFinal()); });
    }

    // record C
    {
        std::string descriptor;
        auto classId = pf->GetClassId(GetTypeDescriptor("C", &descriptor));

        panda_file::ClassDataAccessor cda(*pf, classId);

        ASSERT_TRUE(cda.GetMethodsNumber() == 3);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) { ASSERT_TRUE(mda.IsFinal()); });
    }
}

TEST(emittertests, valid_method)
{
    Parser p;

    auto source = R"(
        .record A <access.record=public> {
        }

        .function void A.method(A a0) <access.function=public> {
            return.void
        }

        .function void A.method() <static, access.function=public> {
            return.void
        }
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;
    auto classId = pf->GetClassId(GetTypeDescriptor("A", &descriptor));

    panda_file::ClassDataAccessor cda(*pf, classId);

    ASSERT_TRUE(cda.GetMethodsNumber() == 2);
    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        ASSERT_EQ(utf::CompareMUtf8ToMUtf8(pf->GetStringData(mda.GetNameId()).data, utf::CStringAsMutf8("method")), 0);
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        if (pda.GetNumArgs() == 1) {
            ASSERT_TRUE(mda.IsStatic());
        }
    });
}

TEST(emittertests, test_union_canonicalization)
{
    Parser p;
    std::string source =
        ".record std.core.Object <external>\n"
        ".record A <extends=std.core.Object, access.record=public> {}\n"
        ".record B <extends=std.core.Object, access.record=public> {}\n"
        ".record C <extends=std.core.Object, access.record=public> {}\n"
        ".record D <extends=std.core.Object, access.record=public> {}\n"

        ".record {UC,C,B,C,B} <external>\n"
        ".record {UD,C,B,A} <external>\n"
        ".record {UA,A,A,B} <external>\n"
        ".record {UA,std.core.Double} <external>\n"
        ".record {Ustd.core.Double,std.core.Double,std.core.Long} <external>\n"
        ".record {Ustd.core.Double,std.core.Int,std.core.Long} <external>\n"
        ".record {UA,B,C,B,A} <external>\n"
        ".record {UA,D,D,D} <external>";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::vector<const char *> classDescriptors = {"{ULB;LC;}",
                                                  "{ULA;LB;LC;LD;}",
                                                  "{ULA;LB;}",
                                                  "{ULA;Lstd/core/Double;}",
                                                  "{ULstd/core/Double;Lstd/core/Long;}",
                                                  "{ULstd/core/Double;Lstd/core/Int;Lstd/core/Long;}",
                                                  "{ULA;LB;LC;}",
                                                  "{ULA;LD;}"};

    for (const auto &desc : classDescriptors) {
        auto classId = pf->GetClassId(utf::CStringAsMutf8(desc));
        ASSERT_TRUE(classId.IsValid());
        ASSERT_TRUE(pf->IsExternal(classId));
    }
}

TEST(emittertests, test_union_canonicalization_arrays)
{
    Parser p;
    std::string source =
        ".record std.core.Object <external>\n"
        ".record A <extends=std.core.Object, access.record=public> {}\n"
        ".record B <extends=std.core.Object, access.record=public> {}\n"
        ".record C <extends=std.core.Object, access.record=public> {}\n"
        ".record D <extends=std.core.Object, access.record=public> {}\n"

        ".record {UB,C,{UA,D,std.core.Double}[],A} <external>\n"
        ".record {U{UD,A,std.core.Int}[],{UA,D,std.core.Double}[],{Ustd.core.Long,D,A}[]} <external>\n"
        ".record {UB,C,{UA,B}[],A} <external>\n"
        ".record {UB,{UA,D}[],{UD,B}[],{UC,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[],{UD,B}[],{UD,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[],{UD,B}[][],{UD,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[][],{UD,B}[]} <external>\n"
        ".record {U{UD,C}[],{UC,B}[]} <external>\n"
        ".record {U{UD,C}[],C,B} <external>\n"
        ".record {U{Ustd.core.Int,std.core.Double}[],std.core.Long,std.core.Double} <external>\n"
        ".record {U{Ustd.core.Int[],std.core.Double[]}[],std.core.Long[],std.core.Double[]} <external>";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::vector<const char *> classDescriptors = {
        "{ULA;LB;LC;[{ULA;LD;Lstd/core/Double;}}",
        "{U[{ULA;LD;Lstd/core/Double;}[{ULA;LD;Lstd/core/Int;}[{ULA;LD;Lstd/core/Long;}}",
        "{ULA;LB;LC;[{ULA;LB;}}",
        "{ULB;[{ULA;LD;}[{ULB;LC;}[{ULB;LD;}[{ULC;LD;}}",
        "{U[{ULA;LD;}[{ULB;LD;}[{ULC;LD;}}",
        "{U[{ULA;LD;}[{ULB;LD;}[[{ULB;LD;}[{ULC;LD;}}",
        "{U[[{ULA;LD;}[{ULB;LD;}}",
        "{U[{ULB;LC;}[{ULC;LD;}}",
        "{ULB;LC;[{ULC;LD;}}",
        "{ULstd/core/Double;Lstd/core/Long;[{ULstd/core/Double;Lstd/core/Int;}}",
        "{U[Lstd/core/Double;[Lstd/core/Long;[{U[Lstd/core/Double;[Lstd/core/Int;}}"};

    for (const auto &desc : classDescriptors) {
        auto classId = pf->GetClassId(utf::CStringAsMutf8(desc));
        ASSERT_TRUE(classId.IsValid());
        ASSERT_TRUE(pf->IsExternal(classId));
    }
}

TEST(emittertests, test_union_in_function)
{
    Parser p;

    std::string source =
        ".record A <external>\n"
        ".record Y <external>\n"
        ".record N <external>\n"
        ".function i32 fooWithBody(Y a0) {\n"
        "    lda.obj a0\n"
        "    isinstance {Ui32[],A}\n"
        "    isinstance {Ui32[],Y}\n"
        "    isinstance {Ui32[],Y[][]}\n"
        "    isinstance {Ui32[],N}\n"
        "    checkcast {Ui32[],A}\n"
        "    checkcast {Ui32[],Y}\n"
        "    checkcast {Ui32[],Y[][]}\n"
        "    checkcast {Ui32[],N}\n"
        "    lda.type {Ui32[],A}\n"
        "    lda.type {Ui32[],Y}\n"
        "    lda.type {Ui32[],Y[][]}\n"
        "    lda.type {Ui32[],N}\n"
        "}";
    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    std::cout << p.ShowError().message << std::endl;
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;
    auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));

    panda_file::ClassDataAccessor cda(*pf, classId);
    ASSERT_TRUE(cda.GetMethodsNumber() == 1U);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid test logic
TEST(emittertests, test_any_in_instructions)
{
    Parser p;

    auto source = R"(
        .record Y <external>
        .record N <external>

        .function void foo1(Y a0) <static, access.function=public> {
            lda.type Y
            return.void
        }

        .function void foo2(N a0) <static, access.function=public> {
            lda.type N
            return.void
        }

        .function void foo3(Y a0) <static, access.function=public> {
            lda.type Y[]
            return.void
        }

       .function void foo4(Y a0) <static, access.function=public> {
            lda.type Y[][]
            return.void
        }

        .function i32 foo5(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance Y
            return
        }

        .function i32 foo6(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance N
            return
        }

        .function i32 foo7(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance Y[]
            return
        }

        .function i32 foo8(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance Y[][]
            return
        }

        .function void foo9(Y a0) <static, access.function=public> {
            lda.obj a0
            checkcast Y
            return.void
        }

        .function void foo10(N a0) <static, access.function=public> {
            lda.obj a0
            checkcast N
            return.void
        }

        .function void foo11(Y a0) <static, access.function=public> {
            lda.obj a0
            checkcast Y[]
            return.void
        }

        .function void foo12(Y a0) <static, access.function=public> {
            lda.obj a0
            checkcast Y[][]
            return.void
        }
    )";
    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;
    auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));

    panda_file::ClassDataAccessor cda(*pf, classId);
    ASSERT_TRUE(cda.GetMethodsNumber() == 12U);
}

TEST(emittertests, test_any_function)
{
    Parser p;

    auto source = R"(
        .record Y <external>
        .record N <external>

        .function i32 Any(Y a0) <static, access.function=public> {}
        .function i32 never(Y a0) <static, access.function=public> {}
        .function i32 foo(Y a0) <static, access.function=public> {}
        .function Y foo1() <static, access.function=public> {}
        .function Y foo2(Y a0) <static, access.function=public> {}
        .function Y foo7(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function Y foo8(Y a0, f32 a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function i32 foo6(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function i32 foo5(Y a0, f32 a1, i32 a2) <static, access.function=public> {}
        .function Y[] foo(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function Y[] foo(Y a0, f32 a1, Y a2, Y[] a3, Y a4) <static, access.function=public> {}

        .function i32 foo(N a0) <static, access.function=public> {}
        .function N foo9() <static, access.function=public> {}
        .function N foo2(N a0) <static, access.function=public> {}
        .function N foo7(N a0, N a1, N a2, N a3, N a4) <static, access.function=public> {}
        .function N foo8(N a0, f32 a1, N a2, N a3, N a4) <static, access.function=public> {}
        .function i32 foo6(N a0, N a1, N a2, N a3, N a4) <static, access.function=public> {}
        .function i32 foo5(N a0, f32 a1, i32 a2) <static, access.function=public> {}
        .function i32 fooWithBody(Y a0) {
            lda.obj a0
            isinstance Y
            checkcast Y
            isinstance Y[]
            checkcast Y[][]
            isinstance N
            checkcast N
            return
        }
    )";
    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;
    auto classId = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));

    panda_file::ClassDataAccessor cda(*pf, classId);
    ASSERT_TRUE(cda.GetMethodsNumber() == 19U);
}

TEST(emittertests, records_with_any_and_never)
{
    Parser p;

    auto source = R"(
        .record Y <external>
        .record N <external>

        .record A {
            Y field <access.field=public>
        }

        .record B {
            Y field <access.field=public>
            Y field2 <access.field=public>
        }

        .record C {
            Y[] field <access.field=public>
        }

        .record F {
            N field <access.field=public>
        }

        .record D {
            N field <access.field=public>
            N field2 <access.field=public>
        }
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);
}

}  // namespace ark::test
