/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "ani_gtest.h"
#include "libani_helpers/ani_signature_builder.h"

namespace ark::ets::ani_helpers::testing {

// NOLINTNEXTLINE
using namespace arkts::ani_signature;
class SignatureBuilderTest : public ani::testing::AniTest {
public:
    const Module module_ = Builder::BuildModule("ani_signature_builder_runtime_test");
    const Namespace namespace_ = Builder::BuildNamespace({module_.Name(), "TestNamespace"});
    const Type class_ = Builder::BuildClass({module_.Name(), "TestClass"});
};

TEST_F(SignatureBuilderTest, BuilderTest)
{
    ani_module mod {nullptr};
    ASSERT_EQ(env_->FindModule(module_.Descriptor().c_str(), &mod), ANI_OK);
    ani_namespace ns {nullptr};
    ASSERT_EQ(env_->FindNamespace(namespace_.Descriptor().c_str(), &ns), ANI_OK);
    ani_class cls {nullptr};
    ASSERT_EQ(env_->FindClass(class_.Descriptor().c_str(), &cls), ANI_OK);
}

TEST_F(SignatureBuilderTest, PrimitiveTypeSignatureTest)
{
    ani_class cls {nullptr};
    ASSERT_EQ(env_->FindClass(class_.Descriptor().c_str(), &cls), ANI_OK);
    ani_static_method sMethod {nullptr};
    std::string signature;

    SignatureBuilder sbBoolean {};
    sbBoolean.AddBoolean().AddBoolean().SetReturnBoolean();
    signature = sbBoolean.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);

    SignatureBuilder sbChar {};
    sbChar.AddChar().AddChar().SetReturnChar();
    signature = sbBoolean.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);

    SignatureBuilder sbByte {};
    sbByte.AddByte().AddByte().SetReturnByte();
    signature = sbByte.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);

    SignatureBuilder sbShort {};
    sbShort.AddShort().AddShort().SetReturnShort();
    signature = sbShort.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);

    SignatureBuilder sbInt {};
    sbInt.AddInt().AddInt().SetReturnInt();
    signature = sbInt.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);

    SignatureBuilder sbLong {};
    sbLong.AddLong().AddLong().SetReturnLong();
    signature = sbLong.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);

    SignatureBuilder sbFloat {};
    sbFloat.AddFloat().AddFloat().SetReturnFloat();
    signature = sbFloat.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);

    SignatureBuilder sbDouble {};
    sbDouble.AddDouble().AddDouble().SetReturnDouble();
    signature = sbDouble.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "func", signature.c_str(), &sMethod), ANI_OK);
}

TEST_F(SignatureBuilderTest, ReferenceTypeSignatureTest)
{
    const Module stdCore = Builder::BuildModule("std.core");

    ani_namespace ns {nullptr};
    ASSERT_EQ(env_->FindNamespace(namespace_.Descriptor().c_str(), &ns), ANI_OK);
    ani_function nsFunction {nullptr};
    std::string signature;

    SignatureBuilder sbString {};
    sbString.AddClass({stdCore.Name(), "String"}).SetReturnInt();
    signature = sbString.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "func", signature.c_str(), &nsFunction), ANI_OK);

    SignatureBuilder sbEnum {};
    sbEnum.AddEnum({module_.Name(), "COLOR"}).SetReturnInt();
    signature = sbEnum.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "func", signature.c_str(), &nsFunction), ANI_OK);

    SignatureBuilder sbFnType {};
    sbFnType.AddFunctionalObject(2U, false).SetReturnInt();
    signature = sbFnType.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "func", signature.c_str(), &nsFunction), ANI_OK);

    SignatureBuilder sbFnTypeRestArgs {};
    sbFnTypeRestArgs.AddFunctionalObject(2U, true).SetReturnInt();
    signature = sbFnTypeRestArgs.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "func", signature.c_str(), &nsFunction), ANI_OK);

    SignatureBuilder sbUndefined {};
    sbUndefined.AddUndefined().SetReturnInt();
    signature = sbUndefined.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funcUndefined", signature.c_str(), &nsFunction), ANI_OK);

    SignatureBuilder sbNull {};
    sbNull.AddNull().SetReturnInt();
    signature = sbNull.BuildSignatureDescriptor();
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funcNull", signature.c_str(), &nsFunction), ANI_OK);
}
}  // namespace ark::ets::ani_helpers::testing