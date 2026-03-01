/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <ani.h>
#include <array>
#include <iostream>
#include <filesystem>
#include "runtime/tooling/backtrace/backtrace.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/thread.h"
#include "libarkbase/utils/logger.h"

static bool ReadMemTestFunc([[maybe_unused]] void *ctx, uintptr_t addr, uintptr_t *value, bool isRead32)
{
    if (addr == 0) {
        return false;
    }
    if (isRead32) {
        *value = *(reinterpret_cast<uint32_t *>(addr));
    } else {
        *value = *(reinterpret_cast<uintptr_t *>(addr));
    }
    return true;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
// NOLINTNEXTLINE(readability-function-size)
static ani_int CallBacktrace([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    uintptr_t fileHeader = 0;
    const char *zipPath = std::getenv("ARK_GTEST_ABC_PATH");
    if (zipPath == nullptr) {
        LOG(INFO, TOOLING) << "abc file is not find";
        return 0;
    }
    std::unique_ptr<const ark::panda_file::File> pf = ark::panda_file::OpenPandaFileOrZip(zipPath);
    uint64_t abcSize = pf->GetHeader()->fileSize;
    std::vector<uint8_t> abcBuffer(abcSize);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(pf->GetBase(), pf->GetBase() + abcSize, abcBuffer.begin());
    pf.reset();

    uintptr_t startFp = 0;
    std::vector<std::tuple<ark::Method *, size_t>> expectFrames;
    for (auto stack = ark::StackWalker::Create(ark::ManagedThread::GetCurrent()); stack.HasFrame(); stack.NextFrame()) {
        if (!stack.IsCFrame()) {
            if (startFp == 0) {
                startFp = reinterpret_cast<uintptr_t>(stack.GetFp());
                fileHeader = reinterpret_cast<uintptr_t>(stack.GetMethod()->GetPandaFile()->GetHeader());
            }
            expectFrames.emplace_back(std::make_tuple(stack.GetMethod(), stack.GetBytecodePc()));
        }
    }
    uintptr_t fp = startFp;
    uintptr_t sp = 0;
    uintptr_t pc = 0;
    uintptr_t bcCode = 0;
    std::vector<ark::tooling::Function> frames;
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    while (fp != 0 &&
           ark::tooling::Backtrace::StepArkByManagedFrame(nullptr, &ReadMemTestFunc, &fp, &sp, &pc, &bcCode)) {
        ark::tooling::Function function;
        if (ark::tooling::Backtrace::SymbolizeByManagedFrame(pc, fileHeader, bcCode, abcBuffer.data(), abcSize,
                                                             &function) == 1) {
            frames.emplace_back(function);
        } else {
            LOG(INFO, TOOLING) << "Symbolize failed";
            return 0;
        }
    }
    if (expectFrames.size() == frames.size()) {
        for (size_t i = 0; i < expectFrames.size(); ++i) {
            ark::Method *method = std::get<0>(expectFrames[i]);
            size_t bytecodePC = std::get<1>(expectFrames[i]);
            std::stringstream ssFunction;
            std::stringstream ssSrc;
            ssFunction << method->GetClass()->GetName() << "." << method->GetName().data;
            ssSrc << frames[i].url << ":" << frames[i].line;
            if (ssFunction.str() != frames[i].functionName ||
                ssSrc.str() != ark::PandaStringToStd(method->GetLineNumberAndSourceFile(bytecodePC))) {
                LOG(INFO, TOOLING) << "Function name or source file mismatch";
                return 0;
            }
        }
    }
    return 1;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"callBacktrace", nullptr, reinterpret_cast<void *>(CallBacktrace)},
    };
    ani_module module;
    if (env->FindModule("call_backtrace", &module) != ANI_OK) {
        LOG(ERROR, TOOLING) << "Cannot find module!";
        return ANI_ERROR;
    }
    if (env->Module_BindNativeFunctions(module, methods.data(), methods.size()) != ANI_OK) {
        LOG(ERROR, TOOLING) << "Cannot bind native functions!";
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
