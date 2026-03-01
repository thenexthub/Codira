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

#include "compiler_logger.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"
#include "pass_manager_statistics.h"

namespace panda::compiler {
PassManagerStatistics::PassManagerStatistics(Graph *graph)
    : graph_(graph),
      pass_stat_list_(graph->GetAllocator()->Adapter()),
      pass_stat_stack_(graph->GetAllocator()->Adapter()),
      enable_ir_stat_(options.IsCompilerEnableIrStats())
{
}

void PassManagerStatistics::PrintStatistics() const
{
    // clang-format off
#ifndef __clang_analyzer__
    auto& out = std::cerr;
    static constexpr size_t BUF_SIZE = 64;
    static constexpr auto OFFSET_STAT = 2;
    static constexpr auto OFFSET_ID = 4;
    static constexpr auto OFFSET_DEFAULT = 12;
    static constexpr auto OFFSET_PASS_NAME = 34;
    static constexpr auto OFFSET_TOTAL = 41;
    char space_buf[BUF_SIZE];
    std::fill(space_buf, space_buf + BUF_SIZE, ' ');
    size_t index = 0;
    out << std::dec
        << std::setw(OFFSET_ID) << std::right << "ID" << " " << std::left
        << std::setw(OFFSET_PASS_NAME) << "Pass Name" << ": " << std::right
        << std::setw(OFFSET_DEFAULT) << "IR mem" << std::right
        << std::setw(OFFSET_DEFAULT) << "Local mem" << std::right
        << std::setw(OFFSET_DEFAULT) << "Time,us" << std::endl;
    out << "-----------------------------------------------------------------------------\n";
    size_t total_time = 0;
    for (const auto& stat : pass_stat_list_) {
        auto indent = stat.run_depth * OFFSET_STAT;
        space_buf[indent] = 0;
        out << std::setw(OFFSET_ID) << std::right << index << space_buf << " " << std::left
            << std::setw(OFFSET_PASS_NAME - indent) << stat.pass_name << ": " << std::right
            << std::setw(OFFSET_DEFAULT) << stat.mem_used_ir << std::setw(OFFSET_DEFAULT)
            << stat.mem_used_local << std::setw(OFFSET_DEFAULT) << stat.time_us << std::endl;
        space_buf[indent] = ' ';
        index++;
        total_time += stat.time_us;
    }
    out << "-----------------------------------------------------------------------------\n";
    out << std::setw(OFFSET_TOTAL) << "TOTAL: "
        << std::right << std::setw(OFFSET_DEFAULT) << graph_->GetAllocator()->GetAllocatedSize()
        << std::setw(OFFSET_DEFAULT) << graph_->GetLocalAllocator()->GetAllocatedSize()
        << std::setw(OFFSET_DEFAULT) << total_time << std::endl;
    out << "PBC instruction number : " << pbc_inst_num_ << std::endl;
#endif
    // clang-format on
}

void PassManagerStatistics::ProcessBeforeRun(const Pass &pass)
{
    size_t allocated_size = graph_->GetAllocator()->GetAllocatedSize();
    constexpr auto OFFSET_NORMAL = 2;
    std::string indent(pass_call_depth_ * OFFSET_NORMAL, '.');
    COMPILER_LOG(DEBUG, PM) << "Run pass: " << indent << pass.GetPassName();

    if (!pass_stat_stack_.empty()) {
        auto top_pass = pass_stat_stack_.top();
        ASSERT(allocated_size >= last_allocated_ir_);
        ASSERT(top_pass != nullptr);
        top_pass->mem_used_ir += allocated_size - last_allocated_ir_;
        if (!options.IsCompilerResetLocalAllocator()) {
            ASSERT(graph_->GetLocalAllocator()->GetAllocatedSize() >= last_allocated_local_);
            top_pass->mem_used_local += graph_->GetLocalAllocator()->GetAllocatedSize() - last_allocated_local_;
        }
        top_pass->time_us +=
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - last_timestamp_)
                .count();
    }

    pass_stat_list_.push_back({pass_call_depth_, pass.GetPassName(), {0, 0}, {0, 0}, 0, 0, 0});
    if (enable_ir_stat_) {
        for (auto block : graph_->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            pass_stat_list_.back().before_pass.num_of_basicblocks++;
            for ([[maybe_unused]] auto inst : block->Insts()) {
                pass_stat_list_.back().before_pass.num_of_instructions++;
            }
        }
    }

    pass_stat_stack_.push(&pass_stat_list_.back());
    // Call `GetAllocator()->GetAllocatedSize()` again to exclude allocations caused by PassManagerStatistics itself:
    last_allocated_ir_ = graph_->GetAllocator()->GetAllocatedSize();
    last_allocated_local_ = graph_->GetLocalAllocator()->GetAllocatedSize();
    last_timestamp_ = std::chrono::steady_clock::now();

    pass_call_depth_++;
}

void PassManagerStatistics::ProcessAfterRun(size_t local_mem_used)
{
    auto top_pass = pass_stat_stack_.top();
    ASSERT(graph_->GetAllocator()->GetAllocatedSize() >= last_allocated_ir_);
    top_pass->mem_used_ir += graph_->GetAllocator()->GetAllocatedSize() - last_allocated_ir_;
    if (options.IsCompilerResetLocalAllocator()) {
        top_pass->mem_used_local = local_mem_used;
    } else {
        ASSERT(graph_->GetLocalAllocator()->GetAllocatedSize() >= last_allocated_local_);
        top_pass->mem_used_local += graph_->GetLocalAllocator()->GetAllocatedSize() - last_allocated_local_;
    }
    top_pass->time_us +=
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - last_timestamp_)
            .count();

    if (enable_ir_stat_) {
        for (auto block : graph_->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            top_pass->after_pass.num_of_basicblocks++;
            for ([[maybe_unused]] auto inst : block->Insts()) {
                top_pass->after_pass.num_of_instructions++;
            }
        }
    }

    pass_stat_stack_.pop();
    last_allocated_ir_ = graph_->GetAllocator()->GetAllocatedSize();
    last_allocated_local_ = graph_->GetLocalAllocator()->GetAllocatedSize();
    last_timestamp_ = std::chrono::steady_clock::now();

    pass_call_depth_--;
    pass_run_index_++;
}

void PassManagerStatistics::DumpStatisticsCsv(char sep) const
{
    ASSERT(options.WasSetCompilerDumpStatsCsv());
    static std::ofstream csv(options.GetCompilerDumpStatsCsv(), std::ofstream::trunc);
    auto m_name = graph_->GetRuntime()->GetMethodFullName(graph_->GetMethod(), true);
    for (const auto &i : pass_stat_list_) {
        csv << "\"" << m_name << "\"" << sep;
        csv << i.pass_name << sep;
        csv << i.mem_used_ir << sep;
        csv << i.mem_used_local << sep;
        csv << i.time_us << sep;
        if (enable_ir_stat_) {
            csv << i.before_pass.num_of_basicblocks << sep;
            csv << i.after_pass.num_of_basicblocks << sep;
            csv << i.before_pass.num_of_instructions << sep;
            csv << i.after_pass.num_of_instructions << sep;
        }
        csv << GetPbcInstNum();
        csv << '\n';
    }
    // Flush stream because it is declared `static`:
    csv << std::flush;
}
}  // namespace panda::compiler
