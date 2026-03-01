/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_TEST_FRAME_H
#define PANDA_TOOLING_INSPECTOR_TEST_TEST_FRAME_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "include/method.h"
#include "include/tooling/debug_interface.h"
#include "include/tooling/pt_thread.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/list.h"

// NOLINTBEGIN

namespace ark::tooling::inspector::test {
class TestFrame : public PtFrame {
public:
    explicit TestFrame(Method *method, uint32_t bytecode_offset) : method_(method), bytecode_offset_(bytecode_offset) {}

    ~TestFrame() override {}

    bool IsInterpreterFrame() const override
    {
        return true;
    }

    void SetMethod(Method *method)
    {
        method_ = method;
    }

    Method *GetMethod() const override
    {
        return method_;
    }

    void SetVReg(size_t index, uint64_t value)
    {
        if (vregs_.size() <= index) {
            vregs_.resize(index + 1);
        }
        vregs_[index] = value;
    }

    uint64_t GetVReg(size_t index) const override
    {
        return vregs_[index];
    }

    void SetVRegKind(size_t index, RegisterKind value)
    {
        if (vreg_kinds_.size() <= index) {
            vreg_kinds_.resize(index + 1);
        }
        vreg_kinds_[index] = value;
    }

    RegisterKind GetVRegKind(size_t i) const override
    {
        return vreg_kinds_[i];
    }

    size_t GetVRegNum() const override
    {
        return vregs_.size();
    }

    void SetArgument(size_t index, uint64_t value)
    {
        if (args_.size() <= index) {
            args_.resize(index + 1);
        }
        args_[index] = value;
    }

    uint64_t GetArgument(size_t index) const override
    {
        return args_[index];
    }

    void SetArgumentKind(size_t index, RegisterKind value)
    {
        if (arg_kinds_.size() <= index) {
            arg_kinds_.resize(index + 1);
        }
        arg_kinds_[index] = value;
    }

    RegisterKind GetArgumentKind(size_t i) const override
    {
        return arg_kinds_[i];
    }

    size_t GetArgumentNum() const override
    {
        return args_.size();
    }

    void SetAccumulator(uint64_t value)
    {
        acc_ = value;
    }

    uint64_t GetAccumulator() const override
    {
        return acc_;
    }

    RegisterKind GetAccumulatorKind() const override
    {
        return acc_kind_;
    }

    panda_file::File::EntityId GetMethodId() const override
    {
        return method_->GetFileId();
    }

    void SetBytecodeOffset(uint32_t bytecode_offset)
    {
        bytecode_offset_ = bytecode_offset;
    }

    uint32_t GetBytecodeOffset() const override
    {
        return bytecode_offset_;
    }

    std::string GetPandaFile() const override
    {
        return method_->GetPandaFile()->GetFilename();
    }

    uint32_t GetFrameId() const override
    {
        return reinterpret_cast<uintptr_t>(this);
    }

private:
    Method *method_ {nullptr};
    uint32_t bytecode_offset_ {0};

    uint64_t acc_ {0};
    std::vector<uint64_t> args_;
    std::vector<uint64_t> vregs_;
    PandaVector<RegisterKind> vreg_kinds_;
    PandaVector<RegisterKind> arg_kinds_;
    RegisterKind acc_kind_ {PtFrame::RegisterKind::PRIMITIVE};
};
}  // namespace ark::tooling::inspector::test

// NOLINTEND

#endif  // PANDA_TOOLING_INSPECTOR_TEST_TEST_FRAME_H
