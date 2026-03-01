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

#ifndef AOT_PROFILING_DATA_H
#define AOT_PROFILING_DATA_H

#include "libarkbase/utils/span.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/jit/profiling_data.h"

#include <cstdint>
#include <atomic>

namespace ark::pgo {
using PandaFileIdxType = int32_t;  // >= -1

class AotProfilingData {
public:
    struct AotCallSiteInlineCache;
    struct AotBranchData;
    struct AotThrowData;
    class AotMethodProfilingData {  // NOLINT(cppcoreguidelines-special-member-functions)
    public:
        AotMethodProfilingData(uint32_t methodIdx, uint32_t classIdx, uint32_t inlineCaches, uint32_t branchData,
                               uint32_t throwData)
            : methodIdx_(methodIdx),
              classIdx_(classIdx),
              inlineCaches_(inlineCaches),
              branchData_(branchData),
              throwData_(throwData)
        {
        }

        AotMethodProfilingData(uint32_t methodIdx, uint32_t classIdx, PandaVector<AotCallSiteInlineCache> inlineCaches,
                               PandaVector<AotBranchData> branchData, PandaVector<AotThrowData> throwData)
            : methodIdx_(methodIdx),
              classIdx_(classIdx),
              inlineCaches_(std::move(inlineCaches)),
              branchData_(std::move(branchData)),
              throwData_(std::move(throwData))
        {
        }

        Span<AotCallSiteInlineCache> GetInlineCaches()
        {
            return Span<AotCallSiteInlineCache>(inlineCaches_.data(), inlineCaches_.size());
        }

        Span<AotBranchData> GetBranchData()
        {
            return Span<AotBranchData>(branchData_.data(), branchData_.size());
        }

        Span<AotThrowData> GetThrowData()
        {
            return Span<AotThrowData>(throwData_.data(), throwData_.size());
        }

        Span<const AotCallSiteInlineCache> GetInlineCaches() const
        {
            return Span<const AotCallSiteInlineCache>(inlineCaches_.data(), inlineCaches_.size());
        }

        Span<const AotBranchData> GetBranchData() const
        {
            return Span<const AotBranchData>(branchData_.data(), branchData_.size());
        }

        Span<const AotThrowData> GetThrowData() const
        {
            return Span<const AotThrowData>(throwData_.data(), throwData_.size());
        }

        uint32_t GetMethodIdx() const
        {
            return methodIdx_;
        }

        uint32_t GetClassIdx() const
        {
            return classIdx_;
        }

    private:
        uint32_t methodIdx_;
        uint32_t classIdx_;

        PandaVector<AotCallSiteInlineCache> inlineCaches_;
        PandaVector<AotBranchData> branchData_;
        PandaVector<AotThrowData> throwData_;
    };

#pragma pack(push, 4)
    struct AotCallSiteInlineCache {                              // NOLINT(cppcoreguidelines-pro-type-member-init)
        static constexpr size_t CLASSES_COUNT = 4;               // CC-OFF(G.NAM.03-CPP) project code style
        static constexpr int32_t MEGAMORPHIC_FLAG = 0xFFFFFFFF;  // CC-OFF(G.NAM.03-CPP) project code style

        static void ClearClasses(std::array<std::pair<uint32_t, PandaFileIdxType>, CLASSES_COUNT> &classes)
        {
            std::fill(classes.begin(), classes.end(), std::make_pair(0, -1));
        }

        uint32_t pc;
        std::array<std::pair<uint32_t, PandaFileIdxType>, CLASSES_COUNT> classes;
    };

    struct AotBranchData {
        uint32_t pc;
        uint64_t taken;
        uint64_t notTaken;
    };

    struct AotThrowData {
        uint32_t pc;
        uint64_t taken;
    };
#pragma pack(pop)

public:
    PandaMap<std::string_view, PandaFileIdxType> &GetPandaFileMap()
    {
        return pandaFileMap_;
    }

    PandaMap<PandaFileIdxType, std::string_view> &GetPandaFileMapReverse()
    {
        return pandaFileMapRev_;
    }

    using MethodsMap = PandaMap<uint32_t, AotMethodProfilingData>;
    PandaMap<PandaFileIdxType, MethodsMap> &GetAllMethods()
    {
        return allMethodsMap_;
    }

    int32_t GetPandaFileIdxByName(std::string_view pandaFileName)
    {
        auto pfIdx = pandaFileMap_.find(pandaFileName);
        if (pfIdx == pandaFileMap_.end()) {
            return -1;
        }
        return pfIdx->second;
    }

    template <typename Rng>
    void AddPandaFiles(Rng &&profiledPandaFiles)
    {
        auto count = static_cast<int32_t>(pandaFileMap_.size());
        for (auto &pandaFile : profiledPandaFiles) {
            if (pandaFileMap_.find(pandaFile) != pandaFileMap_.end()) {
                continue;
            }
            pandaFileMap_[pandaFile] = count;
            pandaFileMapRev_[count] = pandaFile;
            count++;
        }
    }

    void AddMethod(PandaFileIdxType pfIdx, uint32_t methodIdx, AotMethodProfilingData &&profData)
    {
        if (allMethodsMap_[pfIdx].find(methodIdx) != allMethodsMap_[pfIdx].end()) {
            allMethodsMap_[pfIdx].erase(methodIdx);
        }
        allMethodsMap_[pfIdx].insert(std::make_pair(methodIdx, std::move(profData)));
    }

private:
    PandaMap<std::string_view, PandaFileIdxType> pandaFileMap_;
    PandaMap<PandaFileIdxType, std::string_view> pandaFileMapRev_;
    PandaMap<PandaFileIdxType, MethodsMap> allMethodsMap_;
};
}  // namespace ark::pgo

#endif  // AOT_PROFILING_DATA_H
