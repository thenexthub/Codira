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

#ifndef PANDA_ASSEMBLER_MANGLING_H
#define PANDA_ASSEMBLER_MANGLING_H

#include "assembly-function.h"

#include <string>
#include <sstream>
#include <vector>

namespace ark::pandasm {
constexpr const std::string_view MANGLE_BEGIN = ":";
constexpr const std::string_view MANGLE_SEPARATOR = ";";

constexpr const std::string_view SIGNATURE_BEGIN = MANGLE_BEGIN;
constexpr const std::string_view SIGNATURE_TYPES_SEPARATOR = ",";
constexpr const std::string_view SIGNATURE_TYPES_SEPARATOR_L = "(";
constexpr const std::string_view SIGNATURE_TYPES_SEPARATOR_R = ")";

inline std::string DeMangleName(const std::string &name)
{
    auto const pos = name.find_first_of(MANGLE_BEGIN);
    if (pos != std::string::npos) {
        return name.substr(0, pos);
    }
    return name;
}

inline std::string MangleFunctionName(const std::string &name, const std::vector<pandasm::Function::Parameter> &params,
                                      const pandasm::Type &returnType)
{
    std::ostringstream result;
    result << name << MANGLE_BEGIN;
    for (const auto &p : params) {
        result << p.type.GetName() << MANGLE_SEPARATOR;
    }
    result << returnType.GetName() << MANGLE_SEPARATOR;
    return result.str();
}

inline std::string MangleFieldName(const std::string &name, const pandasm::Type &type)
{
    std::ostringstream result;
    result << name << MANGLE_BEGIN << type.GetName() << MANGLE_SEPARATOR;
    return result.str();
}

inline std::string GetFunctionSignatureFromName(const std::string &name,
                                                const std::vector<pandasm::Function::Parameter> &params)
{
    std::ostringstream result;
    result << name << SIGNATURE_BEGIN << SIGNATURE_TYPES_SEPARATOR_L;
    for (auto iter = params.begin(); iter != params.end(); ++iter) {
        result << ((iter != params.begin()) ? SIGNATURE_TYPES_SEPARATOR : "");
        result << iter->type.GetPandasmName();
    }
    result << SIGNATURE_TYPES_SEPARATOR_R;
    return result.str();
}

inline std::string GetFunctionNameFromSignature(const std::string &sig)
{
    return DeMangleName(sig);
}

inline bool IsSignatureOrMangled(const std::string &str)
{
    return str.find(SIGNATURE_BEGIN) != std::string::npos;
}

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_MANGLING_H
