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

#include "function.h"

#include "bytecode_optimizer/runtime_adapter.h"
#include "compiler/optimizer/ir_builder/ir_builder.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "mem/arena_allocator.h"
#include "utils/logger.h"

#include "configs/guard_context.h"
#include "graph_analyzer.h"
#include "inst_obf.h"
#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view TAG = "[Function]";
constexpr std::string_view LINE_DELIMITER = ":";
constexpr std::string_view RECORD_DELIMITER = ".";
constexpr std::string_view SCOPE_DELIMITER = "#";
constexpr size_t SCOPE_AND_NAME_LEN = 2;
constexpr size_t MAX_LINE_NUMBER = 100000;
constexpr std::string_view ENTRY_FUNC_TAG = ".func_main_0";
constexpr std::string_view STATIC_INITIALIZER_TAG = ">#static_initializer";
constexpr std::string_view INSTANCE_INITIALIZER_TAG = ">#instance_initializer";
constexpr std::string_view CONSOLE_INS_VAR = "console";
constexpr std::string_view ANONYMOUS_FUNCTION_NAME = "^0";  // first anonymous function name

const std::map<char, panda::guard::FunctionType> FUNCTION_TYPE_MAP = {
    {'>', panda::guard::FunctionType::INSTANCE_FUNCTION},    {'<', panda::guard::FunctionType::STATIC_FUNCTION},
    {'=', panda::guard::FunctionType::CONSTRUCTOR_FUNCTION}, {'*', panda::guard::FunctionType::NORMAL_FUNCTION},
    {'%', panda::guard::FunctionType::ENUM_FUNCTION},        {'&', panda::guard::FunctionType::NAMESPACE_FUNCTION},
};

/**
 * Is it an entry function (func_main_0)
 * @param functionIdx function Idx
 * @return (true/false)
 */
bool IsEntryMethod(const std::string &functionIdx)
{
    return functionIdx.find(ENTRY_FUNC_TAG) != std::string::npos;
}

/**
 * Is it an implicit function (dynamically generated in bytecode, not reflected in source code)
 * @param functionIdx function Idx
 * @return (true/false)
 */
bool IsImplicitMethod(const std::string &functionIdx)
{
    return functionIdx.find(INSTANCE_INITIALIZER_TAG) != std::string::npos ||
           functionIdx.find(STATIC_INITIALIZER_TAG) != std::string::npos;
}

/**
 * Do not obfuscate the names of whitelist functions (internal instructions and associated properties need to be
 * obfuscated)
 * @param functionIdx function Idx
 * @return (true/false)
 */
bool IsWhiteListFunction(const std::string &functionIdx)
{
    return IsEntryMethod(functionIdx) || IsImplicitMethod(functionIdx);
}

bool GetConsoleLogInfoForStart(const std::vector<panda::pandasm::InsPtr> &insList, size_t &start, uint16_t &reg)
{
    size_t i = 0;
    while (i < insList.size()) {
        auto &ins = insList[i];
        if (ins->opcode == panda::pandasm::Opcode::TRYLDGLOBALBYNAME && ins->GetId(0) == CONSOLE_INS_VAR) {
            size_t nextInsIndex = i + 1;
            PANDA_GUARD_ASSERT_PRINT(nextInsIndex >= insList.size(), TAG, panda::guard::ErrorCode::GENERIC_ERROR,
                                     "get console next ins get bad index:" << nextInsIndex);

            auto &nextIns = insList[nextInsIndex];
            PANDA_GUARD_ASSERT_PRINT(nextIns->opcode != panda::pandasm::Opcode::STA, TAG,
                                     panda::guard::ErrorCode::GENERIC_ERROR, "get console next ins get bad ins type");

            reg = nextIns->GetReg(0);
            start = i;
            return true;
        }
        i++;
    }

    return false;
}

bool HasMovInstForRange(const std::vector<panda::pandasm::InsPtr> &insList, size_t start, size_t end, uint16_t oriReg,
                        uint16_t rangeReg)
{
    size_t i = end - 1;
    while (i > start) {
        auto &ins = insList[i];
        if ((ins->opcode == panda::pandasm::Opcode::MOV) && (ins->GetReg(0) == rangeReg) &&
            (ins->GetReg(1) == oriReg)) {
            return true;
        }
        i--;
    }
    return false;
}

int GetConsoleLogInfoForEnd(const std::vector<panda::pandasm::InsPtr> &insList, size_t start, uint16_t reg, size_t &end)
{
    size_t i = start + 1;
    while (i < insList.size()) {
        auto &ins = insList[i];
        if ((ins->opcode == panda::pandasm::Opcode::CALLTHIS1 || ins->opcode == panda::pandasm::Opcode::CALLTHIS2 ||
             ins->opcode == panda::pandasm::Opcode::CALLTHIS3) &&
            ins->GetReg(0) == reg) {
            end = i + 1;
            return true;
        }

        if ((ins->opcode == panda::pandasm::Opcode::CALLTHISRANGE) &&
            HasMovInstForRange(insList, start, i, reg, ins->GetReg(0))) {
            end = i + 1;
            return true;
        }

        i++;
    }

    return false;
}

/**
 * Retrieve instruction information related to console. xxx from the instruction list [start, end]
 * @param insList instruction list
 * @param start console instruction start index
 * @param end console instruction end index(not included)
 * @return result code
 */
bool GetConsoleLogInfo(const std::vector<panda::pandasm::InsPtr> &insList, size_t &start, size_t &end)
{
    size_t startIndex = 0;
    uint16_t reg = 0;  // console variable stored reg
    size_t endIndex = 0;
    if (!GetConsoleLogInfoForStart(insList, startIndex, reg)) {
        return false;
    }

    if (!GetConsoleLogInfoForEnd(insList, startIndex, reg, endIndex)) {
        return false;
    }

    start = startIndex;
    end = endIndex;

    return true;
}
}  // namespace

void panda::guard::Function::Init()
{
    /* e.g.
     * idx_: &entry/src/main/ets/entryability/EntryAbility&.#~@0>#onCreate
     * recordName_: &entry/src/main/ets/entryability/EntryAbility&
     * rawName_: #~@0>#onCreate
     * scopeTypeStr_: ~@0>
     */
    LOG(INFO, PANDAGUARD) << TAG << "Function:" << this->idx_;
    this->obfIdx_ = this->idx_;
    const auto &[recordName, rawName] = StringUtil::RSplitOnce(this->idx_, RECORD_DELIMITER.data());
    PANDA_GUARD_ASSERT_PRINT(recordName.empty() || rawName.empty(), TAG, ErrorCode::GENERIC_ERROR,
                             "split record and name get bad value");

    this->recordName_ = recordName;
    this->rawName_ = rawName;

    this->InitBaseInfo();

    LOG(INFO, PANDAGUARD) << TAG << "idx:" << this->idx_;
    LOG(INFO, PANDAGUARD) << TAG << "recordName:" << this->recordName_;
    LOG(INFO, PANDAGUARD) << TAG << "rawName:" << this->rawName_;
    LOG(INFO, PANDAGUARD) << TAG << "regsNum:" << this->regsNum_;
    LOG(INFO, PANDAGUARD) << TAG << "startLine:" << this->startLine_;
    LOG(INFO, PANDAGUARD) << TAG << "endLine:" << this->endLine_;
    LOG(INFO, PANDAGUARD) << TAG << "component:" << (this->component_ ? "true" : "false");

    if (!this->useScope_) {
        return;
    }

    std::vector<std::string> scopeAndName = StringUtil::Split(rawName, SCOPE_DELIMITER.data());
    PANDA_GUARD_ASSERT_PRINT(scopeAndName.empty() || scopeAndName.size() > SCOPE_AND_NAME_LEN, TAG,
                             ErrorCode::GENERIC_ERROR, "split scope and name get bad len");

    this->scopeTypeStr_ = scopeAndName[0];
    this->SetFunctionType(scopeAndName[0].back());
    this->name_ = scopeAndName.size() > 1 ? scopeAndName[1] : ANONYMOUS_FUNCTION_NAME;

    if (StringUtil::IsAnonymousFunctionName(this->name_)) {
        this->anonymous_ = true;
    }

    this->obfName_ = this->name_;

    LOG(INFO, PANDAGUARD) << TAG << "scopeTypeStr:" << this->scopeTypeStr_;
    LOG(INFO, PANDAGUARD) << TAG << "type:" << (int)this->type_;
    LOG(INFO, PANDAGUARD) << TAG << "name:" << this->name_;
    LOG(INFO, PANDAGUARD) << TAG << "anonymous:" << (this->anonymous_ ? "true" : "false");
}

panda::pandasm::Function &panda::guard::Function::GetOriginFunction()
{
    return this->program_->prog_->function_table.at(this->obfIdx_);
}

void panda::guard::Function::InitBaseInfo()
{
    const auto &func = this->GetOriginFunction();
    this->name_ = this->idx_;
    this->obfName_ = this->name_;
    this->regsNum_ = func.regs_num;

    if (GuardContext::GetInstance()->GetGuardOptions()->IsCompactObfEnabled()) {
        this->startLine_ = 1;
        this->endLine_ = 1;
    } else {
        size_t startLineIndex = 0;
        while (startLineIndex < func.ins.size()) {
            const size_t lineNumber = func.ins[startLineIndex]->ins_debug.line_number;
            if (lineNumber < MAX_LINE_NUMBER) {
                this->startLine_ = lineNumber;
                break;
            }
            startLineIndex++;
        }

        size_t endLineIndex = func.ins.size() - 1;
        while (endLineIndex >= startLineIndex) {
            const size_t lineNumber = func.ins[endLineIndex]->ins_debug.line_number;
            if (lineNumber < MAX_LINE_NUMBER) {
                this->endLine_ = lineNumber + 1;
                break;
            }
            endLineIndex--;
        }
    }
}

void panda::guard::Function::SetFunctionType(char functionTypeCode)
{
    PANDA_GUARD_ASSERT_PRINT(FUNCTION_TYPE_MAP.find(functionTypeCode) == FUNCTION_TYPE_MAP.end(), TAG,
                             ErrorCode::GENERIC_ERROR, "unsupported function type code:" << functionTypeCode);

    this->type_ = FUNCTION_TYPE_MAP.at(functionTypeCode);
}

void panda::guard::Function::InitNameCacheScope()
{
    this->SetNameCacheScope(this->name_);
}

void panda::guard::Function::Build()
{
    if (IsEntryMethod(this->idx_)) {
        return;
    }

    LOG(INFO, PANDAGUARD) << TAG << "function build for " << this->idx_ << " start";

    this->InitNameCacheScope();

    LOG(INFO, PANDAGUARD) << TAG << "scope:" << (this->scope_ == TOP_LEVEL ? "TOP_LEVEL" : "Function");
    LOG(INFO, PANDAGUARD) << TAG << "nameCacheScope:" << this->GetNameCacheScope();
    LOG(INFO, PANDAGUARD) << TAG << "export:" << (this->export_ ? "true" : "false");

    LOG(INFO, PANDAGUARD) << TAG << "function build for " << this->idx_ << " end";
}

bool panda::guard::Function::IsWhiteListOrAnonymousFunction(const std::string &functionIdx) const
{
    return IsWhiteListFunction(functionIdx) || this->anonymous_;
}

void panda::guard::Function::RefreshNeedUpdate()
{
    this->needUpdate_ = true;
    this->contentNeedUpdate_ = true;
    this->nameNeedUpdate_ = TopLevelOptionEntity::NeedUpdate(*this) && !IsWhiteListOrAnonymousFunction(this->idx_);
    LOG(INFO, PANDAGUARD) << TAG << "Function nameNeedUpdate: " << (this->nameNeedUpdate_ ? "true" : "false");
}

void panda::guard::Function::EnumerateIns(const std::function<InsTraver> &callback)
{
    auto &func = this->GetOriginFunction();
    for (size_t i = 0; i < func.ins.size(); i++) {
        InstructionInfo info(this, func.ins[i].get(), i);
        callback(info);
    }
    this->FreeGraph();
}

void panda::guard::Function::UpdateReference()
{
    LOG(INFO, PANDAGUARD) << TAG << "update reference start:" << this->idx_;
    this->EnumerateIns([&](InstructionInfo &info) -> void { InstObf::UpdateInst(info); });
    LOG(INFO, PANDAGUARD) << TAG << "update reference end:" << this->idx_;
}

void panda::guard::Function::UpdateAnnotationReference()
{
    const auto &function = this->GetOriginFunction();
    function.metadata->EnumerateAnnotations([this](pandasm::AnnotationData &annotation) {
        const std::string annotationFullName = annotation.GetName();  // "recordName.annoName"
        if (Annotation::IsWhiteListAnnotation(annotationFullName)) {
            return;
        }

        PANDA_GUARD_ASSERT_PRINT(!this->node_.has_value(), TAG, ErrorCode::GENERIC_ERROR,
                                 "function associate node is invalid");
        LOG(INFO, PANDAGUARD) << TAG << "update annotation:" << annotationFullName;
        const auto node = this->node_.value();
        // annotation reference name maybe contains namespace name such as: ns1.n2.annoName
        const auto annoNameWithNamespace =
            annotationFullName.substr(annotationFullName.find(node->name_) + node->name_.length() + 1);
        LOG(INFO, PANDAGUARD) << TAG << "annoName:" << annoNameWithNamespace;
        const auto annoNames = StringUtil::Split(annoNameWithNamespace, RECORD_DELIMITER.data());
        auto obfAnnoName = node->obfName_;
        for (auto &name : annoNames) {
            obfAnnoName += RECORD_DELIMITER.data() + GuardContext::GetInstance()->GetNameMapping()->GetName(name);
        }
        LOG(INFO, PANDAGUARD) << TAG << "obfAnnoName:" << obfAnnoName;
        annotation.SetName(obfAnnoName);

        auto &recordTable = this->program_->prog_->record_table;
        if (recordTable.find(annotationFullName) != recordTable.end()) {
            // update reference annotation record
            LOG(INFO, PANDAGUARD) << TAG << "update reference record:" << annotationFullName;
            auto entry = recordTable.extract(annotationFullName);
            entry.key() = obfAnnoName;
            entry.mapped().name = obfAnnoName;
            entry.mapped().metadata->SetAccessFlags(panda::ACC_ANNOTATION);
            recordTable.insert(std::move(entry));
        }
    });
}

void panda::guard::Function::RemoveConsoleLog()
{
    auto &insList = this->GetOriginFunction().ins;
    size_t start = 0;
    size_t end = 0;
    while (GetConsoleLogInfo(insList, start, end)) {
        LOG(INFO, PANDAGUARD) << TAG << "remove console log for:" << this->idx_;
        LOG(INFO, PANDAGUARD) << TAG << "found console ins range:[" << start << ", " << end << ")";
        PANDA_GUARD_ASSERT_PRINT(end >= insList.size(), TAG, ErrorCode::GENERIC_ERROR,
                                 "bad end ins index for console:" << end);
        insList.erase(insList.begin() + start, insList.begin() + end);
    }
}

void panda::guard::Function::RemoveLineNumber()
{
    for (auto &inst : this->GetOriginFunction().ins) {
        inst->ins_debug.line_number = 1;
    }
}

void panda::guard::Function::FillInstInfo(size_t index, InstructionInfo &instInfo)
{
    auto &func = this->GetOriginFunction();
    PANDA_GUARD_ASSERT_PRINT(index >= func.ins.size(), TAG, ErrorCode::GENERIC_ERROR, "out of range index: " << index);

    instInfo.function_ = this;
    instInfo.index_ = index;
    instInfo.ins_ = func.ins[index].get();
}

void panda::guard::Function::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->name_);
    for (const auto &[_, property] : this->propertyTable_) {
        property->ExtractNames(strings);
    }

    for (const auto &property : this->variableProperties_) {
        property->ExtractNames(strings);
    }

    if (GuardContext::GetInstance()->GetGuardOptions()->IsDecoratorObfEnabled()) {
        for (const auto &property : this->objectDecoratorProperties_) {
            property->ExtractNames(strings);
        }
    }
}

void panda::guard::Function::SetExportAndRefreshNeedUpdate(bool isExport)
{
    for (const auto &[_, property] : this->propertyTable_) {
        property->SetExportAndRefreshNeedUpdate(isExport);
    }

    Entity::SetExportAndRefreshNeedUpdate(isExport);
}

std::string panda::guard::Function::GetLines() const
{
    return LINE_DELIMITER.data() + std::to_string(this->startLine_) + LINE_DELIMITER.data() +
           std::to_string(this->endLine_);
}

void panda::guard::Function::UpdateName(const std::shared_ptr<Node> &node)
{
    std::string obfRawName;
    // The judgment here cannot be moved to the outer layer. The function here should not only confuse its own name, but
    // also the file name Even if the file is kept, The function name may also be modified due to file name confusion,
    // and the related logic still needs to be executed by the function itself
    if (node->contentNeedUpdate_ && this->nameNeedUpdate_) {
        /* e.g.
         * #~@0=#EntryAbility
         *  name_: EntryAbility
         *  scopeTypeStr_: ~@0=
         *  obfName_: a
         *  obfRawName: #~@0=#a
         */
        this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
        obfRawName = SCOPE_DELIMITER.data() + this->scopeTypeStr_ + SCOPE_DELIMITER.data() + this->obfName_;
    } else {
        // e.g. rawName_: #~@0>@1*#
        obfRawName = this->rawName_;
    }

    this->obfIdx_ = node->obfName_ + RECORD_DELIMITER.data() + obfRawName;
}

void panda::guard::Function::UpdateDefine() const
{
    bool obfuscated = false;

    for (auto &defineIns : this->defineInsList_) {
        if (!defineIns.IsValid()) {
            LOG(INFO, PANDAGUARD) << TAG << "no bind define ins, ignore update define";
            continue;
        }

        // definefunc, definemethod for function, defineclasswithbuffer, callruntime.definesendableclass for constructor
        PANDA_GUARD_ASSERT_PRINT(defineIns.ins_->opcode != pandasm::Opcode::DEFINEFUNC &&
                                     defineIns.ins_->opcode != pandasm::Opcode::DEFINECLASSWITHBUFFER &&
                                     defineIns.ins_->opcode != pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS &&
                                     defineIns.ins_->opcode != pandasm::Opcode::DEFINEMETHOD,
                                 TAG, ErrorCode::GENERIC_ERROR, "get bad ins type");

        defineIns.ins_->GetId(0) = this->obfIdx_;
        obfuscated = true;
    }

    if (obfuscated) {
        this->program_->prog_->strings.emplace(this->obfIdx_);
    }
}

void panda::guard::Function::UpdateFunctionTable(const std::shared_ptr<Node> &node) const
{
    if (this->idx_ == this->obfIdx_) {
        return;
    }
    auto entry = this->program_->prog_->function_table.extract(this->idx_);
    entry.key() = this->obfIdx_;
    entry.mapped().name = this->obfIdx_;
    if (node->fileNameNeedUpdate_ && !entry.mapped().source_file.empty()) {
        node->UpdateSourceFile(entry.mapped().source_file);
        entry.mapped().source_file = node->obfSourceFile_;
    }
    this->program_->prog_->function_table.insert(std::move(entry));
}

void panda::guard::Function::GetGraph(compiler::Graph *&outGraph)
{
    // the existence of graph_ depends on GraphContext
    auto context = GuardContext::GetInstance()->GetGraphContext();
    PANDA_GUARD_ASSERT_PRINT(context == nullptr, TAG, ErrorCode::GENERIC_ERROR, "graph context not inited");

    if (this->graph_ != nullptr) {
        outGraph = this->graph_;
        return;
    }

    this->methodPtr_ = context->FillMethodPtr(this->recordName_, this->rawName_);
    PANDA_GUARD_ASSERT_PRINT(this->methodPtr_ == 0, TAG, ErrorCode::GENERIC_ERROR,
                             "can not find method ptr for: " << this->idx_);

    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(this->methodPtr_);
    this->allocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);
    this->localAllocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
    this->runtimeInterface_ = std::make_unique<panda::BytecodeOptimizerRuntimeAdapter>(context->GetAbcFile());
    auto graph =
        this->allocator_->New<compiler::Graph>(this->allocator_.get(), this->localAllocator_.get(), Arch::NONE,
                                               methodPtr, this->runtimeInterface_.get(), false, nullptr, true, true);
    PANDA_GUARD_ASSERT_PRINT((graph == nullptr) || !graph->RunPass<panda::compiler::IrBuilder>(), TAG,
                             ErrorCode::GENERIC_ERROR, "Graph " << this->idx_ << ": IR builder failed!");

    this->BuildPcInsMap(graph);
    this->graph_ = graph;
    LOG(INFO, PANDAGUARD) << TAG << "create graph for " << this->idx_ << " end";

    outGraph = graph;
}

void panda::guard::Function::BuildPcInsMap(const compiler::Graph *graph)
{
    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(this->methodPtr_);

    size_t funcSize = this->GetOriginFunction().ins.size();
    this->pcInstMap_.reserve(funcSize);

    auto instructionsBuf = graph->GetRuntime()->GetMethodCode(methodPtr);
    compiler::BytecodeInstructions instructions(instructionsBuf, graph->GetRuntime()->GetMethodCodeSize(methodPtr));
    compiler::BytecodeIterator ins_iter = instructions.begin();

    for (size_t idx = 0; idx < this->GetOriginFunction().ins.size(); idx++) {
        auto ins = this->GetOriginFunction().ins[idx].get();
        if (ins->opcode == pandasm::Opcode::INVALID) {
            continue;
        }
        this->pcInstMap_.emplace(instructions.GetPc(*ins_iter), idx);
        ++ins_iter;
        if (ins_iter == instructions.end()) {
            break;
        }
    }
}

void panda::guard::Function::FreeGraph()
{
    std::unordered_map<size_t, size_t>().swap(this->pcInstMap_);

    this->graph_ = nullptr;
    this->localAllocator_ = nullptr;
    this->runtimeInterface_ = nullptr;
    this->allocator_ = nullptr;
}

void panda::guard::Function::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "function update for " << this->idx_ << " start";

    auto it = this->program_->nodeTable_.find(this->recordName_);
    PANDA_GUARD_ASSERT_PRINT(it == this->program_->nodeTable_.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "not find node: " + this->recordName_);

    this->UpdateName(it->second);
    this->UpdateDefine();
    this->UpdateFunctionTable(it->second);

    if (it->second->contentNeedUpdate_ && this->contentNeedUpdate_) {
        for (const auto &[_, property] : this->propertyTable_) {
            property->Obfuscate();
        }
    }

    for (const auto &property : this->variableProperties_) {
        property->Obfuscate();
    }

    if (GuardContext::GetInstance()->GetGuardOptions()->IsDecoratorObfEnabled()) {
        for (const auto &property : this->objectDecoratorProperties_) {
            property->Obfuscate();
        }
    }

    LOG(INFO, PANDAGUARD) << TAG << "function update for " << this->idx_ << " end";
}

void panda::guard::Function::WriteNameCache(const std::string &filePath)
{
    if (IsNameObfuscated()) {
        this->WriteFileCache(filePath);
        this->WritePropertyCache();
    }

    for (const auto &[_, property] : this->propertyTable_) {
        property->WriteNameCache(filePath);
    }

    for (const auto &property : this->variableProperties_) {
        property->WritePropertyCache();
    }

    if (GuardContext::GetInstance()->GetGuardOptions()->IsDecoratorObfEnabled()) {
        for (const auto &property : this->objectDecoratorProperties_) {
            property->WritePropertyCache();
        }
    }
}

void panda::guard::Function::WriteFileCache(const std::string &filePath)
{
    if (this->type_ != panda::guard::FunctionType::CONSTRUCTOR_FUNCTION) {
        std::string name = this->GetNameCacheScope();
        if (this->type_ != panda::guard::FunctionType::ENUM_FUNCTION) {
            name += this->GetLines();
        }
        GuardContext::GetInstance()->GetNameCache()->AddObfIdentifierName(filePath, name, this->obfName_);
    } else {
        std::string name = this->name_ + this->GetLines();
        GuardContext::GetInstance()->GetNameCache()->AddObfMemberMethodName(filePath, name, this->obfName_);
    }
}

void panda::guard::Function::WritePropertyCache()
{
    TopLevelOptionEntity::WritePropertyCache(*this);
}

bool panda::guard::Function::IsNameObfuscated() const
{
    return !IsWhiteListOrAnonymousFunction(this->idx_);
}
