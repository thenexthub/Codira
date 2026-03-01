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

#include "property.h"

#include "configs/guard_context.h"
#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"
#include "graph_analyzer.h"

namespace {
using OpcodeList = std::vector<panda::pandasm::Opcode>;

constexpr std::string_view TAG = "[Property]";
constexpr std::string_view STATE_VARIABLE_PREFIX = "__";

const OpcodeList PROPERTY_TYPE_LIST_NORMAL = {
    panda::pandasm::Opcode::STOBJBYNAME,          panda::pandasm::Opcode::STSUPERBYNAME,
    panda::pandasm::Opcode::DEFINEPROPERTYBYNAME, panda::pandasm::Opcode::STOBJBYVALUE,
    panda::pandasm::Opcode::STSUPERBYVALUE,
};

bool IsGetLdaStrPropertyIns(const panda::guard::InstructionInfo &info)
{
    return ((info.ins_->opcode == panda::pandasm::Opcode::STOBJBYVALUE) ||
            (info.ins_->opcode == panda::pandasm::Opcode::STSUPERBYVALUE));
}
}  // namespace

bool panda::guard::Property::IsPropertyIns(const panda::guard::InstructionInfo &info)
{
    return std::any_of(PROPERTY_TYPE_LIST_NORMAL.begin(), PROPERTY_TYPE_LIST_NORMAL.end(),
                       [&](panda::pandasm::Opcode opcode) -> bool { return info.ins_->opcode == opcode; });
}

void panda::guard::Property::GetPropertyNameInfo(const InstructionInfo &info, InstructionInfo &nameInfo)
{
    if (!IsGetLdaStrPropertyIns(info)) {
        nameInfo = info;
        return;
    }

    // this.['property']
    GraphAnalyzer::GetLdaStr(info, nameInfo);
}

void panda::guard::Property::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->name_);
}

void panda::guard::Property::Update()
{
    PANDA_GUARD_ASSERT_PRINT(!this->defineInsList_[0].IsValid() || !this->nameInfo_.IsValid(), TAG,
                             ErrorCode::GENERIC_ERROR, "get bad insInfo for ins" << this->name_);

    if (!StringUtil::IsPrefixMatched(this->name_, STATE_VARIABLE_PREFIX.data())) {
        this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
    } else {
        auto tmpName = this->name_.substr(STATE_VARIABLE_PREFIX.size());
        auto obfTmpName = GuardContext::GetInstance()->GetNameMapping()->GetName(tmpName);
        this->obfName_ = STATE_VARIABLE_PREFIX;
        this->obfName_ += obfTmpName;
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->name_, this->obfName_);
    }

    this->nameInfo_.ins_->GetId(0) = this->obfName_;
    this->program_->prog_->strings.emplace(this->obfName_);

    for (auto &defineIns : this->defineInsList_) {
        if (!defineIns.IsValid()) {
            LOG(INFO, PANDAGUARD) << TAG << "no bind define ins, ignore update define";
            continue;
        }

        if (!IsGetLdaStrPropertyIns(defineIns)) {
            defineIns.ins_->GetId(0) = this->obfName_;
        }
    }
}