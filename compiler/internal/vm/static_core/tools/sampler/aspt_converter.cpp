/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "tools/sampler/aspt_converter.h"

namespace ark::tooling::sampler {

size_t AsptConverter::CollectTracesStats()
{
    stackTraces_.clear();

    size_t sampleCounter = 0;
    SampleInfo sample;
    while (reader_.GetNextSample(&sample)) {
        ++sampleCounter;

        if (dumpType_ == DumpType::WITHOUT_THREAD_SEPARATION) {
            // NOTE: zeroing thread_id to make samples indistinguishable in mode without thread separation
            sample.threadInfo.threadId = 0;
        }

        if (!buildColdGraph_) {
            // NOTE: zeroing thread_status to make samples indistinguishable
            // in mode without building cold flamegraph
            sample.threadInfo.threadStatus = SampleInfo::ThreadStatus::UNDECLARED;
        }

        auto it = stackTraces_.find(sample);
        if (it == stackTraces_.end()) {
            stackTraces_.insert({sample, 1});
            continue;
        }
        ++it->second;
    }
    return sampleCounter;
}

bool AsptConverter::CollectModules()
{
    FileInfo mInfo;
    while (reader_.GetNextModule(&mInfo)) {
        modules_.push_back(mInfo);
    }

    return !modules_.empty();
}

bool AsptConverter::BuildModulesMap()
{
    for (auto &mdl : modules_) {
        std::string filepath = mdl.pathname;

        if (substituteDirectories_.has_value()) {
            SubstituteDirectories(&filepath);
        }

        if (!ark::os::IsFileExists(filepath)) {
            LOG(ERROR, PROFILER) << "Module not found, path: " << filepath;
        }

        if (modulesMap_.find(mdl.ptr) == modulesMap_.end()) {
            auto pfUnique = panda_file::OpenPandaFileOrZip(filepath.c_str());
            if (mdl.checksum != pfUnique->GetHeader()->checksum) {
                LOG(FATAL, PROFILER) << "Checksum of panda files isn't equal";
                return false;
            }

            modulesMap_.insert({mdl.ptr, std::move(pfUnique)});
        }
    }
    return !modulesMap_.empty();
}

void AsptConverter::SubstituteDirectories(std::string *pathname) const
{
    for (size_t i = 0; i < substituteDirectories_->source.size(); ++i) {
        auto pos = pathname->find(substituteDirectories_->source[i]);
        if (pos != std::string::npos) {
            pathname->replace(pos, substituteDirectories_->source[i].size(), substituteDirectories_->destination[i]);
            break;
        }
    }
}

bool AsptConverter::DumpResolvedTracesAsCSV(const char *filename)
{
    std::unique_ptr<TraceDumper> dumper;

    switch (dumpType_) {
        case DumpType::WITHOUT_THREAD_SEPARATION:
        case DumpType::THREAD_SEPARATION_BY_TID:
            dumper =
                std::make_unique<SingleCSVDumper>(filename, dumpType_, &modulesMap_, &methodsMap_, buildColdGraph_);
            break;
        case DumpType::THREAD_SEPARATION_BY_CSV:
            dumper = std::make_unique<MultipleCSVDumper>(filename, &modulesMap_, &methodsMap_, buildColdGraph_);
            break;
        default:
            UNREACHABLE();
    }

    dumper->SetBuildSystemFrames(buildSystemFrames_);

    for (auto &[sample, count] : stackTraces_) {
        ASSERT(sample.stackInfo.managedStackSize <= SampleInfo::StackInfo::MAX_STACK_DEPTH);
        dumper->DumpTraces(sample, count);
    }
    return true;
}

void AsptConverter::BuildMethodsMap()
{
    for (const auto &pfPair : modulesMap_) {
        const panda_file::File *pf = pfPair.second.get();
        if (pf == nullptr) {
            continue;
        }
        auto classesSpan = pf->GetClasses();
        BuildMethodsMapHelper(pf, classesSpan);
    }
}

void AsptConverter::BuildMethodsMapHelper(const panda_file::File *pf, Span<const uint32_t> &classesSpan)
{
    for (auto id : classesSpan) {
        if (pf->IsExternal(panda_file::File::EntityId(id))) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*pf, panda_file::File::EntityId(id));
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            std::string methodName = utf::Mutf8AsCString(mda.GetName().data);
            std::string className = utf::Mutf8AsCString(cda.GetDescriptor());
            if (className[className.length() - 1] == ';') {
                className.pop_back();
            }
            std::string fullName = className + "::";
            fullName += methodName;
            methodsMap_[pf][mda.GetMethodId().GetOffset()] = std::move(fullName);
        });
    }
}

bool AsptConverter::DumpModulesToFile(const std::string &outname) const
{
    std::ofstream out(outname, std::ios::binary);
    if (!out) {
        LOG(ERROR, PROFILER) << "Can't open " << outname;
        out.close();
        return false;
    }

    for (auto &mdl : modules_) {
        out << mdl.checksum << " " << mdl.pathname << "\n";
    }

    if (out.fail()) {
        LOG(ERROR, PROFILER) << "Failbit or the badbit (or both) was set";
        out.close();
        return false;
    }

    return true;
}

/* static */
DumpType AsptConverter::GetDumpTypeFromOptions(const Options &cliOptions)
{
    const auto &dumpTypeStr = cliOptions.GetCsvTidSeparation();

    DumpType dumpType;
    if (dumpTypeStr == "single-csv-single-tid") {
        dumpType = DumpType::WITHOUT_THREAD_SEPARATION;
    } else if (dumpTypeStr == "single-csv-multi-tid") {
        dumpType = DumpType::THREAD_SEPARATION_BY_TID;
    } else if (dumpTypeStr == "multi-csv") {
        dumpType = DumpType::THREAD_SEPARATION_BY_CSV;
    } else {
        std::cerr << "unknown value of csv-tid-distribution option: '" << dumpTypeStr
                  << "' single-csv-multi-tid will be set" << std::endl;
        dumpType = DumpType::THREAD_SEPARATION_BY_TID;
    }

    return dumpType;
}

bool AsptConverter::RunDumpModulesMode(const std::string &outname)
{
    if (CollectTracesStats() == 0) {
        LOG(ERROR, PROFILER) << "No samples found in file";
        return false;
    }

    if (!CollectModules()) {
        LOG(ERROR, PROFILER) << "No modules found in file, names would not be resolved";
        return false;
    }

    if (!DumpModulesToFile(outname)) {
        LOG(ERROR, PROFILER) << "Can't dump modules to file";
        return false;
    }

    return true;
}

bool AsptConverter::RunDumpTracesInCsvMode(const std::string &outname)
{
    if (CollectTracesStats() == 0) {
        LOG(ERROR, PROFILER) << "No samples found in file";
        return false;
    }

    if (!CollectModules()) {
        LOG(ERROR, PROFILER) << "No modules found in file, names would not be resolved";
    }

    if (!BuildModulesMap()) {
        LOG(ERROR, PROFILER) << "Can't build modules map";
        return false;
    }

    BuildMethodsMap();
    DumpResolvedTracesAsCSV(outname.c_str());

    return true;
}

bool AsptConverter::RunWithOptions(const Options &cliOptions)
{
    const auto &outname = cliOptions.GetOutput();

    dumpType_ = GetDumpTypeFromOptions(cliOptions);
    buildColdGraph_ = cliOptions.IsColdGraphEnable();
    buildSystemFrames_ = cliOptions.IsDumpSystemFrames();

    if (cliOptions.IsSubstituteModuleDir()) {
        substituteDirectories_ = {cliOptions.GetSubstituteSourceStr(), cliOptions.GetSubstituteDestinationStr()};
        if (substituteDirectories_->source.size() != substituteDirectories_->destination.size()) {
            LOG(FATAL, PROFILER) << "different number of strings in substitute option";
        }
    }

    if (cliOptions.IsDumpModules()) {
        return RunDumpModulesMode(outname);
    }

    return RunDumpTracesInCsvMode(outname);
}

}  // namespace ark::tooling::sampler
