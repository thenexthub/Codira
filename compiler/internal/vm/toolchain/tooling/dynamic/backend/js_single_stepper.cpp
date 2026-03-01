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

#include "backend/js_single_stepper.h"

#include "ecmascript/debugger/debugger_api.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"

namespace panda::ecmascript::tooling {
uint32_t SingleStepper::GetStackDepth() const
{
    return DebuggerApi::GetStackDepth(ecmaVm_);
}

bool SingleStepper::InStepRange(uint32_t pc) const
{
    for (const auto &range : stepRanges_) {
        if (pc >= range.startBcOffset && pc < range.endBcOffset) {
            return true;
        }
    }
    return false;
}

bool SingleStepper::StepComplete(uint32_t bcOffset) const
{
    std::unique_ptr<PtMethod> ptMethod = DebuggerApi::GetMethod(ecmaVm_);
    uint32_t stackDepth = GetStackDepth();

    switch (type_) {
        case Type::STEP_INTO: {
            if ((method_->GetMethodId() == ptMethod->GetMethodId()) &&
                (method_->GetJSPandaFile() == ptMethod->GetJSPandaFile()) &&
                (InStepRange(bcOffset))) {
                return false;
            }
            break;
        }
        case Type::STEP_OVER: {
            if (stackDepth_ < stackDepth) {
                return false;
            }
            if (stackDepth_ == stackDepth && InStepRange(bcOffset)) {
                return false;
            }
            break;
        }
        case Type::STEP_OUT: {
            if (stackDepth_ <= stackDepth) {
                return false;
            }
            break;
        }
        default: {
            return false;
        }
    }

    return true;
}

std::unique_ptr<SingleStepper> SingleStepper::GetStepIntoStepper(const EcmaVM *ecmaVm)
{
    return GetStepper(ecmaVm, SingleStepper::Type::STEP_INTO);
}

std::unique_ptr<SingleStepper> SingleStepper::GetStepOverStepper(const EcmaVM *ecmaVm)
{
    return GetStepper(ecmaVm, SingleStepper::Type::STEP_OVER);
}

std::unique_ptr<SingleStepper> SingleStepper::GetStepOutStepper(const EcmaVM *ecmaVm)
{
    return GetStepper(ecmaVm, SingleStepper::Type::STEP_OUT);
}

std::list<JSPtStepRange> SingleStepper::GetStepRanges(DebugInfoExtractor *extractor,
    panda_file::File::EntityId methodId, uint32_t offset)
{
    std::list<JSPtStepRange> ranges {};
    const LineNumberTable &table = extractor->GetLineNumberTable(methodId);
    auto callbackFunc = [table, &ranges](int32_t line) -> bool {
        for (auto it = table.begin(); it != table.end(); ++it) {
            auto next = it + 1;
            if (it->line == line) {
                JSPtStepRange range {it->offset, next != table.end() ? next->offset : UINT32_MAX};
                ranges.push_back(range);
            }
        }
        return true;
    };
    extractor->MatchLineWithOffset(callbackFunc, methodId, offset);
    return ranges;
}

std::unique_ptr<SingleStepper> SingleStepper::GetStepper(const EcmaVM *ecmaVm,
    SingleStepper::Type type)
{
    std::unique_ptr<PtMethod> ptMethod = DebuggerApi::GetMethod(ecmaVm);
    ASSERT(ptMethod != nullptr);

    DebugInfoExtractor *extractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(
        ptMethod->GetJSPandaFile());
    if (extractor == nullptr) {
        LOG_DEBUGGER(ERROR) << "GetStepper: extractor is null";
        return nullptr;
    }

    if (type == SingleStepper::Type::STEP_OUT) {
        return std::make_unique<SingleStepper>(ecmaVm, std::move(ptMethod), std::list<JSPtStepRange> {}, type);
    }

    std::list<JSPtStepRange> ranges = GetStepRanges(extractor, ptMethod->GetMethodId(),
        DebuggerApi::GetBytecodeOffset(ecmaVm));
    return std::make_unique<SingleStepper>(ecmaVm, std::move(ptMethod), std::move(ranges), type);
}
}  // namespace panda::ecmascript::tooling
