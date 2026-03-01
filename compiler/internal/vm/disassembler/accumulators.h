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

#ifndef DISASSEMBLER_ACCUMULATORS_H
#define DISASSEMBLER_ACCUMULATORS_H

#include <map>
#include <string>
#include <vector>

#include "libpandafile/debug_info_extractor.h"

namespace panda::disasm {
using LabelTable = std::map<size_t, std::string>;
using IdList = std::vector<panda::panda_file::File::EntityId>;

struct MethodInfo {
    std::string method_info;

    std::vector<std::string> instructions_info;

    panda_file::LineNumberTable line_number_table;

    panda_file::ColumnNumberTable column_number_table;

    panda_file::LocalVariableTable local_variable_table;
};

struct RecordInfo {
    std::string record_info;

    std::vector<std::string> fields_info;
};

struct ProgInfo {
    std::map<std::string, RecordInfo> records_info;
    std::map<std::string, MethodInfo> methods_info;
};

using AnnotationList = std::vector<std::pair<std::string, std::string>>;

struct RecordAnnotations {
    AnnotationList ann_list;
    std::map<std::string, AnnotationList> field_annotations;
};

struct ProgAnnotations {
    std::map<std::string, AnnotationList> method_annotations;
    std::map<std::string, RecordAnnotations> record_annotations;
};
}  // namespace panda::disasm

#endif  // DISASSEMBLER_ACCUMULATORS_H
