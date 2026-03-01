/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "compiler_logger.h"
#include "pass_manager_statistics.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"

#include <iomanip>

namespace ark::compiler {
PassManagerStatistics::PassManagerStatistics(Graph *graph)
    : graph_(graph),
      passStatList_(graph->GetAllocator()->Adapter()),
      passStatStack_(graph->GetAllocator()->Adapter()),
      enableIrStat_(g_options.IsCompilerEnableIrStats())
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
    char spaceBuf[BUF_SIZE];
    std::fill(spaceBuf, spaceBuf + BUF_SIZE, ' ');
    size_t index = 0;
    out << std::dec
        << std::setw(OFFSET_ID) << std::right << "ID" << " " << std::left
        << std::setw(OFFSET_PASS_NAME) << "Pass Name" << ": " << std::right
        << std::setw(OFFSET_DEFAULT) << "IR mem" << std::right
        << std::setw(OFFSET_DEFAULT) << "Local mem" << std::right
        << std::setw(OFFSET_DEFAULT) << "Time,us" << std::endl;
    out << "-----------------------------------------------------------------------------\n";
    size_t total_time = 0;
    for (const auto& stat : passStatList_) {
        auto indent = stat.runDepth * OFFSET_STAT;
        spaceBuf[indent] = 0;
        out << std::setw(OFFSET_ID) << std::right << index << spaceBuf << " "
            << std::left << std::setw(OFFSET_PASS_NAME - indent) << stat.passName << ": "
            << std::right << std::setw(OFFSET_DEFAULT) << stat.memUsedIr << std::setw(OFFSET_DEFAULT)
            << stat.memUsedLocal << std::setw(OFFSET_DEFAULT) << stat.timeUs << std::endl;
        spaceBuf[indent] = ' ';
        index++;
        total_time += stat.timeUs;
    }
    out << "-----------------------------------------------------------------------------\n";
    out << std::setw(OFFSET_TOTAL) << "TOTAL: "
        << std::right << std::setw(OFFSET_DEFAULT) << graph_->GetAllocator()->GetAllocatedSize()
        << std::setw(OFFSET_DEFAULT) << graph_->GetLocalAllocator()->GetAllocatedSize()
        << std::setw(OFFSET_DEFAULT) << total_time << std::endl;
    out << "PBC instruction number : " << pbcInstNum_ << std::endl;
#endif
    // clang-format on
}

void PassManagerStatistics::ProcessBeforeRun(const Pass &pass)
{
    size_t allocatedSize = graph_->GetAllocator()->GetAllocatedSize();
    constexpr auto OFFSET_NORMAL = 2;
    std::string indent(passCallDepth_ * OFFSET_NORMAL, '.');
    COMPILER_LOG(DEBUG, PM) << "Run pass: " << indent << pass.GetPassName();

    if (!passStatStack_.empty()) {
        auto topPass = passStatStack_.top();
        ASSERT(allocatedSize >= lastAllocatedIr_);
        topPass->memUsedIr += allocatedSize - lastAllocatedIr_;
        if (!g_options.IsCompilerResetLocalAllocator()) {
            ASSERT(graph_->GetLocalAllocator()->GetAllocatedSize() >= lastAllocatedLocal_);
            topPass->memUsedLocal += graph_->GetLocalAllocator()->GetAllocatedSize() - lastAllocatedLocal_;
        }
        topPass->timeUs +=
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - lastTimestamp_)
                .count();
    }

    passStatList_.push_back({passCallDepth_, pass.GetPassName(), {0, 0}, {0, 0}, 0, 0, 0});
    if (enableIrStat_) {
        for (auto block : graph_->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            passStatList_.back().beforePass.numOfBasicblocks++;
            for ([[maybe_unused]] auto inst : block->Insts()) {
                passStatList_.back().beforePass.numOfInstructions++;
            }
        }
    }

    passStatStack_.push(&passStatList_.back());
    // Call `GetAllocator()->GetAllocatedSize()` again to exclude allocations caused by PassManagerStatistics itself:
    lastAllocatedIr_ = graph_->GetAllocator()->GetAllocatedSize();
    lastAllocatedLocal_ = graph_->GetLocalAllocator()->GetAllocatedSize();
    lastTimestamp_ = std::chrono::steady_clock::now();

    passCallDepth_++;
}

void PassManagerStatistics::ProcessAfterRun(size_t localMemUsed)
{
    auto topPass = passStatStack_.top();
    ASSERT(graph_->GetAllocator()->GetAllocatedSize() >= lastAllocatedIr_);
    topPass->memUsedIr += graph_->GetAllocator()->GetAllocatedSize() - lastAllocatedIr_;
    if (g_options.IsCompilerResetLocalAllocator()) {
        topPass->memUsedLocal = localMemUsed;
    } else {
        ASSERT(graph_->GetLocalAllocator()->GetAllocatedSize() >= lastAllocatedLocal_);
        topPass->memUsedLocal += graph_->GetLocalAllocator()->GetAllocatedSize() - lastAllocatedLocal_;
    }
    topPass->timeUs +=
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - lastTimestamp_)
            .count();

    if (enableIrStat_) {
        for (auto block : graph_->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            topPass->afterPass.numOfBasicblocks++;
            for ([[maybe_unused]] auto inst : block->Insts()) {
                topPass->afterPass.numOfInstructions++;
            }
        }
    }

    passStatStack_.pop();
    lastAllocatedIr_ = graph_->GetAllocator()->GetAllocatedSize();
    lastAllocatedLocal_ = graph_->GetLocalAllocator()->GetAllocatedSize();
    lastTimestamp_ = std::chrono::steady_clock::now();

    passCallDepth_--;
    passRunIndex_++;
}

void PassManagerStatistics::DumpStatisticsCsv(char sep) const
{
    ASSERT(g_options.WasSetCompilerDumpStatsCsv());
    static std::ofstream csv(g_options.GetCompilerDumpStatsCsv(), std::ofstream::trunc);
    auto mName = graph_->GetRuntime()->GetMethodFullName(graph_->GetMethod(), true);
    for (const auto &i : passStatList_) {
        csv << "\"" << mName << "\"" << sep;
        csv << i.passName << sep;
        csv << i.memUsedIr << sep;
        csv << i.memUsedLocal << sep;
        csv << i.timeUs << sep;
        if (enableIrStat_) {
            csv << i.beforePass.numOfBasicblocks << sep;
            csv << i.afterPass.numOfBasicblocks << sep;
            csv << i.beforePass.numOfInstructions << sep;
            csv << i.afterPass.numOfInstructions << sep;
        }
        csv << GetPbcInstNum();
        csv << '\n';
    }
    // Flush stream because it is declared `static`:
    csv << std::flush;
}
}  // namespace ark::compiler
