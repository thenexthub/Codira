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

#ifndef ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H
#define ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H

#include "abc2program_key_data.h"
#include "abc2program_log.h"
#include "abc_file_utils.h"

namespace ark::abc2program {

class AbcFileEntityProcessor {
public:
    AbcFileEntityProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData);

protected:
    template <typename T>
    static void SetEntityAttribute(T &entity, const std::function<bool()> &shouldSet, std::string_view attribute)
    {
        LOG(DEBUG, ABC2PROGRAM) << attribute;
        if (shouldSet()) {
            auto err = entity.metadata->SetAttribute(attribute);
            if (err.has_value()) {
                LOG(DEBUG, ABC2PROGRAM) << err.value().GetMessage();
            }
        }
    }

    template <typename T>
    static void SetEntityAttributeValue(T &entity, const std::function<bool()> &shouldSet, std::string_view attribute,
                                        const char *value)
    {
        LOG(DEBUG, ABC2PROGRAM) << attribute;
        if (shouldSet()) {
            auto err = entity.metadata->SetAttributeValue(attribute, value);
            if (err.has_value()) {
                LOG(DEBUG, ABC2PROGRAM) << err.value().GetMessage();
            }
        }
    }
    panda_file::File::EntityId entityId_;     // NOLINT(misc-non-private-member-variables-in-classes)
    Abc2ProgramKeyData &keyData_;             // NOLINT(misc-non-private-member-variables-in-classes)
    const panda_file::File *file_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
    AbcStringTable *stringTable_ = nullptr;   // NOLINT(misc-non-private-member-variables-in-classes)
    pandasm::Program *program_ = nullptr;     // NOLINT(misc-non-private-member-variables-in-classes)
    friend class AbcFileProcessor;            // NOLINT(misc-non-private-member-variables-in-classes)
};                                            // class AbcFileEntityProcessor

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H
