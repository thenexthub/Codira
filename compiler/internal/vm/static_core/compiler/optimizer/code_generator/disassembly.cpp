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

#include "disassembly.h"
#include "codegen.h"
#include <mutex>

namespace ark::compiler {

// clang-format off
static constexpr std::array INDENT_STRINGS = {
    "  ",
    "    ",
    "      ",
    "        ",
    "          ",
    "            ",
    "              ",
    "                ",
    "                  ",
};
// clang-format on

std::string_view Disassembly::GetIndent(uint32_t depth)
{
    return INDENT_STRINGS[depth];
}

static constexpr auto STREAM_DELETER = [](std::ostream *stream) {
    if (!g_options.IsCompilerDisasmDumpStdout()) {
        delete stream;
    }
};

Disassembly::Disassembly(const Codegen *codegen)
    : codegen_(codegen),
      encoder_(codegen->GetEncoder()),
      stream_(nullptr, STREAM_DELETER),
      isEnabled_(g_options.IsCompilerDisasmDump()),
      isCodeEnabled_(isEnabled_ && g_options.IsCompilerDisasmDumpCode())
{
}

void Disassembly::Init()
{
    if (!IsEnabled()) {
        return;
    }
    if (g_options.IsCompilerDisasmDumpStdout()) {
        stream_.reset(&std::cout);
    } else if (g_options.IsCompilerDisasmDumpSingleFile()) {
        auto os = new std::ofstream;
        if (os == nullptr) {
            UNREACHABLE();
        }
        static std::once_flag flag;
        auto fileName = g_options.GetCompilerDisasmDumpFileName();
        std::call_once(flag, [&fileName]() { std::remove(fileName.c_str()); });
        os->open(fileName, std::ios_base::app);
        if (!os->is_open()) {
            LOG(FATAL, COMPILER) << "Cannot open 'disasm.txt'";
        }
        stream_.reset(os);
    } else {
        auto os = new std::ofstream;
        if (os == nullptr) {
            UNREACHABLE();
        }
        std::stringstream ss;
        auto graph = codegen_->GetGraph();
        auto execNum = graph->GetPassManager()->GetExecutionCounter();
        ss << "disasm_" << execNum << '_' << codegen_->GetRuntime()->GetClassNameFromMethod(graph->GetMethod()) << '_'
           << codegen_->GetRuntime()->GetMethodName(graph->GetMethod()) << (graph->IsOsrMode() ? "_osr" : "") << ".txt";
        os->open(ss.str());
        if (!os->is_open()) {
            LOG(FATAL, COMPILER) << "Cannot open '" << ss.str() << "'";
        }
        stream_.reset(os);
    }
}

void Disassembly::PrintChapter(std::string_view name)
{
    ItemAppender item(this);
    item.GetStream() << name << ":\n";
}

void Disassembly::IncreaseDepth()
{
    if ((depth_ + 1) < INDENT_STRINGS.size()) {
        depth_++;
    }
}

void Disassembly::PrintMethodEntry(const Codegen *codegen)
{
    constexpr const char *INDENT = "  ";
    auto graph = codegen->GetGraph();
    {
        ItemAppender item(this);
        item.GetStream() << "#======================================================================\n";
    }
    PrintChapter("METHOD_INFO");
    {
        ItemAppender item(this);
        auto name = codegen->GetRuntime()->GetMethodFullName(graph->GetMethod(), true);
        item.GetStream() << INDENT << "name: " << name << "\n" << INDENT << "mode: ";
        graph->GetMode().Dump(item.GetStream());
        item.GetStream() << "\n";
    }
    {
        ItemAppender item(this);
        item.GetStream() << INDENT << "id: " << codegen->GetRuntime()->GetMethodId(graph->GetMethod()) << "\n";
    }
    if (graph->IsAotMode()) {
        ItemAppender item = ItemAppender(this);
        auto codeOffset = reinterpret_cast<void *>(graph->GetAotData()->GetCodeOffset());
        item.GetStream() << INDENT << "code_offset: " << codeOffset << "\n";
    }
    {
        ItemAppender item(this);
        auto slotSize = static_cast<ssize_t>(codegen_->GetFrameLayout().GetSlotSize());
        auto dumpRegsInfo = [&item, slotSize](const char *prompt, ssize_t offset, size_t numRegs, bool isRelativeFp) {
            auto &out = item.GetStream();
            out << INDENT << prompt << (isRelativeFp ? "fp" : "sp");
            out << std::showpos << (offset * slotSize) << " bytes (" << offset << " slots)" << std::noshowpos;
            out << ", with " << numRegs << " register(s)\n";
        };
        auto arch = codegen->GetArch();
        auto frame = codegen->GetFrameInfo();
        item.GetStream() << INDENT << "frame_size: " << frame->GetFrameSize() << " bytes\n";
        item.GetStream() << INDENT << "spills_count: " << frame->GetSpillsCount() << "\n";
        bool calleesAreFpBased = frame->GetCalleesRelativeFp();
        dumpRegsInfo("Callees:   ", frame->GetCalleesOffset(), GetCalleeRegsCount(arch, false), calleesAreFpBased);
        dumpRegsInfo("FpCallees: ", frame->GetFpCalleesOffset(), GetCalleeRegsCount(arch, true), calleesAreFpBased);
        bool callersAreFpBased = frame->GetCallersRelativeFp();
        dumpRegsInfo("Callers:   ", frame->GetCallersOffset(), GetCallerRegsCount(arch, false), callersAreFpBased);
        dumpRegsInfo("FpCallers: ", frame->GetFpCallersOffset(), GetCallerRegsCount(arch, true), callersAreFpBased);
    }
    if (IsCodeEnabled()) {
        PrintChapter("DISASSEMBLY");
    }
}

void Disassembly::PrintCodeInfo(const Codegen *codegen)
{
    auto graph = codegen->GetGraph();

    CodeInfo codeInfo;
    ASSERT(!graph->GetCodeInfoData().empty());
    codeInfo.Decode(graph->GetCodeInfoData());
    PrintChapter("CODE_INFO");
    ItemAppender item(this);
    codeInfo.Dump(item.GetStream());
}

void Disassembly::PrintCodeStatistics(const Codegen *codegen)
{
    auto graph = codegen->GetGraph();

    PrintChapter("CODE_STATS");
    ItemAppender item(this);
    item.GetStream() << "  code_size: " << std::dec << graph->GetCode().Size() << "\n";
}

void Disassembly::FlushDisasm()
{
    auto encoder = GetEncoder();
    AddCode(GetPosition(), encoder->GetCursorOffset(), GetDepth());
    SetPosition(encoder->GetCursorOffset());
}

void Disassembly::PrintStackMap(const Codegen *codegen)
{
    FlushDisasm();
    ItemAppender item(this);
    item.GetStream() << GetIndent(GetDepth());
    codegen->GetCodeBuilder()->DumpCurrentStackMap(item.GetStream());
    item.GetStream() << "\n";
}

ScopedDisasmPrinter::ScopedDisasmPrinter(Codegen *codegen, const Inst *inst) : disasm_(codegen->GetDisasm())
{
    if (disasm_->IsCodeEnabled()) {
        disasm_->FlushDisasm();
        ItemAppender item(disasm_);
        item.GetStream() << disasm_->GetIndent(disasm_->GetDepth()) << "# [inst] " << *inst << "\n";
        disasm_->IncreaseDepth();
    }
}

ScopedDisasmPrinter::ScopedDisasmPrinter(Codegen *codegen, const std::string &msg) : disasm_(codegen->GetDisasm())
{
    if (disasm_->IsCodeEnabled()) {
        disasm_->FlushDisasm();
        ItemAppender item(disasm_);
        item.GetStream() << disasm_->GetIndent(disasm_->GetDepth()) << "# " << msg << "\n";
        disasm_->IncreaseDepth();
    }
}

ScopedDisasmPrinter::~ScopedDisasmPrinter()
{
    if (disasm_->IsCodeEnabled()) {
        disasm_->FlushDisasm();
        disasm_->DecreaseDepth();
    }
}

}  // namespace ark::compiler
