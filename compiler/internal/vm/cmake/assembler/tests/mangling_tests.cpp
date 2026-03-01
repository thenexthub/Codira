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
#include "mangling.h"
#include "assembly-function.h"
#include "extensions/extensions.h"

using namespace panda::pandasm;

TEST(mangling_tests, MangleFunctionName)
{
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"type1", 0}, language);
    params.emplace_back(Type {"type2", 0}, language);
    params.emplace_back(Type {"type3", 0}, language);

    auto return_type = Type("type4", 0);

    std::string name = "Asm.main";
    ASSERT_EQ(MangleFunctionName(name, std::move(params), return_type), "Asm.main:type1;type2;type3;type4;");
}

TEST(mangling_tests, DeMangleFunctionName)
{
    std::string name = "Asm.main:type1;type2;type3;type4;";
    ASSERT_EQ(DeMangleName(name), "Asm.main");
}

TEST(mangling_tests, GetFunctionSignatureFromName)
{
    std::vector<Function::Parameter> params;
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    params.emplace_back(Type {"type1", 0}, language);
    params.emplace_back(Type {"type2", 0}, language);
    params.emplace_back(Type {"type3", 0}, language);

    std::string name = "Asm.main";
    ASSERT_EQ(GetFunctionSignatureFromName(name, std::move(params)), "Asm.main:(type1,type2,type3)");
}

TEST(mangling_tests, GetFunctionNameFromSignature)
{
    std::string name = "Asm.main:(type1,type2,type3,type4)";
    ASSERT_EQ(GetFunctionNameFromSignature(name), "Asm.main");
}