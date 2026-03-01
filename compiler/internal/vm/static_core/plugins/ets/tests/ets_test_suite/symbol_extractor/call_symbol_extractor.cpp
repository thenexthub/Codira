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
#include "runtime/tooling/backtrace/symbol_extractor.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/thread.h"
#include "libarkbase/utils/logger.h"

static std::optional<ark::tooling::MethodInfo> ReadMethodInfo(ark::panda_file::MethodDataAccessor &mda)
{
    uintptr_t methodId = mda.GetMethodId().GetOffset();
    auto codeId = mda.GetCodeId();
    if (!codeId) {
        return std::nullopt;
    }
    ark::panda_file::CodeDataAccessor cda(mda.GetPandaFile(), codeId.value());
    uint32_t codeSize = cda.GetCodeSize();
    auto codeBegin = reinterpret_cast<uintptr_t>(cda.GetInstructions());
    return std::make_optional<ark::tooling::MethodInfo>(methodId, codeBegin, codeSize);
}

static std::vector<ark::tooling::MethodInfo> ReadAllMethodInfos(const ark::panda_file::File *file)
{
    std::vector<ark::tooling::MethodInfo> result;
    ark::Span<const uint32_t> classIndexes = file->GetClasses();
    for (const uint32_t index : classIndexes) {
        ark::panda_file::File::EntityId classId(index);
        if (file->IsExternal(classId)) {
            continue;
        }
        ark::panda_file::ClassDataAccessor cda(*file, classId);
        cda.EnumerateMethods([&result](ark::panda_file::MethodDataAccessor &mda) {
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

static bool InitSymbolExtractor(ark::tooling::ArkSymbolExtractor *extractor, uint8_t *data, uintptr_t dataSize,
                                uintptr_t loadOffset)
{
    extractor->CreateArkFileData(data, dataSize, loadOffset);
    const ark::panda_file::File *file = extractor->GetArkPandaFile();
    if (!file) {
        LOG(ERROR, TOOLING) << "Get panda file from extractor failed.";
        return false;
    }

    extractor->SetMethodInfos(ReadAllMethodInfos(file));
    if (!extractor->HasMethodInfos()) {
        LOG(ERROR, TOOLING) << "Set method infos failed.";
        return false;
    }

    return true;
}

static ani_int CheckSymbolExtractor([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    const char *zipPath = std::getenv("ARK_GTEST_ABC_PATH");
    if (zipPath == nullptr) {
        LOG(ERROR, TOOLING) << "abc file is not find";
        return 0;
    }
    std::unique_ptr<const ark::panda_file::File> pf = ark::panda_file::OpenPandaFileOrZip(zipPath);
    uint64_t abcSize = pf->GetHeader()->fileSize;
    std::vector<uint8_t> abcBuffer(abcSize);
    std::copy(pf->GetBase(), pf->GetBase() + abcSize, abcBuffer.begin());
    pf.reset();

    auto extractor = ark::tooling::ArkSymbolExtractor::Create();
    if (extractor == nullptr) {
        LOG(ERROR, TOOLING) << "Create extractor failed.";
        return 0;
    }
    if (!InitSymbolExtractor(extractor, abcBuffer.data(), abcSize, 0)) {
        LOG(ERROR, TOOLING) << "Init extractor failed.";
        return 0;
    }

    uint8_t *dataPtr = extractor->GetData();
    uintptr_t loadOffset = extractor->GetLoadOffset();
    uintptr_t dataSize = extractor->GetDataSize();
    if (dataPtr != abcBuffer.data() || loadOffset != 0 || dataSize != abcSize) {
        LOG(ERROR, TOOLING) << "Check extractor data failed.";
        return 0;
    }

    const ark::panda_file::File *file = extractor->GetArkPandaFile();
    std::vector<ark::tooling::MethodInfo> methodInfos = extractor->GetMethodInfos();
    std::vector<std::string> expectName = {"call_symbol_extractor.ETSGLOBAL.<cctor>",
                                           "call_symbol_extractor.ETSGLOBAL.main"};
    for (int i = 0; i < (int)methodInfos.size(); i++) {
        ark::panda_file::File::EntityId id(methodInfos[i].methodId);
        ark::panda_file::MethodDataAccessor mda(*file, id);
        ark::panda_file::ClassDataAccessor cda(*file, mda.GetClassId());
        constexpr int nameMaxLength = 1024;
        char functionName[nameMaxLength];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        int size = snprintf_s(functionName, sizeof(functionName), sizeof(functionName) - 1, "%s.%s",
                              ark::ClassHelper::GetName(cda.GetDescriptor()).c_str(), mda.GetName().data);
        if (size < 0) {
            LOG(ERROR, TOOLING) << "copy funtionname failed!";
            return 0;
        }
        std::string nameString = functionName;
        if (functionName != expectName[i]) {
            return 0;
        }
    }

    if (!ark::tooling::ArkSymbolExtractor::Destroy(extractor)) {
        return 0;
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
        ani_native_function {"checkSymbolExtractor", ":i", reinterpret_cast<void *>(CheckSymbolExtractor)}};
    ani_module module;
    if (env->FindModule("call_symbol_extractor", &module) != ANI_OK) {
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
