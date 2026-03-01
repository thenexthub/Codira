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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_RECORD_H
#define PANDA_ASSEMBLER_ASSEMBLY_RECORD_H

#include "assembly-field.h"
#include "assembly-file-location.h"
#include "extensions/extensions.h"
#include "ide_helpers.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Record {
    Record() = delete;
    ~Record() = default;

    DEFAULT_MOVE_SEMANTIC(Record);
    NO_COPY_SEMANTIC(Record);

    std::string name;
    std::string sourceFile; /* The file in which the record is defined or empty */
    SourceLocation bodyLocation;

    std::vector<Field> fieldList; /* class fields list */

    FileLocation fileLocation {};
    std::unique_ptr<RecordMetadata> metadata;

#if defined(NOT_OPTIMIZE_PERFORMANCE)
    size_t paramsNum = 0;
#endif

    bool bodyPresence = false;
    bool conflict = false; /* Name is conflict with panda primitive types. Need special handle. */
    ark::panda_file::SourceLang language;

    explicit Record(std::string s, ark::panda_file::SourceLang lang, FileLocation &&fLoc)
        : name(std::move(s)),
          fileLocation {std::move(fLoc)},
          metadata(extensions::MetadataExtension::CreateRecordMetadata(lang)),
          language(lang)
    {
    }

    explicit Record(std::string s, ark::panda_file::SourceLang lang)
        : name(std::move(s)), metadata(extensions::MetadataExtension::CreateRecordMetadata(lang)), language(lang)
    {
    }

    bool HasImplementation() const
    {
        return !metadata->IsForeign();
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_RECORD_H
