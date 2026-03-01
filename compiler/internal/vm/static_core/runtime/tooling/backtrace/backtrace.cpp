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

#include "backtrace.h"
#include "irtoc/backend/compiler/constants.h"
#include "libarkbase/utils/arch.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/code_data_accessor.h"
#include "libarkfile/debug_helpers.h"
#include "libarkfile/file.h"
#include "libarkfile/method_data_accessor.h"
#include "runtime/include/class.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/stack_walker.h"
#include "runtime/tooling/backtrace/symbol_extractor.h"

namespace ark::tooling {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

static std::vector<MethodInfo> ReadAllMethodInfos(const ark::panda_file::File *pandaFile);
static std::optional<MethodInfo> ReadMethodInfo(ark::panda_file::MethodDataAccessor &mda);

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
// CC-OFFNXT(readability-function-size_parameters)
int Backtrace::StepArkByManagedFrame(void *ctx, ReadMemFunc readMem, uintptr_t *fp, uintptr_t *sp, uintptr_t *pc,
                                     uintptr_t *bcOffset)
{
    if (*fp == 0) {
        LOG(ERROR, RUNTIME) << "fp is invalid";
        return 0;
    }

    *sp = *fp;
    auto prevFpOffset = Frame::GetPrevFrameOffset();
    auto ret = readMem(ctx, *sp + prevFpOffset, fp, false);
    if (!ret) {
        LOG(ERROR, RUNTIME) << "read prev fp addr : " << *sp + prevFpOffset << " failed ";
        return 0;
    }

    auto pcOffset = Frame::GetInstructionsOffset();
    ret = readMem(ctx, *sp + pcOffset, pc, false);
    if (!ret) {
        LOG(ERROR, RUNTIME) << "read pc addr : " << *sp + pcOffset << " failed ";
        return 0;
    }

    auto bcOffsetOffset = Frame::GetBytecodeOffsetOffset();
    ret = readMem(ctx, *sp + bcOffsetOffset, bcOffset, true);
    if (!ret) {
        LOG(ERROR, RUNTIME) << "read bcOffsetOffset addr : " << *sp + bcOffsetOffset << " failed ";
        return 0;
    }
    return 1;
}

// The native frame structure of the irtoc interpreter, and the working principle of StepArkByNativeFrame().
// |-----------------------------|------    <--- fp
// |                             |
// | native frame of Interpreter | arkFrameOffset
// |                             |
// |-----------------------------|------
// |        arkFrameNum n        | ARKFRAME_SIZE
// |-----------------------------|------    <--- fp - arkFrameNumOffset
// |   byteCodePc of Method_0    |
// |-----------------------------|
// |   byteCodePc of Method_1    |
// |-----------------------------|
// |   byteCodePc of Method_2    | ARKFRAME_SIZE * n
// |-----------------------------|
// |           ......            |
// |-----------------------------|
// |  byteCodePc of Method_n-1   |          <--- fp - arkFrameNumOffset - ARKFRAME_SIZE * n + ARKPC_OFFSET
// |-----------------------------|------    <--- fp - arkFrameNumOffset - ARKFRAME_SIZE * n

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
bool Backtrace::StepArkByNativeFrame(void *ctx, ReadMemFuncStatic readMem, ArkStepParam *arkStepParam)
{
    ASSERT(arkStepParam != nullptr);
    ASSERT(arkStepParam->fp != nullptr);
    ASSERT(arkStepParam->sp != nullptr);
    ASSERT(arkStepParam->pc != nullptr);
    ASSERT(arkStepParam->isArkFrame != nullptr);
    ASSERT(arkStepParam->frameType != nullptr);

    if (*arkStepParam->fp == 0) {
        LOG(ERROR, RUNTIME) << "StepArkByNativeFrame: fp is invalid";
        return false;
    }
    if (*arkStepParam->frameType != StepFrameType::ARK_MANAGED_FRAME) {
        LOG(ERROR, RUNTIME) << "StepArkByNativeFrame: not ark frame";
        return false;
    }

    auto arch = ark::RUNTIME_ARCH;
    if (arch != Arch::X86_64 && arch != Arch::AARCH64) {
        LOG(ERROR, RUNTIME) << "StepArkByNativeFrame: current arch does not support";
        return false;
    }

    uint64_t arkFrameOffset = compiler::GetIrtocCFrameSizeBytesBelowFP(arch);
    uint64_t arkFrameNumOffset = arkFrameOffset + ARKFRAME_SIZE;
    uintptr_t arkFrameNum = 0;
    // NOTE: (zhushihao8, #32951) It is necessary to design an ark unwinder that does not use counter.
    auto ret = readMem(ctx, *arkStepParam->fp - arkFrameNumOffset, &arkFrameNum);
    if (!ret) {
        LOG(ERROR, RUNTIME) << "read arkFrameNum addr : " << *arkStepParam->fp - arkFrameNumOffset << " failed ";
        return false;
    }

    uint64_t currentFrameIndex = arkStepParam->frameIndex;
    if (currentFrameIndex >= static_cast<uint64_t>(arkFrameNum)) {
        uintptr_t currentPtr = *arkStepParam->fp;
        ret = readMem(ctx, currentPtr, arkStepParam->fp);
        currentPtr += FP_SIZE;
        ret &= readMem(ctx, currentPtr, arkStepParam->pc);
        currentPtr += FP_SIZE;
        *arkStepParam->sp = currentPtr;
        *arkStepParam->frameType = StepFrameType::NATIVE_FRAME;
        *arkStepParam->isArkFrame = false;
        return ret;
    }

    uint64_t currentFrameNum = static_cast<uint64_t>(arkFrameNum) - currentFrameIndex;
    uint64_t pcOffset = arkFrameNumOffset + ARKFRAME_SIZE * currentFrameNum - ARKPC_OFFSET;
    ret = readMem(ctx, *arkStepParam->fp - pcOffset, arkStepParam->pc);
    if (!ret) {
        LOG(ERROR, RUNTIME) << "read pc addr : " << *arkStepParam->fp - pcOffset << " failed ";
        return false;
    }
    *arkStepParam->isArkFrame = true;
    return true;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
// CC-OFFNXT(readability-function-size_parameters, huge_depth[C++])
int Backtrace::SymbolizeByManagedFrame(uintptr_t pc, uintptr_t mapBase, uint32_t bcOffset, uint8_t *abcData,
                                       uint64_t abcSize, Function *function)
{
    if (function == nullptr || abcData == nullptr) {
        LOG(ERROR, RUNTIME) << "parameter invalid!";
        return 0;
    }
    uintptr_t realOffset = pc - mapBase;
    std::unique_ptr<const panda_file::File> file = panda_file::OpenPandaFileFromMemory(abcData, abcSize);
    ASSERT(file != nullptr);
    uintptr_t realPc = realOffset + reinterpret_cast<uintptr_t>(file->GetBase());

    auto methodInfos = ReadAllMethodInfos(file.get());
    int32_t left = 0;
    int32_t right = static_cast<int32_t>(methodInfos.size()) - 1;
    for (; left <= right;) {
        int32_t mid = (left + right) / 2;
        bool isRight = realPc >= (methodInfos[mid].codeBegin + methodInfos[mid].codeSize);
        bool isLeft = realPc < methodInfos[mid].codeBegin;
        if (!isRight && !isLeft) {
            panda_file::File::EntityId id(methodInfos[mid].methodId);
            panda_file::MethodDataAccessor mda(*file, id);
            panda_file::ClassDataAccessor cda(*file, mda.GetClassId());
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            int size = snprintf_s(function->functionName, FUNCTIONNAME_MAX, FUNCTIONNAME_MAX - 1, "%s.%s",
                                  ClassHelper::GetName(cda.GetDescriptor()).c_str(), mda.GetName().data);
            if (size < 0) {
                LOG(ERROR, RUNTIME) << "copy funtionname failed!";
            }
            function->line = static_cast<int32_t>(panda_file::debug_helpers::GetLineNumber(mda, bcOffset, file.get()));
            function->column = 0;
            function->codeBegin = methodInfos[mid].codeBegin;
            function->codeSize = methodInfos[mid].codeSize;
            auto sourceFileId = cda.GetSourceFileId();
            // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
            if (sourceFileId.has_value() &&
                strcpy_s(function->url, URL_MAX,
                         reinterpret_cast<const char *>(file->GetStringData(sourceFileId.value()).data)) == EOK) {
                return 1;
            }
            LOG(ERROR, RUNTIME) << "sourceFile not exist";
        } else if (isRight) {  // NOLINT(readability-else-after-return)
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return 0;
}

static const panda_file::File *GetPandaFile(ArkSymbolExtractor *extractor, uint8_t *abcData, uint64_t abcSize,
                                            uintptr_t loadOffset, std::unique_ptr<const panda_file::File> &localFile)
{
    const panda_file::File *file = nullptr;

    if (extractor != nullptr) {
        file = extractor->GetArkPandaFile();
        if (file == nullptr && abcData != nullptr) {
            extractor->CreateArkFileData(abcData, abcSize, loadOffset);
            file = extractor->GetArkPandaFile();
        }
        return file;
    }

    if (abcData != nullptr) {
        localFile = panda_file::OpenPandaFileFromMemory(abcData, abcSize);
        if (!localFile) {
            return file;
        }
        file = localFile.get();
        return file;
    }
    return file;
}

static std::vector<MethodInfo> GetMethodInfos(ArkSymbolExtractor *extractor, const panda_file::File *file)
{
    std::vector<MethodInfo> methodInfos;
    if (file == nullptr) {
        return methodInfos;
    }

    if (extractor != nullptr) {
        if (!extractor->HasMethodInfos()) {
            methodInfos = ReadAllMethodInfos(file);
            extractor->SetMethodInfos(methodInfos);
        } else {
            methodInfos = extractor->GetMethodInfos();
        }
    } else {
        methodInfos = ReadAllMethodInfos(file);
    }
    return methodInfos;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
// CC-OFFNXT(readability-function-size_parameters, huge_depth[C++])
bool Backtrace::SymbolizeByNativeFrame(uintptr_t pc, uintptr_t mapBase, uintptr_t loadOffset, uint8_t *abcData,
                                       uintptr_t abcSize, Function *function, uintptr_t extractorptr)
{
    auto extractor = reinterpret_cast<ArkSymbolExtractor *>(extractorptr);
    if ((!extractor && !abcData) || !function) {
        LOG(ERROR, RUNTIME) << "Parameter invalid.";
        return false;
    }

    const panda_file::File *file = nullptr;
    std::unique_ptr<const panda_file::File> localFile;
    file = GetPandaFile(extractor, abcData, abcSize, loadOffset, localFile);
    if (file == nullptr) {
        LOG(ERROR, RUNTIME) << "Open panda file failed.";
        return false;
    }

    std::vector<MethodInfo> methodInfos = GetMethodInfos(extractor, file);
    if (methodInfos.empty()) {
        LOG(ERROR, RUNTIME) << "Get methodInfos from .abc failed.";
        return false;
    }

    if (extractor) {
        loadOffset = extractor->GetLoadOffset();
    }
    loadOffset = loadOffset % os::mem::GetPageSize();
    uintptr_t realOffset = pc - mapBase - loadOffset;
    uintptr_t realPc = realOffset + reinterpret_cast<uintptr_t>(file->GetBase());
    int32_t left = 0;
    int32_t right = static_cast<int32_t>(methodInfos.size()) - 1;
    for (; left <= right;) {
        int32_t mid = (left + right) / 2;
        bool isRight = realPc >= (methodInfos[mid].codeBegin + methodInfos[mid].codeSize);
        bool isLeft = realPc < methodInfos[mid].codeBegin;
        if (!isRight && !isLeft) {
            panda_file::File::EntityId id(methodInfos[mid].methodId);
            panda_file::MethodDataAccessor mda(*file, id);
            panda_file::ClassDataAccessor cda(*file, mda.GetClassId());
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            int size = snprintf_s(function->functionName, FUNCTIONNAME_MAX, FUNCTIONNAME_MAX - 1, "%s.%s",
                                  ClassHelper::GetName(cda.GetDescriptor()).c_str(), mda.GetName().data);
            if (size < 0) {
                LOG(ERROR, RUNTIME) << "copy funtionname failed!";
            }
            uint32_t bcOffset = realPc - methodInfos[mid].codeBegin;
            function->line = static_cast<int32_t>(panda_file::debug_helpers::GetLineNumber(mda, bcOffset, file));
            function->column = 0;
            function->codeBegin = methodInfos[mid].codeBegin;
            function->codeSize = methodInfos[mid].codeSize;
            auto sourceFileId = cda.GetSourceFileId();
            // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
            if (sourceFileId.has_value() &&
                strcpy_s(function->url, URL_MAX,
                         reinterpret_cast<const char *>(file->GetStringData(sourceFileId.value()).data)) == EOK) {
                return true;
            }
            LOG(ERROR, RUNTIME) << "sourceFile not exist";
        } else if (isRight) {  // NOLINT(readability-else-after-return)
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    LOG(ERROR, RUNTIME) << "Can not find ark method";
    return false;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
bool Backtrace::SymbolizeByNativeFrame(uintptr_t pc, uintptr_t mapBase, Function *function, uintptr_t extractorptr)
{
    return SymbolizeByNativeFrame(pc, mapBase, 0, nullptr, 0, function, extractorptr);
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
uintptr_t Backtrace::CreateArkSymbolExtractor()
{
    auto extractor = ArkSymbolExtractor::Create();
    auto extractorptr = reinterpret_cast<uintptr_t>(extractor);
    return extractorptr;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
bool Backtrace::DestroyArkSymbolExtractor(uintptr_t extractorptr)
{
    auto extractor = reinterpret_cast<ArkSymbolExtractor *>(extractorptr);
    if (!extractor) {
        return false;
    }
    return ArkSymbolExtractor::Destroy(extractor);
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
bool Backtrace::SetExtractorData(uint8_t *data, uintptr_t dataSize, uintptr_t loadOffset, uintptr_t extractorptr)
{
    auto extractor = reinterpret_cast<ArkSymbolExtractor *>(extractorptr);
    if (!extractor) {
        return false;
    }
    extractor->CreateArkFileData(data, dataSize, loadOffset);
    return true;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
uint8_t *Backtrace::GetExtractorFileData(uintptr_t extractorptr)
{
    auto extractor = reinterpret_cast<ArkSymbolExtractor *>(extractorptr);
    if (!extractor) {
        return nullptr;
    }
    uint8_t *data = extractor->GetData();
    return data;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
uintptr_t Backtrace::GetExtractorFileDataSize(uintptr_t extractorptr)
{
    auto extractor = reinterpret_cast<ArkSymbolExtractor *>(extractorptr);
    if (!extractor) {
        return 0;
    }
    uintptr_t dataSize = extractor->GetDataSize();
    return dataSize;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
uintptr_t Backtrace::GetExtractorFileLoadOffset(uintptr_t extractorptr)
{
    auto extractor = reinterpret_cast<ArkSymbolExtractor *>(extractorptr);
    if (!extractor) {
        return 0;
    }
    uintptr_t loadOffset = extractor->GetLoadOffset();
    return loadOffset;
}

// NOTE: (zhushihao8, #32949) these API signatures should be revised once the tooling interface design is ready
bool Backtrace::HasExtractorFile(uintptr_t extractorptr)
{
    auto extractor = reinterpret_cast<ArkSymbolExtractor *>(extractorptr);
    if (!extractor) {
        return false;
    }
    auto file = extractor->GetArkPandaFile();
    return file != nullptr;
}

std::optional<MethodInfo> ReadMethodInfo(panda_file::MethodDataAccessor &mda)
{
    uintptr_t methodId = mda.GetMethodId().GetOffset();
    auto codeId = mda.GetCodeId();
    if (!codeId) {
        return std::nullopt;
    }
    panda_file::CodeDataAccessor cda(mda.GetPandaFile(), codeId.value());
    uint32_t codeSize = cda.GetCodeSize();
    auto codeBegin = reinterpret_cast<uintptr_t>(cda.GetInstructions());
    return std::make_optional<MethodInfo>(methodId, codeBegin, codeSize);
}

std::vector<MethodInfo> ReadAllMethodInfos(const panda_file::File *file)
{
    std::vector<MethodInfo> result;
    Span<const uint32_t> classIndexes = file->GetClasses();
    for (const uint32_t index : classIndexes) {
        panda_file::File::EntityId classId(index);
        if (file->IsExternal(classId)) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*file, classId);
        cda.EnumerateMethods([&result](panda_file::MethodDataAccessor &mda) {
            auto info = ReadMethodInfo(mda);
            if (!info) {
                return;
            }
            result.push_back(info.value());
        });
    }

    std::sort(result.begin(), result.end());
    return result;
}
}  // namespace ark::tooling
