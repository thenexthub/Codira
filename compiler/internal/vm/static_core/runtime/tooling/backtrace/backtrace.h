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

#ifndef PANDA_RUNTIME_TOOLING_BACKTRACE_BACKTRACE_H
#define PANDA_RUNTIME_TOOLING_BACKTRACE_BACKTRACE_H

#include <cstdint>
#include <vector>

namespace ark::tooling {

constexpr uint16_t FUNCTIONNAME_MAX = 1024;
constexpr uint16_t PACKAGENAME_MAX = 1024;
constexpr uint16_t URL_MAX = 1024;
constexpr uint32_t ARKFRAME_SIZE = 16;
constexpr uint32_t ARKPC_OFFSET = 8;
constexpr uint32_t FP_SIZE = sizeof(uintptr_t);

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct Function {
    char functionName[FUNCTIONNAME_MAX];  // NOLINT(modernize-avoid-c-arrays)
    char packageName[PACKAGENAME_MAX];    // NOLINT(modernize-avoid-c-arrays)
    char url[URL_MAX];                    // NOLINT(modernize-avoid-c-arrays)
    int32_t line = 0;
    int32_t column = 0;
    uintptr_t codeBegin;
    uintptr_t codeSize;
};

enum class StepFrameType : uint8_t {
    NATIVE_FRAME = 0,
    FOREIGN_MANAGED_FRAME = 1,
    ARK_MANAGED_FRAME = 2,
};

struct ArkStepParam {
    uintptr_t *fp;
    uintptr_t *sp;
    uintptr_t *pc;
    bool *isArkFrame;
    StepFrameType *frameType;
    uint64_t frameIndex;
};

class Backtrace {
public:
    using ReadMemFunc = bool (*)(void *, uintptr_t, uintptr_t *, bool);
    using ReadMemFuncStatic = bool (*)(void *, uintptr_t, uintptr_t *);
    // CC-OFFNXT(readability-function-size_parameters)
    static int StepArkByManagedFrame(void *ctx, ReadMemFunc readMem, uintptr_t *fp, uintptr_t *sp, uintptr_t *pc,
                                     uintptr_t *bcOffset);
    // CC-OFFNXT(readability-function-size_parameters)
    static bool StepArkByNativeFrame(void *ctx, ReadMemFuncStatic readMem, ArkStepParam *arkStepParam);
    // CC-OFFNXT(readability-function-size_parameters)
    static int SymbolizeByManagedFrame(uintptr_t pc, uintptr_t mapBase, uint32_t bcOffset, uint8_t *abcData,
                                       uint64_t abcSize, Function *function);
    // CC-OFFNXT(readability-function-size_parameters)
    static bool SymbolizeByNativeFrame(uintptr_t pc, uintptr_t mapBase, uintptr_t loadOffset, uint8_t *abcData,
                                       uintptr_t abcSize, Function *function, uintptr_t extractorptr);
    static bool SymbolizeByNativeFrame(uintptr_t pc, uintptr_t mapBase, Function *function, uintptr_t extractorptr);
    static uintptr_t CreateArkSymbolExtractor();
    static bool DestroyArkSymbolExtractor(uintptr_t extractorptr);
    static bool SetExtractorData(uint8_t *data, uintptr_t dataSize, uintptr_t loadOffset, uintptr_t extractorptr);
    static uint8_t *GetExtractorFileData(uintptr_t extractorptr);
    static uintptr_t GetExtractorFileDataSize(uintptr_t extractorptr);
    static uintptr_t GetExtractorFileLoadOffset(uintptr_t extractorptr);
    static bool HasExtractorFile(uintptr_t extractorptr);
};

}  // namespace ark::tooling
#endif  // PANDA_RUNTIME_TOOLING_BACKTRACE_BACKTRACE_H
