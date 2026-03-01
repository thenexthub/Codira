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
#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_INL_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_INL_H

#include <iostream>
#include <securec.h>
#include <string>

#include "libarkbase/utils/logger.h"

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/sample_reader.h"
#include "runtime/tooling/sampler/sample_writer.h"

namespace ark::tooling::sampler {

// -------------------------------------------------
// ------------- Format reader ---------------------
// -------------------------------------------------

/*
 * Example for 64 bit architecture
 *
 *              Thread id   Thread status    Stack Size      Managed stack frame id
 * Sample row |___________|______________|________________|_____________------___________|
 *              32 bits      32 bits          64 bits       (128 * <stack size>) bits
 *
 *
 *              0xFF..FF    pointer   checksum   name size     module path (ASCII str)
 * Module row |__________|__________|__________|___________|_____________------___________|
 *              64 bits    64 bits    32 bits    64 bits       (8 * <name size>) bits
 */

// CC-OFFNXT(G.FUD.06) Splitting this function will degrade readability. Keyword "inline" needs to satisfy ODR rule.
inline SampleReader::SampleReader(const char *filename)
{
    {
        std::ifstream binFile(filename, std::ios::binary | std::ios::ate);
        if (!binFile) {
            LOG(FATAL, PROFILER) << "Unable to open file \"" << filename << "\"";
            UNREACHABLE();
        }
        std::streamsize bufferSize = binFile.tellg();
        binFile.seekg(0, std::ios::beg);

        buffer_.resize(bufferSize);

        if (!binFile.read(buffer_.data(), bufferSize)) {
            LOG(FATAL, PROFILER) << "Unable to read sampler trace file";
            UNREACHABLE();
        }
        binFile.close();
    }

    size_t bufferCounter = 0;
    while (bufferCounter < buffer_.size()) {
        if (ReadUintptrTBitMisaligned(&buffer_[bufferCounter]) == FileStreamWriter::MODULE_INDICATOR_VALUE) {
            // This entry is panda file
            size_t pfNameSize = ReadUintptrTBitMisaligned(&buffer_[bufferCounter + PANDA_FILE_NAME_SIZE_OFFSET]);
            size_t nextModulePtrOffset = PANDA_FILE_NAME_OFFSET + pfNameSize * sizeof(char);

            if (bufferCounter + nextModulePtrOffset > buffer_.size()) {
                LOG(ERROR, PROFILER) << "ark sampling profiler drop last module, because of invalid trace file";
                return;
            }

            moduleRowPtrs_.push_back(&buffer_[bufferCounter]);
            bufferCounter += nextModulePtrOffset;
            continue;
        }

        // buffer_counter now is entry of a sample, stack size lies after thread_id
        size_t stackSize = ReadUintptrTBitMisaligned(&buffer_[bufferCounter + SAMPLE_STACK_SIZE_OFFSET]);
        if (stackSize > SampleInfo::StackInfo::MAX_STACK_DEPTH) {
            LOG(FATAL, PROFILER) << "ark sampling profiler trace file is invalid, stack_size > MAX_STACK_DEPTH";
            UNREACHABLE();
        }

        size_t nextSamplePtrOffset = SAMPLE_STACK_OFFSET + stackSize * sizeof(SampleInfo::ManagedStackFrameId);
        if (bufferCounter + nextSamplePtrOffset > buffer_.size()) {
            LOG(ERROR, PROFILER) << "ark sampling profiler drop last samples, because of invalid trace file";
            return;
        }

        sampleRowPtrs_.push_back(&buffer_[bufferCounter]);
        bufferCounter += nextSamplePtrOffset;
    }

    if (bufferCounter != buffer_.size()) {
        LOG(ERROR, PROFILER) << "ark sampling profiler trace file is invalid";
    }
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// CC-OFFNXT(G.FUD.06) Splitting this function will degrade readability. Keyword "inline" needs to satisfy ODR rule.
inline bool SampleReader::GetNextSample(SampleInfo *sampleOut)
{
    if (sampleRowPtrs_.size() <= sampleRowCounter_) {
        return false;
    }
    const char *currentSamplePtr = sampleRowPtrs_[sampleRowCounter_];
    sampleOut->threadInfo.threadId = ReadUint32TBitMisaligned(&currentSamplePtr[SAMPLE_THREAD_ID_OFFSET]);
    sampleOut->threadInfo.threadStatus =
        static_cast<SampleInfo::ThreadStatus>(ReadUint32TBitMisaligned(&currentSamplePtr[SAMPLE_THREAD_STATUS_OFFSET]));
    sampleOut->stackInfo.managedStackSize = ReadUintptrTBitMisaligned(&currentSamplePtr[SAMPLE_STACK_SIZE_OFFSET]);

    ASSERT(sampleOut->stackInfo.managedStackSize <= SampleInfo::StackInfo::MAX_STACK_DEPTH);
    auto copySize = sampleOut->stackInfo.managedStackSize * sizeof(SampleInfo::ManagedStackFrameId);
    [[maybe_unused]] int r =
        memcpy_s(sampleOut->stackInfo.managedStack.data(), copySize, currentSamplePtr + SAMPLE_STACK_OFFSET, copySize);
    ASSERT(r == 0);
    ++sampleRowCounter_;
    return true;
}

// CC-OFFNXT(G.FUD.06) Splitting this function will degrade readability. Keyword "inline" needs to satisfy ODR rule.
inline bool SampleReader::GetNextModule(FileInfo *moduleOut)
{
    if (moduleRowPtrs_.size() <= moduleRowCounter_) {
        return false;
    }

    moduleOut->pathname.clear();

    const char *currentModulePtr = moduleRowPtrs_[moduleRowCounter_];
    moduleOut->ptr = ReadUintptrTBitMisaligned(&currentModulePtr[PANDA_FILE_POINTER_OFFSET]);
    moduleOut->checksum = ReadUint32TBitMisaligned(&currentModulePtr[PANDA_FILE_CHECKSUM_OFFSET]);
    size_t strSize = ReadUintptrTBitMisaligned(&currentModulePtr[PANDA_FILE_NAME_SIZE_OFFSET]);
    const char *strPtr = &currentModulePtr[PANDA_FILE_NAME_OFFSET];
    moduleOut->pathname = std::string(strPtr, strSize);

    ++moduleRowCounter_;
    return true;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_INL_H
