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

#ifndef LIBPANDAFILE_PANDA_FILE_SUPPORT_H
#define LIBPANDAFILE_PANDA_FILE_SUPPORT_H
#include "libarkfile/external/file_ext.h"
#include <vector>
#include <memory>

namespace panda_api::panda_file {

#ifdef __cplusplus
extern "C" {
#endif

void LoadPandFileExt();

#ifdef __cplusplus
}  // extern "C"
#endif

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class PandaFileWrapper {
public:
    static std::unique_ptr<PandaFileWrapper> OpenPandafileFromMemory(void *addr, uint64_t *size,
                                                                     const std::string &fileName)
    {
        if (pOpenPandafileFromMemoryExt == nullptr) {
            LoadPandFileExt();
        }

        PandaFileExt *pfExt = nullptr;
        std::unique_ptr<PandaFileWrapper> pfw;
        auto ret = pOpenPandafileFromMemoryExt(addr, size, fileName, &pfExt);
        if (ret) {
            pfw.reset(new PandaFileWrapper(pfExt));
        }
        return pfw;
    }

    static std::unique_ptr<PandaFileWrapper> OpenPandafileFromFd(int fd, uint64_t offset, const std::string &fileName)
    {
        if (pOpenPandafileFromFdExt == nullptr) {
            LoadPandFileExt();
        }

        PandaFileExt *pfExt = nullptr;
        std::unique_ptr<PandaFileWrapper> pfw;
        auto ret = pOpenPandafileFromFdExt(fd, offset, fileName, &pfExt);
        if (ret) {
            pfw.reset(new PandaFileWrapper(pfExt));
        }
        return pfw;
    }

    MethodSymInfoExt QueryMethodSymByOffset(uint64_t offset)
    {
        MethodSymInfoExt methodInfo {0, 0, std::string()};
        if (pQueryMethodSymByOffsetExt == nullptr) {
            return {0, 0, std::string()};
        }
        auto ret = pQueryMethodSymByOffsetExt(pfExt_, offset, &methodInfo);
        if (ret) {
            return methodInfo;
        }
        return {0, 0, std::string()};
    }

    MethodSymInfoExt QueryMethodSymAndLineByOffset(uint64_t offset) const
    {
        MethodSymInfoExt methodInfo {0, 0, std::string()};
        if (pQueryMethodSymAndLineByOffsetExt == nullptr) {
            return {0, 0, std::string()};
        }
        auto ret = pQueryMethodSymAndLineByOffsetExt(pfExt_, offset, &methodInfo);
        if (ret) {
            return methodInfo;
        }
        return {0, 0, std::string()};
    }

    std::vector<MethodSymInfoExt> QueryAllMethodSyms()
    {
        std::vector<MethodSymInfoExt> methodInfos;
        if (pQueryAllMethodSymsExt == nullptr) {
            return methodInfos;
        }
        pQueryAllMethodSymsExt(pfExt_, AppendMethodInfo, static_cast<void *>(&methodInfos));
        return methodInfos;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    static decltype(OpenPandafileFromFdExt) *pOpenPandafileFromFdExt;
    // NOLINTNEXTLINE(readability-identifier-naming)
    static decltype(OpenPandafileFromMemoryExt) *pOpenPandafileFromMemoryExt;
    // NOLINTNEXTLINE(readability-identifier-naming)
    static decltype(QueryMethodSymByOffsetExt) *pQueryMethodSymByOffsetExt;
    // NOLINTNEXTLINE(readability-identifier-naming)
    static decltype(QueryMethodSymAndLineByOffsetExt) *pQueryMethodSymAndLineByOffsetExt;
    // NOLINTNEXTLINE(readability-identifier-naming)
    static decltype(QueryAllMethodSymsExt) *pQueryAllMethodSymsExt;

    ~PandaFileWrapper() = default;

private:
    explicit PandaFileWrapper(PandaFileExt *pfExt) : pfExt_(pfExt) {}
    PandaFileExt *pfExt_;

    // callback
    static void AppendMethodInfo(MethodSymInfoExt *methodInfo, void *userData)
    {
        auto methodInfos = reinterpret_cast<std::vector<MethodSymInfoExt> *>(userData);
        methodInfos->push_back(*methodInfo);
    }

    friend void LoadPandFileExt();
};

}  // namespace panda_api::panda_file

#endif
