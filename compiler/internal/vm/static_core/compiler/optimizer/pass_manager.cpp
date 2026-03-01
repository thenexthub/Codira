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

#include <iomanip>
#include "pass_manager.h"
#include "compiler_logger.h"
#include "libarkbase/trace/trace.h"

#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/ir/visualizer_printer.h"

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/catch_inputs.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/analysis/live_registers.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/monitor_analysis.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/analysis/reg_alloc_verifier.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/types_analysis.h"
#include "optimizer/optimizations/cleanup.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENABLE_IR_DUMP

#ifdef ENABLE_IR_DUMP

#include <fstream>
#include <ctime>
#include "libarkbase/os/filesystem.h"
#endif  // ENABLE_IR_DUMP

namespace ark::compiler {
PassManager::PassManager(Graph *graph, PassManager *parentPm)
    : graph_(graph),
      optimizations_(graph->GetAllocator()->Adapter()),
      analyses_(details::PredefinedAnalyses::Instantiate<Analysis *>(graph_->GetAllocator(), graph_)),
      stats_((parentPm == nullptr) ? graph->GetAllocator()->New<PassManagerStatistics>(graph)
                                   : parentPm->GetStatistics())
{
}

#ifdef ENABLE_IR_DUMP
static std::string ClearFileName(std::string str, std::string_view suffix)
{
    std::string delimiters = "~`@#$%^&*()-+=\\|/\"<>;,.[]";
    for (const char &c : delimiters) {
        std::replace(str.begin(), str.end(), c, '_');
    }
    return str.substr(0, NAME_MAX - suffix.size());
}
#endif

std::string PassManager::GetFileName([[maybe_unused]] const char *passName, [[maybe_unused]] const std::string &suffix)
{
#ifdef ENABLE_IR_DUMP
    std::stringstream ssFilename;
    std::stringstream ssFullpath;
    ASSERT(GetGraph()->GetRuntime() != nullptr);

    const auto &folderName(g_options.GetCompilerDumpFolder());

    os::CreateDirectories(folderName);
    constexpr auto IMM_3 = 3;
    constexpr auto IMM_4 = 4;
    ssFilename << std::setw(IMM_3) << std::setfill('0') << executionCounter_ << "_";
    if (passName != nullptr) {
        ssFilename << "pass_" << std::setw(IMM_4) << std::setfill('0') << stats_->GetCurrentPassIndex() << "_";
    }
    if (GetGraph()->GetParentGraph() != nullptr) {
        ssFilename << "inlined_";
    }
    ssFilename << GetGraph()->GetRuntime()->GetClassNameFromMethod(GetGraph()->GetMethod()) << "_"
               << GetGraph()->GetRuntime()->GetMethodName(GetGraph()->GetMethod());
    if (GetGraph()->IsOsrMode()) {
        ssFilename << "_OSR";
    }
    if (passName != nullptr) {
        ssFilename << std::string(stats_->GetCurrentPassCallDepth() + 1, '_') << passName;
    }
    ssFullpath << folderName.c_str() << "/" << ClearFileName(ssFilename.str(), suffix) << suffix;
    return ssFullpath.str();
#else
    return "";
#endif  // ENABLE_IR_DUMP
}

static void CleanupChecksForDump(Graph *graph)
{
    for (auto *block : graph->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        for (auto inst : block->InstsSafe()) {
            if (inst->IsCheck()) {
                inst->ReplaceUsers(Inst::GetDataFlowInput(inst));
                block->RemoveInst(inst);
            }
        }
    }
}

void PassManager::DumpGraph([[maybe_unused]] const char *passName)
{
#ifdef ENABLE_IR_DUMP
    std::string fileName = GetFileName(passName, ".ir");
    std::ofstream strm(fileName);
    if (!strm.is_open()) {
        std::cerr << errno << " ERROR: " << strerror(errno) << "\n" << fileName << std::endl;
    }
    ASSERT(strm.is_open());
    if (g_options.IsCompilerDumpNoChecks()) {
        auto clone = GraphCloner(GetGraph(), GetAllocator(), GetLocalAllocator()).CloneGraph();
        ASSERT(clone != nullptr);
        clone->GetPassManager()->SetCheckMode(true);
        CleanupChecksForDump(clone);
        clone->Dump(&strm);
    } else {
        GetGraph()->Dump(&strm);
    }
#endif  // ENABLE_IR_DUMP
}
void PassManager::DumpLifeIntervals([[maybe_unused]] const char *passName)
{
#ifdef ENABLE_IR_DUMP
    if (!GetGraph()->IsAnalysisValid<LivenessAnalyzer>()) {
        return;
    }
    std::ofstream strm(GetFileName(passName, ".li"));
    if (!strm.is_open()) {
        std::cerr << errno << " ERROR: " << strerror(errno) << "\n" << GetFileName(passName, ".li") << std::endl;
    }

    ASSERT(strm.is_open());
    GetGraph()->GetAnalysis<LivenessAnalyzer>().DumpLifeIntervals(strm);
#endif  // ENABLE_IR_DUMP
}
void PassManager::InitialDumpVisualizerGraph()
{
#ifdef ENABLE_IR_DUMP
    std::ofstream strm(GetFileName());
    strm << "begin_compilation\n";
    strm << "  name \"" << GetGraph()->GetRuntime()->GetClassNameFromMethod(GetGraph()->GetMethod()) << "_"
         << GetGraph()->GetRuntime()->GetMethodName(GetGraph()->GetMethod()) << "\"\n";
    strm << "  method \"" << GetGraph()->GetRuntime()->GetClassNameFromMethod(GetGraph()->GetMethod()) << "_"
         << GetGraph()->GetRuntime()->GetMethodName(GetGraph()->GetMethod()) << "\"\n";
    strm << "  date " << std::time(nullptr) << "\n";
    strm << "end_compilation\n";
    strm.close();
#endif  // ENABLE_IR_DUMP
}

void PassManager::DumpVisualizerGraph([[maybe_unused]] const char *passName)
{
#ifdef ENABLE_IR_DUMP
    std::ofstream strm(GetFileName(), std::ios::app);
    VisualizerPrinter(GetGraph(), &strm, passName).Print();
    strm.close();
#endif  // ENABLE_IR_DUMP
}

bool PassManager::RunPass(Pass *pass, size_t localMemSizeBeforePass)
{
    if (pass->IsAnalysis() && pass->IsValid()) {
        return true;
    }

    if (!pass->IsAnalysis() && !static_cast<Optimization *>(pass)->IsEnable()) {
        return false;
    }

    if (!IsCheckMode()) {
        stats_->ProcessBeforeRun(*pass);
        if (firstExecution_ && GetGraph()->GetParentGraph() == nullptr) {
            StartExecution();
            firstExecution_ = false;
        }
    }

#ifndef NDEBUG
    if (g_options.IsCompilerEnableTracing()) {
        trace::BeginTracePoint(pass->GetPassName());
    }
#endif  // NDEBUG

    bool result = pass->Run();

#ifndef NDEBUG
    if (g_options.IsCompilerEnableTracing()) {
        trace::EndTracePoint();
    }
#endif  // NDEBUG

    if (!IsCheckMode()) {
        ASSERT(graph_->GetLocalAllocator()->GetAllocatedSize() >= localMemSizeBeforePass);
        stats_->ProcessAfterRun(graph_->GetLocalAllocator()->GetAllocatedSize() - localMemSizeBeforePass);
    }

    if (pass->IsAnalysis()) {
        pass->SetValid(result);
    }
    bool isCodegen = std::string("Codegen") == pass->GetPassName();
    if (g_options.IsCompilerDump() && pass->ShouldDump() && !IsCheckMode()) {
        if (!g_options.IsCompilerDumpFinal() || isCodegen) {
            DumpGraph(pass->GetPassName());
        }
    }

    if (g_options.IsCompilerVisualizerDump() && pass->ShouldDump()) {
        DumpVisualizerGraph(pass->GetPassName());
    }

    result &= RunPassChecker(pass, result, isCodegen);

    return result;
}

bool PassManager::RunPassChecker(Pass *pass, bool result, bool isCodegen)
{
    if (graph_->IsAbcKit()) {
        return RunPassChecker<true>(pass, result, isCodegen);
    }
#ifndef NDEBUG
    RunPassChecker<false>(pass, result, isCodegen);
#endif
    return true;
}

template <bool FORCE_RUN>
bool PassManager::RunPassChecker(Pass *pass, bool result, bool isCodegen)
{
    bool checkerEnabled = g_options.IsCompilerCheckGraph();
    if (g_options.IsCompilerCheckFinal()) {
        checkerEnabled = isCodegen;
    }
    if constexpr (FORCE_RUN) {
        checkerEnabled = true;
    }
    if (result && !pass->IsAnalysis() && checkerEnabled) {
        result &= GraphChecker(graph_, pass->GetPassName()).Check();
    }
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
    if (g_options.IsCompilerPrintStats()) {
        stats_->PrintStatistics();
    }
    if (g_options.WasSetCompilerDumpStatsCsv()) {
        stats_->DumpStatisticsCsv();
    }
}
}  // namespace ark::compiler
