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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_PROFILE_RESULT_H
#define PANDA_TOOLING_INSPECTOR_TYPES_PROFILE_RESULT_H

#include "json_serialization/serializable.h"
#include "runtime/tooling/sampler/samples_record.h"
#include "libarkbase/utils/json_builder.h"

namespace ark::tooling::inspector {
class Profile final : public JsonSerializable {
public:
    explicit Profile(std::unique_ptr<std::vector<std::unique_ptr<sampler::ProfileInfo>>> profileInfos)
        : profileInfos_(std::move(profileInfos))
    {
    }
    void Serialize(JsonObjectBuilder &builder) const override;

private:
    void SerializeProfileInfos(JsonArrayBuilder &arrayBuilder) const;
    void SerializeSingleProfileInfo(JsonObjectBuilder &builder, const sampler::ProfileInfo &profileInfo) const;
    void SerializeNodes(JsonObjectBuilder &builder, const sampler::ProfileInfo &profileInfo) const;
    void SerializeChildren(JsonObjectBuilder &builder, const std::vector<int> children) const;
    void SerializeCallFrame(JsonObjectBuilder &builder, const struct sampler::FrameInfo &codeEntry) const;
    std::unique_ptr<std::vector<std::unique_ptr<sampler::ProfileInfo>>> profileInfos_;
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_PROFILE_RESULT_H
