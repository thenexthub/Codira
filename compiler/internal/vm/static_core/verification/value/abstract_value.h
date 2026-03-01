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

#ifndef PANDA_VERIFIER_ABSTRACT_VALUE_HPP
#define PANDA_VERIFIER_ABSTRACT_VALUE_HPP

#include "verification/value/variables.h"

#include "libarkbase/macros.h"

#include <variant>

namespace ark::verifier {
class AbstractValue {
public:
    struct None {};
    using ContentsData = std::variant<None, Variables::Var>;
    AbstractValue() = default;
    AbstractValue(const AbstractValue &) = default;
    AbstractValue(AbstractValue &&) = default;
    AbstractValue &operator=(const AbstractValue &) = default;
    AbstractValue &operator=(AbstractValue &&) = default;
    ~AbstractValue() = default;
    // NOLINTNEXTLINE(google-explicit-constructor)
    AbstractValue(const Variables::Var &var) : contents_ {var} {}
    Variables::Var GetVar() const
    {
        ASSERT(IsVar());
        return std::get<Variables::Var>(contents_);
    }
    AbstractValue &operator=(const None & /* unused */)
    {
        contents_ = None {};
        return *this;
    }
    AbstractValue &operator=(Variables::Var const &var)
    {
        contents_ = var;
        return *this;
    }
    bool IsNone() const
    {
        return std::holds_alternative<None>(contents_);
    }
    bool IsVar() const
    {
        return std::holds_alternative<Variables::Var>(contents_);
    }
    void Clear()
    {
        contents_ = None {};
    }
    AbstractValue operator&([[maybe_unused]] const AbstractValue &rhs) const
    {
        // currently only None is supported
        // ASSERT(IsNone() && rhs.IsNone())
        return {};
    }

private:
    ContentsData contents_ {None {}};
};
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_ABSTRACT_VALUE_HPP
