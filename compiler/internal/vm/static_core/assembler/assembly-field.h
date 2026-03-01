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

#ifndef PANDA_ASSEMBLER_FIELD_H
#define PANDA_ASSEMBLER_FIELD_H

#include "assembly-type.h"
#include "extensions/extensions.h"
#include "meta.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Field {
    Type type;
    std::string name;
    std::string wholeLine; /* The line in which the field is defined */
    std::unique_ptr<FieldMetadata> metadata;
    size_t lineOfDef = 0; /*  Or line in which the field met, if the field is not defined */
    size_t boundLeft = 0;
    size_t boundRight = 0;
    bool isDefined = true;

    bool IsStatic() const
    {
        return (metadata->GetAccessFlags() & ACC_STATIC) != 0;
    }

    explicit Field(ark::panda_file::SourceLang lang)
        : metadata(extensions::MetadataExtension::CreateFieldMetadata(lang))
    {
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // !PANDA_ASSEMBLER_FIELD_H
