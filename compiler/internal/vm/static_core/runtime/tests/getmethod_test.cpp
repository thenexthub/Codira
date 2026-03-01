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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <sstream>

#include "assembly-parser.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/bytecode_emitter.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/value.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"

namespace ark::test {

class GetMethodTest : public testing::Test {
public:
    GetMethodTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~GetMethodTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(GetMethodTest);
    NO_MOVE_SEMANTIC(GetMethodTest);

private:
    ark::MTManagedThread *thread_ = nullptr;
};

TEST_F(GetMethodTest, GetMethod)
{
    pandasm::Parser p;
    PandaStringStream ss;
    ss << ".record R1 {}" << std::endl;

    // define some methods unsorted
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    std::string methodsName[] = {"ab", "hello", "f1", "say", "world", "k0", "a"};
    size_t methodsNum = sizeof(methodsName) / sizeof(methodsName[0]);
    for (size_t i = 0; i < methodsNum; i++) {
        ss << ".function void R1." << methodsName[i] << "() {" << std::endl;
        ss << "    return.void" << std::endl;
        ss << "}" << std::endl;
    }

    auto source = ss.str();
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    auto res = p.Parse(source.c_str());
    ASSERT_TRUE(res) << res.Error().message;

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr) << pandasm::AsmEmitter::GetLastError();

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;
    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R1"), &descriptor));
    ASSERT_NE(klass, nullptr);

    // check if methods sorted by id and name
    auto methods = klass->GetMethods();
    ASSERT_EQ(methods.size(), methodsNum);
    for (size_t i = 0; i < methodsNum; i++) {
        for (size_t j = i + 1; j < methodsNum; j++) {
            ASSERT_TRUE(methods[i].GetFileId().GetOffset() < methods[j].GetFileId().GetOffset());
            ASSERT_TRUE(methods[i].GetName() < methods[j].GetName());
        }
    }

    // get each method by id and name
    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID)},
                        Method::Proto::RefTypeVector {});
    for (size_t i = 0; i < methodsNum; i++) {
        Method *method = klass->GetClassMethod(utf::CStringAsMutf8(methodsName[i].c_str()), proto);
        ASSERT_NE(method, nullptr);
        ASSERT_EQ(method, klass->GetStaticClassMethod(method->GetFileId()));
    }
}

}  // namespace ark::test
