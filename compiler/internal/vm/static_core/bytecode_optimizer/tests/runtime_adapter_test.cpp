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

#include <string>
#include <vector>

#include "assembler/assembly-program.h"
#include "assembler/assembly-emitter.h"
#include "common.h"
#include "compiler/optimizer/ir/datatype.h"
#include "runtime_adapter.h"
#include "libarkbase/utils/utf.h"

namespace ark::bytecodeopt::test {

std::unique_ptr<const panda_file::File> ParseAndEmit(const std::string &source)
{
    ark::pandasm::Parser parser;
    auto res = parser.Parse(source);
    if (parser.ShowError().err != pandasm::Error::ErrorType::ERR_NONE) {
        std::cerr << "Parse failed: " << parser.ShowError().message << std::endl
                  << parser.ShowError().wholeLine << std::endl;
        ADD_FAILURE();
    }
    auto &program = res.Value();
    return pandasm::AsmEmitter::Emit(program);
}

struct Pointers {
    std::vector<RuntimeInterface::MethodPtr> method;
    std::vector<RuntimeInterface::ClassPtr> klass;
    std::vector<RuntimeInterface::FieldPtr> field;
};

Pointers GetPointers(const panda_file::File *arkf)
{
    Pointers pointers;

    for (uint32_t id : arkf->GetClasses()) {
        panda_file::File::EntityId recordId {id};

        if (arkf->IsExternal(recordId)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*arkf, recordId};
        auto classId = cda.GetClassId().GetOffset();
        auto classPtr = reinterpret_cast<compiler::RuntimeInterface::ClassPtr>(classId);
        pointers.klass.push_back(classPtr);

        cda.EnumerateFields([&pointers](panda_file::FieldDataAccessor &fda) {
            auto fieldPtr = reinterpret_cast<compiler::RuntimeInterface::FieldPtr>(fda.GetFieldId().GetOffset());
            pointers.field.push_back(fieldPtr);
        });

        cda.EnumerateMethods([&pointers](panda_file::MethodDataAccessor &mda) {
            auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());
            pointers.method.push_back(methodPtr);
        });
    }

    return pointers;
}

TEST(RuntimeAdapter, Common)
{
    auto source = std::string(R"(
        .function u1 main() {
            ldai 1
            return
        }
        )");
    std::unique_ptr<const panda_file::File> arkf = ParseAndEmit(source);
    auto pointers = GetPointers(arkf.get());

    ASSERT_EQ(pointers.method.size(), 1U);
    ASSERT_EQ(pointers.klass.size(), 1U);
    ASSERT_EQ(pointers.field.size(), 0U);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf);
    auto main = pointers.method[0U];
    auto global = pointers.klass[0U];

    EXPECT_FALSE(adapter.IsMethodIntrinsic(main));
    EXPECT_NE(adapter.GetBinaryFileForMethod(main), nullptr);
    EXPECT_EQ(adapter.GetMethodById(main, 0U), nullptr);
    EXPECT_NE(adapter.GetMethodId(main), 0U);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 0U), compiler::DataType::Type::ANY);
    EXPECT_EQ(adapter.GetMethodReturnType(pointers.method[0U]), compiler::DataType::Type::BOOL);
    EXPECT_EQ(adapter.GetMethodRegistersCount(main), 0U);
    EXPECT_NE(adapter.GetMethodCode(main), nullptr);
    EXPECT_NE(adapter.GetMethodCodeSize(main), 0U);
    EXPECT_TRUE(adapter.IsMethodStatic(main));
    EXPECT_FALSE(adapter.HasNativeException(main));
    EXPECT_EQ(adapter.GetClassNameFromMethod(main), std::string("_GLOBAL"));
    EXPECT_EQ(adapter.GetMethodName(main), std::string("main"));
    EXPECT_EQ(adapter.GetMethodFullName(main, false), std::string("_GLOBAL::main"));
    EXPECT_EQ(adapter.GetBytecodeString(main, 0U), std::string("ldai 1"));

    EXPECT_EQ(adapter.GetClassName(global), std::string("_GLOBAL"));
    EXPECT_EQ(adapter.GetClass(main), global);
}

TEST(RuntimeAdapter, Klass)
{
    auto source = std::string(R"(
        .record R {
            i32 field
        }

        .function void R.ctor(R a0) <ctor> {
            return.void
        }

        .function void main() {}
        )");
    std::unique_ptr<const panda_file::File> arkf = ParseAndEmit(source);
    auto pointers = GetPointers(arkf.get());

    ASSERT_EQ(pointers.method.size(), 2U);
    ASSERT_EQ(pointers.klass.size(), 2U);
    ASSERT_EQ(pointers.field.size(), 1U);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf);
    auto klass = pointers.klass[0U];
    auto ctor = pointers.method[0U];
    auto main = pointers.method[1U];

    EXPECT_EQ(adapter.GetMethodName(main), std::string("main"));
    EXPECT_EQ(adapter.GetMethodName(ctor), std::string(".ctor"));

    auto classId = reinterpret_cast<uint64_t>(klass);
    EXPECT_FALSE(adapter.IsMethodExternal(main, ctor));
    EXPECT_FALSE(adapter.IsConstructor(main, SourceLanguage::PANDA_ASSEMBLY));
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(ctor, 0U), compiler::DataType::Type::REFERENCE);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(ctor, 1U), compiler::DataType::Type::ANY);
    EXPECT_EQ(adapter.IsArrayClass(ctor, classId), false);
}

TEST(RuntimeAdapter, Methods)
{
    auto source = std::string(R"(
        .record System <external>
        .function void System.exit(i32 a0) <external>

        .function u64 func_ret_u64(u64 a0) {
            return
        }

        .function i16 func_ret_i16(i16 a0) {
            return
        }

        .function u1 main(u32 a0, u16 a1, f32 a2, f64 a3) {
            movi v0, 0
            call System.exit, v0
            ldai 1
            return
        }
        )");
    std::unique_ptr<const panda_file::File> arkf = ParseAndEmit(source);
    auto pointers = GetPointers(arkf.get());

    ASSERT_EQ(pointers.method.size(), 3U);
    ASSERT_EQ(pointers.klass.size(), 1U);
    ASSERT_EQ(pointers.field.size(), 0U);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf);
    auto main = pointers.method[0U];
    auto funcRetI16 = pointers.method[1U];
    auto funcRetU64 = pointers.method[2U];

    EXPECT_EQ(adapter.GetMethodName(funcRetU64), std::string("func_ret_u64"));
    EXPECT_EQ(adapter.GetMethodName(funcRetI16), std::string("func_ret_i16"));
    EXPECT_EQ(adapter.GetMethodName(main), std::string("main"));

    EXPECT_EQ(adapter.GetMethodReturnType(funcRetI16), compiler::DataType::Type::INT16);
    EXPECT_EQ(adapter.GetMethodReturnType(funcRetU64), compiler::DataType::Type::UINT64);

    const auto methodId = adapter.ResolveMethodIndex(main, 0U);
    EXPECT_NE(methodId, 0U);
    EXPECT_NE(adapter.GetClassIdForMethod(main, methodId), 0U);
    EXPECT_EQ(adapter.GetMethodArgumentType(main, methodId, 0U), compiler::DataType::Type::INT32);

    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 0U), compiler::DataType::Type::UINT32);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 1U), compiler::DataType::Type::UINT16);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 2U), compiler::DataType::Type::FLOAT32);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 3U), compiler::DataType::Type::FLOAT64);
}

TEST(RuntimeAdapter, Fields)
{
    auto source = std::string(R"(
        .record R {
            i32 field
        }

        .record Record {
            i64 v_i64             <static>
        }

        .function void store_to_static(i64 a0) {
            lda.64 a0
            ststatic.64 Record.v_i64
            return.void
        }
        )");
    std::unique_ptr<const panda_file::File> arkf = ParseAndEmit(source);
    auto pointers = GetPointers(arkf.get());

    ASSERT_EQ(pointers.method.size(), 1U);
    ASSERT_EQ(pointers.klass.size(), 3U);
    ASSERT_EQ(pointers.field.size(), 2U);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf);
    auto storeToStatic = pointers.method[0U];
    auto recordWithStaticField = pointers.klass[1U];
    auto field = pointers.field[0U];

    EXPECT_EQ(adapter.GetMethodName(storeToStatic), std::string("store_to_static"));

    EXPECT_EQ(adapter.GetFieldName(field), std::string("field"));
    EXPECT_EQ(adapter.GetFieldType(field), compiler::DataType::Type::INT32);
    const auto fieldId = adapter.ResolveFieldIndex(storeToStatic, 0U);
    EXPECT_NE(fieldId, 0U);
    EXPECT_EQ(adapter.GetClassIdForField(storeToStatic, fieldId), reinterpret_cast<uint64_t>(recordWithStaticField));
    uint32_t classId = 0;
    const auto fieldPtr = adapter.ResolveField(storeToStatic, fieldId, true, false, &classId);
    EXPECT_EQ(classId, reinterpret_cast<uint64_t>(recordWithStaticField));
    EXPECT_EQ(adapter.GetClassForField(fieldPtr), recordWithStaticField);
    EXPECT_EQ(adapter.GetFieldTypeById(storeToStatic, fieldId), compiler::DataType::Type::INT64);
    EXPECT_EQ(adapter.IsFieldVolatile(fieldPtr), false);
}

}  // namespace ark::bytecodeopt::test
