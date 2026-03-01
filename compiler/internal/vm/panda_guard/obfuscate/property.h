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

#ifndef PANDA_GUARD_OBFUSCATE_PROPERTY_H
#define PANDA_GUARD_OBFUSCATE_PROPERTY_H

#include "entity.h"

namespace panda::guard {

class Property final : public PropertyOptionEntity, public IExtractNames {
public:
    Property(Program *program, const std::string &name) : PropertyOptionEntity(program)
    {
        this->name_ = name;
        this->obfName_ = this->name_;
    }

    static bool IsPropertyIns(const InstructionInfo &info);

    static void GetPropertyNameInfo(const InstructionInfo &info, InstructionInfo &nameInfo);

    void ExtractNames(std::set<std::string> &strings) const override;

protected:
    void Update() override;

public:
    InstructionInfo nameInfo_ {};
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_PROPERTY_H
