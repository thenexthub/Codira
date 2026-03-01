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

#ifndef PANDA_GUARD_OBFUSCATE_ANNOTATION_H
#define PANDA_GUARD_OBFUSCATE_ANNOTATION_H

#include "entity.h"

namespace panda::guard {

class Annotation final : public Entity {
public:
    /**
     * annotation is in white list, system annotation can't be obfuscated
     * @param name annotation name
     * @return whether in white list
     */
    static bool IsWhiteListAnnotation(const std::string &name);

    Annotation(Program *program, const std::string &name) : Entity(program, name) {}

    void WriteNameCache(const std::string &filePath) override;

protected:
    void Update() override;

    void RefreshNeedUpdate() override;

    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;

public:
    std::string nodeName_;    // associate source file node name
    std::string recordName_;  // the key of pandasm Program record_table
};
}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_ANNOTATION_H
