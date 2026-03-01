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

#include <gtest/gtest.h>
#include "callconv.h"
#include "encode.h"
#include "mem/pool_manager.h"
#include "operands.h"
#include "registers_description.h"

namespace panda::compiler {
class ConstructorTest : public ::testing::Test {
public:
    ConstructorTest()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }
    ~ConstructorTest()
    {
        // For additional destroy MasmHolder:
        // There are two main ways - make ref-counting in each object
        // For this - need create additional Free-method for each of them.
        // Or create and destroy main MasmHolder user - encoder.
        // Here implemented second way.
        // Also it is possible to create id for allocators or some another
        // uniqueness technic. But it may do not solve this issue.
        Encoder *encoder = nullptr;

#ifdef PANDA_COMPILER_TARGET_AARCH32
        encoder = Encoder::Create(GetAllocator(), Arch::AARCH32, false);
        ASSERT(encoder->InitMasm());
        encoder->~Encoder();
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        encoder = Encoder::Create(GetAllocator(), Arch::AARCH64, false);
        ASSERT(encoder->InitMasm());
        encoder->~Encoder();
#endif
#ifdef PANDA_COMPILER_TARGET_X86
        encoder = Encoder::Create(GetAllocator(), Arch::X86, false);
        // TODO! Enable when will supported
        if (encoder != nullptr) {
            encoder->~Encoder();
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        encoder = Encoder::Create(GetAllocator(), Arch::X86_64, false);
        ASSERT(encoder->InitMasm());
        encoder->~Encoder();
#endif
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_;
};

TEST_F(ConstructorTest, BackendSupport)
{
#ifdef PANDA_COMPILER_TARGET_AARCH32
    ASSERT_TRUE(BackendSupport(Arch::AARCH32));
#else
    ASSERT_FALSE(BackendSupport(Arch::AARCH32));
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
    ASSERT_TRUE(BackendSupport(Arch::AARCH64));
#else
    ASSERT_FALSE(BackendSupport(Arch::AARCH64));
#endif
#ifdef PANDA_COMPILER_TARGET_X86
    // TODO! Enable when will supported
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
    ASSERT_TRUE(BackendSupport(Arch::X86_64));
#else
    ASSERT_FALSE(BackendSupport(Arch::X86_64));
#endif

    ASSERT_FALSE(BackendSupport(Arch::NONE));
}

TEST_F(ConstructorTest, Encoder)
{
    Encoder *encoder = nullptr;

    encoder = Encoder::Create(GetAllocator(), Arch::AARCH32, false);
#ifdef PANDA_COMPILER_TARGET_AARCH32
    ASSERT_TRUE(encoder->InitMasm());
    ASSERT_TRUE(encoder->IsValid());
    ASSERT_NE(encoder, nullptr);
    encoder->~Encoder();
#else
    ASSERT_EQ(encoder, nullptr);
#endif

    encoder = Encoder::Create(GetAllocator(), Arch::AARCH64, false);
#ifdef PANDA_COMPILER_TARGET_AARCH64
    ASSERT_TRUE(encoder->InitMasm());
    ASSERT_TRUE(encoder->IsValid());
    ASSERT_NE(encoder, nullptr);
    encoder->~Encoder();
#else
    ASSERT_EQ(encoder, nullptr);
#endif

    encoder = Encoder::Create(GetAllocator(), Arch::X86, false);
#ifdef PANDA_COMPILER_TARGET_X86
    // TODO! Enable when will supported
    if (encoder != nullptr) {
        encoder->~Encoder();
    }
#else
    ASSERT_EQ(encoder, nullptr);
#endif

    encoder = Encoder::Create(GetAllocator(), Arch::X86_64, false);
#ifdef PANDA_COMPILER_TARGET_X86_64
    ASSERT_TRUE(encoder->InitMasm());
    ASSERT_TRUE(encoder->IsValid());
    ASSERT_NE(encoder, nullptr);
    encoder->~Encoder();
#else
    ASSERT_EQ(encoder, nullptr);
#endif

    encoder = Encoder::Create(GetAllocator(), Arch::NONE, false);
    ASSERT_EQ(encoder, nullptr);
}

TEST_F(ConstructorTest, RegistersDescription)
{
    RegistersDescription *regfile = nullptr;

    regfile = RegistersDescription::Create(GetAllocator(), Arch::AARCH32);
#ifdef PANDA_COMPILER_TARGET_AARCH32
    ASSERT_TRUE(regfile->IsValid());
    ASSERT_NE(regfile, nullptr);
#else
    ASSERT_EQ(regfile, nullptr);
#endif

    regfile = RegistersDescription::Create(GetAllocator(), Arch::AARCH64);
#ifdef PANDA_COMPILER_TARGET_AARCH64
    ASSERT_TRUE(regfile->IsValid());
    ASSERT_NE(regfile, nullptr);
#else
    ASSERT_EQ(regfile, nullptr);
#endif

    regfile = RegistersDescription::Create(GetAllocator(), Arch::X86);
#ifdef PANDA_COMPILER_TARGET_X86
    // TODO! Enable when will supported
#else
    ASSERT_EQ(regfile, nullptr);
#endif

    regfile = RegistersDescription::Create(GetAllocator(), Arch::X86_64);
#ifdef PANDA_COMPILER_TARGET_X86_64
    ASSERT_TRUE(regfile->IsValid());
    ASSERT_NE(regfile, nullptr);
#else
    ASSERT_EQ(regfile, nullptr);
#endif

    regfile = RegistersDescription::Create(GetAllocator(), Arch::NONE);
    ASSERT_EQ(regfile, nullptr);
}

TEST_F(ConstructorTest, CallingConvention)
{
    CallingConvention *callconv = nullptr;
    RegistersDescription *regfile = nullptr;
    Encoder *encoder = nullptr;

    callconv = CallingConvention::Create(GetAllocator(), encoder, regfile, Arch::AARCH32);
#ifdef PANDA_COMPILER_TARGET_AARCH32
    ASSERT_TRUE(callconv->IsValid());
    ASSERT_NE(callconv, nullptr);
#else
    ASSERT_EQ(callconv, nullptr);
#endif

    callconv = CallingConvention::Create(GetAllocator(), encoder, regfile, Arch::AARCH64);
#ifdef PANDA_COMPILER_TARGET_AARCH64
    ASSERT_TRUE(callconv->IsValid());
    ASSERT_NE(callconv, nullptr);
#else
    ASSERT_EQ(callconv, nullptr);
#endif

    callconv = CallingConvention::Create(GetAllocator(), encoder, regfile, Arch::X86);
#ifdef PANDA_COMPILER_TARGET_X86
    // TODO! Enable when will supported
#else
    ASSERT_EQ(callconv, nullptr);
#endif

    callconv = CallingConvention::Create(GetAllocator(), encoder, regfile, Arch::X86_64);
#ifdef PANDA_COMPILER_TARGET_X86_64
    ASSERT_TRUE(callconv->IsValid());
    ASSERT_NE(callconv, nullptr);
#else
    ASSERT_EQ(callconv, nullptr);
#endif

    callconv = CallingConvention::Create(GetAllocator(), encoder, regfile, Arch::NONE);
    ASSERT_EQ(callconv, nullptr);
}
}  // namespace panda::compiler
