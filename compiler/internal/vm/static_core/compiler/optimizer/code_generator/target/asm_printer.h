/**
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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_ASM_PRINTER_H
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_ASM_PRINTER_H

#include "encode.h"
#include "compiler/optimizer/code_generator/callconv.h"
#include "registers_description.h"

#ifdef PANDA_COMPILER_TARGET_AARCH32
#include "aarch32/target.h"
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
#include "aarch64/target.h"
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
#include "amd64/target.h"
#else
namespace amd64 {
class [[maybe_unused]] Amd64Encoder;
}  // namespace amd64
#endif

#include <iostream>
#include <string>

namespace ark::compiler {

template <class T>
class PrinterLabelHolder final : public LabelHolder {
public:
    using LabelType = std::string;
    explicit PrinterLabelHolder(Encoder *enc) : LabelHolder(enc), labels_(enc->GetAllocator()->Adapter()) {};

    NO_MOVE_SEMANTIC(PrinterLabelHolder);
    NO_COPY_SEMANTIC(PrinterLabelHolder);
    ~PrinterLabelHolder() override = default;

    LabelId CreateLabel() override
    {
        // Create unnammed label
        ++id_;
        auto label = LabelType(std::to_string(id_ - 1));
        labels_[id_ - 1] = label;
        ASSERT(labels_.size() == id_);
        return id_ - 1;
    }

    LabelId CreateLabel(const LabelType &str)
    {
        // Create nammed label
        ++id_;
        labels_[id_ - 1] = str;
        ASSERT(labels_.size() == id_);
        return id_ - 1;
    }

    void CreateLabels(LabelId size) override
    {
        for (LabelId i = 0; i <= size; ++i) {
            CreateLabel();
        }
    }

    void BindLabel(LabelId id) override;

    LabelType *GetLabel(LabelId id)
    {
        ASSERT(labels_.size() > id);
        return &labels_[id];
    }

    LabelId Size() override
    {
        return labels_.size();
    }

private:
    ArenaMap<LabelId, LabelType> labels_;
    LabelId id_ {0};
    friend T;
};  // Aarch32LabelHolder

template <class T>
class AssemblyPrinter final : public Encoder {
    // Do not inherited from T, because need save `final` on them
public:
    explicit AssemblyPrinter(ArenaAllocator *aa, T *enc);

    ~AssemblyPrinter() override
    {
        enc_->~Encoder();
    }

    NO_COPY_SEMANTIC(AssemblyPrinter);
    NO_MOVE_SEMANTIC(AssemblyPrinter);

    bool InitMasm() override
    {
        return enc_->InitMasm();
    }

    // DUMMY
    void Finalize() override
    {
        enc_->Finalize();
    }

    void SetStream(std::ostream *str)
    {
        str_ = str;
    }

    std::ostream *GetStream()
    {
        return str_;
    }

    bool GetResult() const override
    {
        return Encoder::GetResult() && enc_->GetResult();
    }

    void EmitFunctionName(const void *funcName)
    {
        auto *name = reinterpret_cast<const char *>(funcName);
        *str_ << ".global " << name << std::endl;
        *str_ << ".type " << name << ", %function" << std::endl;
        *str_ << name << ":" << std::endl;
    }

    LabelHolder *GetLabels() const override
    {
        return labels_;
    };

    size_t GetLabelAddress(LabelHolder::LabelId label) override
    {
        return enc_->GetLabelAddress(label);
    }

    bool LabelHasLinks(LabelHolder::LabelId label) override
    {
        return enc_->LabelHasLinks(label);
    }

    // Next interfaces used for create wrappers

    T *GetEncoder()
    {
        return reinterpret_cast<T *>(enc_);
    }

    size_t GetCursorOffset() const override
    {
        return enc_->GetCursorOffset();
    }

    void PreWork() {}

    void PostWork() {}

    void Disasm(size_t from, size_t to)
    {
        for (uint32_t i = from; i < to;) {
            auto tmp = i;
            i = enc_->DisasmInstr(*str_, i, -1);
            if (i == tmp) {
                enc_->SetFalseResult();
                return;
            }
            *str_ << std::endl;
        }
    }

// Define default math operations
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNARY_OPERATION(opc)                    \
    void Encode##opc(Reg dst, Reg src) override \
    {                                           \
        ASSERT(str_ != nullptr);                \
        auto curr_cursor = GetCursorOffset();   \
        PreWork();                              \
        enc_->Encode##opc(dst, src);            \
        PostWork();                             \
        Disasm(curr_cursor, GetCursorOffset()); \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION(opc)                              \
    void Encode##opc(Reg dst, Reg src0, Reg src1) override \
    {                                                      \
        ASSERT(str_ != nullptr);                           \
        auto curr_cursor = enc_->GetCursorOffset();        \
        PreWork();                                         \
        enc_->Encode##opc(dst, src0, src1);                \
        PostWork();                                        \
        Disasm(curr_cursor, enc_->GetCursorOffset());      \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_SHIFTED_REGISTER_OPERATION(opc)            \
    void Encode##opc(Reg dst, Reg src, Shift sh) override \
    {                                                     \
        ASSERT(str_ != nullptr);                          \
        auto curr_cursor = enc_->GetCursorOffset();       \
        PreWork();                                        \
        enc_->Encode##opc(dst, src, sh);                  \
        PostWork();                                       \
        Disasm(curr_cursor, enc_->GetCursorOffset());     \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, MACRO) MACRO(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)
    ENCODE_INST_WITH_SHIFTED_OPERAND(INST_DEF)

#undef UNARY_OPERATION
#undef BINARY_OPERATION
#undef BINARY_SHIFTED_REGISTER_OPERATION

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION(opc)                            \
    void Encode##opc(Reg dst, Reg src, Imm imm) override \
    {                                                    \
        ASSERT(str_ != nullptr);                         \
        auto curr_cursor = enc_->GetCursorOffset();      \
        PreWork();                                       \
        enc_->Encode##opc(dst, src, imm);                \
        PostWork();                                      \
        Disasm(curr_cursor, enc_->GetCursorOffset());    \
    }

    ENCODE_MATH_BINARY_OP_LIST(INST_DEF)

#undef BINARY_OPERATION
#undef INST_DEF

// Call by reg+offset or MemRef - default encode
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_INST_ADDITIONAL_ZERO_ARG(DEF) \
    DEF(EncodeReturn)                        \
    DEF(EncodeAbort)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_ZERO_PARAM_OPERATION(opc)                 \
    void opc() override                               \
    {                                                 \
        ASSERT(str_ != nullptr);                      \
        auto curr_cursor = enc_->GetCursorOffset();   \
        PreWork();                                    \
        enc_->opc();                                  \
        PostWork();                                   \
        Disasm(curr_cursor, enc_->GetCursorOffset()); \
    }

    ENCODE_INST_ADDITIONAL_ZERO_ARG(DEF_ZERO_PARAM_OPERATION)

// Call by reg+offset - no needed, because unusable:
//     * MakeCallAot(intptr_t, offset)
//     * MakeCallByOffset(intptr_t, offset)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_INST_ADDITIONAL_ONE_ARG(DEF) DEF(MakeCall, MemRef, entryPoint)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_ONE_PARAM_OPERATION(opc, type, param)     \
    void opc(type param) override                     \
    {                                                 \
        ASSERT(str_ != nullptr);                      \
        auto curr_cursor = enc_->GetCursorOffset();   \
        PreWork();                                    \
        enc_->opc(param);                             \
        PostWork();                                   \
        Disasm(curr_cursor, enc_->GetCursorOffset()); \
    }

    ENCODE_INST_ADDITIONAL_ONE_ARG(DEF_ONE_PARAM_OPERATION)

    // Aot Table - unusable:
    //   * MakeLoadAotTable(intptr_t, offset, Reg, reg)

#undef ENCODE_INST_ADDITIONAL_ZERO_ARG
#undef DEF_ZERO_PARAM_OPERATION
#undef ENCODE_INST_ADDITIONAL_ONE_ARG
#undef DEF_ONE_PARAM_OPERATION

    // Special cases - call read from
    void MakeCall(const void *entryPoint) override;

private:
    Encoder *enc_;
    std::ostream *str_ {nullptr};
    LabelHolder *labels_ {nullptr};
};

// T - CallConv-origin class
// E - Encoder-printer class
template <class T, class E>
class PrinterCallingConvention : public T {
    // CallingConvention call virtual methods inside (example - Begin call GeneratePrologue)
    // That's why not all methods must be rewrited

public:
    PrinterCallingConvention(ArenaAllocator *allocator, E *enc, RegistersDescription *descr, CallConvMode mode)
        : T(allocator, enc->GetEncoder(), descr, mode), printer_(enc) {};

    NO_MOVE_SEMANTIC(PrinterCallingConvention);
    NO_COPY_SEMANTIC(PrinterCallingConvention);

    ~PrinterCallingConvention() override = default;

    Encoder *GetEncoder() const
    {
        return CallingConvention::GetEncoder();
    }

    void PreWork()
    {
        cursor_ = printer_->GetCursorOffset();
        printer_->PreWork();
    }

    void PostWork()
    {
        printer_->PostWork();
        size_t endCursor = printer_->GetCursorOffset();
        if (endCursor == cursor_) {
            return;
        }
        printer_->Disasm(cursor_, endCursor);
    }

    void GeneratePrologue(const FrameInfo &frameInfo) override
    {
        PreWork();
        T::GeneratePrologue(frameInfo);
        PostWork();
    }
    void GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override
    {
        PreWork();
        T::GenerateEpilogue(frameInfo, postJob);
        PostWork();
    }

    // Copy register to return register
    // Interfaces after finalize code
    void *GetCodeEntry() override
    {
        PreWork();
        auto tmp = T::GetCodeEntry();
        PostWork();
        return tmp;
    }
    // Get return register
    uint32_t GetCodeSize() override
    {
        PreWork();
        auto tmp = T::GetCodeSize();
        PostWork();
        return tmp;
    }
    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regsOffset) override
    {
        PreWork();
        auto tmp = T::GetParameterInfo(regsOffset);
        PostWork();
        return tmp;
    }

private:
    E *printer_;
    size_t cursor_ {0};
};

#ifdef PANDA_COMPILER_TARGET_AARCH32
namespace aarch32 {
using Aarch32Assembly = AssemblyPrinter<Aarch32Encoder>;
using Aarch32AssemblyLabelHolder = PrinterLabelHolder<Aarch32Encoder>;
}  // namespace aarch32
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
namespace aarch64 {
using Aarch64Assembly = AssemblyPrinter<Aarch64Encoder>;
using Aarch64AssemblyLabelHolder = PrinterLabelHolder<Aarch64Encoder>;
}  // namespace aarch64
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
namespace amd64 {
using Amd64Assembly = AssemblyPrinter<Amd64Encoder>;
using Amd64AssemblyLabelHolder = PrinterLabelHolder<Amd64Encoder>;
}  // namespace amd64
#endif

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_ASM_PRINTER_H
