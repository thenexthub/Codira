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

#include "trace/trace.h"
#include "pass_manager.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/optimizations/cleanup.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENABLE_IR_DUMP

namespace panda::compiler {
PassManager::PassManager(Graph *graph, PassManager *parent_pm)
    : graph_(graph),
      optimizations_(graph->GetAllocator()->Adapter()),
      ANALYSES(details::PredefinedAnalyses::Instantiate<Analysis *>(graph_->GetAllocator(), graph_)),
      stats_((parent_pm == nullptr) ? graph->GetAllocator()->New<PassManagerStatistics>(graph)
                                    : parent_pm->GetStatistics())
{
}

#if defined(ENABLE_IR_DUMP) && !defined(PANDA_TARGET_MACOS)
static std::string ClearFileName(std::string str, std::string_view suffix)
{
    std::string delimiters = "~`@#$%^&*()-+=\\|/\"<>;,.[]";
    for (const char &c : delimiters) {
        std::replace(str.begin(), str.end(), c, '_');
    }
    return str.substr(0, NAME_MAX - suffix.size());
}
#endif  // ENABLE_IR_DUMP && !PANDA_TARGET_MACOS

std::string PassManager::GetFileName([[maybe_unused]] const char *pass_name, [[maybe_unused]] const std::string &suffix)
{
#if defined(ENABLE_IR_DUMP) && !defined(PANDA_TARGET_MACOS)
    std::stringstream ss_filename;
    std::stringstream ss_fullpath;
    ASSERT(GetGraph()->GetRuntime() != nullptr);

    std::string folder_name(options.GetCompilerDumpFolder());

    os::CreateDirectories(folder_name);
    constexpr auto IMM_3 = 3;
    constexpr auto IMM_4 = 4;
    ss_filename << std::setw(IMM_3) << std::setfill('0') << execution_counter << "_";
    if (pass_name != nullptr) {
        ss_filename << "pass_" << std::setw(IMM_4) << std::setfill('0') << stats_->GetCurrentPassIndex() << "_";
    }
    if (GetGraph()->GetParentGraph() != nullptr) {
        ss_filename << "inlined_";
    }
    ss_filename << GetGraph()->GetRuntime()->GetClassNameFromMethod(GetGraph()->GetMethod()) << "_"
                << GetGraph()->GetRuntime()->GetMethodName(GetGraph()->GetMethod());
    if (GetGraph()->IsOsrMode()) {
        ss_filename << "_OSR";
    }
    if (pass_name != nullptr) {
        ss_filename << "_" << pass_name;
    }
    ss_fullpath << folder_name.c_str() << "/" << ClearFileName(ss_filename.str(), suffix) << suffix;
    return ss_fullpath.str();
#else
    return "";
#endif  // ENABLE_IR_DUMP && !PANDA_TARGET_MACOS
}
void PassManager::DumpGraph([[maybe_unused]] const char *pass_name)
{
#if defined(ENABLE_IR_DUMP) && !defined(PANDA_TARGET_MACOS)
    std::ofstream strm(GetFileName(pass_name, ".ir"));
    if (!strm.is_open()) {
        std::cerr << errno << " ERROR: " << strerror(errno) << "\n" << GetFileName(pass_name, ".ir") << std::endl;
    }
    ASSERT(strm.is_open());
    GetGraph()->Dump(&strm);
#endif  // ENABLE_IR_DUMP && !PANDA_TARGET_MACOS
}
void PassManager::DumpLifeIntervals([[maybe_unused]] const char *pass_name)
{
#if defined(ENABLE_IR_DUMP) && !defined(PANDA_TARGET_MACOS)
    if (!GetGraph()->IsAnalysisValid<LivenessAnalyzer>()) {
        return;
    }
    std::ofstream strm(GetFileName(pass_name, ".li"));
    if (!strm.is_open()) {
        std::cerr << errno << " ERROR: " << strerror(errno) << "\n" << GetFileName(pass_name, ".li") << std::endl;
    }

    ASSERT(strm.is_open());
    GetGraph()->GetAnalysis<LivenessAnalyzer>().DumpLifeIntervals(strm);
#endif  // ENABLE_IR_DUMP && !PANDA_TARGET_MACOS
}

bool PassManager::RunPass(Pass *pass, size_t local_mem_size_before_pass)
{
    if (pass->IsAnalysis() && pass->IsValid()) {
        return true;
    }

    if (!pass->IsAnalysis() && !static_cast<Optimization *>(pass)->IsEnable()) {
        return false;
    }

    if (!IsCheckMode()) {
        stats_->ProcessBeforeRun(*pass);
        if (first_execution_ && GetGraph()->GetParentGraph() == nullptr) {
            StartExecution();
            first_execution_ = false;
        }
    }

#ifndef NDEBUG
    if (options.IsCompilerEnableTracing()) {
        trace::BeginTracePoint(pass->GetPassName());
    }
#endif  // NDEBUG

    bool result = pass->Run();

#ifndef NDEBUG
    if (options.IsCompilerEnableTracing()) {
        trace::EndTracePoint();
    }
#endif  // NDEBUG

    if (!IsCheckMode()) {
        ASSERT(graph_->GetLocalAllocator()->GetAllocatedSize() >= local_mem_size_before_pass);
        stats_->ProcessAfterRun(graph_->GetLocalAllocator()->GetAllocatedSize() - local_mem_size_before_pass);
    }

    if (pass->IsAnalysis()) {
        pass->SetValid(result);
    }
    if (options.IsCompilerDump() && pass->ShouldDump() && !IsCheckMode()) {
        if (!options.IsCompilerDumpFinal()) {
            DumpGraph(pass->GetPassName());
        }
    }

#ifndef NDEBUG
    bool checker_enabled = options.IsCompilerCheckGraph();
    if (options.IsCompilerCheckFinal()) {
        checker_enabled = false;
    }
    if (result && !pass->IsAnalysis() && checker_enabled) {
        GraphChecker(graph_).Check();
    }
#endif
    return result;
}

ArenaAllocator *PassManager::GetAllocator()
{
    return graph_->GetAllocator();
}

ArenaAllocator *PassManager::GetLocalAllocator()
{
    return graph_->GetLocalAllocator();
}

void PassManager::Finalize() const
{
    if (options.IsCompilerPrintStats()) {
        stats_->PrintStatistics();
    }
    if (options.WasSetCompilerDumpStatsCsv()) {
        stats_->DumpStatisticsCsv();
    }
}
}  // namespace panda::compiler
