/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "symbol_extractor.h"

namespace ark::tooling {

ArkSymbolExtractor::~ArkSymbolExtractor()
{
    arkPandaFile_.reset();
}

ArkSymbolExtractor *ArkSymbolExtractor::Create()
{
    auto extractor = new ArkSymbolExtractor();
    return extractor;
}

bool ArkSymbolExtractor::Destroy(ArkSymbolExtractor *extractor)
{
    if (extractor == nullptr) {
        LOG(ERROR, RUNTIME) << "Destroy ark symbol extractor failed, extractor is nullptr.";
        return false;
    }
    delete extractor;
    return true;
}

void ArkSymbolExtractor::CreateArkPandaFile()
{
    if (data_ == nullptr) {
        return;
    }
    auto pf = panda_file::OpenPandaFileFromMemory(data_, dataSize_);
    if (pf == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to open panda file.";
        return;
    }
    arkPandaFile_ = std::shared_ptr<const panda_file::File>(pf.release());
}

void ArkSymbolExtractor::CreateArkFileData(uint8_t *data, uintptr_t dataSize, uintptr_t loadOffset)
{
    if (data == nullptr) {
        return;
    }
    data_ = data;
    dataSize_ = dataSize;
    loadOffset_ = loadOffset;
    CreateArkPandaFile();
}

const panda_file::File *ArkSymbolExtractor::GetArkPandaFile()
{
    if (arkPandaFile_ == nullptr && data_ != nullptr) {
        CreateArkPandaFile();
    }
    // Returns null pointer when the panda file is not created.
    return arkPandaFile_.get();
}

void ArkSymbolExtractor::SetMethodInfos(std::vector<MethodInfo> methodInfos)
{
    methodInfos_ = std::move(methodInfos);
}

const std::vector<MethodInfo> &ArkSymbolExtractor::GetMethodInfos()
{
    return methodInfos_;
}

bool ArkSymbolExtractor::HasMethodInfos()
{
    return !methodInfos_.empty();
}

uint8_t *ArkSymbolExtractor::GetData()
{
    return data_;
}

uintptr_t ArkSymbolExtractor::GetLoadOffset()
{
    return loadOffset_;
}

uintptr_t ArkSymbolExtractor::GetDataSize()
{
    return dataSize_;
}

}  // namespace ark::tooling
