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
#include "abckit_wrapper/method.h"
#include <regex>
#include <numeric>
#include "abckit_wrapper/modifiers.h"
#include "string_util.h"
#include "logger.h"

namespace {
constexpr std::string_view RAW_NAME_AND_TYPE_NAME_SEP = ":";
constexpr size_t RAW_NAME_AND_TYPE_NAME_SIZE = 2;
constexpr std::string_view TYPE_NAME_SEP = ";";
const std::regex SETTER_GETTER_PATTERN(R"(%%(set|get)-)");

bool IsSetterOrGetterMethod(const std::string &methodRawName)
{
    return std::regex_search(methodRawName, SETTER_GETTER_PATTERN);
}
}  // namespace

std::string abckit_wrapper::Parameter::GetName() const
{
    return "";  // function param name is ignored
}

bool abckit_wrapper::Parameter::SetName(const std::string &name)
{
    return false;  // function param set name is ignored
}

std::string abckit_wrapper::Parameter::GetTypeName() const
{
    return this->impl_.GetType().GetName();
}

bool abckit_wrapper::Parameter::Accept(ParameterVisitor &visitor)
{
    return visitor.Visit(this);
}

AbckitWrapperErrorCode abckit_wrapper::Method::Init()
{
    this->InitAccessFlags();

    auto rc = this->InitSignature();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init method descriptor for:" << this->GetFullyQualifiedName();
        return rc;
    }

    LOG_I << "signature:" << this->rawName_ + this->descriptor_;

    rc = this->InitAnnotations();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init method annotations:" << rc;
        return rc;
    }

    rc = this->InitParameters();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init method parameters:" << rc;
        return rc;
    }

    return ERR_SUCCESS;
}

std::string abckit_wrapper::Method::GetName() const
{
    return this->GetObjectName();
}

bool abckit_wrapper::Method::SetName(const std::string &name)
{
    const auto function = this->GetArkTsImpl<abckit::arkts::Function>();
    if (function.IsAnonymous() || function.IsExternal()) {
        return true;
    }
    if (!function.SetName(name)) {
        return false;
    }

    if (ABCKIT_WRAPPER_ERROR(this->InitSignature())) {
        LOG_W << "Failed to refresh signature";
        return false;
    }

    return true;
}

bool abckit_wrapper::Method::IsConstructor() const
{
    return this->impl_.IsCctor() || this->impl_.IsCtor();
}

AbckitWrapperErrorCode abckit_wrapper::Method::InitSignature()
{
    // split method name and params type name by ":"
    const auto cachedName = this->GetName();
    const auto rawNameAndTypeName = StringUtil::Split(cachedName, RAW_NAME_AND_TYPE_NAME_SEP.data());
    if (rawNameAndTypeName.size() != RAW_NAME_AND_TYPE_NAME_SIZE) {
        LOG_E << "get bad len of rawName and typeNames for method:" << cachedName;
        return ERR_INNER_ERROR;
    }
    this->rawName_ = rawNameAndTypeName[0];

    // setter or getter method descriptor is the field name
    if (IsSetterOrGetterMethod(rawNameAndTypeName[0])) {
        this->rawName_ = std::regex_replace(rawNameAndTypeName[0], SETTER_GETTER_PATTERN, "");
        this->descriptor_ = "";
        return ERR_SUCCESS;
    }

    // split params type name by ";"
    const auto paramsAndReturnTypeName = StringUtil::Split(rawNameAndTypeName[1], TYPE_NAME_SEP.data());
    if (paramsAndReturnTypeName.empty()) {
        LOG_E << "typeNames is empty for method:" << cachedName;
        return ERR_INNER_ERROR;
    }

    // build descriptor
    this->descriptor_ += "(";
    if (paramsAndReturnTypeName.size() > 1) {
        auto begin = paramsAndReturnTypeName.begin();
        // if method is not static, first param will be ignored, first param is the class full name
        if (!this->impl_.IsStatic()) {
            begin = std::next(begin);
        }
        // combine params to param1Type, param2Type, ..., paramNType
        this->descriptor_ +=
            std::accumulate(begin, std::prev(paramsAndReturnTypeName.end()), std::string(),
                            [](const std::string &a, const std::string &b) { return a.empty() ? b : a + ", " + b; });
    }
    this->descriptor_ += ")" + paramsAndReturnTypeName[paramsAndReturnTypeName.size() - 1];

    this->cachedName_ = cachedName;

    return ERR_SUCCESS;
}

void abckit_wrapper::Method::InitAccessFlags()
{
    this->accessFlags_ |= this->impl_.IsPublic() ? AccessFlags::PUBLIC : AccessFlags::NONE;
    this->accessFlags_ |= this->impl_.IsProtected() ? AccessFlags::PROTECTED : AccessFlags::NONE;
    this->accessFlags_ |= this->impl_.IsPrivate() ? AccessFlags::PRIVATE : AccessFlags::NONE;
    this->accessFlags_ |= this->impl_.IsInternal() ? AccessFlags::INTERNAL : AccessFlags::NONE;

    this->accessFlags_ |= this->impl_.IsCctor() ? AccessFlags::CCTOR : AccessFlags::NONE;
    this->accessFlags_ |= this->impl_.IsCtor() ? AccessFlags::CTOR : AccessFlags::NONE;
    this->accessFlags_ |= this->impl_.IsStatic() ? AccessFlags::STATIC : AccessFlags::NONE;

    const auto function = this->GetArkTsImpl<abckit::arkts::Function>();
    this->accessFlags_ |= function.IsFinal() ? AccessFlags::FINAL : AccessFlags::NONE;
    this->accessFlags_ |= function.IsAbstract() ? AccessFlags::ABSTRACT : AccessFlags::NONE;
    this->accessFlags_ |= function.IsAsync() ? AccessFlags::ASYNC : AccessFlags::NONE;
    // async function has the native attr, exclude the native attr for async function
    this->accessFlags_ |= function.IsNative() && !function.IsAsync() ? AccessFlags::NATIVE : AccessFlags::NONE;
}

void abckit_wrapper::Method::InitAnnotation(Annotation *annotation)
{
    annotation->owner_ = this;
}

std::vector<abckit::core::Annotation> abckit_wrapper::Method::GetAnnotations() const
{
    return this->impl_.GetAnnotations();
}

AbckitWrapperErrorCode abckit_wrapper::Method::InitParameters()
{
    for (auto &param : this->impl_.GetParameters()) {
        auto parameter = std::make_unique<Parameter>(param);
        parameter->owingMethod_ = this;
        const auto rc = parameter->Init();
        if (ABCKIT_WRAPPER_ERROR(rc)) {
            LOG_E << "Failed to init parameter:" << rc;
            return rc;
        }
        LOG_I << "Found parameter:" << parameter->GetTypeName();
        this->parameters_.emplace_back(std::move(parameter));
    }

    return ERR_SUCCESS;
}

bool abckit_wrapper::Method::Accept(MethodVisitor &visitor)
{
    return visitor.Visit(this);
}

bool abckit_wrapper::Method::Accept(ChildVisitor &visitor)
{
    return visitor.VisitMethod(this);
}

bool abckit_wrapper::Method::ParametersAccept(ParameterVisitor &visitor) const
{
    for (const auto &param : this->parameters_) {
        if (!param->Accept(visitor)) {
            return false;
        }
    }

    return true;
}