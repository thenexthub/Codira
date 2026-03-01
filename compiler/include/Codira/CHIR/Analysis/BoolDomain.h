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

#ifndef CODIRA_CHIR_ANALYSIS_BOOL_DOMAIN_H
#define CODIRA_CHIR_ANALYSIS_BOOL_DOMAIN_H

#include "Codira/CHIR/Expression/Terminator.h"

namespace Codira::CHIR {
using PtrSymbol = Ptr<Value>;

/// Represents all possible values of a CHIRNode that has Ty bool.
class BoolDomain {
public:
    /// deleted constructor, use BoolDomain::FromBool instead.
    BoolDomain(bool) = delete;

    BoolDomain(const BoolDomain& other);

    BoolDomain(BoolDomain&& other);

    BoolDomain& operator=(const BoolDomain& other);

    BoolDomain& operator=(BoolDomain&& other);

    ~BoolDomain();

    /// Shared instances that represents all the possible values of BoolDomain.
    static BoolDomain True();
    static BoolDomain False();
    static BoolDomain Top();
    static BoolDomain Bottom();

    bool IsTrue() const;
    bool IsFalse() const;
    /// every bool is possible.
    bool IsTop() const;
    /// every bool is not possible or init state.
    bool IsBottom() const;
    /// non top
    bool IsNonTrivial() const;
    /// whether state is determined.
    bool IsSingleValue() const;
    /// get determined state.
    bool GetSingleValue() const;

    /// Construct from bool value
    static BoolDomain FromBool(bool v);

    /// operator of bool
    friend BoolDomain operator&(const BoolDomain& a, const BoolDomain& b);
    friend BoolDomain operator|(const BoolDomain& a, const BoolDomain& b);
    friend BoolDomain LogicalAnd(const BoolDomain& a, const BoolDomain& b);
    friend BoolDomain LogicalOr(const BoolDomain& a, const BoolDomain& b);
    friend BoolDomain operator!(const BoolDomain& v);
    friend std::ostream& operator<<(std::ostream& out, const BoolDomain& v);

    /// union of two states
    static BoolDomain Union(const BoolDomain& a, const BoolDomain& b);

    /// whether two states are same
    bool IsSame(const BoolDomain& domain) const;
private:
    unsigned v;
    // Construct from integer value \p v. This constructor is private; use True/False/Top/Bottom instead.
    explicit BoolDomain(unsigned v);
};
// operator== on BoolDomain is deleted because there is no definite meaning of equality on BoolDomain, be it the
// identity of a boolean domain or the identity of boolean logical value. Use IsTrue/IsTop/... to check the value
// of a BoolDomain
bool operator==(const BoolDomain& a, const BoolDomain& b) = delete;
} // namespace Codira::CHIR

#endif
