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
#include "utils/utf.h"

namespace panda::bytecodeopt::test {

std::unique_ptr<const panda_file::File> ParseAndEmit(const std::string &source)
{
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    if (parser.ShowError().err != pandasm::Error::ErrorType::ERR_NONE) {
        std::cerr << "Parse failed: " << parser.ShowError().message << std::endl
                  << parser.ShowError().whole_line << std::endl;
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
        panda_file::File::EntityId record_id {id};

        if (arkf->IsExternal(record_id)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*arkf, record_id};
        auto class_id = cda.GetClassId().GetOffset();
        auto class_ptr = reinterpret_cast<compiler::RuntimeInterface::ClassPtr>(class_id);
        pointers.klass.push_back(class_ptr);

        cda.EnumerateFields([&pointers](panda_file::FieldDataAccessor &fda) {
            auto field_ptr = reinterpret_cast<compiler::RuntimeInterface::FieldPtr>(fda.GetFieldId().GetOffset());
            pointers.field.push_back(field_ptr);
        });

        cda.EnumerateMethods([&pointers](panda_file::MethodDataAccessor &mda) {
            auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());
            pointers.method.push_back(method_ptr);
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

    ASSERT_EQ(pointers.method.size(), 1);
    ASSERT_EQ(pointers.klass.size(), 1);
    ASSERT_EQ(pointers.field.size(), 0);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf.get());
    auto main = pointers.method[0];
    auto global = pointers.klass[0];

    EXPECT_FALSE(adapter.IsMethodIntrinsic(main));
    EXPECT_NE(adapter.GetBinaryFileForMethod(main), nullptr);
    EXPECT_EQ(adapter.GetMethodById(main, 0), nullptr);
    EXPECT_NE(adapter.GetMethodId(main), 0);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 0), compiler::DataType::Type::ANY);
    EXPECT_EQ(adapter.GetMethodReturnType(pointers.method[0]), compiler::DataType::Type::BOOL);
    EXPECT_EQ(adapter.GetMethodRegistersCount(main), 0);
    EXPECT_NE(adapter.GetMethodCode(main), nullptr);
    EXPECT_NE(adapter.GetMethodCodeSize(main), 0);
    EXPECT_TRUE(adapter.IsMethodStatic(main));
    EXPECT_FALSE(adapter.HasNativeException(main));
    EXPECT_EQ(adapter.GetClassNameFromMethod(main), std::string("L_GLOBAL;"));
    EXPECT_EQ(adapter.GetMethodName(main), std::string("main"));
    EXPECT_EQ(adapter.GetMethodFullName(main, false), std::string("L_GLOBAL;::main"));
    EXPECT_EQ(adapter.GetBytecodeString(main, 0), std::string("ldai 1"));

    EXPECT_EQ(adapter.GetClassName(global), std::string("L_GLOBAL;"));
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

    ASSERT_EQ(pointers.method.size(), 2);
    ASSERT_EQ(pointers.klass.size(), 2);
    ASSERT_EQ(pointers.field.size(), 1);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf.get());
    auto klass = pointers.klass[0];
    auto ctor = pointers.method[0];
    auto main = pointers.method[1];

    EXPECT_EQ(adapter.GetMethodName(main), std::string("main"));
    EXPECT_EQ(adapter.GetMethodName(ctor), std::string(".ctor"));

    auto class_id = reinterpret_cast<uint64_t>(klass);
    EXPECT_FALSE(adapter.IsMethodExternal(main, ctor));
    EXPECT_FALSE(adapter.IsConstructor(main, class_id));
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(ctor, 0), compiler::DataType::Type::REFERENCE);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(ctor, 1), compiler::DataType::Type::ANY);
    EXPECT_EQ(adapter.IsArrayClass(ctor, class_id), false);
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

    ASSERT_EQ(pointers.method.size(), 3);
    ASSERT_EQ(pointers.klass.size(), 1);
    ASSERT_EQ(pointers.field.size(), 0);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf.get());
    auto main = pointers.method[0];
    auto func_ret_i16 = pointers.method[1];
    auto func_ret_u64 = pointers.method[2];

    EXPECT_EQ(adapter.GetMethodName(func_ret_u64), std::string("func_ret_u64"));
    EXPECT_EQ(adapter.GetMethodName(func_ret_i16), std::string("func_ret_i16"));
    EXPECT_EQ(adapter.GetMethodName(main), std::string("main"));

    EXPECT_EQ(adapter.GetMethodReturnType(func_ret_i16), compiler::DataType::Type::INT16);
    EXPECT_EQ(adapter.GetMethodReturnType(func_ret_u64), compiler::DataType::Type::UINT64);

    const auto method_id = adapter.ResolveMethodIndex(main, 0);
    EXPECT_NE(method_id, 0);
    EXPECT_NE(adapter.GetClassIdForMethod(main, method_id), 0);
    EXPECT_EQ(adapter.GetMethodArgumentType(main, method_id, 0), compiler::DataType::Type::INT32);

    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 0), compiler::DataType::Type::UINT32);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 1), compiler::DataType::Type::UINT16);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 2), compiler::DataType::Type::FLOAT32);
    EXPECT_EQ(adapter.GetMethodTotalArgumentType(main, 3), compiler::DataType::Type::FLOAT64);
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

        .function void store_to_static(i64 a0){
            lda.64 a0
            ststatic.64 Record.v_i64
            return.void
        }
        )");
    std::unique_ptr<const panda_file::File> arkf = ParseAndEmit(source);
    auto pointers = GetPointers(arkf.get());

    ASSERT_EQ(pointers.method.size(), 1);
    ASSERT_EQ(pointers.klass.size(), 3);
    ASSERT_EQ(pointers.field.size(), 2);

    BytecodeOptimizerRuntimeAdapter adapter(*arkf.get());
    auto store_to_static = pointers.method[0];
    auto record_with_static_field = pointers.klass[1];
    auto field = pointers.field[0];

    EXPECT_EQ(adapter.GetMethodName(store_to_static), std::string("store_to_static"));

    EXPECT_EQ(adapter.GetFieldName(field), std::string("field"));
    EXPECT_EQ(adapter.GetFieldType(field), compiler::DataType::Type::INT32);
    const auto field_id = adapter.ResolveFieldIndex(store_to_static, 0);
    EXPECT_NE(field_id, 0);
    EXPECT_EQ(adapter.GetClassIdForField(store_to_static, field_id),
              reinterpret_cast<uint64_t>(record_with_static_field));
    uint32_t immut_var = 0;
    const auto field_ptr = adapter.ResolveField(store_to_static, field_id, false, &immut_var);
    EXPECT_EQ(immut_var, 0);
    EXPECT_EQ(adapter.GetClassForField(field_ptr), record_with_static_field);
    EXPECT_EQ(adapter.GetFieldTypeById(store_to_static, field_id), compiler::DataType::Type::INT64);
    EXPECT_EQ(adapter.IsFieldVolatile(field_ptr), false);
}

}  // namespace panda::bytecodeopt::test
