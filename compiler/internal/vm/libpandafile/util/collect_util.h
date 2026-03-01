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

#ifndef LIBPANDAFILE_UTIL_COLLECT_UTIL_H
#define LIBPANDAFILE_UTIL_COLLECT_UTIL_H

#include <string>
#include <unordered_set>
#include "libpandafile/bytecode_instruction-inl.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/literal_data_accessor-inl.h"

namespace panda::libpandafile {
class CollectUtil {
public:
    void CollectLiteralArray(const panda_file::File &file, std::unordered_set<uint32_t> &processed_ids);

private:
    void CollectClassLiteralArray(panda_file::ClassDataAccessor &class_data_accessor,
                                  std::unordered_set<uint32_t> &processed_ids,
                                  std::unordered_set<uint32_t> &nest_unprocess_ids);
    void ProcessNestLiteralArray(const panda_file::File &file_, std::unordered_set<uint32_t> &processed_ids,
                                 std::unordered_set<uint32_t> &nest_unprocess_ids);
    panda_file::File::EntityId GetLiteralArrayIdInBytecodeInst(const panda_file::File &file,
                                                               panda_file::File::EntityId method_id,
                                                               panda::BytecodeInst<BytecodeInstMode::FAST> bc_ins);
    static constexpr std::string_view ES_MODULE_RECORD = "_ESModuleRecord;";
    static constexpr std::string_view ES_SCOPE_NAMES_RECORD = "_ESScopeNamesRecord";
    static constexpr std::string_view SCOPE_NAMES = "scopeNames";
    static constexpr std::string_view MODULE_RECORD_IDX = "moduleRecordIdx";
    static constexpr std::string_view MODULE_REQUEST_PHASE_IDX = "moduleRequestPhaseIdx";
};
}  // namespace panda::libpandafile
#endif