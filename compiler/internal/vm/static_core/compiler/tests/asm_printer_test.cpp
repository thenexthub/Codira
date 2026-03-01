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

#include "libarkbase/os/filesystem.h"
#include "libarkbase/mem/base_mem_stats.h"
#include "libarkbase/mem/code_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include "target/asm_printer.h"

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::string g_outputDir = "asm_output";

// Debug print to stdout
#if ENABLE_DEBUG_STDOUT_PRINT
#define STDOUT_PRINT
#endif  // ENABLE_DEBUG_STDOUT_PRINT

namespace ark::compiler {

enum ParamCount { ONE = 1, TWO = 2 };

template <ParamCount PARAMS>
using EncodeFuncType =
    std::conditional_t<PARAMS == ParamCount::TWO, void (Encoder::*)(Reg, Reg, Reg), void (Encoder::*)(Reg, Reg)>;

template <Arch ARCH, ParamCount PARAMS>
class PrinterTest : public testing::TestWithParam<std::pair<std::string_view, EncodeFuncType<PARAMS>>> {
public:
    using Base = testing::TestWithParam<std::pair<std::string_view, EncodeFuncType<PARAMS>>>;

    PrinterTest()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0U, 0U);
#ifdef STDOUT_PRINT
        curr_stream_ = &std::cout;
#else
        currStream_ = new std::stringstream();
#endif
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        // Create printer
        encoder_ = Encoder::Create(allocator_, ARCH, true);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, ARCH);
        SetArch();
        memStats_ = new BaseMemStats();
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
        // Create dir if it is not exist
        auto execPath = ark::os::file::File::GetExecutablePath();
        ASSERT(execPath);
        execPath_ = execPath.Value();
        os::CreateDirectories(execPath_ + "/" + g_outputDir);
        os::CreateDirectories(GetDir());
    }

    ~PrinterTest() override
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete codeAlloc_;
        delete memStats_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
        delete currStream_;
    }

    void SetArch()
    {
        bool pabi = false;
        bool osr = false;
        bool dyn = false;
        bool print = true;
#ifdef PANDA_COMPILER_TARGET_AARCH32
        if constexpr (ARCH == Arch::AARCH32) {
            dirSuffix_ = "aarch32";
            auto enc = reinterpret_cast<aarch32::Aarch32Assembly *>(encoder_);
            enc->SetStream(currStream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, ARCH, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        if constexpr (ARCH == Arch::AARCH64) {
            dirSuffix_ = "aarch64";
            auto enc = reinterpret_cast<aarch64::Aarch64Assembly *>(encoder_);
            enc->SetStream(currStream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, ARCH, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        if constexpr (ARCH == Arch::X86_64) {
            dirSuffix_ = "amd64";
            auto enc = reinterpret_cast<amd64::Amd64Assembly *>(encoder_);
            enc->SetStream(currStream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, ARCH, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
    }

    NO_MOVE_SEMANTIC(PrinterTest);
    NO_COPY_SEMANTIC(PrinterTest);

    CodeAllocator *GetCodeAllocator()
    {
        return codeAlloc_;
    }

    std::string GetDir()
    {
        return execPath_ + "/" + g_outputDir + "/" + dirSuffix_ + "/";
    }

    void ResetCodeAllocator(void *ptr, size_t size)
    {
        os::mem::MapRange<std::byte> memRange(static_cast<std::byte *>(ptr), size);
        memRange.MakeReadWrite();
        delete codeAlloc_;
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
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
        return currCursor_;
    }

    // Warning! Do not use multiply times with different types!
    Reg GetParameter(TypeInfo type, size_t id = 0U)
    {
        ASSERT(id < 4U);
        if (type.IsFloat()) {
            return Reg(id, type);
        }
        if constexpr (ARCH == Arch::AARCH32) {
            // special offset for double-reg param
            if (id == 1U && type.GetSize() == DOUBLE_WORD_SIZE) {
                return Target::Current().GetParamReg(2U, type);
            }
        }
        return Target::Current().GetParamReg(id, type);
    }

    void PreWork()
    {
        // Curor need to encode multiply tests due one execution
        currCursor_ = 0;
        encoder_->SetCursorOffset(0U);

        [[maybe_unused]] std::string funcName = "test_" + GetTestName();
#ifdef PANDA_COMPILER_TARGET_AARCH32
        if constexpr (ARCH == Arch::AARCH32) {
            auto enc = reinterpret_cast<aarch32::Aarch32Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(funcName.c_str()));
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        if constexpr (ARCH == Arch::AARCH64) {
            auto enc = reinterpret_cast<aarch64::Aarch64Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(funcName.c_str()));
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        if constexpr (ARCH == Arch::X86_64) {
            auto enc = reinterpret_cast<amd64::Amd64Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(funcName.c_str()));
        }
#endif
        callconv_->GeneratePrologue(FrameInfo::FullPrologue());
    }

    void PostWork()
    {
        auto param = Target::Current().GetParamReg(0U);
        auto returnReg = Target::Current().GetReturnReg();
        if (param.GetId() != returnReg.GetId()) {
            GetEncoder()->EncodeMov(returnReg, param);
        }
        callconv_->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});
        encoder_->Finalize();
    }

    void DoTest(TypeInfo typeInfo, std::string_view opName, EncodeFuncType<PARAMS> encodeFunc)
    {
        SetTestName(std::string {opName} + "_" + std::to_string(typeInfo.GetSize()));
        PreWork();
        if constexpr (PARAMS == ParamCount::TWO) {
            auto param1 = GetParameter(typeInfo, 0U);
            auto param2 = GetParameter(typeInfo, 1U);
            (GetEncoder()->*encodeFunc)(param1, param1, param2);
        } else {
            auto param = GetParameter(typeInfo);
            (GetEncoder()->*encodeFunc)(param, param);
        }
        PostWork();
        EXPECT_TRUE(GetEncoder()->GetResult());
    }

    void DoTest()
    {
        auto [opName, func] = Base::GetParam();
        for (auto typeId : {TypeInfo::INT8, TypeInfo::INT16, TypeInfo::INT32, TypeInfo::INT64}) {
            DoTest(TypeInfo {typeId}, opName, func);
        }
        SetTestName(std::string {opName});
        FinalizeTest();
    }

#ifdef STDOUT_PRINT
    std::ostream *GetStream()
#else
    std::stringstream *GetStream()
#endif
    {
        return currStream_;
    }

    void SetTestName(std::string name)
    {
        testName_ = std::move(name);
    }

    std::string GetTestName()
    {
        return testName_;
    }

    void FinalizeTest()
    {
        // Make them separate!
        std::string filename = GetTestName() + ".S";
        std::ofstream asmFile;
        asmFile.open(GetDir() + "/" + filename);
        // Test must generate asembly-flie
        ASSERT(asmFile.is_open());
        // Compile by assembler
#ifndef STDOUT_PRINT
        asmFile << GetStream()->str();
#endif
        asmFile.close();
    }

private:
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    // Callconv for printing
    CallingConvention *cc_ {nullptr};
    // Callconv for masm initialization
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *codeAlloc_ {nullptr};
    BaseMemStats *memStats_ {nullptr};
    size_t currCursor_ {0U};
#ifdef STDOUT_PRINT
    std::ostream *curr_stream_;
#else
    std::stringstream *currStream_;
#endif
    std::string testName_;
    std::string execPath_;
    std::string dirSuffix_;
};

static auto Values1()
{
    std::vector<std::pair<std::string_view, EncodeFuncType<ONE>>> singleFunctions {{"mov", &Encoder::EncodeMov},
                                                                                   {"neg", &Encoder::EncodeNeg},
                                                                                   {"abs", &Encoder::EncodeAbs},
                                                                                   {"not", &Encoder::EncodeNot}};
    return testing::ValuesIn(singleFunctions);
}

static auto Values2()
{
    std::vector<std::pair<std::string_view, EncodeFuncType<TWO>>> doubleFunctions {
        {"add", &Encoder::EncodeAdd}, {"sub", &Encoder::EncodeSub}, {"mul", &Encoder::EncodeMul},
        {"shl", &Encoder::EncodeShl}, {"shr", &Encoder::EncodeShr}, {"ashr", &Encoder::EncodeAShr},
        {"and", &Encoder::EncodeAnd}, {"or", &Encoder::EncodeOr},   {"xor", &Encoder::EncodeXor}};
    return testing::ValuesIn(doubleFunctions);
}

struct GetGTestName {
    template <typename T>
    std::string operator()(const T &value)
    {
        return std::string {"test_"} + std::string {value.param.first};
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CREATE_TEST(ARCH, PARAMS)                                                               \
    using Printer_##ARCH##_##PARAMS = PrinterTest<Arch::ARCH, static_cast<ParamCount>(PARAMS)>; \
    TEST_P(Printer_##ARCH##_##PARAMS, Test)                                                     \
    {                                                                                           \
        DoTest();                                                                               \
    }                                                                                           \
    INSTANTIATE_TEST_SUITE_P(Printer_##ARCH##_##PARAMS, Printer_##ARCH##_##PARAMS, Values##PARAMS(), GetGTestName {})

#ifdef PANDA_COMPILER_TARGET_X86_64
CREATE_TEST(X86_64, 1);
CREATE_TEST(X86_64, 2);
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
CREATE_TEST(AARCH64, 1);
CREATE_TEST(AARCH64, 2);
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH32
CREATE_TEST(AARCH32, 1);
CREATE_TEST(AARCH32, 2);
#endif

}  // namespace ark::compiler
