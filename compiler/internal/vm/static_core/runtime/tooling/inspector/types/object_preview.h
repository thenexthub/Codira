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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_OBJECT_PREVIEW_H
#define PANDA_TOOLING_INSPECTOR_TYPES_OBJECT_PREVIEW_H

#include "types/remote_object_type.h"
#include "types/property_preview.h"

namespace ark::tooling::inspector {

class PropertyDescriptor;

class ObjectPreview final : public JsonSerializable {
public:
    ObjectPreview(RemoteObjectType type, const std::vector<PropertyDescriptor> &properties);

    void Serialize(JsonObjectBuilder &builder) const override;

private:
    RemoteObjectType type_;

    bool overflow_ {false};

    std::vector<PropertyPreview> properties_;
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_OBJECT_PREVIEW_H
