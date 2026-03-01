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

#ifndef PANDA_SLOW_PATH_H
#define PANDA_SLOW_PATH_H

#include "optimizer/ir/graph.h"
#include "optimizer/code_generator/encode.h"

namespace ark::compiler {
class Codegen;

class SlowPathBase {
public:
    using EntrypointId = RuntimeInterface::EntrypointId;

    explicit SlowPathBase(LabelHolder::LabelId label) : SlowPathBase(label, nullptr) {}
    SlowPathBase(LabelHolder::LabelId label, Inst *inst)
        : label_(label), labelBack_(LabelHolder::INVALID_LABEL), inst_(inst)
    {
    }
    virtual ~SlowPathBase() = default;

    NO_COPY_SEMANTIC(SlowPathBase);
    NO_MOVE_SEMANTIC(SlowPathBase);

    Inst *GetInst() const
    {
        return inst_;
    }

    auto &GetLabel() const
    {
        return label_;
    }

    void BindBackLabel(Encoder *encoder)
    {
        if (!encoder->IsLabelValid(labelBack_)) {
            labelBack_ = encoder->CreateLabel();
        }
        encoder->BindLabel(labelBack_);
    }

    void CreateBackLabel(Encoder *encoder)
    {
        labelBack_ = encoder->CreateLabel();
    }

    LabelHolder::LabelId GetBackLabel()
    {
        return labelBack_;
    }

    void Generate(Codegen *codegen);

    virtual void GenerateImpl(Codegen *codegen) = 0;

private:
    LabelHolder::LabelId label_ {LabelHolder::INVALID_LABEL};
    LabelHolder::LabelId labelBack_ {LabelHolder::INVALID_LABEL};
    Inst *inst_ {nullptr};

#ifndef NDEBUG
    // Intended to check that slow path is generated only once.
    bool generated_ {false};
#endif
};

class SlowPathIntrinsic : public SlowPathBase {
public:
    using SlowPathBase::SlowPathBase;

    SlowPathIntrinsic(LabelHolder::LabelId label, Inst *inst) : SlowPathBase(label, inst) {}
    ~SlowPathIntrinsic() override = default;

    NO_COPY_SEMANTIC(SlowPathIntrinsic);
    NO_MOVE_SEMANTIC(SlowPathIntrinsic);

    void GenerateImpl(Codegen *codegen) override;
};

class SlowPathEntrypoint : public SlowPathBase {
public:
    explicit SlowPathEntrypoint(LabelHolder::LabelId label) : SlowPathBase(label) {}
    SlowPathEntrypoint(LabelHolder::LabelId label, Inst *inst, EntrypointId eid)
        : SlowPathBase(label, inst), entrypoint_(eid)
    {
    }
    ~SlowPathEntrypoint() override = default;

    void GenerateImpl(Codegen *codegen) override;

    EntrypointId GetEntrypoint()
    {
        return entrypoint_;
    }

    NO_COPY_SEMANTIC(SlowPathEntrypoint);
    NO_MOVE_SEMANTIC(SlowPathEntrypoint);

private:
    bool GenerateThrowOutOfBoundsException(Codegen *codegen);
    bool GenerateInitializeClass(Codegen *codegen);
    bool GenerateIsInstance(Codegen *codegen);
    bool GenerateCheckCast(Codegen *codegen);
    bool GenerateByEntry(Codegen *codegen);
    bool GenerateCreateObject(Codegen *codegen);

private:
    EntrypointId entrypoint_ {EntrypointId::COUNT};
};

class SlowPathDeoptimize : public SlowPathEntrypoint {
public:
    ~SlowPathDeoptimize() override = default;
    NO_COPY_SEMANTIC(SlowPathDeoptimize);
    NO_MOVE_SEMANTIC(SlowPathDeoptimize);

    SlowPathDeoptimize(LabelHolder::LabelId label, Inst *inst, DeoptimizeType deoptimizeType)
        : SlowPathEntrypoint(label, inst, EntrypointId::DEOPTIMIZE), deoptimizeType_ {deoptimizeType}
    {
    }

    void GenerateImpl(Codegen *codegen) override;

private:
    DeoptimizeType deoptimizeType_;
};

class SlowPathImplicitNullCheck : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;
};

class SlowPathShared : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;
    void SetTmpReg(Reg reg)
    {
        tmpReg_ = reg;
    }
    Reg GetTmpReg() const
    {
        return tmpReg_;
    }

private:
    Reg tmpReg_ {INVALID_REGISTER};
};

class SlowPathResolveStringAot : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;

    void SetDstReg(Reg reg)
    {
        dstReg_ = reg;
    }

    void SetAddrReg(Reg reg)
    {
        addrReg_ = reg;
    }

    void SetStringId(uint32_t stringId)
    {
        stringId_ = stringId;
    }

    void SetMethod(void *method)
    {
        method_ = method;
    }

private:
    Reg dstReg_ {INVALID_REGISTER};
    Reg addrReg_ {INVALID_REGISTER};
    uint32_t stringId_ {0};
    void *method_ {nullptr};
};

class SlowPathCheck : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;
};

class SlowPathCheckCast : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;
    void GenerateImpl(Codegen *codegen) override;
    void SetClassReg(Reg reg)
    {
        classReg_ = reg;
    }

private:
    Reg classReg_ {INVALID_REGISTER};
};

class SlowPathAbstract : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;
    void GenerateImpl(Codegen *codegen) override;
    void SetMethodReg(Reg reg)
    {
        methodReg_ = reg;
    }

private:
    Reg methodReg_ {INVALID_REGISTER};
};

class SlowPathRefCheck : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;

    void SetRegs(Reg arrayReg, Reg refReg)
    {
        arrayReg_ = arrayReg;
        refReg_ = refReg;
    }

private:
    Reg arrayReg_ {INVALID_REGISTER};
    Reg refReg_ {INVALID_REGISTER};
};

class SlowPathUnresolved : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;

    void SetUnresolvedType(void *method, uint32_t typeId)
    {
        method_ = method;
        typeId_ = typeId;
    }

    void SetSlotAddr(uintptr_t addr)
    {
        slotAddr_ = addr;
    }

    void SetDstReg(Reg dstReg)
    {
        dstReg_ = dstReg;
    }

    void SetArgReg(Reg argReg)
    {
        argReg_ = argReg;
    }

private:
    Reg dstReg_ {INVALID_REGISTER};
    Reg argReg_ {INVALID_REGISTER};
    void *method_ {nullptr};
    uint32_t typeId_ {0};
    uintptr_t slotAddr_ {0};
};

class SlowPathJsCastDoubleToInt32 : public SlowPathBase {
public:
    using SlowPathBase::SlowPathBase;

    ~SlowPathJsCastDoubleToInt32() override = default;

    NO_COPY_SEMANTIC(SlowPathJsCastDoubleToInt32);
    NO_MOVE_SEMANTIC(SlowPathJsCastDoubleToInt32);

    void SetSrcReg(Reg srcReg)
    {
        srcReg_ = srcReg;
    }

    void SetDstReg(Reg dstReg)
    {
        dstReg_ = dstReg;
    }

    void GenerateImpl(Codegen *codegen) override;

private:
    Reg srcReg_ {INVALID_REGISTER};
    Reg dstReg_ {INVALID_REGISTER};
};

class SlowPathStringHashCode : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    ~SlowPathStringHashCode() override = default;

    NO_COPY_SEMANTIC(SlowPathStringHashCode);
    NO_MOVE_SEMANTIC(SlowPathStringHashCode);

    void SetDstReg(Reg dstReg)
    {
        dstReg_ = dstReg;
    }

    void SetSrcReg(Reg srcReg)
    {
        srcReg_ = srcReg;
    }

    void GenerateImpl(Codegen *codegen) override;

private:
    Reg dstReg_ {INVALID_REGISTER};
    Reg srcReg_ {INVALID_REGISTER};
};

}  // namespace ark::compiler

#endif  // PANDA_SLOW_PATH_H
