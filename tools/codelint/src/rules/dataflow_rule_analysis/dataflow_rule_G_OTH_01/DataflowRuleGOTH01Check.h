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

#ifndef DATAFLOW_RULE_G_OTH_01_CHECK_H
#define DATAFLOW_RULE_G_OTH_01_CHECK_H

#include "../DataflowRule.h"
#include "common/CommonFunc.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "nlohmann/json.hpp"

namespace Codira::CodeCheck {
class DataflowRuleGOTH01Check : public DataflowRule {
public:
    DataflowRuleGOTH01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGOTH01Check(CodeCheckDiagnosticEngine* diagEngine);
    ~DataflowRuleGOTH01Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package& package) override;

private:
    nlohmann::json sensitiveKeys;
    template <typename T> void CheckApplyOrInvoke(Ptr<T> apply, const CHIR::ConstDomain& state);
    static std::set<std::string> logMsgGetter;
};
} // namespace Codira::CodeCheck
#endif
