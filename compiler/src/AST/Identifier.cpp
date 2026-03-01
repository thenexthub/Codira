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

#include "Codira/AST/Identifier.h"

#include <iostream>

#include "Codira/Utils/ConstantsUtils.h"

namespace Codira {
Identifier::Identifier(std::string s, const Position& begin, const Position& end)
{
    SetValue(std::move(s));
    SetPos(begin, end);
}

Identifier& Identifier::operator=(std::string_view identifier)
{
    std::string s{identifier};
    SetValue(std::move(s));
    return *this;
}
Identifier& Identifier::operator=(const std::string& identifier)
{
    std::string s{identifier};
    SetValue(std::move(s));
    return *this;
}
Identifier& Identifier::operator=(std::string&& identifier)
{
    SetValue(std::move(identifier));
    return *this;
}

bool Identifier::Valid() const
{
    return v != INVALID_IDENTIFIER;
}

std::ostream& operator<<(std::ostream& out, const Identifier& identifier)
{
    return out << identifier.Val() << ", begin = " << identifier.Begin() << ", end = " << identifier.End();
}

void Identifier::SetValue(std::string&& s)
{
    std::swap(v, s);
}
} // namespace Codira
