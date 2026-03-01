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

#ifndef CPP_ABCKIT_LITERAL_ARRAY_IMPL_H
#define CPP_ABCKIT_LITERAL_ARRAY_IMPL_H

#include "literal_array.h"
#include "literal.h"
#include <functional>
#include <memory>
#include <string_view>
#include <vector>

namespace abckit {

inline bool LiteralArray::EnumerateElements(const std::function<bool(Literal)> &cb) const
{
    Payload<const std::function<bool(Literal)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->literalArrayEnumerateElements(
        GetView(), &payload, [](AbckitFile *, AbckitLiteral *lit, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(Literal)> &> *>(data);
            return payload.data(Literal(lit, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_ARRAY_IMPL_H
