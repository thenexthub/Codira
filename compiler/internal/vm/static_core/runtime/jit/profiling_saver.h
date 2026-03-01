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

#ifndef PROFILING_SAVE_H
#define PROFILING_SAVE_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include "libprofile/aot_profiling_data.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/include/class.h"
#include "runtime/include/class_linker.h"
#include "runtime/jit/libprofile/pgo_file_builder.h"

namespace ark {
class ProfilingSaver {
public:
    void UpdateInlineCaches(pgo::AotProfilingData::AotCallSiteInlineCache *ic, std::vector<Class *> &runtimeClasses,
                            pgo::AotProfilingData *profileData);
    void CreateInlineCaches(pgo::AotProfilingData::AotMethodProfilingData *profilingData,
                            Span<CallSiteInlineCache> &runtimeICs, pgo::AotProfilingData *profileData);
    void CreateBranchData(pgo::AotProfilingData::AotMethodProfilingData *profilingData,
                          Span<BranchData> &runtimeBranch);
    void CreateThrowData(pgo::AotProfilingData::AotMethodProfilingData *profilingData, Span<ThrowData> &runtimeThrow);
    void AddMethod(pgo::AotProfilingData *profileData, Method *method, int32_t pandaFileIdx);
    void AddProfiledMethods(pgo::AotProfilingData *profileData, PandaList<Method *> &profiledMethods,
                            PandaList<Method *>::const_iterator profiledMethodsFinal);
    void SaveProfile(const PandaString &saveFilePath, const PandaString &classCtxStr,
                     PandaList<Method *> &profiledMethods, PandaList<Method *>::const_iterator profiledMethodsFinal,
                     PandaUnorderedSet<std::string_view> &profiledPandaFiles);
};
}  // namespace ark
#endif  // PROFILING_SAVE_H
