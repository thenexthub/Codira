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

#include "collect_util.h"

namespace panda::libpandafile {

/**
 * processed_ids: The literal array ids are collected from field and ins, needn't process nest.
 * nest_unprocessed_ids: The literal array ids are collected from ins, need process nest.
 */
void CollectUtil::CollectLiteralArray(const panda_file::File &file_, std::unordered_set<uint32_t> &processed_ids)
{
    std::unordered_set<uint32_t> nest_unprocessed_ids;

    for (uint32_t id : file_.GetClasses()) {
        panda_file::File::EntityId class_id(id);
        if (file_.IsExternal(class_id)) {
            continue;
        }
        panda_file::ClassDataAccessor class_data_accessor(file_, class_id);
        CollectClassLiteralArray(class_data_accessor, processed_ids, nest_unprocessed_ids);
    }
    ProcessNestLiteralArray(file_, processed_ids, nest_unprocessed_ids);
}

void CollectUtil::CollectClassLiteralArray(panda_file::ClassDataAccessor &class_data_accessor,
                                           std::unordered_set<uint32_t> &processed_ids,
                                           std::unordered_set<uint32_t> &nest_unprocessed_ids)
{
    panda_file::File::StringData csd = class_data_accessor.GetName();
    const char *cn = utf::Mutf8AsCString(csd.data);
    class_data_accessor.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) -> void {
        panda_file::File::EntityId field_name_id = field_accessor.GetNameId();
        panda_file::File::StringData fsd = class_data_accessor.GetPandaFile().GetStringData(field_name_id);
        const char *fn = utf::Mutf8AsCString(fsd.data);
        if (std::strcmp(cn, ES_MODULE_RECORD.data()) != 0 &&
            std::strcmp(cn, ES_SCOPE_NAMES_RECORD.data()) != 0 &&
            std::strcmp(fn, SCOPE_NAMES.data()) != 0 &&
            std::strcmp(fn, MODULE_RECORD_IDX.data()) != 0) {
            return;
        }
        auto module_offset = field_accessor.GetValue<uint32_t>().value();
        processed_ids.emplace(module_offset);
    });
    class_data_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
        if (!method_accessor.GetCodeId().has_value()) {
            return;
        }
        panda_file::File::EntityId method_id = method_accessor.GetMethodId();
        panda_file::File::EntityId code_id = method_accessor.GetCodeId().value();
        panda_file::CodeDataAccessor code_data_accessor {class_data_accessor.GetPandaFile(), code_id};
        uint32_t ins_size_ = code_data_accessor.GetCodeSize();
        const uint8_t *ins_arr = code_data_accessor.GetInstructions();
        auto bc_ins = panda::BytecodeInst<BytecodeInstMode::FAST>(ins_arr);
        const auto bc_ins_last = bc_ins.JumpTo(ins_size_);
        while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
            if (!bc_ins.IsPrimaryOpcodeValid()) {
                LOG(FATAL, PANDAFILE) << "Fail to verify primary opcode!";
            }
            if (bc_ins.HasFlag(panda::BytecodeInst<BytecodeInstMode::FAST>::Flags::LITERALARRAY_ID)) {
                const auto literal_id =
                    GetLiteralArrayIdInBytecodeInst(class_data_accessor.GetPandaFile(), method_id, bc_ins);
                nest_unprocessed_ids.insert(literal_id.GetOffset());
            }
            bc_ins = bc_ins.GetNext();
        }
    });
}

void CollectUtil::ProcessNestLiteralArray(const panda_file::File &file_, std::unordered_set<uint32_t> &processed_ids,
                                          std::unordered_set<uint32_t> &nest_unprocessed_ids)
{
    if (nest_unprocessed_ids.empty()) {
        return;
    }

    panda_file::File::EntityId lit_array_invalid(panda_file::INVALID_OFFSET);
    panda_file::LiteralDataAccessor literal_data_accessor {file_, lit_array_invalid};
    while (!nest_unprocessed_ids.empty()) {
        auto nest_unprocess_id_iterator = nest_unprocessed_ids.begin();
        uint32_t nest_unprocess_id = *nest_unprocess_id_iterator;
        processed_ids.emplace(nest_unprocess_id);
        panda_file::File::EntityId nest_unprocess_id_entity_id(nest_unprocess_id);
        literal_data_accessor.EnumerateLiteralVals(
            nest_unprocess_id_entity_id,
            [processed_ids, &nest_unprocessed_ids](const panda_file::LiteralDataAccessor::LiteralValue &value,
                                                 const panda_file::LiteralTag &tag) {
                if (tag != panda_file::LiteralTag::LITERALARRAY) {
                    return;
                }
                uint32_t idx = std::get<uint32_t>(value);
                if ((processed_ids.find(idx) != processed_ids.end()) ||
                    (nest_unprocessed_ids.find(idx) != nest_unprocessed_ids.end())) {
                    return;
                }
                nest_unprocessed_ids.emplace(idx);
            });
        nest_unprocessed_ids.erase(nest_unprocess_id);
    }
}

panda_file::File::EntityId CollectUtil::GetLiteralArrayIdInBytecodeInst(
    const panda_file::File &file_, panda_file::File::EntityId method_id,
    panda::BytecodeInst<BytecodeInstMode::FAST> bc_ins)
{
    size_t idx = bc_ins.GetLiteralIndex();
    if (idx < 0) {
        LOG(FATAL, PANDAFILE) << "Fail to verify ID Index!";
    }
    const auto arg_literal_idx = bc_ins.GetId(idx).AsIndex();
    const auto literal_id = file_.ResolveMethodIndex(method_id, arg_literal_idx);
    return literal_id;
}

}  // namespace panda::libpandafile