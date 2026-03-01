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

#ifndef ASSEMBLER_MANGLING_H
#define ASSEMBLER_MANGLING_H

#include "assembly-function.h"

#include <string>
#include <vector>

namespace panda::pandasm {
constexpr const std::string_view MANGLE_BEGIN = ":";
constexpr const std::string_view MANGLE_SEPARATOR = ";";

constexpr const std::string_view SIGNATURE_BEGIN = MANGLE_BEGIN;
constexpr const std::string_view SIGNATURE_TYPES_SEPARATOR = ",";
constexpr const std::string_view SIGNATURE_TYPES_SEPARATOR_L = "(";
constexpr const std::string_view SIGNATURE_TYPES_SEPARATOR_R = ")";

inline std::string DeMangleName(const std::string &name)
{
    auto iter = name.find_first_of(MANGLE_BEGIN);
    if (iter != std::string::npos) {
        return name.substr(0, name.find_first_of(MANGLE_BEGIN));
    }
    return name;
}

inline std::string MangleFunctionName(const std::string &name, const std::vector<pandasm::Function::Parameter> &params,
                                      const pandasm::Type &return_type)
{
    std::string mangle_name {name};
    mangle_name += MANGLE_BEGIN;
    for (const auto &p : params) {
        mangle_name += p.type.GetName() + std::string(MANGLE_SEPARATOR);
    }
    mangle_name += return_type.GetName() + std::string(MANGLE_SEPARATOR);

    return mangle_name;
}

inline std::string MangleFieldName(const std::string &name, const pandasm::Type &type)
{
    std::string mangle_name {name};
    mangle_name += MANGLE_BEGIN;
    mangle_name += type.GetName() + std::string(MANGLE_SEPARATOR);
    return mangle_name;
}

inline std::string GetFunctionSignatureFromName(std::string name,
                                                const std::vector<pandasm::Function::Parameter> &params)
{
    name += SIGNATURE_BEGIN;
    name += SIGNATURE_TYPES_SEPARATOR_L;

    bool first = true;
    for (const auto &p : params) {
        name += (first) ? "" : SIGNATURE_TYPES_SEPARATOR;
        first = false;
        name += p.type.GetPandasmName();
    }

    name += SIGNATURE_TYPES_SEPARATOR_R;

    return name;
}

inline std::string GetFunctionNameFromSignature(const std::string &sig)
{
    return DeMangleName(sig);
}

inline bool IsSignatureOrMangled(const std::string &str)
{
    return str.find(SIGNATURE_BEGIN) != std::string::npos;
}

}  // namespace panda::pandasm

#endif  // ASSEMBLER_MANGLING_H
