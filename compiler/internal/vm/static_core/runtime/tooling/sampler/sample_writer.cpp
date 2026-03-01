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

#include <iostream>
#include <fstream>
#include <string>

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/sample_writer.h"

namespace ark::tooling::sampler {

void FileStreamWriter::WriteSample(const SampleInfo &sample) const
{
    ASSERT(writeStreamPtr_ != nullptr);
    ASSERT(sample.stackInfo.managedStackSize <= SampleInfo::StackInfo::MAX_STACK_DEPTH);

    static_assert(sizeof(sample.threadInfo.threadId) == sizeof(uint32_t));
    static_assert(sizeof(sample.threadInfo.threadStatus) == sizeof(uint32_t));
    static_assert(sizeof(sample.stackInfo.managedStackSize) == sizeof(uintptr_t));

    writeStreamPtr_->write(reinterpret_cast<const char *>(&sample.threadInfo.threadId),
                           sizeof(sample.threadInfo.threadId));
    writeStreamPtr_->write(reinterpret_cast<const char *>(&sample.threadInfo.threadStatus),
                           sizeof(sample.threadInfo.threadStatus));
    writeStreamPtr_->write(reinterpret_cast<const char *>(&sample.stackInfo.managedStackSize),
                           sizeof(sample.stackInfo.managedStackSize));
    writeStreamPtr_->write(reinterpret_cast<const char *>(sample.stackInfo.managedStack.data()),
                           sample.stackInfo.managedStackSize * sizeof(SampleInfo::ManagedStackFrameId));
}

void FileStreamWriter::WriteModule(const FileInfo &moduleInfo)
{
    ASSERT(writeStreamPtr_ != nullptr);
    static_assert(sizeof(MODULE_INDICATOR_VALUE) == sizeof(uintptr_t));
    static_assert(sizeof(moduleInfo.ptr) == sizeof(uintptr_t));
    static_assert(sizeof(moduleInfo.checksum) == sizeof(uint32_t));
    static_assert(sizeof(moduleInfo.pathname.length()) == sizeof(uintptr_t));

    if (moduleInfo.pathname.empty()) {
        return;
    }
    size_t strSize = moduleInfo.pathname.length();

    writeStreamPtr_->write(reinterpret_cast<const char *>(&MODULE_INDICATOR_VALUE), sizeof(MODULE_INDICATOR_VALUE));
    writeStreamPtr_->write(reinterpret_cast<const char *>(&moduleInfo.ptr), sizeof(moduleInfo.ptr));
    writeStreamPtr_->write(reinterpret_cast<const char *>(&moduleInfo.checksum), sizeof(moduleInfo.checksum));
    writeStreamPtr_->write(reinterpret_cast<const char *>(&strSize), sizeof(moduleInfo.pathname.length()));
    writeStreamPtr_->write(moduleInfo.pathname.data(), moduleInfo.pathname.length() * sizeof(char));

    writtenModules_.insert(moduleInfo);
}

}  // namespace ark::tooling::sampler
