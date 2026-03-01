/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECMASCRIPT_TOOLING_BACKEND_JS_SINGLE_STEPPER_H
#define ECMASCRIPT_TOOLING_BACKEND_JS_SINGLE_STEPPER_H

#include "ecmascript/debugger/js_debugger_interface.h"
#include "ecmascript/debugger/js_pt_method.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/jspandafile/debug_info_extractor.h"

#include "libpandabase/macros.h"

namespace panda::ecmascript::tooling {
class SingleStepper {
public:
    enum class Type { STEP_INTO, STEP_OVER, STEP_OUT };
    SingleStepper(const EcmaVM *ecmaVm, std::unique_ptr<PtMethod> ptMethod,
        std::list<JSPtStepRange> stepRanges, Type type)
        : ecmaVm_(ecmaVm),
          method_(std::move(ptMethod)),
          stepRanges_(std::move(stepRanges)),
          stackDepth_(GetStackDepth()),
          type_(type) {}

    virtual ~SingleStepper() = default;
    NO_COPY_SEMANTIC(SingleStepper);
    NO_MOVE_SEMANTIC(SingleStepper);

    bool StepComplete(uint32_t bcOffset) const;
    Type GetStepperType() const
    {
        return type_;
    }

    static std::unique_ptr<SingleStepper> GetStepIntoStepper(const EcmaVM *ecmaVm);
    static std::unique_ptr<SingleStepper> GetStepOverStepper(const EcmaVM *ecmaVm);
    static std::unique_ptr<SingleStepper> GetStepOutStepper(const EcmaVM *ecmaVm);

private:
    uint32_t GetStackDepth() const;
    bool InStepRange(uint32_t pc) const;
    static std::list<JSPtStepRange> GetStepRanges(DebugInfoExtractor *extractor,
        panda_file::File::EntityId methodId, uint32_t offset);
    static std::unique_ptr<SingleStepper> GetStepper(const EcmaVM *ecmaVm, SingleStepper::Type type);

    const EcmaVM *ecmaVm_;
    std::unique_ptr<PtMethod> method_;
    std::list<JSPtStepRange> stepRanges_;
    uint32_t stackDepth_;
    Type type_;
};
}  // namespace panda::ecmascript::tooling
#endif  // ECMASCRIPT_TOOLING_BACKEND_JS_SINGLE_STEPPER_H
