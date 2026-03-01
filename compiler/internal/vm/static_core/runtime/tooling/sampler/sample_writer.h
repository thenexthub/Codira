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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_WRITER_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_WRITER_H

#include <iostream>
#include <string>
#include <fstream>

#include "libarkbase/os/thread.h"

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/samples_record.h"

#include <unordered_set>

namespace ark::tooling::sampler {

class StreamWriter {
public:
    StreamWriter() = default;
    virtual ~StreamWriter() = default;
    NO_COPY_SEMANTIC(StreamWriter);
    NO_MOVE_SEMANTIC(StreamWriter);

    virtual void WriteModule(const FileInfo &moduleInfo) = 0;
    virtual void WriteSample(const SampleInfo &sample) const = 0;
    virtual bool IsModuleWritten(const FileInfo &moduleInfo) const = 0;
};

/*
 * =======================================================
 * =============  Sampler binary format ==================
 * =======================================================
 *
 * Writing with the fasters and more convenient format .aspt
 * Then it should be converted to flamegraph
 *
 * .aspt - ark sampling profiler trace file, binary format
 *
 * .aspt consists of 2 type information:
 *   - module row (panda file and its pointer)
 *   - sample row (sample information)
 *
 * module row for 64-bits:
 *   first 8 byte is 0xFFFFFFFF (to recognize that it's not a sample row)
 *   next 8 byte is pointer module
 *   next 8 byte is size of panda file name
 *   next bytes is panda file name in ASCII symbols
 *
 * sample row for 64-bits:
 *   first 4 bytes is thread id of thread from sample was obtained
 *   next 4 bytes is thread status of thread from sample was obtained
 *   next 8 bytes is stack size
 *   next bytes is stack frame
 *   one stack frame is panda file ptr and file id
 *
 * Example for 64-bit architecture:
 *
 *            Thread id   Thread status    Stack Size      Managed stack frame id
 * Sample row |___________|___________|________________|_____________------___________|
 *              32 bits      32 bits        64 bits       (128 * <stack size>) bits
 *
 *              0xFF..FF    pointer   checksum   name size     module path (ASCII str)
 * Module row |__________|__________|__________|___________|_____________------___________|
 *              64 bits    64 bits    32 bits    64 bits       (8 * <name size>) bits
 */
class FileStreamWriter final : public StreamWriter {
public:
    explicit FileStreamWriter(const std::string &filename)
    {
        /*
         * This class instance should be used only from one thread
         * It may lead to format invalidation
         * This class wasn't made thread safe for performance reason
         */
        std::string finalFileName;
        if (filename.empty()) {
            std::time_t currentTime = std::time(nullptr);
            std::tm *localTime = std::localtime(&currentTime);
            finalFileName = std::to_string(localTime->tm_hour) + "-" + std::to_string(localTime->tm_min) + "-" +
                            std::to_string(localTime->tm_sec) + ".aspt";
        } else {
            finalFileName = filename;
        }
        writeStreamPtr_ = std::make_unique<std::ofstream>(finalFileName.c_str(), std::ios::binary);
        ASSERT(writeStreamPtr_ != nullptr);
    }

    ~FileStreamWriter() override
    {
        if (writeStreamPtr_ && writeStreamPtr_->is_open()) {
            writeStreamPtr_->flush();
        }
    };

    PANDA_PUBLIC_API void WriteModule(const FileInfo &moduleInfo) override;
    PANDA_PUBLIC_API void WriteSample(const SampleInfo &sample) const override;

    bool IsModuleWritten(const FileInfo &moduleInfo) const override
    {
        return writtenModules_.find(moduleInfo) != writtenModules_.end();
    }

    NO_COPY_SEMANTIC(FileStreamWriter);
    NO_MOVE_SEMANTIC(FileStreamWriter);

    static constexpr uintptr_t MODULE_INDICATOR_VALUE = 0xFFFFFFFF;

private:
    std::unique_ptr<std::ofstream> writeStreamPtr_;
    std::unordered_set<FileInfo> writtenModules_;
};

class InspectorStreamWriter final : public StreamWriter {
public:
    explicit InspectorStreamWriter(std::shared_ptr<SamplesRecord> samplesRecord)
        : samplesRecord_(std::move(samplesRecord))
    {
    }
    ~InspectorStreamWriter() override
    {
        samplesRecord_.reset();
    }
    PANDA_PUBLIC_API void WriteModule(const FileInfo &moduleInfo) override
    {
        UNUSED_VAR(moduleInfo);
    }
    PANDA_PUBLIC_API void WriteSample(const SampleInfo &sample) const override
    {
        samplesRecord_->AddSampleInfo(sample.threadInfo.threadId, std::make_unique<SampleInfo>(sample));
    }
    bool IsModuleWritten(const FileInfo &moduleInfo) const override
    {
        UNUSED_VAR(moduleInfo);
        return true;
    }
    NO_COPY_SEMANTIC(InspectorStreamWriter);
    NO_MOVE_SEMANTIC(InspectorStreamWriter);

private:
    std::shared_ptr<SamplesRecord> samplesRecord_ = nullptr;
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_WRITER_H
