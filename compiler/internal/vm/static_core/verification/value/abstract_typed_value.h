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

#ifndef PANDA_VERIFIER_ABSTRACT_TYPED_VALUE_HPP
#define PANDA_VERIFIER_ABSTRACT_TYPED_VALUE_HPP

#include "verification/type/type_type.h"
#include "verification/value/abstract_value.h"

#include "verification/value/origin.h"

#include "libarkfile/bytecode_instruction.h"

#include "libarkbase/macros.h"

namespace ark::verifier {
class AbstractTypedValue {
    using ValueOrigin = Origin<ark::BytecodeInstructionSafe>;

public:
    bool IsNone() const
    {
        return type_.IsNone();
    }
    AbstractTypedValue() = default;
    AbstractTypedValue(const AbstractTypedValue &) = default;
    AbstractTypedValue(AbstractTypedValue &&) = default;
    AbstractTypedValue &operator=(const AbstractTypedValue &) = default;
    AbstractTypedValue &operator=(AbstractTypedValue &&) = default;
    ~AbstractTypedValue() = default;
    AbstractTypedValue(Type type, AbstractValue value) : value_ {std::move(value)}, type_ {type} {}
    AbstractTypedValue(const AbstractTypedValue &atv, const ark::BytecodeInstructionSafe &inst)
        : value_ {atv.value_}, type_ {atv.type_}, origin_ {inst}
    {
    }
    AbstractTypedValue(Type type, AbstractValue value, const ark::BytecodeInstructionSafe &inst)
        : value_ {std::move(value)}, type_ {type}, origin_ {inst}
    {
    }
    AbstractTypedValue(Type type, AbstractValue value, ValueOrigin origin)
        : value_ {std::move(value)}, type_ {type}, origin_ {std::move(origin)}
    {
    }
    struct Start {};
    AbstractTypedValue(Type type, AbstractValue value, [[maybe_unused]] Start start, size_t n)
        : value_ {std::move(value)}, type_ {type}, origin_ {OriginType::START, n}
    {
    }
    AbstractTypedValue &SetAbstractType(Type type)
    {
        type_ = type;
        return *this;
    }
    AbstractTypedValue &SetAbstractValue(const AbstractValue &value)
    {
        value_ = value;
        return *this;
    }
    Type GetAbstractType() const
    {
        return type_;
    }
    const AbstractValue &GetAbstractValue() const
    {
        return value_;
    }
    bool IsConsistent() const
    {
        return type_.IsConsistent();
    }
    ValueOrigin &GetOrigin()
    {
        return origin_;
    }
    const ValueOrigin &GetOrigin() const
    {
        return origin_;
    }
    PandaString ToString(TypeSystem const *tsys) const
    {
        // currently only types and origin printed
        PandaString result {GetAbstractType().ToString(tsys)};
        if (origin_.IsValid()) {
            if (origin_.AtStart()) {
                result += "@start";
            } else {
                result += "@" + OffsetToHexStr(origin_.GetOffset());
            }
        }
        return result;
    }

private:
    AbstractValue value_;
    Type type_;
    ValueOrigin origin_;

    friend AbstractTypedValue AtvJoin(AbstractTypedValue const * /* lhs */, AbstractTypedValue const * /* rhs */,
                                      TypeSystem * /* tsys */);
};

inline AbstractTypedValue AtvJoin(AbstractTypedValue const *lhs, AbstractTypedValue const *rhs, TypeSystem *tsys)
{
    if (lhs->origin_.IsValid() && rhs->origin_.IsValid() && lhs->origin_ == rhs->origin_) {
        return {TpUnion(lhs->type_, rhs->GetAbstractType(), tsys), lhs->value_ & rhs->GetAbstractValue(), lhs->origin_};
    }
    return {TpUnion(lhs->type_, rhs->GetAbstractType(), tsys), lhs->value_ & rhs->GetAbstractValue()};
}

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_ABSTRACT_TYPED_VALUE_HPP
