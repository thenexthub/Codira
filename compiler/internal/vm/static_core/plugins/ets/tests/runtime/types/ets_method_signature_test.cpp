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

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "plugins/ets/runtime/types/ets_method_signature.h"

namespace ark::ets::test {

namespace {
void MethodSignaturePrologue()
{
    EtsMethodSignature minimal(":V");
    EXPECT_EQ(minimal.GetProto(), Method::Proto(
                                      Method::Proto::ShortyVector {
                                          panda_file::Type {panda_file::Type::TypeId::VOID},
                                      },
                                      Method::Proto::RefTypeVector {}));
    EtsMethodSignature minimalint("I:V");
    EXPECT_EQ(minimalint.GetProto(), Method::Proto(
                                         Method::Proto::ShortyVector {
                                             panda_file::Type {panda_file::Type::TypeId::VOID},
                                             panda_file::Type {panda_file::Type::TypeId::I32},
                                         },
                                         Method::Proto::RefTypeVector {}));

    EtsMethodSignature simple("BCSIJFD:Z");
    EXPECT_EQ(simple.GetProto(), Method::Proto(
                                     Method::Proto::ShortyVector {
                                         panda_file::Type {panda_file::Type::TypeId::U1},
                                         panda_file::Type {panda_file::Type::TypeId::I8},
                                         panda_file::Type {panda_file::Type::TypeId::U16},
                                         panda_file::Type {panda_file::Type::TypeId::I16},
                                         panda_file::Type {panda_file::Type::TypeId::I32},
                                         panda_file::Type {panda_file::Type::TypeId::I64},
                                         panda_file::Type {panda_file::Type::TypeId::F32},
                                         panda_file::Type {panda_file::Type::TypeId::F64},
                                     },
                                     Method::Proto::RefTypeVector {}));
}

}  // namespace

// NOTE(a.urakov): move initialization to a common internal objects testing base class
class EtsMethodSignatureTest : public testing::Test {
public:
    EtsMethodSignatureTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        options.SetUseStringCaches(false);

        Runtime::Create(options);
    }

    ~EtsMethodSignatureTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsMethodSignatureTest);
    NO_MOVE_SEMANTIC(EtsMethodSignatureTest);
};

TEST_F(EtsMethodSignatureTest, MethodSignature)
{
    MethodSignaturePrologue();

    EtsMethodSignature arrays("SB[J[Lstd/core/String;I:LT;");
    EXPECT_EQ(arrays.GetProto(), Method::Proto(
                                     Method::Proto::ShortyVector {
                                         panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                         panda_file::Type {panda_file::Type::TypeId::I16},
                                         panda_file::Type {panda_file::Type::TypeId::I8},
                                         panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                         panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                         panda_file::Type {panda_file::Type::TypeId::I32},
                                     },
                                     Method::Proto::RefTypeVector {
                                         std::string_view {"LT;"},
                                         std::string_view {"[J"},
                                         std::string_view {"[Lstd/core/String;"},
                                     }));

    EtsMethodSignature multiarr("[[[LT;:[[I");
    EXPECT_EQ(multiarr.GetProto(), Method::Proto(
                                       Method::Proto::ShortyVector {
                                           panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                           panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                       },
                                       Method::Proto::RefTypeVector {
                                           std::string_view {"[[I"},
                                           std::string_view {"[[[LT;"},
                                       }));
    EtsMethodSignature big("J[BI[CI[IZ:I");
    EXPECT_EQ(big.GetProto(), Method::Proto(
                                  Method::Proto::ShortyVector {
                                      panda_file::Type {panda_file::Type::TypeId::I32},
                                      panda_file::Type {panda_file::Type::TypeId::I64},
                                      panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                      panda_file::Type {panda_file::Type::TypeId::I32},
                                      panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                      panda_file::Type {panda_file::Type::TypeId::I32},
                                      panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                      panda_file::Type {panda_file::Type::TypeId::U1},
                                  },
                                  Method::Proto::RefTypeVector {
                                      std::string_view {"[B"},
                                      std::string_view {"[C"},
                                      std::string_view {"[I"},
                                  }));
}

TEST_F(EtsMethodSignatureTest, InvalidMethodSignature)
{
    EtsMethodSignature invalid1("");
    EXPECT_FALSE(invalid1.IsValid());

    EtsMethodSignature invalid3(":L");
    EXPECT_FALSE(invalid3.IsValid());

    EtsMethodSignature invalid4("[]");
    EXPECT_FALSE(invalid4.IsValid());

    EtsMethodSignature invalid5("IL:J");
    EXPECT_FALSE(invalid5.IsValid());

    EtsMethodSignature invalid6("[:S");
    EXPECT_FALSE(invalid6.IsValid());

    EtsMethodSignature invalid7("V:Lg;");
    EXPECT_FALSE(invalid7.IsValid());

    EtsMethodSignature invalid8("L;:[");
    EXPECT_FALSE(invalid8.IsValid());

    EtsMethodSignature invalid9("::");
    EXPECT_FALSE(invalid9.IsValid());

    EtsMethodSignature invalid10("I:I:Z");
    EXPECT_FALSE(invalid10.IsValid());
}

}  // namespace ark::ets::test
