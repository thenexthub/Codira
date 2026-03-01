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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_METHOD_SIGNATURE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_METHOD_SIGNATURE_H_

#include "libarkbase/utils/logger.h"
#include "runtime/include/method.h"
#include "plugins/ets/runtime/types/ets_value.h"

namespace ark::ets {

/**
 * @brief Represents arguments type separated from return type by ":".
 * Object names bounded by 'L' and ';', and union constituent types are bounded by "{U" and '}'.
 */
class EtsMethodSignature {
public:
    explicit EtsMethodSignature(const std::string_view sign)
    {
        auto signature = PandaString(sign);
        size_t dots = signature.find(':');
        // Return if ':' wasn't founded or was founded at the end
        if (dots == PandaString::npos || dots == signature.size() - 1) {
            return;
        }
        // Process return type after ':'
        if (ProcessParameter(signature, dots + 1) != signature.size() - 1) {
            return;
        }
        // Process arguments
        size_t i;
        for (i = 0; i < dots; i++) {
            i = ProcessParameter(signature, i);
            if (i == PandaString::npos) {
                return;
            }
        }
        isValid_ = true;
        for (auto &paramType : paramTypes_) {
            pandaProto_.GetRefTypes().push_back(paramType);
        }
    }

    explicit EtsMethodSignature(Method::Proto &&pandaProto, PandaSmallVector<PandaString> &&paramTypes)
        : paramTypes_(std::move(paramTypes)), pandaProto_(std::move(pandaProto)), isValid_(true)
    {
        for (auto &paramType : paramTypes_) {
            pandaProto_.GetRefTypes().push_back(paramType);
        }
    }

    bool IsValid() const
    {
        return isValid_;
    }

    const Method::Proto &GetProto() const
    {
        return pandaProto_;
    }

private:
    size_t ProcessParameter(const PandaString &signature, size_t i)
    {
        EtsType paramType = GetTypeByFirstChar(signature[i]);
        // Check that type is valid and return type is not void
        if (paramType == EtsType::UNKNOWN || (paramType == EtsType::VOID && (i != signature.size() - 1))) {
            return PandaString::npos;
        }
        pandaProto_.GetShorty().push_back(ets::ConvertEtsTypeToPandaType(paramType));

        if (paramType != EtsType::OBJECT) {
            return i;
        }
        size_t nameStart = i;
        i = ProcessObjectParameter(signature, i);
        if (i == PandaString::npos) {
            return i;
        }
        paramTypes_.push_back(signature.substr(nameStart, i + 1 - nameStart));
        return i;
    }

    PandaSmallVector<PandaString> paramTypes_;
    Method::Proto pandaProto_;
    bool isValid_ {false};

    size_t ProcessObjectParameter(const PandaString &signature, size_t i)
    {
        ASSERT(i < signature.size());
        while (signature[i] == '[') {
            ++i;
            if (i >= signature.size()) {
                return PandaString::npos;
            }
        }
        if (GetTypeByFirstChar(signature[i]) == EtsType::UNKNOWN) {
            return PandaString::npos;
        }

        if (signature[i] == 'L') {
            // Get object name bounded by 'L' and ';'
            size_t prevI = i;
            i = signature.find(';', i);
            if (i == PandaString::npos || i == (prevI + 1)) {
                return PandaString::npos;
            }
        } else if (signature[i] == '{') {
            if (i == signature.size() - 1 || signature[i + 1] != 'U') {
                return PandaString::npos;
            }
            // Get union constituent types bounded between "{U" and '}'
            size_t prevI = i + 1;
            i = signature.find('}', prevI);
            if (i == PandaString::npos || i == (prevI + 1)) {
                return PandaString::npos;
            }
        }
        return i;
    }
    EtsType GetTypeByFirstChar(char c)
    {
        switch (c) {
            case 'L':
            case '{':
            case '[':
                return EtsType::OBJECT;
            case 'Z':
                return EtsType::BOOLEAN;
            case 'B':
                return EtsType::BYTE;
            case 'C':
                return EtsType::CHAR;
            case 'S':
                return EtsType::SHORT;
            case 'I':
                return EtsType::INT;
            case 'J':
                return EtsType::LONG;
            case 'F':
                return EtsType::FLOAT;
            case 'D':
                return EtsType::DOUBLE;
            case 'V':
                return EtsType::VOID;
            default:
                return EtsType::UNKNOWN;
        }
    }
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_METHOD_SIGNATURE_H_
