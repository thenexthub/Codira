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

#include <string_view>
#include "ani.h"
#include "ani_gtest.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets::ani::testing {

class MangleSignatureToProtoTest : public AniTest {
public:
    void CheckSignatureParsing(const std::string_view signature, Method::Proto &&expectedProto)
    {
        std::optional<EtsMethodSignature> methodSignature;
        Mangle::ConvertSignatureToProto(methodSignature, signature);
        ASSERT_TRUE(methodSignature.has_value()) << "Signature \"" << signature << "\" should be valid";
        EXPECT_EQ(methodSignature.value().GetProto(), expectedProto)
            << "Signature \"" << signature << "\" was parsed incorrectly";
    }

    void CheckInvalidSignatureParsing(const std::string_view signature)
    {
        std::optional<EtsMethodSignature> methodSignature;
        Mangle::ConvertSignatureToProto(methodSignature, signature);
        EXPECT_FALSE(methodSignature.has_value()) << "Signature \"" << signature << "\" should be invalid";
    }
};

TEST_F(MangleSignatureToProtoTest, PrimitiveSignature)
{
    CheckSignatureParsing(":", Method::Proto(
                                   Method::Proto::ShortyVector {
                                       panda_file::Type {panda_file::Type::TypeId::VOID},
                                   },
                                   Method::Proto::RefTypeVector {}));

    CheckSignatureParsing("i:", Method::Proto(
                                    Method::Proto::ShortyVector {
                                        panda_file::Type {panda_file::Type::TypeId::VOID},
                                        panda_file::Type {panda_file::Type::TypeId::I32},
                                    },
                                    Method::Proto::RefTypeVector {}));

    CheckSignatureParsing("bcsilfd:z", Method::Proto(
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

TEST_F(MangleSignatureToProtoTest, ObjectsSignature)
{
    CheckSignatureParsing("C{std.core.String}i:C{T}", Method::Proto(
                                                          Method::Proto::ShortyVector {
                                                              panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                              panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                              panda_file::Type {panda_file::Type::TypeId::I32},
                                                          },
                                                          Method::Proto::RefTypeVector {
                                                              std::string_view {"LT;"},
                                                              std::string_view {"Lstd/core/String;"},
                                                          }));

    CheckSignatureParsing("sbA{l}A{C{std.core.String}}i:C{T}",
                          Method::Proto(
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
}

TEST_F(MangleSignatureToProtoTest, UnionsSignature)
{
    CheckSignatureParsing("X{C{std.core.String}dC{Fun}}:",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::VOID},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {std::string_view {"{ULFun;DLstd/core/String;}"}}));

    CheckSignatureParsing(":X{C{Sun}zC{Fun}}", Method::Proto(
                                                   Method::Proto::ShortyVector {
                                                       panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                   },
                                                   Method::Proto::RefTypeVector {std::string_view {"{ULFun;LSun;Z}"}}));
}

TEST_F(MangleSignatureToProtoTest, ArraysSignature)
{
    CheckSignatureParsing("A{A{A{C{T}}}}:A{A{i}}", Method::Proto(
                                                       Method::Proto::ShortyVector {
                                                           panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                           panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                       },
                                                       Method::Proto::RefTypeVector {
                                                           std::string_view {"[[I"},
                                                           std::string_view {"[[[LT;"},
                                                       }));

    CheckSignatureParsing("lA{b}iA{c}iA{i}z:i", Method::Proto(
                                                    // CC-OFFNXT(G.FMT.06-CPP) project code style
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

TEST_F(MangleSignatureToProtoTest, InvalidSignature)
{
    CheckInvalidSignatureParsing("");
    CheckInvalidSignatureParsing("sbz");
    CheckInvalidSignatureParsing("{}");

    CheckInvalidSignatureParsing("i:{l}");
    CheckInvalidSignatureParsing("{i:s");
    CheckInvalidSignatureParsing(":C{T};");

    CheckInvalidSignatureParsing(":V");
    CheckInvalidSignatureParsing("::");
    CheckInvalidSignatureParsing("i:i:z");
}

}  // namespace ark::ets::ani::testing
