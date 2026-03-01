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

#include "Codira/CHIR/Analysis/BoolDomain.h"

namespace Codira::CHIR {
constexpr unsigned N{0};
constexpr unsigned F{1};
constexpr unsigned T{2};
constexpr unsigned A{3};
constexpr unsigned OP_TABLE_LEN{4};

BoolDomain::BoolDomain(const BoolDomain& other) : v{other.v}
{
}

BoolDomain::BoolDomain(BoolDomain&& other) : v{other.v}
{
}

BoolDomain& BoolDomain::operator=(const BoolDomain& other)
{
    if (&other != this) {
        v = other.v;
    }
    return *this;
}
BoolDomain& BoolDomain::operator=(BoolDomain&& other)
{
    if (&other != this) {
        v = other.v;
    }
    return *this;
}

BoolDomain::~BoolDomain()
{
}

BoolDomain::BoolDomain(unsigned v) : v{v}
{
}

BoolDomain BoolDomain::Bottom()
{
    return BoolDomain{N};
}

BoolDomain BoolDomain::False()
{
    return BoolDomain{F};
}

BoolDomain BoolDomain::True()
{
    return BoolDomain{T};
}

BoolDomain BoolDomain::Top()
{
    return BoolDomain{A};
}

bool BoolDomain::IsTrue() const
{
    return v == T;
}

bool BoolDomain::IsFalse() const
{
    return v == F;
}

bool BoolDomain::IsTop() const
{
    return v == A;
}

bool BoolDomain::IsBottom() const
{
    return v == N;
}

bool BoolDomain::IsNonTrivial() const
{
    return !IsTop();
}

bool BoolDomain::IsSingleValue() const
{
    return v == T || v == F;
}

bool BoolDomain::GetSingleValue() const
{
    return v == T;
}

BoolDomain BoolDomain::FromBool(bool v)
{
    return v ? BoolDomain{T} : BoolDomain{F};
}

BoolDomain LogicalAnd(const BoolDomain& a, const BoolDomain& b)
{
    static constexpr unsigned logicalAndTable[OP_TABLE_LEN][OP_TABLE_LEN]{
        {N, N, N, N},
        {N, F, F, F},
        {N, F, T, A},
        {N, F, A, A},
    };
    return BoolDomain{logicalAndTable[a.v][b.v]};
}

BoolDomain LogicalOr(const BoolDomain& a, const BoolDomain& b)
{
    static constexpr unsigned logicalOrTable[OP_TABLE_LEN][OP_TABLE_LEN]{
        {N, N, N, N},
        {N, F, T, A},
        {N, T, T, T},
        {N, A, T, A},
    };
    return BoolDomain{logicalOrTable[a.v][b.v]};
}

BoolDomain operator&(const BoolDomain& a, const BoolDomain& b)
{
    return BoolDomain{a.v & b.v};
}

BoolDomain operator|(const BoolDomain& a, const BoolDomain& b)
{
    return BoolDomain{a.v | b.v};
}

BoolDomain operator!(const BoolDomain& v)
{
    static constexpr unsigned logicalNotTable[]{N, T, F, A};
    return BoolDomain{logicalNotTable[v.v]};
}

std::ostream& operator<<(std::ostream& out, const BoolDomain& v)
{
    static const std::string B[OP_TABLE_LEN]{"<>", "false", "true", "<t,f>"};
    return out << B[v.v];
}

BoolDomain BoolDomain::Union(const BoolDomain& a, const BoolDomain& b)
{
    return a | b;
}

bool BoolDomain::IsSame(const BoolDomain& domain) const
{
    return v == domain.v;
}

} // namespace Codira::CHIR
