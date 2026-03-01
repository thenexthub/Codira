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

#include "ani_signature_builder.h"
#include <cstddef>
#include <vector>
#include <algorithm>
#include <sstream>

namespace {

std::string JoinParts(std::initializer_list<std::string_view> parts)
{
    std::ostringstream oss;
    bool first = true;
    for (auto part : parts) {
        if (!first) {
            oss << '.';
        }
        oss << part;
        first = false;
    }
    return oss.str();
}

}  // namespace

namespace arkts::ani_signature {

class Type::Impl {
public:
    std::string descriptor;
    std::string aniDescriptor;
};

Type::Type(std::unique_ptr<Type::Impl> impl) : impl_(std::move(impl)) {}

Type::~Type() = default;

// CC-OFFNXT(G.FUN.02-CPP) different coedcheck for .h and .cpp
Type::Type(const Type &other) : impl_(other.impl_ ? std::make_unique<Type::Impl>(*other.impl_) : nullptr) {}

// CC-OFFNXT(G.FUN.02-CPP) different coedcheck for .h and .cpp
Type &Type::operator=(const Type &other)
{
    if (this != &other) {
        impl_ = other.impl_ ? std::make_unique<Type::Impl>(*other.impl_) : nullptr;
    }
    return *this;
}

Type::Type(Type &&other) : impl_(std::move(other.impl_)) {}

Type &Type::operator=(Type &&other)
{
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

std::string Type::Descriptor() const
{
    return (impl_ != nullptr) ? impl_->descriptor : "";
}

std::string Type::AniDescriptor() const
{
    return (impl_ != nullptr) ? impl_->aniDescriptor : "";
}

static Type CreateType(const std::string &desc, const std::string &prefix = "")
{
    auto tImpl = std::make_unique<Type::Impl>();
    tImpl->descriptor = desc;
    tImpl->aniDescriptor = prefix.empty() ? desc : prefix + "{" + desc + "}";
    Type t(std::move(tImpl));
    return t;
}

static Type CreateArrayType(const std::string &anidesc)
{
    auto tImpl = std::make_unique<Type::Impl>();
    tImpl->aniDescriptor = "A{" + anidesc + "}";
    tImpl->descriptor = tImpl->aniDescriptor;
    Type t(std::move(tImpl));
    return t;
}

class Module::Impl {
public:
    std::string name;
    std::string descriptor;
};

Module::Module(std::unique_ptr<Module::Impl> impl) : impl_(std::move(impl)) {}

Module::~Module() = default;

// CC-OFFNXT(G.FUN.02-CPP) different coedcheck for .h and .cpp
Module::Module(const Module &other) : impl_(other.impl_ ? std::make_unique<Module::Impl>(*other.impl_) : nullptr) {}

// CC-OFFNXT(G.FUN.02-CPP) different coedcheck for .h and .cpp
Module &Module::operator=(const Module &other)
{
    if (this != &other) {
        impl_ = other.impl_ ? std::make_unique<Module::Impl>(*other.impl_) : nullptr;
    }
    return *this;
}

Module::Module(Module &&other) : impl_(std::move(other.impl_)) {}

Module &Module::operator=(Module &&other)
{
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

std::string Module::Descriptor() const
{
    return (impl_ != nullptr) ? impl_->descriptor : "";
}

std::string Module::Name() const
{
    return (impl_ != nullptr) ? impl_->name : "";
}

class Namespace::Impl {
public:
    std::string name;
    std::string descriptor;
};

Namespace::Namespace(std::unique_ptr<Namespace::Impl> impl) : impl_(std::move(impl)) {}

Namespace::~Namespace() = default;

// CC-OFFNXT(G.FUN.02-CPP) different coedcheck for .h and .cpp
Namespace::Namespace(const Namespace &other)
    : impl_(other.impl_ ? std::make_unique<Namespace::Impl>(*other.impl_) : nullptr)
{
}
// CC-OFFNXT(G.FUN.02-CPP) different coedcheck for .h and .cpp
Namespace &Namespace::operator=(const Namespace &other)
{
    if (this != &other) {
        impl_ = other.impl_ ? std::make_unique<Namespace::Impl>(*other.impl_) : nullptr;
    }
    return *this;
}

Namespace::Namespace(Namespace &&other) : impl_(std::move(other.impl_)) {}

Namespace &Namespace::operator=(Namespace &&other)
{
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

std::string Namespace::Descriptor() const
{
    return (impl_ != nullptr) ? impl_->descriptor : "";
}

std::string Namespace::Name() const
{
    return (impl_ != nullptr) ? impl_->name : "";
}

Type Builder::BuildUndefined()
{
    return CreateType("std.core.Object", "C");
}

Type Builder::BuildNull()
{
    return CreateType("std.core.Null", "C");
}

Type Builder::BuildAny()
{
    return CreateType("Y");
}

Type Builder::BuildBoolean()
{
    return CreateType("z");
}

Type Builder::BuildChar()
{
    return CreateType("c");
}

Type Builder::BuildByte()
{
    return CreateType("b");
}

Type Builder::BuildShort()
{
    return CreateType("s");
}

Type Builder::BuildInt()
{
    return CreateType("i");
}

Type Builder::BuildLong()
{
    return CreateType("l");
}

Type Builder::BuildFloat()
{
    return CreateType("f");
}

Type Builder::BuildDouble()
{
    return CreateType("d");
}

Module Builder::BuildModule(std::string_view fullName)
{
    auto mImpl = std::make_unique<Module::Impl>();
    mImpl->name = std::string(fullName);
    mImpl->descriptor = std::string(fullName);
    Module m(std::move(mImpl));
    return m;
}

Namespace Builder::BuildNamespace(std::initializer_list<std::string_view> fullName)
{
    auto nsImpl = std::make_unique<Namespace::Impl>();
    std::string origName = JoinParts(fullName);
    nsImpl->name = origName;
    nsImpl->descriptor = origName;
    Namespace ns(std::move(nsImpl));
    return ns;
}

Namespace Builder::BuildNamespace(std::string_view fullName)
{
    auto nsImpl = std::make_unique<Namespace::Impl>();
    std::string origName = std::string(fullName);
    nsImpl->name = origName;
    nsImpl->descriptor = origName;
    Namespace ns(std::move(nsImpl));
    return ns;
}

Type Builder::BuildClass(std::initializer_list<std::string_view> fullName)
{
    std::string origName = JoinParts(fullName);
    return CreateType(origName, "C");
}

Type Builder::BuildClass(std::string_view fullName)
{
    std::string origName = std::string(fullName);
    return CreateType(origName, "C");
}

Type Builder::BuildEnum(std::initializer_list<std::string_view> fullName)
{
    std::string origName = JoinParts(fullName);
    return CreateType(origName, "E");
}

Type Builder::BuildEnum(std::string_view fullName)
{
    std::string origName = std::string(fullName);
    return CreateType(origName, "E");
}

Type Builder::BuildPartial(std::initializer_list<std::string_view> fullName)
{
    std::string origName = JoinParts(fullName);
    return CreateType(origName, "P");
}

Type Builder::BuildPartial(std::string_view fullName)
{
    std::string origName = std::string(fullName);
    return CreateType(origName, "P");
}

Type Builder::BuildRequired(std::initializer_list<std::string_view> fullName)
{
    std::string origName = JoinParts(fullName);
    return CreateType(origName, "C");
}

Type Builder::BuildRequired(std::string_view fullName)
{
    std::string origName = std::string(fullName);
    return CreateType(origName, "C");
}

Type Builder::BuildFunctionalObject(std::size_t nrRequiredArgs, bool hasResetArgs)
{
    std::string descriptor;
    const int reserveLength = 30U;
    descriptor.reserve(reserveLength);

    descriptor += "std.core.Function";
    if (hasResetArgs) {
        descriptor += "R";
    }
    descriptor += std::to_string(nrRequiredArgs);

    return CreateType(descriptor, "C");
}

Type Builder::BuildArray(Type const &type)
{
    return CreateArrayType(type.AniDescriptor());
}

std::string Builder::BuildSignatureDescriptor(std::initializer_list<Type> args)
{
    std::string result;
    for (const auto &arg : args) {
        result += arg.AniDescriptor();
    }
    result += ":";
    return result;
}

std::string Builder::BuildSignatureDescriptor(std::initializer_list<Type> args, Type const &ret)
{
    std::string result;
    for (const auto &arg : args) {
        result += arg.AniDescriptor();
    }
    result += ":" + ret.AniDescriptor();
    return result;
}

std::string Builder::BuildConstructorName()
{
    return "<ctor>";
}

std::string Builder::BuildSetterName(std::string_view name)
{
    return GetSetterNamePrefix() + std::string(name);
}

std::string Builder::BuildGetterName(std::string_view name)
{
    return GetGetterNamePrefix() + std::string(name);
}

std::string Builder::BuildPropertyName(std::string_view name)
{
    return GetPropertyNamePrefix() + std::string(name);
}

std::string Builder::Builder::BuildPartialName(std::string_view name)
{
    return GetPartialNamePrefix() + std::string(name);
}

std::string Builder::BuildAsyncName(std::string_view name)
{
    return GetAsyncNamePrefix() + std::string(name);
}

std::string Builder::GetSetterNamePrefix()
{
    return "%%set-";
}

std::string Builder::GetGetterNamePrefix()
{
    return "%%get-";
}

std::string Builder::GetPropertyNamePrefix()
{
    return "%%property-";
}

std::string Builder::GetPartialNamePrefix()
{
    return "%%partial-";
}

std::string Builder::GetAsyncNamePrefix()
{
    return "%%async-";
}

std::string Builder::GetUnionPropertyNamePrefix()
{
    return "%%union_prop-";
}

std::string Builder::GetLambdaPrefix()
{
    return "%%lambda-";
}

std::string Builder::GetLambdaInvokePrefix()
{
    return "lambda_invoke-";
}

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class SignatureBuilder::Impl {
public:
    Impl() : ret_(Builder::BuildUndefined()) {}

    void AddParam(const Type &type)
    {
        params_.push_back(type);
    }

    void SetReturn(const Type &type)
    {
        ret_ = type;
        retSet_ = true;
    }

    std::string BuildSignature() const
    {
        std::string result;
        for (const auto &param : params_) {
            result += param.AniDescriptor();
        }
        result += ":" + (retSet_ ? ret_.AniDescriptor() : "");
        return result;
    }

private:
    std::vector<Type> params_;
    Type ret_;
    bool retSet_ {false};
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

SignatureBuilder::SignatureBuilder() : impl_(std::make_unique<Impl>()) {}

SignatureBuilder::~SignatureBuilder() = default;

SignatureBuilder::SignatureBuilder(SignatureBuilder &&other) : impl_(std::move(other.impl_)) {}

SignatureBuilder &SignatureBuilder::operator=(SignatureBuilder &&other)
{
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

SignatureBuilder &SignatureBuilder::AddUndefined()
{
    impl_->AddParam(Builder::BuildUndefined());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddNull()
{
    impl_->AddParam(Builder::BuildNull());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddAny()
{
    impl_->AddParam(Builder::BuildAny());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddBoolean()
{
    impl_->AddParam(Builder::BuildBoolean());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddChar()
{
    impl_->AddParam(Builder::BuildChar());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddByte()
{
    impl_->AddParam(Builder::BuildByte());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddShort()
{
    impl_->AddParam(Builder::BuildShort());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddInt()
{
    impl_->AddParam(Builder::BuildInt());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddLong()
{
    impl_->AddParam(Builder::BuildLong());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddFloat()
{
    impl_->AddParam(Builder::BuildFloat());
    return *this;
}

SignatureBuilder &SignatureBuilder::AddDouble()
{
    impl_->AddParam(Builder::BuildDouble());
    return *this;
}

SignatureBuilder &SignatureBuilder::Add(Type const &type)
{
    impl_->AddParam(type);
    return *this;
}

SignatureBuilder &SignatureBuilder::AddClass(std::string_view fullName)
{
    impl_->AddParam(Builder::BuildClass(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddClass(std::initializer_list<std::string_view> fullName)
{
    impl_->AddParam(Builder::BuildClass(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddEnum(std::string_view fullName)
{
    impl_->AddParam(Builder::BuildEnum(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddEnum(std::initializer_list<std::string_view> fullName)
{
    impl_->AddParam(Builder::BuildEnum(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddPartial(std::string_view fullName)
{
    impl_->AddParam(Builder::BuildPartial(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddPartial(std::initializer_list<std::string_view> fullName)
{
    impl_->AddParam(Builder::BuildPartial(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddRequired(std::string_view fullName)
{
    impl_->AddParam(Builder::BuildRequired(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddRequired(std::initializer_list<std::string_view> fullName)
{
    impl_->AddParam(Builder::BuildRequired(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::AddFunctionalObject(std::size_t nrRequiredArgs, bool hasResetArgs)
{
    impl_->AddParam(Builder::BuildFunctionalObject(nrRequiredArgs, hasResetArgs));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnUndefined()
{
    impl_->SetReturn(Builder::BuildUndefined());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnNull()
{
    impl_->SetReturn(Builder::BuildNull());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnAny()
{
    impl_->SetReturn(Builder::BuildAny());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnBoolean()
{
    impl_->SetReturn(Builder::BuildBoolean());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnChar()
{
    impl_->SetReturn(Builder::BuildChar());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnByte()
{
    impl_->SetReturn(Builder::BuildByte());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnShort()
{
    impl_->SetReturn(Builder::BuildShort());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnInt()
{
    impl_->SetReturn(Builder::BuildInt());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnLong()
{
    impl_->SetReturn(Builder::BuildLong());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnFloat()
{
    impl_->SetReturn(Builder::BuildFloat());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnDouble()
{
    impl_->SetReturn(Builder::BuildDouble());
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturn(Type const &type)
{
    impl_->SetReturn(type);
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnClass(std::string_view fullName)
{
    impl_->SetReturn(Builder::BuildClass(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnClass(std::initializer_list<std::string_view> fullName)
{
    impl_->SetReturn(Builder::BuildClass(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnEnum(std::string_view fullName)
{
    impl_->SetReturn(Builder::BuildEnum(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnEnum(std::initializer_list<std::string_view> fullName)
{
    impl_->SetReturn(Builder::BuildEnum(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnPartial(std::string_view fullName)
{
    impl_->SetReturn(Builder::BuildPartial(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnPartial(std::initializer_list<std::string_view> fullName)
{
    impl_->SetReturn(Builder::BuildPartial(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnRequired(std::string_view fullName)
{
    impl_->SetReturn(Builder::BuildRequired(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnRequired(std::initializer_list<std::string_view> fullName)
{
    impl_->SetReturn(Builder::BuildRequired(fullName));
    return *this;
}

SignatureBuilder &SignatureBuilder::SetReturnFunctionalObject(std::size_t nrRequiredArgs, bool hasResetArgs)
{
    impl_->SetReturn(Builder::BuildFunctionalObject(nrRequiredArgs, hasResetArgs));
    return *this;
}

std::string SignatureBuilder::BuildSignatureDescriptor()
{
    return impl_->BuildSignature();
}

}  // namespace arkts::ani_signature
