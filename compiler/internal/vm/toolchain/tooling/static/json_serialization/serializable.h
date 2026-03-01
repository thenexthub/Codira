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

#ifndef PANDA_TOOLING_INSPECTOR_JSON_SERIALIZATION_SERIALIZABLE_H
#define PANDA_TOOLING_INSPECTOR_JSON_SERIALIZATION_SERIALIZABLE_H

#include "libarkbase/macros.h"

namespace ark {
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {

/// Interface for inspector objects that may be serialized into json.
class JsonSerializable {
public:
    JsonSerializable() = default;
    virtual ~JsonSerializable() = default;

    DEFAULT_COPY_SEMANTIC(JsonSerializable);
    DEFAULT_MOVE_SEMANTIC(JsonSerializable);

    virtual void Serialize(JsonObjectBuilder &builder) const = 0;

    /// Operator() necessary to use std::invoke, for example in json_builder.h::Stringify method.
    void operator()(JsonObjectBuilder &builder) const
    {
        Serialize(builder);
    }
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_JSON_SERIALIZATION_SERIALIZABLE_H
