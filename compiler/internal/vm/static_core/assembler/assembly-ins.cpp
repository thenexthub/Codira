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

#include "assembly-ins.h"

namespace ark::pandasm {

std::string ark::pandasm::Ins::RegsToString(bool &first, bool printArgs, size_t firstArgIdx) const
{
    std::string translator {};
    for (std::size_t i = 0U; i < RegSize(); ++i) {
        if (!first) {
            translator += ',';
        } else {
            first = false;
        }

        auto const reg = static_cast<std::size_t>(GetReg(i));
        if (printArgs && reg >= firstArgIdx) {
            translator += " a" + std::to_string(reg - firstArgIdx);
        } else {
            translator += " v" + std::to_string(reg);
        }
    }
    return translator;
}

std::string ark::pandasm::Ins::ImmsToString(bool &first) const
{
    std::stringstream translator {};
    for (std::size_t i = 0U; i < ImmSize(); ++i) {
        if (!first) {
            translator << ',';
        } else {
            first = false;
        }

        auto value = GetImm(i);
        ;
        if (std::holds_alternative<double>(value)) {
            translator << " " << std::scientific << std::get<double>(value);
        } else {
            translator << " 0x" << std::hex << std::get<int64_t>(value);
        }
        translator.clear();
    }
    return translator.str();
}

std::string ark::pandasm::Ins::IdsToString(bool &first) const
{
    std::string translator {};
    for (std::size_t i = 0U; i < IDSize(); ++i) {
        if (!first) {
            translator += ',';
        } else {
            first = false;
        }

        translator += GetID(i);
    }
    return translator;
}

std::string ark::pandasm::Ins::OperandsToString(bool printArgs, size_t firstArgIdx) const
{
    bool first = true;
    std::stringstream ss {};

    ss << this->RegsToString(first, printArgs, firstArgIdx) << this->ImmsToString(first) << this->IdsToString(first);

    return ss.str();
}

std::string ark::pandasm::Ins::RegToString(size_t idx, bool isFirst, bool printArgs, size_t firstArgIdx) const
{
    if (idx >= RegSize()) {
        return std::string("");
    }

    std::string translator {!isFirst ? ", " : " "};
    auto const reg = static_cast<std::size_t>(GetReg(idx));
    if (printArgs && reg >= firstArgIdx) {
        translator += 'a' + std::to_string(reg - firstArgIdx);
    } else {
        translator += 'v' + std::to_string(reg);
    }

    return translator;
}

std::string ark::pandasm::Ins::ImmToString(size_t idx, bool isFirst) const
{
    if (idx >= ImmSize()) {
        return std::string("");
    }

    auto *number = std::get_if<double>(&(imms[idx]));
    std::stringstream translator;

    if (!isFirst) {
        translator << ", ";
    } else {
        translator << " ";
    }

    if (number != nullptr) {
        translator << std::scientific << *number;
    } else {
        translator << "0x" << std::hex << std::get<int64_t>(imms[idx]);
    }

    return translator.str();
}

std::string ark::pandasm::Ins::IdToString(size_t idx, bool isFirst) const
{
    if (idx >= IDSize()) {
        return std::string("");
    }

    return (isFirst ? " " : ", ") + GetID(idx);
}

}  // namespace ark::pandasm
