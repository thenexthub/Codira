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

#ifndef ASSEMBLER_ASSEMBLY_RECORD_H
#define ASSEMBLER_ASSEMBLY_RECORD_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "assembly-field.h"
#include "extensions/extensions.h"
#include "ide_helpers.h"

namespace panda::pandasm {

struct Record {
    std::string name = "";
    bool conflict = false; /* Name is conflict with panda primitive types. Need special handle. */
    panda::panda_file::SourceLang language;
    std::unique_ptr<RecordMetadata> metadata;
    std::vector<Field> field_list; /* class fields list */
    size_t params_num = 0;
    bool body_presence = false;
    SourceLocation body_location;
    std::string source_file; /* The file in which the record is defined or empty */
    std::optional<FileLocation> file_location;

    Record(std::string s, panda::panda_file::SourceLang lang, size_t b_l, size_t b_r, std::string f_c, bool d,
           size_t l_n)
        : name(std::move(s)),
          language(lang),
          metadata(extensions::MetadataExtension::CreateRecordMetadata(lang)),
          file_location({f_c, b_l, b_r, l_n, d})
    {
        metadata->SetAccessFlags(panda::ACC_PUBLIC);
    }

    Record(std::string s, panda::panda_file::SourceLang lang)
        : name(std::move(s)), language(lang), metadata(extensions::MetadataExtension::CreateRecordMetadata(lang))
    {
        metadata->SetAccessFlags(panda::ACC_PUBLIC);
    }

    bool HasImplementation() const
    {
        return !metadata->IsForeign();
    }
};

}  // namespace panda::pandasm

#endif  // ASSEMBLER_ASSEMBLY_RECORD_H
