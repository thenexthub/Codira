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

#ifndef PANDA_TOOLS_SAMPLER_CONVERTER_IMPL_H
#define PANDA_TOOLS_SAMPLER_CONVERTER_IMPL_H

#include "libarkbase/os/filesystem.h"
#include "libarkbase/utils/utf.h"

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/sample_reader-inl.h"
#include "tools/sampler/trace_dumper.h"
#include "tools/sampler/args_parser.h"

#include <optional>

namespace ark::tooling::sampler {

struct SubstituteModules {
    std::vector<std::string> source;
    std::vector<std::string> destination;
};

class AsptConverter {
public:
    NO_COPY_SEMANTIC(AsptConverter);
    NO_MOVE_SEMANTIC(AsptConverter);

    using StackTraceMap = std::unordered_map<SampleInfo, size_t>;

    explicit AsptConverter(const char *filename) : reader_(filename) {}
    ~AsptConverter() = default;

    size_t CollectTracesStats();

    bool CollectModules();

    bool BuildModulesMap();

    void BuildMethodsMap();

    void SubstituteDirectories(std::string *pathname) const;

    bool DumpModulesToFile(const std::string &outname) const;

    bool DumpResolvedTracesAsCSV(const char *filename);

    bool RunWithOptions(const Options &cliOptions);

    bool RunDumpModulesMode(const std::string &outname);

    bool RunDumpTracesInCsvMode(const std::string &outname);

    static DumpType GetDumpTypeFromOptions(const Options &cliOptions);

private:
    void BuildMethodsMapHelper(const panda_file::File *pf, Span<const uint32_t> &classesSpan);

    SampleReader reader_;

    std::vector<FileInfo> modules_;

    ModuleMap modulesMap_;
    MethodMap methodsMap_;
    StackTraceMap stackTraces_;

    DumpType dumpType_ {DumpType::THREAD_SEPARATION_BY_TID};
    bool buildColdGraph_ {false};

    bool buildSystemFrames_ {false};

    std::optional<SubstituteModules> substituteDirectories_;
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_TOOLS_SAMPLER_CONVERTER_IMPL_H
