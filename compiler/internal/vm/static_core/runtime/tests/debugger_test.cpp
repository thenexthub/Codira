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

#include <gtest/gtest.h>

#include <vector>

#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/method.h"
#include "runtime/tooling/debugger.h"
#include "runtime/tests/test_utils.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

namespace ark::debugger::test {

class DebuggerTest : public testing::Test {
public:
    DebuggerTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        // this test doesn't check GC logic - just to make test easier without any handles
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~DebuggerTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(DebuggerTest);
    NO_MOVE_SEMANTIC(DebuggerTest);

private:
    ark::MTManagedThread *thread_;
};

static ObjectHeader *ToPtr(uint64_t v)
{
    return reinterpret_cast<ObjectHeader *>(static_cast<ObjectPointerType>(v));
}

static uint64_t FromPtr(ObjectHeader *ptr)
{
    return static_cast<ObjectPointerType>(reinterpret_cast<uint64_t>(ptr));
}

template <bool IS_DYNAMIC = false>
static Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
{
    uint32_t extSz = EMPTY_EXT_FRAME_DATA_SIZE;
    void *mem = aligned_alloc(8, Frame::GetAllocSize(Frame::GetActualSize<IS_DYNAMIC>(nregs), extSz));
    return new (Frame::FromExt(mem, extSz)) Frame(mem, method, prev, nregs);
}

static void FreeFrame(Frame *frame)
{
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    std::free(frame->GetExt());
}

struct VRegValue {
    uint64_t value {};
    bool isRef {};
};

static void SetVRegs(Frame *frame, std::vector<VRegValue> &regs)
{
    auto frameHandler = StaticFrameHandler(frame);
    for (size_t i = 0; i < regs.size(); i++) {
        if (regs[i].isRef) {
            frameHandler.GetVReg(i).SetReference(ToPtr(regs[i].value));
        } else {
            frameHandler.GetVReg(i).SetPrimitive(static_cast<int64_t>(regs[i].value));
        }
    }
}

static constexpr size_t BYTECODE_OFFSET = 0xeeff;

struct MethodInfo {
    uint32_t nregs;
    uint32_t nargs;
    panda_file::File::EntityId methodId;
};

static void CheckFrame(Frame *frame, std::vector<VRegValue> &regs, const MethodInfo &methodInfo)
{
    SetVRegs(frame, regs);
    auto nregs = methodInfo.nregs;
    auto nargs = methodInfo.nargs;
    auto methodId = methodInfo.methodId;

    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        VRegValue acc {0xaaaaaaaabbbbbbbb, false};
        frame->GetAccAsVReg().SetPrimitive(static_cast<int64_t>(acc.value));
        tooling::PtDebugFrame debugFrame(frame->GetMethod(), frame);

        EXPECT_EQ(debugFrame.GetVRegNum(), nregs);
        EXPECT_EQ(debugFrame.GetArgumentNum(), nargs);
        EXPECT_EQ(debugFrame.GetMethodId(), methodId);
        EXPECT_EQ(debugFrame.GetBytecodeOffset(), BYTECODE_OFFSET);
        EXPECT_EQ(debugFrame.GetAccumulator(), acc.value);
        EXPECT_EQ(debugFrame.GetAccumulatorKind(), tooling::PtFrame::RegisterKind::PRIMITIVE);

        for (size_t i = 0; i < debugFrame.GetVRegNum(); i++) {
            EXPECT_EQ(debugFrame.GetVReg(i), regs[i].value);
            EXPECT_EQ(debugFrame.GetVRegKind(i), regs[i].isRef ? tooling::PtFrame::RegisterKind::REFERENCE
                                                               : tooling::PtFrame::RegisterKind::PRIMITIVE);
        }

        for (size_t i = 0; i < debugFrame.GetArgumentNum(); i++) {
            EXPECT_EQ(debugFrame.GetArgument(i), regs[i + nregs].value);
            EXPECT_EQ(debugFrame.GetArgumentKind(i), regs[i + nregs].isRef ? tooling::PtFrame::RegisterKind::REFERENCE
                                                                           : tooling::PtFrame::RegisterKind::PRIMITIVE);
        }
    }

    {
        VRegValue acc {FromPtr(ark::mem::AllocateNullifiedPayloadString(1)), true};
        frame->GetAccAsVReg().SetReference(ToPtr(acc.value));
        tooling::PtDebugFrame debugFrame(frame->GetMethod(), frame);

        EXPECT_EQ(debugFrame.GetVRegNum(), nregs);
        EXPECT_EQ(debugFrame.GetArgumentNum(), nargs);
        EXPECT_EQ(debugFrame.GetMethodId(), methodId);
        EXPECT_EQ(debugFrame.GetBytecodeOffset(), BYTECODE_OFFSET);
        EXPECT_EQ(debugFrame.GetAccumulator(), acc.value);
        EXPECT_EQ(debugFrame.GetAccumulatorKind(), tooling::PtFrame::RegisterKind::REFERENCE);

        for (size_t i = 0; i < debugFrame.GetVRegNum(); i++) {
            EXPECT_EQ(debugFrame.GetVReg(i), regs[i].value);
            EXPECT_EQ(debugFrame.GetVRegKind(i), regs[i].isRef ? tooling::PtFrame::RegisterKind::REFERENCE
                                                               : tooling::PtFrame::RegisterKind::PRIMITIVE);
        }

        for (size_t i = 0; i < debugFrame.GetArgumentNum(); i++) {
            EXPECT_EQ(debugFrame.GetArgument(i), regs[i + nregs].value);
            EXPECT_EQ(debugFrame.GetArgumentKind(i), regs[i + nregs].isRef ? tooling::PtFrame::RegisterKind::REFERENCE
                                                                           : tooling::PtFrame::RegisterKind::PRIMITIVE);
        }
    }
}

TEST_F(DebuggerTest, Frame)
{
    pandasm::Parser p;

    auto source = R"(
        .function void foo(i32 a0, i32 a1) {
            movi v0, 1
            movi v1, 2
            return.void
        }
    )";

    std::string srcFilename = "src.pa";
    auto res = p.Parse(source, srcFilename);
    ASSERT(p.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);

    auto filePtr = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(filePtr != nullptr);

    PandaString descriptor;
    auto classId = filePtr->GetClassId(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_TRUE(classId.IsValid());

    panda_file::ClassDataAccessor cda(*filePtr, classId);
    panda_file::File::EntityId methodId;
    panda_file::File::EntityId codeId;

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        methodId = mda.GetMethodId();
        ASSERT_TRUE(mda.GetCodeId());
        codeId = mda.GetCodeId().value();
    });

    panda_file::CodeDataAccessor codeDataAccessor(*filePtr, codeId);
    auto nargs = codeDataAccessor.GetNumArgs();
    auto nregs = codeDataAccessor.GetNumVregs();

    Method method(nullptr, filePtr.get(), methodId, codeId, 0, nargs, nullptr);
    ark::Frame *frame = test::CreateFrame(nregs + nargs, &method, nullptr);
    frame->SetBytecodeOffset(BYTECODE_OFFSET);

    // NOLINTBEGIN(readability-magic-numbers)
    std::vector<VRegValue> regs {{0x1111111122222222, false},
                                 {FromPtr(ark::mem::AllocateNullifiedPayloadString(1)), true},
                                 {0x3333333344444444, false},
                                 {FromPtr(ark::mem::AllocateNullifiedPayloadString(1)), true}};
    // NOLINTEND(readability-magic-numbers)
    MethodInfo methodInfo {nregs, nargs, methodId};
    CheckFrame(frame, regs, methodInfo);

    test::FreeFrame(frame);
}

}  // namespace ark::debugger::test
