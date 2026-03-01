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

#include "types/profile_result.h"

namespace ark::tooling::inspector {
void Profile::Serialize(JsonObjectBuilder &builder) const
{
    if (!profileInfos_) {
        return;
    }
    builder.AddProperty("profile",
                        [this](JsonArrayBuilder &arrayBuilder) { this->SerializeProfileInfos(arrayBuilder); });
}

void Profile::SerializeProfileInfos(JsonArrayBuilder &arrayBuilder) const
{
    for (const auto &profileInfoPtr : *profileInfos_) {
        if (!profileInfoPtr) {
            continue;
        }
        arrayBuilder.Add([this, &profileInfoPtr](JsonObjectBuilder &objBuilder) {
            this->SerializeSingleProfileInfo(objBuilder, *profileInfoPtr);
        });
    }
}

void Profile::SerializeSingleProfileInfo(JsonObjectBuilder &builder, const sampler::ProfileInfo &profileInfo) const
{
    // add normal data
    builder.AddProperty("tid", profileInfo.tid);
    builder.AddProperty("startTime", profileInfo.startTime);
    builder.AddProperty("endTime", profileInfo.stopTime);
    builder.AddProperty("gcTime", profileInfo.gcTime);
    builder.AddProperty("cInterpreterTime", profileInfo.cInterpreterTime);
    builder.AddProperty("asmInterpreterTime", profileInfo.asmInterpreterTime);
    builder.AddProperty("aotTime", profileInfo.aotTime);
    builder.AddProperty("builtinTime", profileInfo.builtinTime);
    builder.AddProperty("napiTime", profileInfo.napiTime);
    builder.AddProperty("arkuiEngineTime", profileInfo.arkuiEngineTime);
    builder.AddProperty("runtimeTime", profileInfo.runtimeTime);
    builder.AddProperty("otherTime", profileInfo.otherTime);

    // add nodes array
    SerializeNodes(builder, profileInfo);

    // add sample and timeDeltas array
    builder.AddProperty("samples", [&profileInfo](JsonArrayBuilder &arrayBuilder) {
        for (const auto &child : profileInfo.samples) {
            arrayBuilder.Add(child);
        }
    });
    builder.AddProperty("timeDeltas", [&profileInfo](JsonArrayBuilder &arrayBuilder) {
        for (const auto &child : profileInfo.timeDeltas) {
            arrayBuilder.Add(child);
        }
    });
}
void Profile::SerializeNodes(JsonObjectBuilder &builder, const sampler::ProfileInfo &profileInfo) const
{
    builder.AddProperty("nodes", [this, &profileInfo](JsonArrayBuilder &arrayBuilder) {
        for (int i = 0; i < profileInfo.nodeCount; ++i) {
            arrayBuilder.Add([this, &profileInfo, i](JsonObjectBuilder &objBuilder) {
                objBuilder.AddProperty("id", profileInfo.nodes[i].id);
                this->SerializeCallFrame(objBuilder, profileInfo.nodes[i].codeEntry);
                objBuilder.AddProperty("hitCount", profileInfo.nodes[i].hitCount);
                this->SerializeChildren(objBuilder, profileInfo.nodes[i].children);
            });
        }
    });
}

void Profile::SerializeChildren(JsonObjectBuilder &builder, const std::vector<int> children) const
{
    builder.AddProperty("children", [&children](JsonArrayBuilder &childrenBuilder) {
        for (const auto &child : children) {
            childrenBuilder.Add(child);
        }
    });
}

void Profile::SerializeCallFrame(JsonObjectBuilder &builder, const struct sampler::FrameInfo &codeEntry) const
{
    builder.AddProperty("callFrame", [&codeEntry](JsonObjectBuilder &objBuilder) {
        objBuilder.AddProperty("functionName", codeEntry.functionName);
        objBuilder.AddProperty("moduleName", codeEntry.moduleName);
        objBuilder.AddProperty("scriptId", codeEntry.scriptId);
        objBuilder.AddProperty("url", codeEntry.url);
        objBuilder.AddProperty("lineNumber", codeEntry.lineNumber);
        objBuilder.AddProperty("columnNumber", codeEntry.columnNumber);
    });
}
}  // namespace ark::tooling::inspector
