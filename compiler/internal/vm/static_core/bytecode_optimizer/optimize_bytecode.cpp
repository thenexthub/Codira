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

#include "optimize_bytecode.h"

#include "assembler/assembly-emitter.h"
#include "assembler/extensions/extensions.h"
#include "libarkfile/bytecode_instruction.h"
#include "bytecodeopt_options.h"
#include "bytecodeopt_peepholes.h"
#include "call_devirtualization.h"
#include "canonicalization.h"
#include "check_resolver.h"
#include "codegen.h"
#include "common.h"
#include "const_array_resolver.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir_builder/ir_builder.h"
#include "compiler/optimizer/ir_builder/pbc_iterator.h"
#include "compiler/optimizer/optimizations/branch_elimination.h"
#include "compiler/optimizer/optimizations/code_sink.h"
#include "compiler/optimizer/optimizations/cse.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/if_merging.h"
#include "compiler/optimizer/optimizations/licm.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/lse.h"
#include "compiler/optimizer/optimizations/move_constants.h"
#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc.h"
#include "compiler/optimizer/optimizations/simplify_string_builder.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/method_data_accessor.h"
#include "reg_acc_alloc.h"
#include "reg_encoder.h"

#include "generated/runtime_adapter_inc.h"

#include <regex>

namespace ark::bytecodeopt {
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
ark::bytecodeopt::Options g_options("");

template <typename T>
constexpr void RunOpts(compiler::Graph *graph, [[maybe_unused]] BytecodeOptIrInterface *iface)
{
    graph->RunPass<compiler::Cleanup>(false);
#ifndef NDEBUG
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (std::is_same_v<T, compiler::Lowering>) {
        graph->SetLowLevelInstructionsEnabled();
    }
#endif  // NDEBUG
        // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
    if constexpr (std::is_same_v<T, ConstArrayResolver>) {
        graph->RunPass<ConstArrayResolver>(iface);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else if constexpr (std::is_same_v<T, compiler::SimplifyStringBuilder>) {
        if (NeedToRunSimplifyStringBuilder(graph->GetLanguage())) {
            graph->RunPass<compiler::SimplifyStringBuilder>();
        }  // else skip pass
    } else {
        graph->RunPass<T>();
    }
}

template <typename First, typename Second, typename... Rest>
constexpr void RunOpts(compiler::Graph *graph, BytecodeOptIrInterface *iface = nullptr)
{
    RunOpts<First>(graph, iface);
    RunOpts<Second, Rest...>(graph, iface);
}

bool RunOptimizations(compiler::Graph *graph, BytecodeOptIrInterface *iface)
{
    constexpr int OPT_LEVEL_0 = 0;
    constexpr int OPT_LEVEL_1 = 1;
    constexpr int OPT_LEVEL_2 = 2;

    if (ark::bytecodeopt::g_options.GetOptLevel() == OPT_LEVEL_0) {
        return false;
    }

    graph->RunPass<CheckResolver>();
    graph->RunPass<compiler::Cleanup>(false);
    // NB! Canonicalization and compiler::Lowering should be present in all levels
    // since without Lowering pass, RegEncoder will not work for Compare instructions,
    // and we will not be able to optimize functions where there is branching.
    // Lowering can't work without Canonicalization pass.
    if (graph->IsDynamicMethod()) {
        RunOpts<compiler::ValNum, compiler::Lowering, compiler::MoveConstants>(graph);
    } else if (ark::bytecodeopt::g_options.GetOptLevel() == OPT_LEVEL_1) {
        RunOpts<compiler::Peepholes, Canonicalization, compiler::Lowering, compiler::MoveConstants,
                BytecodeOptPeepholes>(graph);
    } else if (ark::bytecodeopt::g_options.GetOptLevel() == OPT_LEVEL_2) {
        // ConstArrayResolver Pass is disabled as it requires fixes for stability
        RunOpts<ConstArrayResolver, compiler::BranchElimination, compiler::SimplifyStringBuilder, compiler::ValNum,
                compiler::IfMerging, compiler::Cse, compiler::Peepholes, CallDevirtualization, compiler::Licm,
                compiler::Lse, compiler::ValNum, compiler::Cse, compiler::BranchElimination, Canonicalization,
                compiler::Lowering, compiler::MoveConstants, BytecodeOptPeepholes>(graph, iface);
    } else {
        UNREACHABLE();
    }

    // this pass should run just before register allocator
    graph->RunPass<compiler::Cleanup>(false);
    graph->RunPass<RegAccAlloc>();

    graph->RunPass<compiler::Cleanup>(false);
    if (!RegAlloc(graph)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Failed compiler::RegAlloc";
        return false;
    }

    if (!graph->RunPass<RegEncoder>()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Failed RegEncoder";
        return false;
    }

    return true;
}

void BuildMapFromPcToIns(pandasm::Function &function, BytecodeOptIrInterface &irInterface, const compiler::Graph *graph,
                         compiler::RuntimeInterface::MethodPtr methodPtr)
{
    function.localVariableDebug.clear();
    auto *pcInsMap = irInterface.GetPcInsMap();
    pcInsMap->reserve(function.ins.size());
    auto instructionsBuf = graph->GetRuntime()->GetMethodCode(methodPtr);
    compiler::BytecodeInstructions instructions(instructionsBuf, graph->GetRuntime()->GetMethodCodeSize(methodPtr));
    size_t idx = 0;
    for (auto insn : instructions) {
        pandasm::Ins &ins = function.ins[idx++];
        pcInsMap->emplace(instructions.GetPc(insn), &ins);
        if (idx >= function.ins.size()) {
            break;
        }
    }
}

static void ColumnNumberPropagate(pandasm::Function *function)
{
    auto &insVec = function->ins;
    uint32_t cn = compiler::INVALID_COLUMN_NUM;
    // handle the instructions that are at the beginning of code but do not have column number
    size_t k = 0;
    while (k < insVec.size() && cn == compiler::INVALID_COLUMN_NUM) {
        cn = insVec[k++].insDebug.ColumnNumber();
    }
    if (cn == compiler::INVALID_COLUMN_NUM) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "Failed ColumnNumberPropagate: All insts have invalid column number";
        return;
    }
    for (size_t j = 0; j < k - 1L; j++) {
        insVec[j].insDebug.SetColumnNumber(cn);
    }

    // handle other instructions that do not have column number
    for (; k < insVec.size(); k++) {
        if (insVec[k].insDebug.ColumnNumber() != compiler::INVALID_COLUMN_NUM) {
            cn = insVec[k].insDebug.ColumnNumber();
        } else {
            insVec[k].insDebug.SetColumnNumber(cn);
        }
    }
}

static void LineNumberPropagate(pandasm::Function *function)
{
    if (function == nullptr || function->ins.empty()) {
        return;
    }
    std::uint32_t ln = 0;
    auto &insVec = function->ins;

    // handle the instructions that are at the beginning of code but do not have line number
    size_t i = 0;
    while (i < insVec.size() && ln == 0U) {
        ln = insVec[i++].insDebug.LineNumber();
    }
    if (ln == 0U) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "Failed LineNumberPropagate: All insts have invalid line number";
        return;
    }
    for (size_t j = 0; j < i - 1L; j++) {
        insVec[j].insDebug.SetLineNumber(ln);
    }

    // handle other instructions that do not have line number
    for (; i < insVec.size(); i++) {
        if (insVec[i].insDebug.LineNumber() != 0U) {
            ln = insVec[i].insDebug.LineNumber();
        } else {
            insVec[i].insDebug.SetLineNumber(ln);
        }
    }
}

static void DebugInfoPropagate(pandasm::Function &function, const compiler::Graph *graph,
                               BytecodeOptIrInterface &irInterface)
{
    LineNumberPropagate(&function);
    if (graph->IsDynamicMethod()) {
        ColumnNumberPropagate(&function);
    }
    irInterface.ClearPcInsMap();
}

static bool SkipFunction(const pandasm::Function &function, const std::string &funcName)
{
    if (ark::bytecodeopt::g_options.WasSetMethodRegex()) {
        static std::regex rgx(ark::bytecodeopt::g_options.GetMethodRegex());
        if (!std::regex_match(funcName, rgx)) {
            LOG(INFO, BYTECODE_OPTIMIZER) << "Skip Function " << funcName << ":Function's name doesn't match regex";
            return true;
        }
    }

    if (ark::bytecodeopt::g_options.IsSkipMethodsWithEh() && !function.catchBlocks.empty()) {
        LOG(INFO, BYTECODE_OPTIMIZER) << "Was not optimized " << funcName << ":Function has catch blocks";
        return true;
    }

    if ((function.GetTotalRegs() + function.GetParamsNum()) > compiler::VIRTUAL_FRAME_SIZE) {
        LOG(WARNING, BYTECODE_OPTIMIZER) << "Unable to optimize " << funcName
                                         << ": Function frame size is larger than allowed one";
        return true;
    }
    return false;
}

static void SetCompilerOptions(bool isDynamic)
{
    compiler::g_options.SetCompilerUseSafepoint(false);
    compiler::g_options.SetCompilerSupportInitObjectInst(true);
    if (!compiler::g_options.WasSetCompilerMaxBytecodeSize()) {
        compiler::g_options.SetCompilerMaxBytecodeSize(~0U);
    }
    if (isDynamic) {
        ark::bytecodeopt::g_options.SetSkipMethodsWithEh(true);
    }
}

// CC-OFFNXT(huge_method) solid logic
bool OptimizeFunction(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const panda_file::MethodDataAccessor &mda, bool isDynamic, SourceLanguage lang)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator localAllocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

    SetCompilerOptions(isDynamic);

    auto irInterface = BytecodeOptIrInterface(maps, prog);

    auto funcName = irInterface.GetMethodIdByOffset(mda.GetMethodId().GetOffset());
    LOG(INFO, BYTECODE_OPTIMIZER) << "Optimizing function: " << funcName;

    auto it = mda.IsStatic() ? prog->functionStaticTable.find(funcName) : prog->functionInstanceTable.find(funcName);
    if ((mda.IsStatic() && it == prog->functionStaticTable.end()) ||
        (!mda.IsStatic() && it == prog->functionInstanceTable.end())) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Cannot find function: " << funcName;
        return false;
    }
    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());

    auto adapter = CreateBytecodeOptimizerRuntimeAdapter(mda.GetPandaFile(), allocator, lang);
    auto graphArgs = compiler::Graph::GraphArgs {&allocator, &localAllocator, Arch::NONE, methodPtr, adapter};
    auto graph = allocator.New<compiler::Graph>(graphArgs, nullptr, false, isDynamic, true);
    if (graph == nullptr) {
        return false;
    }
    graph->SetLanguage(lang);

    ark::pandasm::Function &function = it->second;

    if (SkipFunction(function, funcName)) {
        return false;
    }

    // build map from pc to pandasm::ins (to re-build line-number info in BytecodeGen)
    BuildMapFromPcToIns(function, irInterface, graph, methodPtr);

    if (!graph->RunPass<ark::compiler::IrBuilder>()) {
        LOG(WARNING, BYTECODE_OPTIMIZER) << "Optimizing " << funcName << ": IR builder failed!";
        return false;
    }

    if (graph->HasIrreducibleLoop()) {
        // RegAlloc doesn't play well with such graphs
        LOG(WARNING, BYTECODE_OPTIMIZER) << "Optimizing " << funcName << ": Graph has irreducible loop!";
        return false;
    }

    if (!RunOptimizations(graph, &irInterface)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << funcName << ": Running optimizations failed!";
        return false;
    }

    if (!graph->RunPass<BytecodeGen>(&function, &irInterface)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << funcName << ": Code generation failed!";
        return false;
    }

    DebugInfoPropagate(function, graph, irInterface);

    function.valueOfFirstParam = static_cast<int64_t>(graph->GetStackSlotsCount()) - 1L;  // Work-around promotion rules
    function.regsNum = static_cast<std::uint32_t>(function.valueOfFirstParam + 1);

    if (auto frameSize = function.GetTotalRegs() + function.GetParamsNum(); frameSize >= NUM_COMPACTLY_ENCODED_REGS) {
        LOG(INFO, BYTECODE_OPTIMIZER) << "Function " << funcName << " has frame size " << frameSize;
    }

    LOG(DEBUG, BYTECODE_OPTIMIZER) << "Optimized " << funcName;

    return true;
}

bool OptimizePandaFile(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                       const std::string &pfileName, bool isDynamic)
{
    auto pfile = panda_file::OpenPandaFile(pfileName);
    if (!pfile) {
        LOG(FATAL, BYTECODE_OPTIMIZER) << "Can not open binary file: " << pfileName;
    }

    bool result = true;

    for (uint32_t id : pfile->GetClasses()) {
        panda_file::File::EntityId recordId {id};

        if (pfile->IsExternal(recordId)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*pfile, recordId};
        auto lang = cda.GetSourceLang().value_or(SourceLanguage::PANDA_ASSEMBLY);

        cda.EnumerateMethods([prog, maps, isDynamic, lang, &result](panda_file::MethodDataAccessor &mda) {
            if (!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative()) {
                result = OptimizeFunction(prog, maps, mda, isDynamic, lang) && result;
            }
        });
    }

    return result;
}

bool OptimizeBytecode(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const std::string &pandafileName, bool isDynamic, bool hasMemoryPool)
{
    ASSERT(prog != nullptr);
    ASSERT(maps != nullptr);

    if (!hasMemoryPool) {
        PoolManager::Initialize(PoolType::MALLOC);
    }

    auto res = OptimizePandaFile(prog, maps, pandafileName, isDynamic);

    if (!hasMemoryPool) {
        PoolManager::Finalize();
    }

    return res;
}
}  // namespace ark::bytecodeopt
