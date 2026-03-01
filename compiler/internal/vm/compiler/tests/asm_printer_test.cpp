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

#include <climits>

#include <gtest/gtest.h>
#include <random>
#include <sstream>

#include "libpandabase/os/filesystem.h"
#include "mem/base_mem_stats.h"
#include "mem/code_allocator.h"
#include "mem/pool_manager.h"
#include "os/filesystem.h"
#include "target/asm_printer.h"

static const std::string output_dir = "asm_output";

namespace panda::compiler {

template <Arch arch>
class PrinterTest : public ::testing::Test {
public:
    PrinterTest()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB);
#ifdef STDOUT_PRINT
        curr_stream_ = &std::cout;
#else
        curr_stream_ = new std::stringstream();
#endif
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        // Create printer
        encoder_ = Encoder::Create(allocator_, arch, true);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, arch);
        bool pabi = false;
        bool osr = false;
        bool dyn = false;
        bool print = true;
#ifdef PANDA_COMPILER_TARGET_AARCH32
        if constexpr (arch == Arch::AARCH32) {
            dir_suffix_ = "aarch32";
            auto enc = reinterpret_cast<aarch32::Aarch32Assembly *>(encoder_);
            enc->SetStream(curr_stream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, arch, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        if constexpr (arch == Arch::AARCH64) {
            dir_suffix_ = "aarch64";
            auto enc = reinterpret_cast<aarch64::Aarch64Assembly *>(encoder_);
            enc->SetStream(curr_stream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, arch, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        if constexpr (arch == Arch::X86_64) {
            dir_suffix_ = "amd64";
            auto enc = reinterpret_cast<amd64::Amd64Assembly *>(encoder_);
            enc->SetStream(curr_stream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, arch, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
        mem_stats_ = new BaseMemStats();
        code_alloc_ = new (std::nothrow) CodeAllocator(mem_stats_);
        // Create dir if it is not exist
        auto exec_path = panda::os::file::File::GetExecutablePath();
        ASSERT(exec_path);
        exec_path_ = exec_path.Value();
        os::CreateDirectories(exec_path_ + "/" + output_dir);
        os::CreateDirectories(GetDir());
    }
    ~PrinterTest()
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete code_alloc_;
        delete mem_stats_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
        delete curr_stream_;
    }

    CodeAllocator *GetCodeAllocator()
    {
        return code_alloc_;
    }

    std::string GetDir()
    {
        return exec_path_ + "/" + output_dir + "/" + dir_suffix_ + "/";
    }

    void ResetCodeAllocator(void *ptr, size_t size)
    {
        os::mem::MapRange<std::byte> mem_range(static_cast<std::byte *>(ptr), size);
        mem_range.MakeReadWrite();
        delete code_alloc_;
        code_alloc_ = new (std::nothrow) CodeAllocator(mem_stats_);
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    Encoder *GetEncoder()
    {
        return encoder_;
    }

    RegistersDescription *GetRegfile()
    {
        return regfile_;
    }

    CallingConvention *GetCallconv()
    {
        return callconv_;
    }

    size_t GetCursor()
    {
        return curr_cursor_;
    }

    // Warning! Do not use multiply times with different types!
    Reg GetParameter(TypeInfo type, int id = 0)
    {
        ASSERT(id < 4);
        if (type.IsFloat()) {
            return Reg(id, type);
        }
        if constexpr (arch == Arch::AARCH32) {
            // special offset for double-reg param
            if (id == 1 && type.GetSize() == DOUBLE_WORD_SIZE) {
                return Target::Current().GetParamReg(2, type);
            }
        }
        return Target::Current().GetParamReg(id, type);
    }

    void PreWork()
    {
        // Curor need to encode multiply tests due one execution
        curr_cursor_ = 0;
        encoder_->SetCursorOffset(0);

        [[maybe_unused]] std::string func_name = "test_" + GetTestName();
#ifdef PANDA_COMPILER_TARGET_AARCH32
        if constexpr (arch == Arch::AARCH32) {
            auto enc = reinterpret_cast<aarch32::Aarch32Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(func_name.c_str()));
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        if constexpr (arch == Arch::AARCH64) {
            auto enc = reinterpret_cast<aarch64::Aarch64Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(func_name.c_str()));
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        if constexpr (arch == Arch::X86_64) {
            auto enc = reinterpret_cast<amd64::Amd64Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(func_name.c_str()));
        }
#endif
        callconv_->GeneratePrologue(FrameInfo::FullPrologue());
    }

    void PostWork()
    {
        auto param = Target::Current().GetParamReg(0);
        auto return_reg = Target::Current().GetReturnReg();
        if (param.GetId() != return_reg.GetId()) {
            GetEncoder()->EncodeMov(return_reg, param);
        }
        callconv_->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});
        encoder_->Finalize();
    }

#ifdef STDOUT_PRINT
    std::ostream *GetStream()
#else
    std::stringstream *GetStream()
#endif
    {
        return curr_stream_;
    }

    void SetTestName(const std::string &name)
    {
        test_name_ = name;
    }

    const std::string GetTestName()
    {
        return test_name_;
    }

    void FinalizeTest()
    {
        // Make them separate!
        std::string filename = GetTestName() + ".S";
        std::ofstream asm_file;
        asm_file.open(GetDir() + "/" + filename.c_str());
        // Test must generate asembly-flie
        ASSERT(asm_file.is_open());
        // Compile by assembler
#ifndef STDOUT_PRINT
        asm_file << GetStream()->str();
#endif
        asm_file.close();
    }

private:
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    // Callconv for printing
    CallingConvention *cc_ {nullptr};
    // Callconv for masm initialization
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *code_alloc_ {nullptr};
    BaseMemStats *mem_stats_ {nullptr};
    size_t curr_cursor_ {0};
#ifdef STDOUT_PRINT
    std::ostream *curr_stream_;
#else
    std::stringstream *curr_stream_;
#endif
    std::string test_name_;
    std::string exec_path_;
    std::string dir_suffix_;
};

#ifdef PANDA_COMPILER_TARGET_AARCH32
using PrinterAarch32Test = PrinterTest<Arch::AARCH32>;
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
using PrinterAarch64Test = PrinterTest<Arch::AARCH64>;
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
using PrinterAmd64Test = PrinterTest<Arch::X86_64>;
#endif

#define SINGLE_PARAM_TEST_TEMPLATE(test_name, encode_func) \
    template <typename T, Arch arch>                       \
    bool test_name(PrinterTest<arch> *test)                \
    {                                                      \
        test->PreWork();                                   \
        auto param = test->GetParameter(TypeInfo(T(0)));   \
        test->GetEncoder()->encode_func(param, param);     \
        test->PostWork();                                  \
        if (!test->GetEncoder()->GetResult()) {            \
            return false;                                  \
        }                                                  \
        return true;                                       \
    }

SINGLE_PARAM_TEST_TEMPLATE(TestMov, EncodeMov)
SINGLE_PARAM_TEST_TEMPLATE(TestNeg, EncodeNeg)
SINGLE_PARAM_TEST_TEMPLATE(TestAbs, EncodeAbs)
SINGLE_PARAM_TEST_TEMPLATE(TestNot, EncodeNot)
// SINGLE_PARAM_TEST_TEMPLATE(TestSqrt, EncodeSqrt)

#undef SINGLE_PARAM_TEST_TEMPLATE

#define DOUBLE_PARAM_TEST_TEMPLATE(test_name, encode_func)       \
    template <typename T, Arch arch>                             \
    bool test_name(PrinterTest<arch> *test)                      \
    {                                                            \
        test->PreWork();                                         \
        auto param1 = test->GetParameter(TypeInfo(T(0)), 0);     \
        auto param2 = test->GetParameter(TypeInfo(T(0)), 1);     \
        test->GetEncoder()->encode_func(param1, param1, param2); \
        test->PostWork();                                        \
        if (!test->GetEncoder()->GetResult()) {                  \
            return false;                                        \
        }                                                        \
        return true;                                             \
    }

DOUBLE_PARAM_TEST_TEMPLATE(TestAdd, EncodeAdd)
DOUBLE_PARAM_TEST_TEMPLATE(TestSub, EncodeSub)
DOUBLE_PARAM_TEST_TEMPLATE(TestMul, EncodeMul)
DOUBLE_PARAM_TEST_TEMPLATE(TestShl, EncodeShl)
DOUBLE_PARAM_TEST_TEMPLATE(TestShr, EncodeShr)
DOUBLE_PARAM_TEST_TEMPLATE(TestAShr, EncodeAShr)
DOUBLE_PARAM_TEST_TEMPLATE(TestAnd, EncodeAnd)
DOUBLE_PARAM_TEST_TEMPLATE(TestOr, EncodeOr)
DOUBLE_PARAM_TEST_TEMPLATE(TestXor, EncodeXor)

#undef DOUBLE_PARAM_TEST_TEMPLATE

#define ONE_TEST_BODY(test_class, test_method, test_name, arch) \
    TEST_F(test_class, test_method)                             \
    {                                                           \
        bool success = true;                                    \
        SetTestName(#test_name "_8");                           \
        success &= test_method<uint8_t, Arch::arch>(this);      \
        SetTestName(#test_name "_16");                          \
        success &= test_method<uint16_t, Arch::arch>(this);     \
        SetTestName(#test_name "_32");                          \
        success &= test_method<uint32_t, Arch::arch>(this);     \
        SetTestName(#test_name "_64");                          \
        success &= test_method<uint64_t, Arch::arch>(this);     \
        SetTestName(#test_name);                                \
        FinalizeTest();                                         \
        EXPECT_TRUE(success);                                   \
    }

#ifdef PANDA_COMPILER_TARGET_AARCH64
ONE_TEST_BODY(PrinterAarch64Test, TestMov, mov, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestNeg, neg, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAbs, abs, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestNot, not, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAdd, add, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestSub, sub, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestMul, mul, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestShl, shl, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestShr, shr, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAShr, ashr, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAnd, and, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestOr, or, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestXor, xor, AARCH64)
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH32
ONE_TEST_BODY(PrinterAarch32Test, TestMov, mov, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestNeg, neg, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAbs, abs, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestNot, not, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAdd, add, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestSub, sub, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestMul, mul, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestShl, shl, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestShr, shr, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAShr, ashr, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAnd, and, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestOr, or, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestXor, xor, AARCH32)
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
ONE_TEST_BODY(PrinterAmd64Test, TestMov, mov, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestNeg, neg, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAbs, abs, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestNot, not, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAdd, add, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestSub, sub, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestMul, mul, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestShl, shl, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestShr, shr, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAShr, ashr, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAnd, and, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestOr, or, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestXor, xor, X86_64)
#endif

}  // namespace panda::compiler
