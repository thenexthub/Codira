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

#include "Codira/CHIR/AttributeInfo.h"
#include <iostream>
#include <sstream>

using namespace Codira::CHIR;

std::string AttributeInfo::ToString() const
{
    std::stringstream ss;
    for (int attr = static_cast<int>(Attribute::STATIC); attr < static_cast<int>(Attribute::ATTR_END); attr++) {
        if (TestAttr(static_cast<Attribute>(attr))) {
            ss << "[" << ATTR_TO_STRING.at(static_cast<Attribute>(attr)) << "] ";
        }
    }
    return ss.str();
}

void AttributeInfo::Dump() const
{
    std::cout << ToString() << std::endl;
}
