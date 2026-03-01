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

#include "compiler/tests/unit_test.h"
#include "irtoc/backend/compiler/dangling_pointers_checker.h"
#include "irtoc/backend/function.h"
#include "irtoc/backend/irtoc_runtime.h"

namespace ark::compiler {
class DanglingPointersCheckerTest : public GraphTest {};

class RelocationHandlerTest : public ark::irtoc::Function {
public:
    void MakeGraphImpl() override {};
    const char *GetName() const override
    {
        return "Test";
    };
    void SetTestExternalFunctions(std::initializer_list<std::string> funcs)
    {
        this->SetExternalFunctions(funcs);
    }
};

// NOLINTBEGIN(readability-magic-numbers)

// Correct load accumulator from frame with one instruction:
//    correct_acc_load  := LoadI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
// Correct store into frame with instruction:
//    correct_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test0)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::LoadI).Inputs(0U).ref().Imm(frameAccOffset);
            INST(6U, Opcode::StoreI).ref().Inputs(0U, 5U).Imm(frameAccOffset);

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from frame with two instructions:
//    acc_ptr           := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
//    correct_acc_load  := LoadI(acc_ptr).Imm(frame_acc_offset).ref
// Correct store into frame with instruction:
//    correct_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test1)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10U, 10U).i64();
        CONSTANT(11U, 15U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::AddI).Inputs(0U).Imm(frameAccOffset).ptr();
            INST(6U, Opcode::LoadI).Inputs(5U).ref().Imm(0U);
            INST(8U, Opcode::StoreI).ref().Inputs(0U, 6U).Imm(frameAccOffset);
            INST(7U, Opcode::Mul).Inputs(10U, 11U).i64();

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).Inputs({{compiler::DataType::INT64, 7U}}).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Incorrect load accumulator from fake frame with two instructions:
//    fake_acc_ptr       := AddI(fake_frame).Imm(frame_acc_offset).ptr
//    incorrect_acc_load := LoadI(fake_acc_ptr).Imm(0).ref
// Correct store into frame with instruction:
//    correct_acc_store  := StoreI(LiveIn(frame).ptr, LiveIn(acc).ptr).Imm(frame_acc_offset)
TEST_F(DanglingPointersCheckerTest, test2)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).i64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::AddI).Inputs(1U).Imm(frameAccOffset).ptr();
            INST(6U, Opcode::LoadI).Inputs(5U).ref().Imm(0U);
            INST(8U, Opcode::StoreI).u64().Inputs(0U, 1U).Imm(frameAccOffset);

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Incorrect load accumulator from fake frame with two instructions:
//    fake_acc_ptr        := AddI(fake_frame).Imm(frame_acc_offset).ptr
//    incorrect_acc_load  := LoadI(fake_acc_ptr).Imm(0).ref
// Incorrect store fake_acc_load into frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, fake_acc_load).Imm(frame_acc_offset)
TEST_F(DanglingPointersCheckerTest, test3)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).i64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::AddI).Inputs(1U).Imm(frameAccOffset).ptr();
            INST(6U, Opcode::LoadI).Inputs(5U).ref().Imm(0U);
            INST(8U, Opcode::StoreI).ref().Inputs(0U, 6U).Imm(frameAccOffset);

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Incorrect load accumulator from fake frame with two instructions:
//    fake_acc_ptr        := AddI(fake_frame).Imm(frame_acc_offset).ptr
//    incorrect_acc_load  := LoadI(fake_acc_ptr).Imm(0).ref
// Incorrect store fake_acc_load into frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, fake_acc_load).Imm(frame_acc_offset)
// But acc type is integer: LiveIn(acc).u64; so the check is not performed
TEST_F(DanglingPointersCheckerTest, test4)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).i64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(10U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::AddI).Inputs(10U).Imm(frameAccOffset).ptr();
            INST(6U, Opcode::LoadI).Inputs(5U).ref().Imm(0U);
            INST(8U, Opcode::StoreI).ref().Inputs(0U, 6U).Imm(frameAccOffset);

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from last frame definition with two instructions:
//    last_frame_def    := LoadI(LiveIn(frame).ptr).Imm(GetPrevFrameOffset()).ptr
//    acc_ptr           := AddI(last_frame_def).Imm(frame_acc_offset).ptr
//    correct_acc_load  := LoadI(acc_ptr).Imm(0).ref
// Correct store into frame with instruction:
//    correct_acc_store := StoreI(last_frame_def, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test5)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10U, 10U).i64();
        CONSTANT(11U, 15U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::LoadI).Inputs(0U).ptr().Imm(Frame::GetPrevFrameOffset());
            INST(5U, Opcode::AddI).Inputs(8U).Imm(frameAccOffset).ptr();
            INST(6U, Opcode::LoadI).Inputs(5U).ref().Imm(0U);
            INST(7U, Opcode::Mul).Inputs(10U, 11U).i64();
            INST(9U, Opcode::StoreI).ref().Inputs(8U, 6U).Imm(frameAccOffset);

            // store tag
            INST(31U, Opcode::StoreI).u64().Inputs(5U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).Inputs({{compiler::DataType::INT64, 7U}}).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from last frame definition with one instructions:
//    last_frame_def      := LoadI(LiveIn(frame).ptr).Imm(GetPrevFrameOffset()).ptr
//    correct_acc_load    := LoadI(last_frame_def).Imm(frame_acc_offset).ref
// Incorrect store into old frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test6)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);
        INST(10U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::LoadI).Inputs(2U).ptr().Imm(ManagedThread::GetFrameOffset());
            INST(5U, Opcode::LoadI).Inputs(8U).Imm(frameAccOffset).ref();
            INST(9U, Opcode::StoreI).ref().Inputs(0U, 5U).Imm(frameAccOffset);

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 10U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Correct load accumulator from last frame definition with one instructions:
//    last_frame_def      := LoadI(LiveIn(frame).ptr).Imm(GetPrevFrameOffset()).ptr
//    correct_acc_load    := LoadI(last_frame_def).Imm(frame_acc_offset).ref
// Incorrect store into old frame with instruction:
//    incorrect_acc_store := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
TEST_F(DanglingPointersCheckerTest, test7)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10U, 10U).i64();
        CONSTANT(11U, 15U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::LoadI).Inputs(0U).ptr().Imm(Frame::GetPrevFrameOffset());
            INST(5U, Opcode::AddI).Inputs(8U).Imm(frameAccOffset).ptr();
            INST(6U, Opcode::LoadI).Inputs(5U).ref().Imm(0U);
            INST(7U, Opcode::Mul).Inputs(10U, 11U).i64();
            INST(9U, Opcode::StoreI).ref().Inputs(0U, 6U).Imm(frameAccOffset);

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).Inputs({{compiler::DataType::INT64, 7U}}).ptr();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Use old accumulator after Call
TEST_F(DanglingPointersCheckerTest, test8)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::LoadI).Inputs(0U).ref().Imm(frameAccOffset);
            INST(6U, Opcode::StoreI).ref().Inputs(0U, 5U).Imm(frameAccOffset);

            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 5U).Imm(frameAccOffset);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// The correct graph. Not use old acc after call.
//          start_bb
//             |
//             V
//   ------------------------------------
//  | bb_2 | new_acc_load, new_acc_store |
//   ------------------------------------
//      |
//      V
//   --------------------    --------------------
//  | bb_3 | use_old_acc |  | bb_4 | use_old_acc |
//   --------------------    --------------------
//      |                      |
//      V                      V
//    -------------          ------
//   | bb_5 | Call |        | bb_8 |
//    -------------\        -------
//      |           |       A     |
//      V           V       |     V
//    ------         --------     ------
//   | bb_6 |       |  bb_7  |-->| bb_9 |--> end_bb
//    ------         --------     ------
// CC-OFFNXT(huge_method[C++],G.FUN.01-CPP) graph creation
TEST_F(DanglingPointersCheckerTest, test9)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10U, 10U);
        CONSTANT(11U, 11U);
        CONSTANT(12U, 12U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::LoadI).Inputs(0U).ref().Imm(frameAccOffset);
            INST(5U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
            INST(19U, Opcode::AddI).Inputs(11U).Imm(frameAccOffset).u64();
            INST(20U, Opcode::Mul).Inputs(12U, 19U).u64();
            INST(22U, Opcode::If).Inputs(19U, 20U).b().CC(CC_LE);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
        }
        BASIC_BLOCK(4U, 8U)
        {
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(17U, Opcode::AddI).Inputs(11U).Imm(frameAccOffset).u64();
            INST(18U, Opcode::Mul).Inputs(12U, 17U).u64();
            INST(21U, Opcode::If).Inputs(17U, 18U).b().CC(CC_NE);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(8U, Opcode::AddI).Inputs(10U).Imm(frameAccOffset).u64();
        }
        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(16U, Opcode::AddI).Inputs(11U).Imm(frameAccOffset).u64();
            INST(9U, Opcode::Mul).Inputs(12U, 16U).u64();
            INST(15U, Opcode::If).Inputs(9U, 16U).b().CC(CC_EQ);
        }
        BASIC_BLOCK(8U, 9U)
        {
            INST(13U, Opcode::AddI).Inputs(12U).Imm(frameAccOffset).u64();
        }

        BASIC_BLOCK(9U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// The incorrect graph. Use old acc after call.
//          start_bb
//             |
//             V
//   ------------------------------------
//  | bb_2 | new_acc_load, new_acc_store |
//   ------------------------------------
//      |
//      V
//   --------------------    --------------------
//  | bb_3 | use_old_acc |  | bb_4 | use_old_acc |
//   --------------------    --------------------
//      |                      |
//      V                      V
//    -------------          --------------------
//   | bb_5 | Call |        | bb_8 | use_old_acc |
//    -------------\        ---------------------
//      |           |       A     |
//      V           V       |     V
//    ------         --------     ------
//   | bb_6 |       |  bb_7  |-->| bb_9 |--> end_bb
//    ------         --------     ------
// CC-OFFNXT(huge_method[C++],G.FUN.01-CPP) graph creation
TEST_F(DanglingPointersCheckerTest, test10)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10U, 10U);
        CONSTANT(11U, 11U);
        CONSTANT(12U, 12U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::LoadI).Inputs(0U).ref().Imm(frameAccOffset);
            INST(5U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
            INST(19U, Opcode::AddI).Inputs(11U).Imm(frameAccOffset).u64();
            INST(20U, Opcode::Mul).Inputs(12U, 19U).u64();
            INST(22U, Opcode::If).Inputs(19U, 20U).b().CC(CC_LE);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
        }
        BASIC_BLOCK(4U, 8U)
        {
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            // store tag
            INST(30U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(31U, Opcode::StoreI).u64().Inputs(30U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(17U, Opcode::AddI).Inputs(11U).Imm(frameAccOffset).u64();
            INST(18U, Opcode::Mul).Inputs(12U, 17U).u64();
            INST(21U, Opcode::If).Inputs(17U, 18U).b().CC(CC_NE);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(8U, Opcode::AddI).Inputs(10U).Imm(frameAccOffset).u64();
        }
        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(16U, Opcode::AddI).Inputs(11U).Imm(frameAccOffset).u64();
            INST(9U, Opcode::Mul).Inputs(12U, 16U).u64();
            INST(15U, Opcode::If).Inputs(9U, 16U).b().CC(CC_EQ);
        }
        BASIC_BLOCK(8U, 9U)
        {
            INST(23U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
            INST(13U, Opcode::AddI).Inputs(12U).Imm(frameAccOffset).u64();
        }

        BASIC_BLOCK(9U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// The correct graph. Use Phi to define acc that has been changed in one branch.
//          start_bb
//             |
//             V
//         -----------------------------
//        |           bb_2              |
//         -----------------------------
//         |                           |
//         V                           |
//   -----------------------------     |
//  | bb_3 | load_acc, change_acc |    |
//   -----------------------------     |
//                             |       |
//                             V       V
//               ------------------------------------------------
//              |      | last_acc_def = Phi(live_in, change_acc) |
//              | bb_4 | store(last_acc_def)                     |
//              |      | Call                                    |
//               ------------------------------------------------
//                                     |
//                                     V
//                                  end_bb

TEST_F(DanglingPointersCheckerTest, test11)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        CONSTANT(10U, 10U);
        CONSTANT(11U, 11U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::AddI).Inputs(10U).Imm(frameAccOffset).u64();
            INST(4U, Opcode::If).Inputs(3U, 11U).b().CC(CC_NE);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(5U, Opcode::LoadI).Inputs(0U).ref().Imm(frameAccOffset);
            INST(6U, Opcode::AddI).Inputs(5U).ptr().Imm(10U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Phi).Inputs(1U, 6U).ptr();
            INST(8U, Opcode::StoreI).ref().Inputs(0U, 7U).Imm(frameAccOffset);

            // store tag
            INST(21U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(22U, Opcode::StoreI).u64().Inputs(21U, 2U).Imm(accTagOffset);

            INST(9U, Opcode::Call).TypeId(0U).ptr();
            INST(20U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

/*
 * Use object after call
 * frame_live_in := LiveIn(frame)
 * Call
 * frame_use := LoadI(frame_live_in).Imm(frame_acc_offset).i64
 */
TEST_F(DanglingPointersCheckerTest, test12)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 1U).Imm(frameAccOffset);

            // store tag
            INST(8U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(9U, Opcode::StoreI).u64().Inputs(8U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(5U, Opcode::AddI).Inputs(1U).ptr().Imm(frameAccOffset);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

/*
 * Use primitive after call:
 *    frame_live_in := LiveIn(frame)
 *    primitive := LoadI(frame_live_in).Imm(offset).u64
 *    Call
 *    primitive_use := AddI(not_object).Imm(10).u64
 */
TEST_F(DanglingPointersCheckerTest, test13)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadI).Inputs(0U).u64().Imm(10U);
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 1U).Imm(frameAccOffset);

            // store tag
            INST(8U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(9U, Opcode::StoreI).u64().Inputs(8U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(5U, Opcode::AddI).Inputs(6U).u64().Imm(frameAccOffset);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// Use skipped pointer inst after call:
//    frame_live_in := LiveIn(frame)
//    pointer := AddI(frame_live_in).Imm(frame_acc_offset).ptr
//    Call
//    primitive := LoadI(pointer).Imm(0).u64
TEST_F(DanglingPointersCheckerTest, test14)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 1U).Imm(frameAccOffset);

            // store tag
            INST(8U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(9U, Opcode::StoreI).u64().Inputs(8U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(5U, Opcode::LoadI).Inputs(6U).u64().Imm(0U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

/*
 * Use ref inst after call:
 *    pointer := AddI(acc_live_in).Imm(10).ptr
 *    Call
 *    primitive := LoadI(pointer).Imm(0).u64
 */
TEST_F(DanglingPointersCheckerTest, test15)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::LoadI).Inputs(0U).ref().Imm(10U);
            INST(6U, Opcode::AddI).Inputs(1U).ptr().Imm(10U);
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 1U).Imm(frameAccOffset);

            // store tag
            INST(8U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(9U, Opcode::StoreI).u64().Inputs(8U, 2U).Imm(accTagOffset);

            INST(3U, Opcode::Call).TypeId(0U).ptr();
            INST(5U, Opcode::LoadI).Inputs(10U).u64().Imm(0U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

/*
 * Correct load accumulator from frame:
 *    correct_acc_load      := LoadI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
 * Correct accumulatore and tag store:
 *    correct_acc_store     := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
 *    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
 *    correct_acc_tag_store := StoreI(acc_ptr, LiveIn(acc_tag).u64).Imm(acc_tag_offset).u64
 */
TEST_F(DanglingPointersCheckerTest, test16)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::LoadI).Inputs(0U).ref().Imm(frameAccOffset);
            INST(5U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(6U, Opcode::StoreI).ref().Inputs(0U, 4U).Imm(frameAccOffset);
            INST(7U, Opcode::StoreI).u64().Inputs(5U, 2U).Imm(accTagOffset);
            INST(8U, Opcode::Call).TypeId(0U).ptr();
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

/*
 * Correct load accumulator and tag:
 *    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
 *    correct_acc_load      := LoadI(acc_ptr).Imm(frame_acc_offset).ref
 *    correct_acc_tag_load  := LoadI(acc_ptr).Imm(acc_tag_offset).u64
 * Correct accumulatore and tag store:
 *    correct_acc_store     := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
 *    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
 *    correct_acc_tag_store := StoreI(acc_ptr, correct_acc_tag_load).Imm(acc_tag_offset).u64
 */
TEST_F(DanglingPointersCheckerTest, test17)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(5U, Opcode::LoadI).Inputs(4U).ref().Imm(0U);
            INST(6U, Opcode::LoadI).Inputs(4U).i64().Imm(accTagOffset);
            INST(7U, Opcode::StoreI).ref().Inputs(0U, 5U).Imm(frameAccOffset);
            INST(8U, Opcode::StoreI).u64().Inputs(4U, 6U).Imm(accTagOffset);
            INST(9U, Opcode::Call).TypeId(0U).ptr();
            INST(10U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

/*
 * Correct load accumulator and tag:
 *    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
 *    correct_acc_load      := LoadI(acc_ptr).Imm(frame_acc_offset).ref
 *    acc_tag_ptr           := AddI(acc_ptr).Imm(acc_tag_offset).ref
 *    correct_acc_tag_load  := LoadI(acc_tag_ptr).Imm(0).u64
 * Correct accumulatore and tag store:
 *    correct_acc_store     := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
 *    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
 *    correct_acc_tag_store := StoreI(acc_ptr, correct_acc_tag_load).Imm(acc_tag_offset).u64
 */
TEST_F(DanglingPointersCheckerTest, test18)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(5U, Opcode::LoadI).Inputs(4U).ref().Imm(0U);
            INST(6U, Opcode::AddI).Inputs(4U).ptr().Imm(accTagOffset);
            INST(7U, Opcode::LoadI).Inputs(6U).i64().Imm(0U);
            INST(8U, Opcode::StoreI).ref().Inputs(0U, 5U).Imm(frameAccOffset);
            INST(9U, Opcode::StoreI).u64().Inputs(4U, 7U).Imm(accTagOffset);
            INST(10U, Opcode::Call).TypeId(0U).ptr();
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_TRUE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

/*
 * Correct load accumulator and tag:
 *    acc_ptr               := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ptr
 *    correct_acc_load      := LoadI(acc_ptr).Imm(frame_acc_offset).ref
 *    acc_tag_ptr           := AddI(acc_ptr).Imm(acc_tag_offset).ref
 *    correct_acc_tag_load  := LoadI(acc_tag_ptr).Imm(0).u64
 * Correct accumulatore and tag store:
 *    correct_acc_store       := StoreI(LiveIn(frame).ptr, correct_acc_load).Imm(frame_acc_offset).ref
 *    acc_ptr                 := AddI(LiveIn(frame).ptr).Imm(frame_acc_offset).ref
 *    incorrect_acc_tag_store := StoreI(acc_ptr, LiveIn(acc_tag).u64).Imm(acc_tag_offset).u64
 */
TEST_F(DanglingPointersCheckerTest, test19)
{
    auto arch = ark::RUNTIME_ARCH;
    SetGraphArch(arch);
    if (DanglingPointersChecker::regmap_.find(arch) == DanglingPointersChecker::regmap_.end()) {
        return;
    }
    auto frameAccOffset = cross_values::GetFrameAccOffset(arch);
    auto accTagOffset = cross_values::GetFrameAccMirrorOffset(arch);
    RelocationHandlerTest relocationHandler;
    relocationHandler.SetTestExternalFunctions({*DanglingPointersChecker::targetFuncs_.begin()});
    GetGraph()->SetRelocationHandler(&relocationHandler);
    GetGraph()->SetMethod(&relocationHandler);
    GetGraph()->SetMode(GraphMode::InterpreterEntry());

    GRAPH(GetGraph())
    {
        INST(0U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["frame"]);
        INST(1U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["acc"]);
        INST(2U, Opcode::LiveIn).u64().DstReg(DanglingPointersChecker::regmap_[arch]["acc_tag"]);
        INST(3U, Opcode::LiveIn).ptr().DstReg(DanglingPointersChecker::regmap_[arch]["thread"]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::AddI).Inputs(0U).ptr().Imm(frameAccOffset);
            INST(5U, Opcode::LoadI).Inputs(4U).ref().Imm(0U);
            INST(6U, Opcode::AddI).Inputs(4U).ptr().Imm(accTagOffset);
            INST(7U, Opcode::LoadI).Inputs(6U).i64().Imm(0U);
            INST(8U, Opcode::StoreI).ref().Inputs(0U, 5U).Imm(frameAccOffset);
            INST(9U, Opcode::StoreI).u64().Inputs(4U, 2U).Imm(accTagOffset);
            INST(10U, Opcode::Call).TypeId(0U).ptr();
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    irtoc::IrtocRuntimeInterface runtime;
    GetGraph()->SetRuntime(&runtime);

    ASSERT_FALSE(GetGraph()->RunPass<ark::compiler::DanglingPointersChecker>());
}

// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
