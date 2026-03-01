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

#include "constant_propagation.h"

namespace panda::bytecodeopt {

LatticeElement::LatticeElement(LatticeType type) : type_(type) {}

TopElement::TopElement() : LatticeElement(LatticeType::LATTICE_TOP) {}

LatticeElement *TopElement::Meet(LatticeElement *other)
{
    return other;
}

LatticeElement *TopElement::GetInstance()
{
    static TopElement instance;
    return &instance;
}

std::string TopElement::ToString()
{
    return "Top";
}

BottomElement::BottomElement() : LatticeElement(LatticeType::LATTICE_BOTTOM) {}

LatticeElement *BottomElement::Meet(LatticeElement *other)
{
    return this;
}

LatticeElement *BottomElement::GetInstance()
{
    static BottomElement instance;
    return &instance;
}

std::string BottomElement::ToString()
{
    return "Bottom";
}

ConstantElement::ConstantElement(bool val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), value_(val)
{
}

ConstantElement::ConstantElement(int32_t val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), value_(val)
{
}

ConstantElement::ConstantElement(int64_t val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), value_(val)
{
}

ConstantElement::ConstantElement(double val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), value_(val)
{
}

ConstantElement::ConstantElement(std::string val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), value_(val)
{
}

ConstantElement::ConstantElement(const ConstantValue &val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), value_(val)
{
}

LatticeElement *ConstantElement::Meet(LatticeElement *other)
{
    if (other->IsTopElement()) {
        return this;
    }
    if (other->IsBottomElement()) {
        return other;
    }

    if (value_ == other->AsConstant()->value_) {
        return this;
    }

    return BottomElement::GetInstance();
}

std::string ConstantElement::ToString()
{
    return "Constant: " + value_.ToString();
}

ConstantElement *ConstantElement::AsConstant()
{
    return this;
}

}  // namespace panda::bytecodeopt

