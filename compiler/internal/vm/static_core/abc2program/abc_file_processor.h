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

#ifndef ABC2PROGRAM_ABC_FILE_PROCESSOR_H
#define ABC2PROGRAM_ABC_FILE_PROCESSOR_H

#include <libarkfile/include/source_lang_enum.h>
#include "libarkfile/value.h"
#include "abc2program_key_data.h"
#include "libarkfile/annotation_data_accessor.h"

namespace ark::abc2program {

using AnnotationList = std::vector<std::pair<std::string, std::string>>;

struct RecordAnnotations {
    AnnotationList annList;
    std::map<std::string, AnnotationList> fieldAnnotations;
};

struct ProgAnnotations {
    std::map<std::string, AnnotationList> methodAnnotations;
    std::map<std::string, RecordAnnotations> recordAnnotations;
};

class AbcFileProcessor {
public:
    explicit AbcFileProcessor(Abc2ProgramKeyData &keyData);
    bool ProcessFile();

private:
#include "abc2program_plugins.inc"
    ark::panda_file::SourceLang GetRecordLanguage(panda_file::File::EntityId classId) const;
    std::string ScalarValueToString(const panda_file::ScalarValue &value, const std::string &type);
    std::string ArrayValueToString(const panda_file::ArrayValue &value, const std::string &type, const size_t idx);
    std::string AnnotationTagToString(const char tag) const;
    void FillExternalFieldsToRecords();
    void FillLiteralArrays();
    void ProcessClasses();
    void FillProgramStrings();
    void GetLanguageSpecificMetadata();
    Abc2ProgramKeyData &keyData_;
    const panda_file::File *file_ = nullptr;
    AbcStringTable *stringTable_ = nullptr;
    pandasm::Program *program_ = nullptr;
};

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC_FILE_PROCESSOR_H
