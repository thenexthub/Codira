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

#include "profiling_loader.h"
#include "libprofile/pgo_file_builder.h"

namespace ark {
Expected<PandaString, PandaString> ProfilingLoader::LoadProfile(const PandaString &fileName)
{
    PandaString classCtxStr;
    auto resOrError = pgo::AotPgoFile::Load(fileName, classCtxStr, pandaFiles_);
    if (!resOrError) {
        return Unexpected(resOrError.Error());
    }
    aotProfilingData_ = std::move(*resOrError);
    return classCtxStr;
}

Span<BranchData> ProfilingLoader::CreateBranchData(void *branchesMem,
                                                   Span<const pgo::AotProfilingData::AotBranchData> aotBranchData)
{
    auto branchData = Span(reinterpret_cast<BranchData *>(branchesMem), aotBranchData.size());
    for (size_t i = 0; i < aotBranchData.size(); i++) {
        branchData[i].Init(aotBranchData[i].pc, aotBranchData[i].taken, aotBranchData[i].notTaken);
    }

    return branchData;
}

Span<ThrowData> ProfilingLoader::CreateThrowData(void *throwsMem,
                                                 Span<const pgo::AotProfilingData::AotThrowData> aotThrowData)
{
    auto throwData = Span(reinterpret_cast<ThrowData *>(throwsMem), aotThrowData.size());
    for (size_t i = 0; i < aotThrowData.size(); i++) {
        throwData[i].Init(aotThrowData[i].pc, aotThrowData[i].taken);
    }

    return throwData;
}
}  // namespace ark
