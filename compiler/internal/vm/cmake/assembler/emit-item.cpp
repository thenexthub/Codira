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

#include "emit-item.h"

namespace panda::pandasm {
using panda::panda_file::MethodItem;

std::mutex EmitFunctionsJob::mutex_;

template <class T>
typename T::mapped_type Find(const T &map, typename T::key_type key)
{
    auto res = map.find(key);
    ASSERT(res != map.end());
    return res->second;
}

void EmitFunctionsQueue::Schedule()
{
    ASSERT(jobsCount_ == 0);
    std::unique_lock<std::mutex> lock(m_);

    for (auto prog : progs_) {
        auto *fileJob = new EmitFunctionsJob(items_, *prog, emit_debug_info_, entities_);
        jobs_.push_back(fileJob);
        jobsCount_++;
    }

    lock.unlock();
    jobsAvailable_.notify_all();
}

bool EmitFunctionsJob::Run()
{
    return EmitFunctions(entities_, emit_debug_info_);
}

bool EmitFunctionsJob::EmitFunctions(const panda::pandasm::AsmEmitter::AsmEntityCollections &entities,
    bool emit_debug_info)
{
    for (const auto &f : program_.function_table) {
        const auto &[name, func] = f;

        if (func.metadata->IsForeign()) {
            continue;
        }

        auto emitter = BytecodeEmitter {};
        auto *method = static_cast<MethodItem *>(Find(entities.method_items, name));
        if (!func.Emit(emitter, method, entities.method_items, entities.field_items, entities.class_items,
                       entities.string_items, entities.literalarray_items)) {
            std::unique_lock<std::mutex> lock(mutex_);
            AsmEmitter::SetLastError("Internal error during emitting function: " + func.name);
            return false;
        }

        auto *code = method->GetCode();
        code->SetNumVregs(func.regs_num);
        code->SetNumArgs(func.GetParamsNum());

        auto num_ins = static_cast<size_t>(
            std::count_if(func.ins.begin(), func.ins.end(), [](auto &it) { return it->opcode != Opcode::INVALID; }));
        code->SetNumInstructions(num_ins);

        auto *bytes = code->GetInstructions();
        auto status = emitter.Build(static_cast<std::vector<unsigned char> *>(bytes));
        if (status != BytecodeEmitter::ErrorCode::SUCCESS) {
            std::unique_lock<std::mutex> lock(mutex_);
            AsmEmitter::SetLastError("Internal error during emitting binary code, status=" +
                std::to_string(static_cast<int>(status)));
            return false;
        }
        auto try_blocks = func.BuildTryBlocks(method, entities.class_items, *bytes);
        for (auto &try_block : try_blocks) {
            code->AddTryBlock(try_block);
        }

        EmitDebugInfo(bytes, method, func, name, emit_debug_info);
    }
    return true;
}

void EmitFunctionsJob::EmitDebugInfo(const std::vector<uint8_t> *bytes, const panda_file::MethodItem *method,
    const Function &func, const std::string &name, bool emit_debug_info)
{
    auto *debug_info = method->GetDebugInfo();
    if (debug_info == nullptr) {
        return;
    }

    auto *line_number_program = debug_info->GetLineNumberProgram();
    auto *constant_pool = debug_info->GetConstantPool();

    std::string record_name = GetOwnerName(name);
    std::string record_source_file;
    if (!record_name.empty()) {
        auto &rec = program_.record_table.find(record_name)->second;
        record_source_file = rec.source_file;
    }

    if (!func.source_file.empty() && func.source_file != record_source_file) {
        if (!func.source_code.empty()) {
            auto *source_code_item = items_.GetStringItem(func.source_code);
            ASSERT(source_code_item->GetOffset() != 0);
            line_number_program->EmitSetSourceCode(constant_pool, source_code_item);
        }
        auto *source_file_item = items_.GetStringItem(func.source_file);
        ASSERT(source_file_item->GetOffset() != 0);
        line_number_program->EmitSetFile(constant_pool, source_file_item);
    }
    func.BuildLineNumberProgram(debug_info, *bytes, &items_, constant_pool, emit_debug_info);
}
}