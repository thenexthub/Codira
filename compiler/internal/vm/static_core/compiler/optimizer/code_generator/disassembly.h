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

#ifndef PANDA_DISASSEMBLY_H
#define PANDA_DISASSEMBLY_H

#include "compiler_options.h"
#include "libarkbase/macros.h"
#include <fstream>
#include <string>
#include <variant>
#include <memory>

namespace ark::compiler {

class Codegen;
class Encoder;
class Inst;
class StackMap;

class CodeItem {
public:
    CodeItem(size_t position, size_t cursorOffset, size_t depth)
        : position_(position), cursorOffset_(cursorOffset), depth_(depth)
    {
    }
    size_t GetPosition() const
    {
        return position_;
    }
    size_t GetCursorOffset() const
    {
        return cursorOffset_;
    }
    size_t GetDepth() const
    {
        return depth_;
    }

private:
    size_t position_ {0};
    size_t cursorOffset_ {0};
    size_t depth_ {0};
};

using DisassemblyItem = std::variant<CodeItem, std::string>;

class Disassembly {
public:
    explicit Disassembly(const Codegen *codegen);
    ~Disassembly() = default;
    NO_COPY_SEMANTIC(Disassembly);
    NO_MOVE_SEMANTIC(Disassembly);

    void Init();

    std::ostream &GetStream()
    {
        return *stream_;
    }
    std::string_view GetIndent(uint32_t depth);
    uint32_t GetDepth() const
    {
        return depth_;
    }
    uint32_t GetPosition() const
    {
        return position_;
    }
    void SetPosition(uint32_t pos)
    {
        position_ = pos;
    }
    const Encoder *GetEncoder() const
    {
        return encoder_;
    }
    void SetEncoder(const Encoder *encoder)
    {
        encoder_ = encoder;
    }
    void PrintChapter(std::string_view name);
    void IncreaseDepth();
    void DecreaseDepth()
    {
        depth_--;
    }
    bool IsEnabled() const
    {
        return isEnabled_;
    }
    bool IsCodeEnabled() const
    {
        return isCodeEnabled_;
    }
    std::vector<DisassemblyItem> &GetItems()
    {
        return items_;
    }

    void PrintMethodEntry(const Codegen *codegen);
    void PrintCodeInfo(const Codegen *codegen);
    void PrintCodeStatistics(const Codegen *codegen);
    void PrintStackMap(const Codegen *codegen);

    void AddCode(size_t position, size_t cursorOffset, size_t depth)
    {
        CodeItem item(position, cursorOffset, depth);
        items_.emplace_back(item);
    }

private:
    void FlushDisasm();

private:
    using StreamDeleterType = void (*)(std::ostream *stream);
    const Codegen *codegen_ {nullptr};
    const Encoder *encoder_ {nullptr};
    std::unique_ptr<std::ostream, StreamDeleterType> stream_;
    uint32_t depth_ {0};
    uint32_t position_ {0};
    bool isEnabled_ {false};
    bool isCodeEnabled_ {false};
    std::vector<DisassemblyItem> items_;

    friend class ScopedDisasmPrinter;
};

class ScopedDisasmPrinter {
public:
    ScopedDisasmPrinter(Codegen *codegen, const std::string &msg);
    ScopedDisasmPrinter(Codegen *codegen, const Inst *inst);
    ~ScopedDisasmPrinter();

    NO_COPY_SEMANTIC(ScopedDisasmPrinter);
    NO_MOVE_SEMANTIC(ScopedDisasmPrinter);

private:
    Disassembly *disasm_ {nullptr};
};

class ItemAppender {
public:
    explicit ItemAppender(Disassembly *disasm) : disasm_(disasm) {}
    ~ItemAppender()
    {
        disasm_->GetItems().emplace_back(std::move(ss_).str());
    }

    NO_COPY_SEMANTIC(ItemAppender);
    NO_MOVE_SEMANTIC(ItemAppender);

    std::ostringstream &GetStream()
    {
        return ss_;
    }

private:
    Disassembly *disasm_;
    std::ostringstream ss_;
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISASM_VAR_CONCAT2(a, b) a##b
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISASM_VAR_CONCAT(a, b) DISASM_VAR_CONCAT2(a, b)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SCOPED_DISASM_INST(codegen, inst) ScopedDisasmPrinter DISASM_VAR_CONCAT(disasm_, __LINE__)(codegen, inst)
#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SCOPED_DISASM_STR(codegen, str) ScopedDisasmPrinter DISASM_VAR_CONCAT(disasm_, __LINE__)(codegen, str)
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SCOPED_DISASM_STR(codegen, str) (void)codegen
#endif

}  // namespace ark::compiler

#endif  // PANDA_DISASSEMBLY_H
