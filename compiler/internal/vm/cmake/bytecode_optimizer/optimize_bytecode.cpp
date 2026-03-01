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

#include "bytecodeopt_options.h"
#include "bytecode_analysis_results.h"
#include "codegen.h"
#include "common.h"
#include "compiler/optimizer/ir_builder/ir_builder.h"
#include "compiler/optimizer/optimizations/branch_elimination.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/move_constants.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "constant_propagation/constant_propagation.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/module_data_accessor-inl.h"
#include "reg_acc_alloc.h"
#include "reg_encoder.h"
#include "runtime_adapter.h"

namespace panda::bytecodeopt {
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
panda::bytecodeopt::Options options("");

template <typename T>
constexpr void RunOpts(compiler::Graph *graph)
{
    graph->RunPass<compiler::Cleanup>();
    graph->RunPass<T>();
}

template <typename First, typename Second, typename... Rest>
constexpr void RunOpts(compiler::Graph *graph)
{
    RunOpts<First>(graph);
    RunOpts<Second, Rest...>(graph);
}

bool RunOptimizations(compiler::Graph *graph, BytecodeOptIrInterface *iface)
{
    constexpr int OPT_LEVEL_0 = 0;

    if (panda::bytecodeopt::options.GetOptLevel() == OPT_LEVEL_0) {
        return false;
    }

    graph->RunPass<compiler::Cleanup>();
    ASSERT(graph->IsDynamicMethod());

    if (compiler::options.IsCompilerBranchElimination()) {
        graph->RunPass<ConstantPropagation>(iface);
        RunOpts<compiler::BranchElimination>(graph);
    }

    RunOpts<compiler::ValNum, compiler::Lowering, compiler::MoveConstants>(graph);

    // this pass should run just before register allocator
    graph->RunPass<compiler::Cleanup>();
    graph->RunPass<RegAccAlloc>();

    graph->RunPass<compiler::Cleanup>();
    if (!RegAlloc(graph)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Failed compiler::RegAlloc";
        return false;
    }

    graph->RunPass<compiler::Cleanup>();
    if (!graph->RunPass<RegEncoder>()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Failed RegEncoder";
        return false;
    }

    return true;
}

void BuildMapFromPcToIns(pandasm::Function &function, BytecodeOptIrInterface &ir_interface,
                         const compiler::Graph *graph, compiler::RuntimeInterface::MethodPtr method_ptr)
{
    CHECK_NOT_NULL(graph);
    function.local_variable_debug.clear();
    auto *pc_ins_map = ir_interface.GetPcInsMap();
    pc_ins_map->reserve(function.ins.size());
    auto instructions_buf = graph->GetRuntime()->GetMethodCode(method_ptr);
    compiler::BytecodeInstructions instructions(instructions_buf, graph->GetRuntime()->GetMethodCodeSize(method_ptr));
    compiler::BytecodeIterator insn_iter = instructions.begin();
    for (auto &ins : function.ins) {
        /**
         * pc_ins_map is built with instructions data from the emitted abc file and the original function assembly.
         * Instructions of invalid opcode will be removed during emitter, but kept within function assembly structure,
         * therefore these instructions need to be skipped here
         **/
        if (ins->opcode == pandasm::Opcode::INVALID) {
            continue;
        }
        pc_ins_map->emplace(instructions.GetPc(*insn_iter), ins.get());
        ++insn_iter;
        if (insn_iter == instructions.end()) {
            break;
        }
    }
}

static void ColumnNumberPropagate(pandasm::Function *function)
{
    auto &ins_vec = function->ins;
    uint32_t cn = compiler::INVALID_COLUMN_NUM;
    // handle the instructions that are at the beginning of code but do not have column number
    size_t k = 0;
    while (k < ins_vec.size() && cn == compiler::INVALID_COLUMN_NUM) {
        cn = ins_vec[k++]->ins_debug.column_number;
    }
    if (cn == compiler::INVALID_COLUMN_NUM) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "Failed ColumnNumberPropagate: All insts have invalid column number";
        return;
    }
    for (size_t j = 0; j < k - 1; j++) {
        ins_vec[j]->ins_debug.SetColumnNumber(cn);
    }

    // handle other instructions that do not have column number
    for (; k < ins_vec.size(); k++) {
        if (ins_vec[k]->ins_debug.column_number != compiler::INVALID_COLUMN_NUM) {
            cn = ins_vec[k]->ins_debug.column_number;
        } else {
            ins_vec[k]->ins_debug.SetColumnNumber(cn);
        }
    }
}

static void LineNumberPropagate(pandasm::Function *function)
{
    if (function == nullptr || function->ins.empty()) {
        return;
    }
    size_t ln = 0;
    auto &ins_vec = function->ins;

    // handle the instructions that are at the beginning of code but do not have line number
    size_t i = 0;
    while (i < ins_vec.size() && ln == 0) {
        ln = ins_vec[i++]->ins_debug.line_number;
    }
    if (ln == 0) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "Failed LineNumberPropagate: All insts have invalid line number";
        return;
    }
    for (size_t j = 0; j < i - 1; j++) {
        ins_vec[j]->ins_debug.SetLineNumber(ln);
    }

    // handle other instructions that do not have line number
    for (; i < ins_vec.size(); i++) {
        if (ins_vec[i]->ins_debug.line_number != 0) {
            ln = ins_vec[i]->ins_debug.line_number;
        } else {
            ins_vec[i]->ins_debug.SetLineNumber(ln);
        }
    }
}

static void DebugInfoPropagate(pandasm::Function &function, const compiler::Graph *graph,
                               BytecodeOptIrInterface &ir_interface)
{
    LineNumberPropagate(&function);
    if (graph->IsDynamicMethod()) {
        ColumnNumberPropagate(&function);
    }
    ir_interface.ClearPcInsMap();
}

static bool SkipFunction(const pandasm::Function &function, const std::string &func_name)
{
    if (panda::bytecodeopt::options.WasSetMethodRegex()) {
        static std::regex rgx(panda::bytecodeopt::options.GetMethodRegex());
        if (!std::regex_match(func_name, rgx)) {
            LOG(INFO, BYTECODE_OPTIMIZER) << "Skip Function " << func_name << ": Function's name doesn't match regex";
            return true;
        }
    }

    if (panda::bytecodeopt::options.IsSkipMethodsWithEh() && !function.catch_blocks.empty()) {
        LOG(INFO, BYTECODE_OPTIMIZER) << "Was not optimized " << func_name << ": Function has catch blocks";
        return true;
    }

    if ((function.regs_num + function.GetParamsNum()) > compiler::VIRTUAL_FRAME_SIZE) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Unable to optimize " << func_name
                                       << ": Function frame size is larger than allowed one";
        return true;
    }
    return false;
}

static void SetCompilerOptions()
{
    compiler::options.SetCompilerUseSafepoint(false);
    compiler::options.SetCompilerSupportInitObjectInst(true);
    if (!compiler::options.WasSetCompilerMaxBytecodeSize()) {
        compiler::options.SetCompilerMaxBytecodeSize(MAX_BYTECODE_SIZE);
    }
}

static bool StringStartsWith(const std::string &str, const std::string &prefix)
{
    return (str.size() >= prefix.size()) &&
        std::equal(prefix.begin(), prefix.end(), str.begin());
}

static std::string ModuleRequestOffsetToRecordName(const panda_file::File &pfile,
                                                   uint32_t module_request_offset)
{
    constexpr char AND_TOKEN = '&';
    const std::string BUNDLE_PREFIX = "@bundle:";
    const std::string PACKAGE_PREFIX = "@package:";
    const std::string NORMALIZED_NON_NATIVE_PREFIX = "@normalized:N&";

    auto record_ohmurl = GetStringFromPandaFile(pfile, module_request_offset);
    // Assumptions of the current possible ohmurl formats:
    // @bundle:record_name
    // @package:record_name
    // @normalized:N&xxxx&record_name
    // Extract record_name from each possible cases.
    if (StringStartsWith(record_ohmurl, BUNDLE_PREFIX)) {
        return record_ohmurl.substr(BUNDLE_PREFIX.size());
    } else if (StringStartsWith(record_ohmurl, PACKAGE_PREFIX)) {
        return record_ohmurl.substr(PACKAGE_PREFIX.size());
    } else if (StringStartsWith(record_ohmurl, NORMALIZED_NON_NATIVE_PREFIX)) {
        size_t second_and_pos = record_ohmurl.find(AND_TOKEN, NORMALIZED_NON_NATIVE_PREFIX.size());
        if (second_and_pos != std::string::npos) {
            return record_ohmurl.substr(second_and_pos + 1);
        }
    }
    // Otherwise, return empty string to represent no ohmurl is found
    return "";
}

static void AnalysisModuleRecordInfoOfModuleDataAccessor(const panda_file::File &pfile,
                                                         panda_file::ModuleDataAccessor &mda,
                                                         BytecodeAnalysisResult &result)
{
    const auto &request_modules_offsets = mda.getRequestModules();
    int regular_import_idx = 0;
    std::unordered_set<std::string> local_export_local_names;
    mda.EnumerateModuleRecord([&](panda_file::ModuleTag tag, uint32_t export_name_offset,
                                  uint32_t request_module_idx, uint32_t import_name_offset,
                                  uint32_t local_name_offset) {
        switch (tag) {
            case panda_file::ModuleTag::LOCAL_EXPORT: {
                std::string export_name = GetStringFromPandaFile(pfile, export_name_offset);
                std::string local_name = GetStringFromPandaFile(pfile, local_name_offset);
                // Slot of stmodulevar/ldlocalmodulevar is the index of its local name, while
                // one local name can match multiple external names with "export...as...".
                // Local export entries are sorted by their local name, thus using an unrodered_set
                // can get the correct index form (size - 1) (starts from 0).
                // See SourceTextModuleRecord::AssignIndexToModuleVariable for more details
                local_export_local_names.insert(local_name);
                result.SetLocalExportInfo(local_export_local_names.size() - 1, export_name);
                break;
            }
            case panda_file::ModuleTag::REGULAR_IMPORT: {
                std::string request_module_name =
                    ModuleRequestOffsetToRecordName(pfile, request_modules_offsets[request_module_idx]);
                if (!request_module_name.empty()) {
                    std::string import_name = GetStringFromPandaFile(pfile, import_name_offset);
                    result.SetRegularImportInfo(regular_import_idx, import_name, request_module_name);
                }
                regular_import_idx++;
                break;
            }
            case panda_file::ModuleTag::NAMESPACE_IMPORT: {
                // Slot of getmodulenamespace bytecode is its request_module_idx
                std::string namespace_name =
                    ModuleRequestOffsetToRecordName(pfile, request_modules_offsets[request_module_idx]);
                if (!namespace_name.empty()) {
                    result.SetNamespaceImportInfo(request_module_idx, namespace_name);
                }
                break;
            }
            default:
                break;
        }
    });
}

static void AnalysisModuleRecordInfo(const panda_file::File &pfile,
                                     panda_file::ClassDataAccessor &cda,
                                     BytecodeAnalysisResult &result)
{
    const std::string MODULE_RECORD_IDX_FIELD_NAME = "moduleRecordIdx";
    // RequireGlobalOptimization is true only under mergeAbc mode, where module record is stored
    // in the moduleRecordIdx field according to Emitter::AddSourceTextModuleRecord
    cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) -> void {
        if (fda.IsExternal()) {
            return;
        }
        std::string field_name = GetStringFromPandaFile(pfile, fda.GetNameId().GetOffset());
        if (field_name == MODULE_RECORD_IDX_FIELD_NAME) {
            panda_file::File::EntityId module_entity_id(fda.GetValue<uint32_t>().value());
            panda_file::ModuleDataAccessor mda(pfile, module_entity_id);
            AnalysisModuleRecordInfoOfModuleDataAccessor(pfile, mda, result);
        }
    });
}

static void AnalysisModuleConstantValue(panda_file::ClassDataAccessor &cda, const std::string &record_name,
                                        bool is_dynamic, const BytecodeOptIrInterface &ir_interface,
                                        BytecodeAnalysisResult &result)
{
    const std::string MAIN_METHOD_NAME = ".func_main_0";
    cda.EnumerateMethods([MAIN_METHOD_NAME, record_name, is_dynamic, ir_interface, &result](
            panda_file::MethodDataAccessor &mda) {
        if (mda.IsExternal()) {
            return false;
        }

        // Only analysis func_main_0 for now, since the assignment instruction of all exported constants
        // are in func_main_0, and the bytecode analysis phase only contains analysing initial value of
        // module constants for branch-elimination for now
        auto func_name = ir_interface.GetMethodIdByOffset(mda.GetMethodId().GetOffset());
        if (func_name != record_name + MAIN_METHOD_NAME) {
            return true;
        }

        ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
        ArenaAllocator local_allocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

        auto *prog = ir_interface.GetProgram();
        auto it = prog->function_table.find(func_name);
        if (it == prog->function_table.end()) {
            LOG(ERROR, BYTECODE_OPTIMIZER) << "Cannot find function: " << func_name;
            return false;
        }

        panda::pandasm::Function &function = it->second;
        if (SkipFunction(function, func_name)) {
            return false;
        }

        auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());
        panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
        auto graph = allocator.New<compiler::Graph>(&allocator, &local_allocator, Arch::NONE, method_ptr, &adapter,
                                                    false, nullptr, is_dynamic, true);
        if ((graph == nullptr) || !graph->RunPass<panda::compiler::IrBuilder>()) {
            LOG(ERROR, BYTECODE_OPTIMIZER) << "Analysis " << func_name << ": IR builder failed!";
            return false;
        }

        ModuleConstantAnalysisResult module_constant_results;
        ModuleConstantAnalyzer analyzer(graph, result.GetConstantLocalExportSlots(),
                                        module_constant_results, &ir_interface);
        graph->RunPass<ModuleConstantAnalyzer>(&analyzer);
        result.SetModuleConstantAnalysisResult(module_constant_results);
        return true;
    });
}

bool AnalysisBytecode(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const std::string &pfile_name, bool is_dynamic, bool has_memory_pool)
{
    if (!has_memory_pool) {
        PoolManager::Initialize(PoolType::MALLOC);
    }

    auto pfile = panda_file::OpenPandaFile(pfile_name);
    if (!pfile) {
        LOG(FATAL, BYTECODE_OPTIMIZER) << "Can not open binary file: " << pfile_name;
        return false;
    }

    for (uint32_t id : pfile->GetClasses()) {
        panda_file::File::EntityId record_id {id};

        if (pfile->IsExternal(record_id)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*pfile, record_id};
        // Skip annotation records since they do not contain real code for now
        if (cda.IsAnnotation()) {
            continue;
        }
        std::string record_type_descriptor(utf::Mutf8AsCString(cda.GetName().data));
        std::string record_name = pandasm::Type::FromDescriptor(record_type_descriptor).GetName();

        bool exists = false;
        auto &result = BytecodeAnalysisResults::GetOrCreateBytecodeAnalysisResult(record_name, exists);
        if (exists) {
            return true;
        }
        auto ir_interface = BytecodeOptIrInterface(maps, prog);
        AnalysisModuleRecordInfo(*pfile, cda, result);
        AnalysisModuleConstantValue(cda, record_name, is_dynamic, ir_interface, result);
    }

    if (!has_memory_pool) {
        PoolManager::Finalize();
    }

    return true;
}

bool OptimizeFunction(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const panda_file::MethodDataAccessor &mda, bool is_dynamic)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator local_allocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

    SetCompilerOptions();

    auto ir_interface = BytecodeOptIrInterface(maps, prog);

    auto func_name = ir_interface.GetMethodIdByOffset(mda.GetMethodId().GetOffset());
    LOG(INFO, BYTECODE_OPTIMIZER) << "Optimizing function: " << func_name;

    auto it = prog->function_table.find(func_name);
    if (it == prog->function_table.end()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Cannot find function: " << func_name;
        return false;
    }
    auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());

    panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
    auto graph = allocator.New<compiler::Graph>(&allocator, &local_allocator, Arch::NONE, method_ptr, &adapter, false,
                                                nullptr, is_dynamic, true);

    panda::pandasm::Function &function = it->second;

    if (SkipFunction(function, func_name)) {
        return false;
    }

    // build map from pc to pandasm::ins (to re-build line-number info in BytecodeGen)
    BuildMapFromPcToIns(function, ir_interface, graph, method_ptr);

    if ((graph == nullptr) || !graph->RunPass<panda::compiler::IrBuilder>()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": IR builder failed!";
        return false;
    }

    if (graph->HasIrreducibleLoop()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": Graph has irreducible loop!";
        return false;
    }

    if (!RunOptimizations(graph, &ir_interface)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": Running optimizations failed!";
        return false;
    }

    if (!graph->RunPass<BytecodeGen>(&function, &ir_interface, prog)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": Code generation failed!";
        return false;
    }

    DebugInfoPropagate(function, graph, ir_interface);

    function.value_of_first_param =
        static_cast<int64_t>(graph->GetStackSlotsCount()) - 1;  // Work-around promotion rules
    function.regs_num = static_cast<size_t>(function.value_of_first_param + 1);

    if (auto frame_size = function.regs_num + function.GetParamsNum(); frame_size >= NUM_COMPACTLY_ENCODED_REGS) {
        LOG(INFO, BYTECODE_OPTIMIZER) << "Function " << func_name << " has frame size " << frame_size;
    }

    LOG(DEBUG, BYTECODE_OPTIMIZER) << "Optimized " << func_name;

    return true;
}

bool OptimizePandaFile(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                       const std::string &pfile_name, bool is_dynamic)
{
    auto pfile = panda_file::OpenPandaFile(pfile_name);
    if (!pfile) {
        LOG(FATAL, BYTECODE_OPTIMIZER) << "Can not open binary file: " << pfile_name;
    }

    bool result = true;

    CHECK_NOT_NULL(pfile);
    for (uint32_t id : pfile->GetClasses()) {
        panda_file::File::EntityId record_id {id};

        if (pfile->IsExternal(record_id)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*pfile, record_id};
        cda.EnumerateMethods([prog, maps, is_dynamic, &result](panda_file::MethodDataAccessor &mda) {
            if (!mda.IsExternal()) {
                result = OptimizeFunction(prog, maps, mda, is_dynamic) && result;
            }
        });
    }

    return result;
}

bool OptimizeBytecode(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const std::string &pandafile_name, bool is_dynamic, bool has_memory_pool)
{
    ASSERT(prog != nullptr);
    ASSERT(maps != nullptr);

    if (!has_memory_pool) {
        PoolManager::Initialize(PoolType::MALLOC);
    }

    auto res = OptimizePandaFile(prog, maps, pandafile_name, is_dynamic);

    if (!has_memory_pool) {
        PoolManager::Finalize();
    }

    return res;
}
}  // namespace panda::bytecodeopt
