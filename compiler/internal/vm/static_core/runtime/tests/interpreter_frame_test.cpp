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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assembly-parser.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/utils.h"
#include "libarkfile/bytecode_emitter.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/value.h"
#include "optimizer/ir/runtime_interface.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/core/core_class_linker_extension.h"
#include "runtime/tests/class_linker_test_extension.h"
#include "runtime/tests/interpreter/test_interpreter.h"
#include "runtime/tests/interpreter/test_runtime_interface.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/hclass.h"
#include "runtime/handle_base-inl.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/native_pointer.h"
#include "runtime/tests/test_utils.h"
#include "libarkbase/test_utilities.h"
#include "runtime/tests/interpreter_test_utils.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::interpreter::test {

class InterpreterFrameWithSTWTest : public testing::Test {
public:
    InterpreterFrameWithSTWTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetRunGcInPlace(true);
        options.SetGcTriggerType("debug-never");
        options.SetVerifyCallStack(false);
        options.SetGcType("stw");
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~InterpreterFrameWithSTWTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(InterpreterFrameWithSTWTest);
    NO_MOVE_SEMANTIC(InterpreterFrameWithSTWTest);

private:
    ark::MTManagedThread *thread_;
};

static inline void RunTask(ark::mem::GC *gc)
{
    ScopedNativeCodeThread sn(ManagedThread::GetCurrent());
    GCTask task(GCTaskCause::OOM_CAUSE);
    task.Run(*gc);
}

#if defined(PANDA_TARGET_ARM32) && defined(NDEBUG)
DEATH_TEST_F(InterpreterFrameWithSTWTest, DISABLED_TestCreateFrame)
#else
DEATH_TEST_F(InterpreterFrameWithSTWTest, TestCreateFrame)
#endif
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    size_t vregNum1 = 16U;

    BytecodeEmitter emitter1;

    emitter1.CallShort(1U, 3U, interpreter::test::RuntimeInterface::METHOD_ID.AsIndex());
    emitter1.ReturnWide();

    std::vector<uint8_t> bytecode1;
    ASSERT_EQ(emitter1.Build(&bytecode1), BytecodeEmitter::ErrorCode::SUCCESS);

    auto f1 = CreateFrame(vregNum1, nullptr, nullptr);

    auto cls1 = CreateClass(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto methodData1 = CreateMethod(cls1, f1.get(), bytecode1);
    auto method1 = std::move(methodData1.first);
    f1->SetMethod(method1.get());

    auto frameHandler1 = StaticFrameHandler(f1.get());
    frameHandler1.GetVReg(1U).SetPrimitive(1_I);
    frameHandler1.GetVReg(3U).SetPrimitive(2_I);

    size_t vregNum2 = 65535;

    BytecodeEmitter emitter2;

    emitter2.LdaObj(1);
    emitter2.ReturnObj();

    std::vector<uint8_t> bytecode2;
    ASSERT_EQ(emitter2.Build(&bytecode2), BytecodeEmitter::ErrorCode::SUCCESS);

    auto f2 = interpreter::test::CreateFrameWithSize(Frame::GetActualSize<false>(vregNum2), vregNum2, method1.get(),
                                                     f1.get(), ManagedThread::GetCurrent());

    auto cls2 = CreateClass(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto methodData2 = CreateMethod(cls2, f2, bytecode2);
    auto method2 = std::move(methodData2.first);
    f2->SetMethod(method2.get());
    ManagedThread::GetCurrent()->SetCurrentFrame(f2);

    auto frameHandler2 = StaticFrameHandler(f2);
    for (size_t i = 0; i < vregNum2; i++) {
        frameHandler2.GetVReg(i).SetReference(ark::mem::AllocNonMovableObject());
    }

    size_t allocSz = sizeof(interpreter::VRegister) * vregNum2;
    memset_s(ToVoidPtr(ToUintPtr(f2) + CORE_EXT_FRAME_DATA_SIZE + sizeof(Frame)), allocSz, 0xff, allocSz);

    ark::mem::GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();

    {
        ScopedNativeCodeThread sn(ManagedThread::GetCurrent());
        GCTask task(GCTaskCause::OOM_CAUSE);
        ASSERT_DEATH(task.Run(*gc), "");
    }

    ark::FreeFrameInterp(f2, ManagedThread::GetCurrent());

    f2 = CreateFrameWithSize(Frame::GetActualSize<false>(vregNum2), vregNum2, method1.get(), f1.get(),
                             ManagedThread::GetCurrent());

    RunTask(gc);

    ark::FreeFrameInterp(f2, ManagedThread::GetCurrent());
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::interpreter::test
