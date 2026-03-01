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

#include <cerrno>
#include <cstdarg>
#include <gtest/gtest.h>
#include <sstream>
#include <string_view>

#include "runtime/include/class_helper.h"

namespace ark::test {

std::string GetExpectedType(panda_file::Type::TypeId id)
{
    std::string expected;
    switch (id) {
        case panda_file::Type::TypeId::VOID:
            expected = "void";
            break;
        case panda_file::Type::TypeId::U1:
            expected = "bool";
            break;
        case panda_file::Type::TypeId::I8:
            expected = "i8";
            break;
        case panda_file::Type::TypeId::U8:
            expected = "u8";
            break;
        case panda_file::Type::TypeId::I16:
            expected = "i16";
            break;
        case panda_file::Type::TypeId::U16:
            expected = "u16";
            break;
        case panda_file::Type::TypeId::I32:
            expected = "i32";
            break;
        case panda_file::Type::TypeId::U32:
            expected = "u32";
            break;
        case panda_file::Type::TypeId::I64:
            expected = "i64";
            break;
        case panda_file::Type::TypeId::U64:
            expected = "u64";
            break;
        case panda_file::Type::TypeId::F32:
            expected = "f32";
            break;
        case panda_file::Type::TypeId::F64:
            expected = "f64";
            break;
        case panda_file::Type::TypeId::TAGGED:
            expected = "any";
            break;
        default:
            expected = "";
            break;
    }
    return expected;
}

TEST(ClassHelpers, GetPrimitiveTypeStr)
{
    int start = static_cast<int>(panda_file::Type::TypeId::VOID);
    int end = static_cast<int>(panda_file::Type::TypeId::TAGGED);
    for (int i = start; i <= end; ++i) {
        auto id = static_cast<panda_file::Type::TypeId>(i);
        std::string expected = GetExpectedType(id);
        const char *typeStr = ark::ClassHelper::GetPrimitiveTypeStr(id);
        ASSERT_EQ(std::string_view(typeStr), std::string_view(expected));
    }
}

}  // namespace ark::test
