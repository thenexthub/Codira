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

#include "graph.h"
#include "basicblock.h"
#include "dump.h"
#include "visualizer_printer.h"

namespace ark::compiler {
void VisualizerPrinter::Print()
{
    PrintBeginTag("cfg");
    PrintProperty("name", ArenaString(passName_, graph_->GetLocalAllocator()->Adapter()));
    for (const auto &block : graph_->GetBlocksRPO()) {
        PrintBasicBlock(block);
    }
    PrintEndTag("cfg");
}

void VisualizerPrinter::PrintBeginTag(const char *tag)
{
    (*output_) << MakeOffset() << "begin_" << tag << "\n";
    ++offset_;
}

void VisualizerPrinter::PrintEndTag(const char *tag)
{
    --offset_;
    (*output_) << MakeOffset() << "end_" << tag << "\n";
}

std::string VisualizerPrinter::MakeOffset()
{
    return std::string(offset_ << 1U, ' ');
}

void VisualizerPrinter::PrintProperty(const char *prop, const ArenaString &value)
{
    (*output_) << MakeOffset() << prop;
    if (!value.empty()) {
        (*output_) << " \"" << value << "\"";
    }
    (*output_) << "\n";
}

void VisualizerPrinter::PrintProperty(const char *prop, int value)
{
    (*output_) << MakeOffset() << std::dec << prop << " " << value << "\n";
}

template <typename T>
void VisualizerPrinter::PrintDependences(const std::string &preffix, const T &blocks)
{
    (*output_) << MakeOffset() << preffix << " ";
    for (const auto &block : blocks) {
        (*output_) << "\"B" << BBId(block, graph_->GetLocalAllocator()) << "\" ";
    }
    (*output_) << "\n";
}

void VisualizerPrinter::PrintBasicBlock(BasicBlock *block)
{
    PrintBeginTag("block");
    PrintProperty("name", "B" + BBId(block, graph_->GetLocalAllocator()));
    PrintProperty("from_bci", -1);
    PrintProperty("to_bci", -1);
    PrintDependences("predecessors", block->GetPredsBlocks());
    PrintDependences("successors", block->GetSuccsBlocks());
    PrintProperty("xhandlers", ArenaString("", graph_->GetLocalAllocator()->Adapter()));
    PrintProperty("flags", ArenaString("", graph_->GetLocalAllocator()->Adapter()));
    PrintProperty("loop_depth", 0);
    PrintBeginTag("states");
    PrintBeginTag("locals");
    PrintProperty("size", 0);
    PrintProperty("method", ArenaString("None", graph_->GetLocalAllocator()->Adapter()));
    PrintEndTag("locals");
    PrintEndTag("states");
    PrintBeginTag("HIR");
    PrintInsts(block);
    PrintEndTag("HIR");
    PrintEndTag("block");
}

void VisualizerPrinter::PrintInsts(BasicBlock *block)
{
    if (block->AllInsts().begin() != block->AllInsts().end()) {
        for (auto inst : block->AllInsts()) {
            PrintInst(inst);
        }
    } else {
        (*output_) << MakeOffset() << "0 0 -  ";
        if (block->IsStartBlock()) {
            (*output_) << "EMPTY_START_BLOCK";
        } else if (block->IsEndBlock()) {
            (*output_) << "EXIT_BLOCK";
        } else {
            (*output_) << "EMPTY_BLOCK";
        }
        (*output_) << " <|@\n";
    }
}

void VisualizerPrinter::PrintInst(Inst *inst)
{
    uint32_t usersSize = 0;
    for (auto it = inst->GetUsers().begin(); it != inst->GetUsers().end(); ++it) {
        usersSize++;
    }
    (*output_) << MakeOffset() << "0 " << std::dec << usersSize << " ";
    (*output_) << InstId(inst, graph_->GetLocalAllocator()) << " ";
    inst->DumpOpcode(output_);
    (*output_) << " type:" << DataType::ToString(inst->GetType()) << " ";
    (*output_) << "inputs: ";
    bool hasInput = inst->DumpInputs(output_);
    if (hasInput && !inst->GetUsers().Empty()) {
        (*output_) << " users: ";
    }
    DumpUsers(inst, output_);
    (*output_) << " <|@\n";
}
}  // namespace ark::compiler
