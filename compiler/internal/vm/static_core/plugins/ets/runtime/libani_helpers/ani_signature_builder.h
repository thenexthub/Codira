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

#ifndef ANI_SIGNATURE_BUILDER_H
#define ANI_SIGNATURE_BUILDER_H

#include <initializer_list>
#include <string>
#include <string_view>
#include <memory>

namespace arkts::ani_signature {

class Module {
public:
    class Impl;

    explicit Module(std::unique_ptr<Impl> impl);
    ~Module();
    Module(const Module & /*other*/);
    Module &operator=(const Module & /*other*/);
    Module(Module &&other);
    Module &operator=(Module &&other);

    std::string Descriptor() const;
    std::string Name() const;

private:
    std::unique_ptr<Impl> impl_;
};

class Namespace {
public:
    class Impl;

    explicit Namespace(std::unique_ptr<Impl> impl);
    ~Namespace();
    Namespace(const Namespace & /*other*/);
    Namespace &operator=(const Namespace & /*other*/);
    Namespace(Namespace &&other);
    Namespace &operator=(Namespace &&other);

    std::string Descriptor() const;
    std::string Name() const;

private:
    std::unique_ptr<Impl> impl_;
};

class Type final {
public:
    class Impl;

    explicit Type(std::unique_ptr<Impl> impl);
    ~Type();
    Type(const Type & /*other*/);
    Type &operator=(const Type & /*other*/);
    Type(Type &&other);
    Type &operator=(Type &&other);

    std::string Descriptor() const;

private:
    std::string AniDescriptor() const;
    std::unique_ptr<Impl> impl_;

    friend class Builder;
    friend class SignatureBuilder;
};

class Builder {
public:
    static Type BuildUndefined();
    static Type BuildNull();
    static Type BuildAny();

    static Type BuildBoolean();
    static Type BuildChar();
    static Type BuildByte();
    static Type BuildShort();
    static Type BuildInt();
    static Type BuildLong();
    static Type BuildFloat();
    static Type BuildDouble();

    static Module BuildModule(std::string_view fullName);
    static Namespace BuildNamespace(std::initializer_list<std::string_view> fullName);
    static Namespace BuildNamespace(std::string_view fullName);
    static Type BuildClass(std::initializer_list<std::string_view> fullName);
    static Type BuildClass(std::string_view fullName);
    static Type BuildEnum(std::initializer_list<std::string_view> fullName);
    static Type BuildEnum(std::string_view fullName);
    static Type BuildPartial(std::initializer_list<std::string_view> fullName);
    static Type BuildPartial(std::string_view fullName);
    static Type BuildRequired(std::initializer_list<std::string_view> fullName);
    static Type BuildRequired(std::string_view fullName);
    static Type BuildFunctionalObject(std::size_t nrRequiredArgs, bool hasResetArgs);
    static Type BuildArray(Type const &type);

    static std::string BuildSignatureDescriptor(std::initializer_list<Type> args);
    static std::string BuildSignatureDescriptor(std::initializer_list<Type> args, Type const &ret);

    static std::string BuildConstructorName();
    static std::string BuildSetterName(std::string_view name);
    static std::string BuildGetterName(std::string_view name);
    static std::string BuildPropertyName(std::string_view name);
    static std::string BuildPartialName(std::string_view name);
    static std::string BuildAsyncName(std::string_view name);
    static std::string GetSetterNamePrefix();
    static std::string GetGetterNamePrefix();
    static std::string GetPropertyNamePrefix();
    static std::string GetPartialNamePrefix();
    static std::string GetAsyncNamePrefix();
    static std::string GetUnionPropertyNamePrefix();
    static std::string GetLambdaPrefix();
    static std::string GetLambdaInvokePrefix();
};

class SignatureBuilder {
public:
    SignatureBuilder();
    ~SignatureBuilder();

    SignatureBuilder(const SignatureBuilder &) = delete;
    SignatureBuilder &operator=(const SignatureBuilder &) = delete;

    SignatureBuilder(SignatureBuilder &&other);
    SignatureBuilder &operator=(SignatureBuilder &&other);

    SignatureBuilder &AddUndefined();
    SignatureBuilder &AddNull();
    SignatureBuilder &AddAny();

    SignatureBuilder &AddBoolean();
    SignatureBuilder &AddChar();
    SignatureBuilder &AddByte();
    SignatureBuilder &AddShort();
    SignatureBuilder &AddInt();
    SignatureBuilder &AddLong();
    SignatureBuilder &AddFloat();
    SignatureBuilder &AddDouble();

    SignatureBuilder &Add(Type const &type);

    SignatureBuilder &AddClass(std::string_view fullName);
    SignatureBuilder &AddClass(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &AddEnum(std::string_view fullName);
    SignatureBuilder &AddEnum(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &AddPartial(std::string_view fullName);
    SignatureBuilder &AddPartial(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &AddRequired(std::string_view fullName);
    SignatureBuilder &AddRequired(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &AddFunctionalObject(std::size_t nrRequiredArgs, bool hasResetArgs);

    SignatureBuilder &SetReturnUndefined();
    SignatureBuilder &SetReturnNull();
    SignatureBuilder &SetReturnAny();

    SignatureBuilder &SetReturnBoolean();
    SignatureBuilder &SetReturnChar();
    SignatureBuilder &SetReturnByte();
    SignatureBuilder &SetReturnShort();
    SignatureBuilder &SetReturnInt();
    SignatureBuilder &SetReturnLong();
    SignatureBuilder &SetReturnFloat();
    SignatureBuilder &SetReturnDouble();

    SignatureBuilder &SetReturn(Type const &type);

    SignatureBuilder &SetReturnClass(std::string_view fullName);
    SignatureBuilder &SetReturnClass(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &SetReturnEnum(std::string_view fullName);
    SignatureBuilder &SetReturnEnum(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &SetReturnPartial(std::string_view fullName);
    SignatureBuilder &SetReturnPartial(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &SetReturnRequired(std::string_view fullName);
    SignatureBuilder &SetReturnRequired(std::initializer_list<std::string_view> fullName);
    SignatureBuilder &SetReturnFunctionalObject(std::size_t nrRequiredArgs, bool hasResetArgs);

    std::string BuildSignatureDescriptor();

    class Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace arkts::ani_signature

#endif  // ANI_SIGNATURE_BUILDER_H
