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
//
#ifndef LIBPANDAFILE_EXTERNAL_FILE_EXT_H
#define LIBPANDAFILE_EXTERNAL_FILE_EXT_H

#include <string>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

struct MethodSymInfoExt {
    uint64_t offset;
    uint64_t length;
    std::string name;
};

struct PandaFileExt;

bool OpenPandafileFromMemoryExt(void *addr, const uint64_t *size, const std::string &fileName,
                                struct PandaFileExt **pandaFileExt);

bool OpenPandafileFromFdExt([[maybe_unused]] int fd, [[maybe_unused]] uint64_t offset, const std::string &fileName,
                            struct PandaFileExt **pandaFileExt);

bool QueryMethodSymByOffsetExt(struct PandaFileExt *pf, uint64_t offset, struct MethodSymInfoExt *methodInfo);

bool QueryMethodSymAndLineByOffsetExt(struct PandaFileExt *pf, uint64_t offset, struct MethodSymInfoExt *methodInfo);

using MethodSymInfoExtCallBack = void(struct MethodSymInfoExt *, void *);

void QueryAllMethodSymsExt(PandaFileExt *pf, MethodSymInfoExtCallBack callback, void *userData);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // LIBPANDAFILE_EXTERNAL_FILE_EXT_H
