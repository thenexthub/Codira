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

#ifndef CPP_ABCKIT_FILE_IMPL_H
#define CPP_ABCKIT_FILE_IMPL_H

#include <string_view>
#include "file.h"
#include "js/module.h"
#include "arkts/module.h"

namespace abckit {

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Module> File::GetModules() const
{
    std::vector<core::Module> modules;
    Payload<std::vector<core::Module> *> payload {&modules, GetApiConfig(), this};

    GetApiConfig()->cIapi_->fileEnumerateModules(GetResource(), &payload, [](AbckitCoreModule *module, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Module> *> *>(data);
        payload.data->push_back(core::Module(module, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return modules;
}

inline abckit::LiteralArray File::CreateLiteralArray(const std::vector<abckit::Literal> &literals) const
{
    std::vector<AbckitLiteral *> litsImpl;
    litsImpl.reserve(literals.size());
    for (const auto &literal : literals) {
        litsImpl.push_back(literal.GetView());
    }
    AbckitLiteralArray *litaIml =
        GetApiConfig()->cMapi_->createLiteralArray(GetResource(), litsImpl.data(), litsImpl.size());
    CheckError(GetApiConfig());
    return abckit::LiteralArray(litaIml, GetApiConfig(), this);
}

inline bool File::EnumerateModules(const std::function<bool(core::Module)> &cb) const
{
    Payload<const std::function<bool(core::Module)> &> payload {cb, GetApiConfig(), this};

    auto isNormalExit =
        GetApiConfig()->cIapi_->fileEnumerateModules(GetResource(), &payload, [](AbckitCoreModule *module, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Module)> &> *>(data);
            return payload.data(core::Module(module, payload.config, payload.resource));
        });

    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool File::EnumerateExternalModules(const std::function<bool(core::Module)> &cb) const
{
    Payload<const std::function<bool(core::Module)> &> payload {cb, GetApiConfig(), this};

    auto isNormalExit = GetApiConfig()->cIapi_->fileEnumerateExternalModules(
        GetResource(), &payload, [](AbckitCoreModule *module, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Module)> &> *>(data);
            return payload.data(core::Module(module, payload.config, payload.resource));
        });

    CheckError(GetApiConfig());
    return isNormalExit;
}

inline Type File::CreateType(enum AbckitTypeId id) const
{
    auto t = GetApiConfig()->cMapi_->createType(GetResource(), id);
    CheckError(GetApiConfig());
    return Type(t, GetApiConfig(), this);
}

inline Type File::CreateReferenceType(core::Class &klass) const
{
    auto rt = GetApiConfig()->cMapi_->createReferenceType(GetResource(), klass.GetView());
    CheckError(GetApiConfig());
    return Type(rt, GetApiConfig(), this);
}

inline abckit::Value File::CreateValueU1(bool val) const
{
    AbckitValue *value = GetApiConfig()->cMapi_->createValueU1(GetResource(), val);
    CheckError(GetApiConfig());
    return abckit::Value(value, GetApiConfig(), this);
}

inline abckit::Value File::CreateValueInt(int val) const
{
    AbckitValue *value = GetApiConfig()->cMapi_->createValueInt(GetResource(), val);
    CheckError(GetApiConfig());
    return abckit::Value(value, GetApiConfig(), this);
}

inline abckit::Value File::CreateValueDouble(double val) const
{
    AbckitValue *value = GetApiConfig()->cMapi_->createValueDouble(GetResource(), val);
    CheckError(GetApiConfig());
    return abckit::Value(value, GetApiConfig(), this);
}

inline abckit::Literal File::CreateLiteralBool(bool val) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralBool(GetResource(), val);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline abckit::Literal File::CreateLiteralDouble(double val) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralDouble(GetResource(), val);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline abckit::Literal File::CreateLiteralLiteralArray(const abckit::LiteralArray &val) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralLiteralArray(GetResource(), val.GetView());
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline abckit::Literal File::CreateLiteralString(std::string_view val) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralString(GetResource(), val.data(), val.size());
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline abckit::Literal File::CreateLiteralNullValue() const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralNullValue(GetResource());
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline Value File::CreateValueString(std::string_view value) const
{
    auto val = GetApiConfig()->cMapi_->createValueString(GetResource(), value.data(), value.size());
    CheckError(GetApiConfig());
    return Value(val, GetApiConfig(), this);
}

inline Value File::CreateLiteralArrayValue(std::vector<Value> &value, size_t size) const
{
    std::vector<AbckitValue *> values(size);
    for (auto &val : value) {
        values.push_back(val.GetView());
    }
    auto arr = GetApiConfig()->cMapi_->createLiteralArrayValue(GetResource(), &values[0], size);
    CheckError(GetApiConfig());
    return Value(arr, GetApiConfig(), this);
}

inline abckit::Literal File::CreateLiteralU8(uint8_t value) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralU8(GetResource(), value);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline abckit::Literal File::CreateLiteralU16(uint16_t value) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralU16(GetResource(), value);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline Literal File::CreateLiteralMethodAffiliate(uint16_t value) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralMethodAffiliate(GetResource(), value);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline Literal File::CreateLiteralU32(uint32_t value) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralU32(GetResource(), value);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline Literal File::CreateLiteralU64(uint64_t value) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralU64(GetResource(), value);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline Literal File::CreateLiteralFloat(float value) const
{
    AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralFloat(GetResource(), value);
    CheckError(GetApiConfig());
    return abckit::Literal(literal, GetApiConfig(), this);
}

inline Literal File::CreateLiteralMethod(core::Function &function) const
{
    auto func = GetApiConfig()->cMapi_->createLiteralMethod(GetResource(), function.GetView());
    CheckError(GetApiConfig());
    return Literal(func, GetApiConfig(), this);
}

inline arkts::Module File::AddExternalModuleArktsV1(std::string_view name) const
{
    const struct AbckitArktsV1ExternalModuleCreateParams params {
        name.data()
    };
    auto mod = GetApiConfig()->cArktsMapi_->fileAddExternalModuleArktsV1(GetResource(), &params);
    CheckError(GetApiConfig());
    auto coreMod = GetApiConfig()->cArktsIapi_->arktsModuleToCoreModule(mod);
    CheckError(GetApiConfig());
    return arkts::Module(core::Module(coreMod, GetApiConfig(), this));
}

inline arkts::Module File::AddExternalModuleArktsV2(std::string_view name) const
{
    auto mod = GetApiConfig()->cArktsMapi_->fileAddExternalModuleArktsV2(GetResource(), name.data());
    CheckError(GetApiConfig());
    auto coreMod = GetApiConfig()->cArktsIapi_->arktsModuleToCoreModule(mod);
    CheckError(GetApiConfig());
    return arkts::Module(core::Module(coreMod, GetApiConfig(), this));
}

inline js::Module File::AddExternalModuleJs(std::string_view name) const
{
    const struct AbckitJsExternalModuleCreateParams params {
        name.data()
    };
    auto jsMod = GetApiConfig()->cJsMapi_->fileAddExternalModule(GetResource(), &params);
    CheckError(GetApiConfig());
    auto coreMod = GetApiConfig()->cJsIapi_->jsModuleToCoreModule(jsMod);
    CheckError(GetApiConfig());
    return js::Module(core::Module(coreMod, GetApiConfig(), this));
}

}  // namespace abckit

#endif  // CPP_ABCKIT_FILE_IMPL_H
