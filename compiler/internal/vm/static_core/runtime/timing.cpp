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

#include "runtime/timing.h"

#include <iomanip>

#include "libarkbase/utils/logger.h"

namespace ark {

constexpr uint64_t NS_PER_SECOND = 1000000000;
constexpr uint64_t NS_PER_MILLISECOND = 1000000;
constexpr uint64_t NS_PER_MICROSECOND = 1000;

PandaString Timing::PrettyTimeNs(uint64_t duration)
{
    uint64_t timeUint;
    PandaString timeUintName;
    uint64_t mainPart;
    uint64_t fractionalPart;
    if (duration > NS_PER_SECOND) {
        timeUint = NS_PER_SECOND;
        mainPart = duration / timeUint;
        fractionalPart = duration % timeUint / NS_PER_MILLISECOND;
        timeUintName = "s";
    } else if (duration > NS_PER_MILLISECOND) {
        timeUint = NS_PER_MILLISECOND;
        mainPart = duration / timeUint;
        fractionalPart = duration % timeUint / NS_PER_MICROSECOND;
        timeUintName = "ms";
    } else {
        timeUint = NS_PER_MICROSECOND;
        mainPart = duration / timeUint;
        fractionalPart = duration % timeUint;
        timeUintName = "us";
    }
    PandaStringStream ss;
    constexpr size_t FRACTION_WIDTH = 3U;
    ss << mainPart << "." << std::setfill('0') << std::setw(FRACTION_WIDTH) << fractionalPart << timeUintName;
    return ss.str();
}

void Timing::Process()
{
    PandaStack<PandaVector<TimeLabel>::iterator> labelStack;
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (auto it = labels_.begin(); it != labels_.end(); it++) {
        if (it->GetType() == TimeLabelType::BEGIN) {
            labelStack.push(it);
            continue;
        }
        auto beginIt = labelStack.top();
        labelStack.pop();
        uint64_t duration = it->GetTime() - beginIt->GetTime();
        uint64_t cpuDuration = it->GetCPUTime() - beginIt->GetCPUTime();
        beginIt->SetTime(duration);
        beginIt->SetCPUTime(cpuDuration);
    }
}

PandaString Timing::Dump()
{
    Process();
    PandaStringStream ss;
    std::string indent = "    ";
    size_t indentCount = 0;
    for (auto &label : labels_) {
        if (label.GetType() == TimeLabelType::BEGIN) {
            for (size_t i = 0; i < indentCount; i++) {
                ss << indent;
            }
            ss << label.GetName() << " " << PrettyTimeNs(label.GetCPUTime()) << "/" << PrettyTimeNs(label.GetTime())
               << std::endl;
            indentCount++;
            continue;
        }
        if (indentCount > 0U) {
            indentCount--;
        }
    }
    return ss.str();
}

}  // namespace ark
