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

#ifndef PANDA_TOOLS_SAMPLER_TRACE_DUMPER_H
#define PANDA_TOOLS_SAMPLER_TRACE_DUMPER_H

#include <unordered_map>
#include <fstream>

#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/class_data_accessor-inl.h"

#include "runtime/tooling/sampler/sample_info.h"

namespace ark::tooling::sampler {

using ModuleMap = std::unordered_map<uintptr_t, std::unique_ptr<const panda_file::File>>;
using MethodMap = std::unordered_map<const panda_file::File *, std::unordered_map<uint32_t, std::string>>;

enum class DumpType {
    WITHOUT_THREAD_SEPARATION,  // Creates single csv file without separating threads
    THREAD_SEPARATION_BY_TID,   // Creates single csv file with separating threads by thread_id
    THREAD_SEPARATION_BY_CSV    // Creates csv file for each thread
};

class TraceDumper {
public:
    NO_COPY_SEMANTIC(TraceDumper);
    NO_MOVE_SEMANTIC(TraceDumper);

    explicit TraceDumper(ModuleMap *modulesMap, MethodMap *methodsMap)
        : modulesMap_(modulesMap), methodsMap_(methodsMap)
    {
    }
    virtual ~TraceDumper() = default;

    void DumpTraces(const SampleInfo &sample, size_t count);

    void SetBuildSystemFrames(bool buildSystemFrames);

protected:
    static void WriteThreadId(std::ofstream &stream, uint32_t threadId);
    static void WriteThreadStatus(std::ofstream &stream, SampleInfo::ThreadStatus threadStatus);

private:
    virtual std::ofstream &ResolveStream(const SampleInfo &sample) = 0;

    std::string ResolveName(const panda_file::File *pf, uint64_t fileId) const;

private:
    ModuleMap *modulesMap_ {nullptr};
    MethodMap *methodsMap_ {nullptr};

    bool buildSystemFrames_ {false};
};

class SingleCSVDumper final : public TraceDumper {
public:
    NO_COPY_SEMANTIC(SingleCSVDumper);
    NO_MOVE_SEMANTIC(SingleCSVDumper);

    explicit SingleCSVDumper(const char *filename, DumpType option, ModuleMap *modulesMap, MethodMap *methodsMap,
                             bool buildColdGraph)
        : TraceDumper(modulesMap, methodsMap),
          stream_(std::ofstream(filename)),
          option_(option),
          buildColdGraph_(buildColdGraph)
    {
    }
    ~SingleCSVDumper() override = default;

private:
    std::ofstream &ResolveStream(const SampleInfo &sample) override;

private:
    std::ofstream stream_;
    DumpType option_;
    bool buildColdGraph_;
};

class MultipleCSVDumper final : public TraceDumper {
public:
    NO_COPY_SEMANTIC(MultipleCSVDumper);
    NO_MOVE_SEMANTIC(MultipleCSVDumper);

    explicit MultipleCSVDumper(const char *filename, ModuleMap *modulesMap, MethodMap *methodsMap, bool buildColdGraph)
        : TraceDumper(modulesMap, methodsMap), filename_(filename), buildColdGraph_(buildColdGraph)
    {
    }
    ~MultipleCSVDumper() override = default;

private:
    std::ofstream &ResolveStream(const SampleInfo &sample) override;

    static std::string AddThreadIdToFilename(const std::string &filename, uint32_t threadId);

private:
    std::string filename_;
    std::unordered_map<size_t, std::ofstream> threadIdMap_;
    bool buildColdGraph_;
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_TOOLS_SAMPLER_TRACE_DUMPER_H
