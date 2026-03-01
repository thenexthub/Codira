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

TEST(BuilderTest, PrimitiveTypes)
{
    ASSERT_EQ(Builder::BuildUndefined().Descriptor(), "std.core.Object");
    ASSERT_EQ(Builder::BuildNull().Descriptor(), "std.core.Null");
    ASSERT_EQ(Builder::BuildAny().Descriptor(), "Y");
    ASSERT_EQ(Builder::BuildBoolean().Descriptor(), "z");
    ASSERT_EQ(Builder::BuildChar().Descriptor(), "c");
    ASSERT_EQ(Builder::BuildByte().Descriptor(), "b");
    ASSERT_EQ(Builder::BuildShort().Descriptor(), "s");
    ASSERT_EQ(Builder::BuildInt().Descriptor(), "i");
    ASSERT_EQ(Builder::BuildLong().Descriptor(), "l");
    ASSERT_EQ(Builder::BuildFloat().Descriptor(), "f");
    ASSERT_EQ(Builder::BuildDouble().Descriptor(), "d");
}

TEST(BuilderTest, StdlibClass)
{
    SignatureBuilder sb {};
    sb.AddClass({"std.core", "String"});
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "C{std.core.String}:");
}

TEST(BuilderTest, ArrayTest)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildArray(Builder::BuildInt()));
    sb.SetReturn(Builder::BuildArray(Builder::BuildInt()));
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "A{i}:A{i}");

    SignatureBuilder sb2 {};
    sb2.Add(Builder::BuildArray(Builder::BuildArray(Builder::BuildClass("com.example.MyClass"))));
    sb2.SetReturn(Builder::BuildArray(Builder::BuildInt()));
    ASSERT_EQ(sb2.BuildSignatureDescriptor(), "A{A{C{com.example.MyClass}}}:A{i}");
}

TEST(BuilderTest, ModuleAndNamespace)
{
    Module mod = Builder::BuildModule("com.example.Module");
    ASSERT_EQ(mod.Name(), "com.example.Module");
    ASSERT_EQ(mod.Descriptor(), "com.example.Module");

    Namespace ns1 = Builder::BuildNamespace("com.example.Namespace");
    ASSERT_EQ(ns1.Name(), "com.example.Namespace");
    ASSERT_EQ(ns1.Descriptor(), "com.example.Namespace");

    Namespace ns2 = Builder::BuildNamespace({"com", "example", "SubNamespace"});
    ASSERT_EQ(ns2.Name(), "com.example.SubNamespace");
    ASSERT_EQ(ns2.Descriptor(), "com.example.SubNamespace");
}

TEST(BuilderTest, ClassEnumPartialRequired)
{
    Type cls = Builder::BuildClass("com.example.MyClass");
    ASSERT_EQ(cls.Descriptor(), "com.example.MyClass");

    Type enumType = Builder::BuildEnum({"com", "example", "MyEnum"});
    ASSERT_EQ(enumType.Descriptor(), "com.example.MyEnum");

    Type partialType = Builder::BuildPartial("com.example.MyClass");
    ASSERT_EQ(partialType.Descriptor(), "com.example.MyClass");

    Type requiredType = Builder::BuildRequired({"com", "example", "RequiredClass"});
    ASSERT_EQ(requiredType.Descriptor(), "com.example.RequiredClass");
}

TEST(BuilderTest, FunctionalObject)
{
    Type funcType = Builder::BuildFunctionalObject(2, false);
    ASSERT_EQ(funcType.Descriptor(), "std.core.Function2");

    Type funcTypeReset = Builder::BuildFunctionalObject(3, true);
    ASSERT_EQ(funcTypeReset.Descriptor(), "std.core.FunctionR3");
}

TEST(BuilderTest, SignatureDescriptorDirect)
{
    std::string sig1 = Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildFloat()});
    ASSERT_EQ(sig1, "if:");
    std::string sig2 =
        Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildFloat()}, Builder::BuildBoolean());
    ASSERT_EQ(sig2, "if:z");
}

TEST(SignatureBuilderTest, BasicSignatureBuilder)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt()).Add(Builder::BuildFloat()).AddClass("com.example.MyClass");
    sb.SetReturnDouble();
    std::string sig = sb.BuildSignatureDescriptor();
    ASSERT_EQ(sig, "ifC{com.example.MyClass}:d");
}

TEST(BuilderTest, SpecialNames)
{
    ASSERT_EQ(Builder::BuildConstructorName(), "<ctor>");
    ASSERT_EQ(Builder::BuildSetterName("value"), "%%set-value");
    ASSERT_EQ(Builder::BuildGetterName("value"), "%%get-value");
    ASSERT_EQ(Builder::BuildPropertyName("value"), "%%property-value");
    ASSERT_EQ(Builder::BuildPartialName("value"), "%%partial-value");
    ASSERT_EQ(Builder::BuildAsyncName("value"), "%%async-value");
    ASSERT_EQ(Builder::GetSetterNamePrefix(), "%%set-");
    ASSERT_EQ(Builder::GetGetterNamePrefix(), "%%get-");
    ASSERT_EQ(Builder::GetPropertyNamePrefix(), "%%property-");
    ASSERT_EQ(Builder::GetPartialNamePrefix(), "%%partial-");
    ASSERT_EQ(Builder::GetAsyncNamePrefix(), "%%async-");
    ASSERT_EQ(Builder::GetUnionPropertyNamePrefix(), "%%union_prop-");
    ASSERT_EQ(Builder::GetLambdaPrefix(), "%%lambda-");
    ASSERT_EQ(Builder::GetLambdaInvokePrefix(), "lambda_invoke-");
}

TEST(SignatureBuilderTest, ComplexSignatureBuilder)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt())
        .Add(Builder::BuildFloat())
        .Add(Builder::BuildBoolean())
        .AddClass({"com", "example", "ComplexClass"})
        .AddEnum("com.example.MyEnum")
        .AddPartial("com.example.PartialClass")
        .AddRequired({"com", "example", "RequiredClass"})
        .AddFunctionalObject(4, true);
    sb.SetReturnClass("com.example.ReturnClass");

    std::string sig = sb.BuildSignatureDescriptor();
    std::string expectedSig =
        "ifzC{com.example.ComplexClass}"
        "E{com.example.MyEnum}P{com.example.PartialClass}"
        "C{com.example.RequiredClass}"
        "C{std.core.FunctionR4}"
        ":C{com.example.ReturnClass}";
    ASSERT_EQ(sig, expectedSig);
}

TEST(SignatureBuilderTest, MultipleBuilders)
{
    SignatureBuilder sb1;
    SignatureBuilder sb2;
    sb1.Add(Builder::BuildInt()).Add(Builder::BuildFloat()).SetReturnBoolean();
    sb2.Add(Builder::BuildLong()).Add(Builder::BuildDouble()).SetReturnChar();

    ASSERT_EQ(sb1.BuildSignatureDescriptor(), "if:z");
    ASSERT_EQ(sb2.BuildSignatureDescriptor(), "ld:c");
}

TEST(BuilderTest, SignatureDescriptorVoidReturn)
{
    std::string sig = Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildClass("com.example.Demo")});
    ASSERT_EQ(sig, "iC{com.example.Demo}:");
}

TEST(SignatureBuilderTest, OverrideReturnType)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt()).Add(Builder::BuildBoolean());
    sb.SetReturnInt();
    sb.SetReturnChar();
    std::string sig = sb.BuildSignatureDescriptor();
    ASSERT_EQ(sig, "iz:c");
}

TEST(SignatureBuilderTest, DefaultReturnTypeVoid)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt());
    std::string sig = sb.BuildSignatureDescriptor();
    ASSERT_EQ(sig, "i:");
}

TEST(SignatureBuilderExtraTest, UndefinedAndNullReturn)
{
    SignatureBuilder sb1;
    sb1.Add(Builder::BuildInt()).Add(Builder::BuildFloat());
    sb1.SetReturnUndefined();
    ASSERT_EQ(sb1.BuildSignatureDescriptor(), "if:C{std.core.Object}");

    SignatureBuilder sb2;
    sb2.Add(Builder::BuildInt()).Add(Builder::BuildFloat());
    sb2.SetReturnNull();
    ASSERT_EQ(sb2.BuildSignatureDescriptor(), "if:C{std.core.Null}");
}

TEST(SignatureBuilderExtraTest, AddTypeDirectly)
{
    SignatureBuilder sb {};
    Type intType = Builder::BuildInt();
    Type floatType = Builder::BuildFloat();
    sb.Add(intType).Add(floatType);
    sb.SetReturnBoolean();
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "if:z");
}

TEST(SignatureBuilderExtraTest, ClassAndEnumMixed)
{
    SignatureBuilder sb {};
    sb.AddClass("com.example.ClassA")
        .AddEnum({"com", "example", "EnumB"})
        .AddPartial("com.example.PartialC")
        .AddRequired({"com", "example", "RequiredD"});
    sb.SetReturnClass("com.example.ReturnE");
    std::string expectedSig =
        "C{com.example.ClassA}"
        "E{com.example.EnumB}"
        "P{com.example.PartialC}"
        "C{com.example.RequiredD}"
        ":C{com.example.ReturnE}";
    ASSERT_EQ(sb.BuildSignatureDescriptor(), expectedSig);
}

TEST(SignatureBuilderExtraTest, SetReturnFunctionalObject)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt()).Add(Builder::BuildFloat());
    sb.SetReturnFunctionalObject(3, true);
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "if:C{std.core.FunctionR3}");
}

TEST(SignatureBuilderExtraTest, MultipleSetReturnCalls)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildBoolean()).Add(Builder::BuildByte());
    sb.SetReturnInt();
    sb.SetReturnLong();
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "zb:l");
}

TEST(SignatureBuilderExtraTest, EmptySignature)
{
    SignatureBuilder sb {};
    ASSERT_EQ(sb.BuildSignatureDescriptor(), ":");
}

TEST(SignatureBuilderExtraTest, SingleArgumentSignatureDescriptor)
{
    std::string sig = Builder::BuildSignatureDescriptor({Builder::BuildInt()}, Builder::BuildLong());
    ASSERT_EQ(sig, "i:l");
}

TEST(SignatureBuilderExtraTest, EmptyArgsSignatureDescriptor)
{
    std::string sig = Builder::BuildSignatureDescriptor({}, Builder::BuildNull());
    ASSERT_EQ(sig, ":C{std.core.Null}");
}

TEST(SignatureBuilderExtraTest, OverloadConsistency)
{
    ASSERT_EQ(Builder::BuildNamespace("com.example1.example2.example3.Test").Descriptor(),
              Builder::BuildNamespace({"com", "example1", "example2", "example3", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("com.example1.example2.Test").Descriptor(),
              Builder::BuildNamespace({"com", "example1", "example2", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("com.example.Test").Descriptor(),
              Builder::BuildNamespace({"com", "example", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("com.Test").Descriptor(), Builder::BuildNamespace({"com", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("Test").Descriptor(), Builder::BuildNamespace({"Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("").Descriptor(), Builder::BuildNamespace({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency2)
{
    ASSERT_EQ(Builder::BuildClass("com.example1.example2.example3.TestClass").Descriptor(),
              Builder::BuildClass({"com", "example1", "example2", "example3", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("com.example1.example2.TestClass").Descriptor(),
              Builder::BuildClass({"com", "example1", "example2", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("com.example.TestClass").Descriptor(),
              Builder::BuildClass({"com", "example", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("com.TestClass").Descriptor(),
              Builder::BuildClass({"com", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("TestClass").Descriptor(), Builder::BuildClass({"TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("").Descriptor(), Builder::BuildClass({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency3)
{
    ASSERT_EQ(Builder::BuildEnum("com.example1.example2.example3.TestEnum").Descriptor(),
              Builder::BuildEnum({"com", "example1", "example2", "example3", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("com.example1.example2.TestEnum").Descriptor(),
              Builder::BuildEnum({"com", "example1", "example2", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("com.example.TestEnum").Descriptor(),
              Builder::BuildEnum({"com", "example", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("com.TestEnum").Descriptor(), Builder::BuildEnum({"com", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("TestEnum").Descriptor(), Builder::BuildEnum({"TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("").Descriptor(), Builder::BuildEnum({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency4)
{
    ASSERT_EQ(Builder::BuildPartial("com.example1.example2.example3.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "example1", "example2", "example3", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("com.example1.example2.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "example1", "example2", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("com.example.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "example", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("com.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("TestPartial").Descriptor(), Builder::BuildPartial({"TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("").Descriptor(), Builder::BuildPartial({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency5)
{
    ASSERT_EQ(Builder::BuildRequired("com.example1.example2.example3.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "example1", "example2", "example3", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("com.example1.example2.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "example1", "example2", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("com.example.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "example", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("com.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("TestRequired").Descriptor(),
              Builder::BuildRequired({"TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("").Descriptor(), Builder::BuildRequired({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, AllPrimitiveTypes)
{
    SignatureBuilder sb {};
    sb.AddUndefined().AddNull().AddBoolean().AddChar().AddByte().AddShort().AddInt().AddLong().AddFloat().AddDouble();
    sb.SetReturnDouble();
    std::string expectedSig =
        "C{std.core.Object}"
        "C{std.core.Null}"
        "z"
        "c"
        "b"
        "s"
        "i"
        "l"
        "f"
        "d"
        ":d";
    ASSERT_EQ(sb.BuildSignatureDescriptor(), expectedSig);
}

}  // namespace ark::ets::ani_helpers::testing
