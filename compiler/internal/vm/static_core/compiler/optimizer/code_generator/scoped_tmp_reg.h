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

#ifndef COMPILER_OPTIMIZER_CODEGEN_SCOPED_TMP_REG_H
#define COMPILER_OPTIMIZER_CODEGEN_SCOPED_TMP_REG_H

#include "encode.h"

namespace ark::compiler {

/**
 * This class is using to acquire/release temp register using RAII technique.
 *
 * @tparam lazy if true, temp register will be acquired in the constructor, otherwise user should acquire it explicitly.
 */
template <bool LAZY>
class ScopedTmpRegImpl {
public:
    explicit ScopedTmpRegImpl(Encoder *encoder) : ScopedTmpRegImpl(encoder, false) {}
    ScopedTmpRegImpl(Encoder *encoder, bool withLr) : encoder_(encoder)
    {
        if constexpr (!LAZY) {  // NOLINT
            auto linkReg = encoder->GetTarget().GetLinkReg();
            withLr &= encoder->IsLrAsTempRegEnabled();
            if (withLr && encoder->IsScratchRegisterReleased(linkReg)) {
                reg_ = linkReg;
                encoder->AcquireScratchRegister(linkReg);
            } else {
                reg_ = encoder->AcquireScratchRegister(Is64BitsArch(encoder->GetArch()) ? INT64_TYPE : INT32_TYPE);
            }
        }
    }

    ScopedTmpRegImpl(Encoder *encoder, TypeInfo type) : encoder_(encoder), reg_(encoder->AcquireScratchRegister(type))
    {
        static_assert(!LAZY);
    }

    ScopedTmpRegImpl(Encoder *encoder, Reg reg) : encoder_(encoder), reg_(reg)
    {
        static_assert(!LAZY);
        encoder->AcquireScratchRegister(reg);
    }

    ScopedTmpRegImpl(ScopedTmpRegImpl &&other) noexcept
    {
        encoder_ = other.encoder_;
        reg_ = other.reg_;
        other.reg_ = Reg();
        ASSERT(!other.reg_.IsValid());
    }

    virtual ~ScopedTmpRegImpl()
    {
        if (reg_.IsValid()) {
            encoder_->ReleaseScratchRegister(reg_);
        }
    }

    NO_COPY_SEMANTIC(ScopedTmpRegImpl);
    NO_MOVE_OPERATOR(ScopedTmpRegImpl);

    Reg GetReg() const
    {
        return reg_;
    }

    TypeInfo GetType() const
    {
        return reg_.GetType();
    }

    // NOLINTNEXTLINE(*-explicit-constructor)
    operator Reg() const
    {
        return reg_;
    }

    void ChangeType(TypeInfo tp)
    {
        ASSERT(tp.IsScalar() == reg_.IsScalar());
        reg_ = Reg(reg_.GetId(), tp);
    }

    virtual void Release()
    {
        if (reg_.IsValid()) {
            encoder_->ReleaseScratchRegister(reg_);
            reg_ = INVALID_REGISTER;
        }
    }

    void Acquire()
    {
        ASSERT(!reg_.IsValid());
        reg_ = encoder_->AcquireScratchRegister(Is64BitsArch(encoder_->GetArch()) ? INT64_TYPE : INT32_TYPE);
        ASSERT(reg_.IsValid());
    }

    void AcquireWithLr()
    {
        ASSERT(!reg_.IsValid());
        auto linkReg = encoder_->GetTarget().GetLinkReg();
        if (encoder_->IsLrAsTempRegEnabled() && encoder_->IsScratchRegisterReleased(linkReg)) {
            reg_ = linkReg;
            encoder_->AcquireScratchRegister(linkReg);
        } else {
            reg_ = encoder_->AcquireScratchRegister(Is64BitsArch(encoder_->GetArch()) ? INT64_TYPE : INT32_TYPE);
        }
        ASSERT(reg_.IsValid());
    }

    void AcquireIfInvalid()
    {
        if (!reg_.IsValid()) {
            reg_ = encoder_->AcquireScratchRegister(Is64BitsArch(encoder_->GetArch()) ? INT64_TYPE : INT32_TYPE);
            ASSERT(reg_.IsValid());
        }
    }

protected:
    Encoder *GetEncoder()
    {
        return encoder_;
    }

private:
    Encoder *encoder_ {nullptr};
    Reg reg_;
};

struct ScopedTmpReg : public ScopedTmpRegImpl<false> {
    using ScopedTmpRegImpl<false>::ScopedTmpRegImpl;
};

struct ScopedTmpRegLazy : public ScopedTmpRegImpl<true> {
    using ScopedTmpRegImpl<true>::ScopedTmpRegImpl;
};

struct ScopedTmpRegU16 : public ScopedTmpReg {
    explicit ScopedTmpRegU16(Encoder *encoder) : ScopedTmpReg(encoder, INT16_TYPE) {}
};

struct ScopedTmpRegU32 : public ScopedTmpReg {
    explicit ScopedTmpRegU32(Encoder *encoder) : ScopedTmpReg(encoder, INT32_TYPE) {}
};

struct ScopedTmpRegU64 : public ScopedTmpReg {
    explicit ScopedTmpRegU64(Encoder *encoder) : ScopedTmpReg(encoder, INT64_TYPE) {}
};

struct ScopedTmpRegF32 : public ScopedTmpReg {
    explicit ScopedTmpRegF32(Encoder *encoder) : ScopedTmpReg(encoder, FLOAT32_TYPE) {}
};

struct ScopedTmpRegF64 : public ScopedTmpReg {
    explicit ScopedTmpRegF64(Encoder *encoder) : ScopedTmpReg(encoder, FLOAT64_TYPE) {}
};

struct ScopedTmpRegRef : public ScopedTmpReg {
    explicit ScopedTmpRegRef(Encoder *encoder) : ScopedTmpReg(encoder, encoder->GetRefType()) {}
};

class ScopedLiveTmpReg : public ScopedTmpReg {
public:
    explicit ScopedLiveTmpReg(Encoder *encoder) : ScopedTmpReg(encoder)
    {
        encoder->AddRegInLiveMask(GetReg());
    }
    ScopedLiveTmpReg(Encoder *encoder, TypeInfo type) : ScopedTmpReg(encoder, type)
    {
        encoder->AddRegInLiveMask(GetReg());
    }

    void Release() final
    {
        ASSERT(GetReg().IsValid());
        GetEncoder()->RemoveRegFromLiveMask(GetReg());
        ScopedTmpReg::Release();
    }

    ~ScopedLiveTmpReg() override
    {
        if (GetReg().IsValid()) {
            GetEncoder()->RemoveRegFromLiveMask(GetReg());
        }
    }

    NO_COPY_SEMANTIC(ScopedLiveTmpReg);
    NO_MOVE_SEMANTIC(ScopedLiveTmpReg);
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_SCOPED_TMP_REG_H
