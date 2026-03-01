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

#include "gtest/gtest.h"
#include <iostream>
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "assembly-parser.h"

namespace libabckit::test::adapter_static {
using namespace ark;
using namespace ark::panda_file;
using namespace ark::pandasm;
class RuntimeAdapterStaticTest : public testing::Test {
public:
    RuntimeAdapterStaticTest() {}
    ~RuntimeAdapterStaticTest() override {}

    NO_COPY_SEMANTIC(RuntimeAdapterStaticTest);
    NO_MOVE_SEMANTIC(RuntimeAdapterStaticTest);
};

static const uint8_t *GetTypeDescriptor(const std::string &name, std::string *storage)
{
    *storage = "L" + name + ";";
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

static void MainTestFunc(const std::string &source, compiler::RuntimeInterface::IntrinsicId expectId)
{
    Parser p;
    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;
    auto class_id = pf->GetClassId(GetTypeDescriptor("IO", &descriptor));
    EXPECT_TRUE(class_id.IsValid());
    EXPECT_FALSE(pf->IsExternal(class_id));
    libabckit::AbckitRuntimeAdapterStatic abckitRuntimeAdapterStatic(*pf);
    panda_file::ClassDataAccessor cda(*pf, class_id);
    cda.EnumerateMethods([&](MethodDataAccessor &mda) {
        auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());
        auto intrinsicId = abckitRuntimeAdapterStatic.GetIntrinsicId(methodPtr);
        EXPECT_EQ(intrinsicId, expectId);
    });
}
TEST_F(RuntimeAdapterStaticTest, RuntimeAdapterStaticTestNormal)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printI64(i64 a0) <native>
    )";
    MainTestFunc(source, compiler::RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_I64);
}

TEST_F(RuntimeAdapterStaticTest, RuntimeAdapterStaticTestInvalid)
{
    std::string source = R"(
        .record IO {}
        .function i32 IO.printU32(i32 a0) <native>
    )";
    MainTestFunc(source, compiler::RuntimeInterface::IntrinsicId::INVALID);
}
}  // namespace libabckit::test::adapter_static
