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

#include "trace_dumper.h"

namespace ark::tooling::sampler {

void TraceDumper::DumpTraces(const SampleInfo &sample, size_t count)
{
    std::ofstream &stream = ResolveStream(sample);

    for (size_t i = sample.stackInfo.managedStackSize; i-- > 0;) {
        uintptr_t pfId = sample.stackInfo.managedStack[i].pandaFilePtr;
        uint64_t fileId = sample.stackInfo.managedStack[i].fileId;

        std::string fullMethodName;
        if (pfId == helpers::ToUnderlying(FrameKind::BRIDGE)) {
            if (!buildSystemFrames_) {
                continue;
            }

            fullMethodName = "System_Frame";
        } else {
            const panda_file::File *pf = nullptr;
            auto it = modulesMap_->find(pfId);
            if (it != modulesMap_->end()) {
                pf = it->second.get();
            }

            fullMethodName = ResolveName(pf, fileId);
        }
        stream << fullMethodName << "; ";
    }
    stream << count << "\n";
}

void TraceDumper::SetBuildSystemFrames(bool buildSystemFrames)
{
    buildSystemFrames_ = buildSystemFrames;
}

/* static */
void TraceDumper::WriteThreadId(std::ofstream &stream, uint32_t threadId)
{
    stream << "thread_id = " << threadId << "; ";
}

/* static */
void TraceDumper::WriteThreadStatus(std::ofstream &stream, SampleInfo::ThreadStatus threadStatus)
{
    stream << "status = ";

    switch (threadStatus) {
        case SampleInfo::ThreadStatus::RUNNING: {
            stream << "active; ";
            break;
        }
        case SampleInfo::ThreadStatus::SUSPENDED: {
            stream << "suspended; ";
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

std::string TraceDumper::ResolveName(const panda_file::File *pf, uint64_t fileId) const
{
    if (pf == nullptr) {
        return std::string("__unknown_module::" + std::to_string(fileId));
    }

    auto it = methodsMap_->find(pf);
    if (it != methodsMap_->end()) {
        return it->second.at(fileId);
    }

    return pf->GetFilename() + "::__unknown_" + std::to_string(fileId);
}

/* override */
std::ofstream &SingleCSVDumper::ResolveStream(const SampleInfo &sample)
{
    if (option_ == DumpType::THREAD_SEPARATION_BY_TID) {
        WriteThreadId(stream_, sample.threadInfo.threadId);
    }
    if (buildColdGraph_) {
        WriteThreadStatus(stream_, sample.threadInfo.threadStatus);
    }
    return stream_;
}

/* override */
std::ofstream &MultipleCSVDumper::ResolveStream(const SampleInfo &sample)
{
    auto it = threadIdMap_.find(sample.threadInfo.threadId);
    if (it == threadIdMap_.end()) {
        std::string filenameWithThreadId = AddThreadIdToFilename(filename_, sample.threadInfo.threadId);

        auto returnPair = threadIdMap_.insert({sample.threadInfo.threadId, std::ofstream(filenameWithThreadId)});
        it = returnPair.first;

        auto isSuccessInsert = returnPair.second;
        if (!isSuccessInsert) {
            LOG(FATAL, PROFILER) << "Failed while insert in unordored_map";
        }
    }

    WriteThreadId(it->second, sample.threadInfo.threadId);

    if (buildColdGraph_) {
        WriteThreadStatus(it->second, sample.threadInfo.threadStatus);
    }

    return it->second;
}

/* static */
std::string MultipleCSVDumper::AddThreadIdToFilename(const std::string &filename, uint32_t threadId)
{
    std::string filenameWithThreadId(filename);

    std::size_t pos = filenameWithThreadId.find("csv");
    if (pos == std::string::npos) {
        LOG(FATAL, PROFILER) << "Incorrect output filename, *.csv format expected";
    }

    filenameWithThreadId.insert(pos - 1, std::to_string(threadId));

    return filenameWithThreadId;
}

}  // namespace ark::tooling::sampler
