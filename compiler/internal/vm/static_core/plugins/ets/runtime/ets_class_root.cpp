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

#include "plugins/ets/runtime/ets_class_root.h"

namespace ark::ets {

// CC-OFFNXT(huge_cyclomatic_complexity) big switch case
EtsClassRoot ToEtsClassRoot(ClassRoot classRoot)
{
    switch (classRoot) {
        // Primitives types
        case ClassRoot::V:
            return EtsClassRoot::VOID;
        case ClassRoot::U1:
            return EtsClassRoot::BOOLEAN;
        case ClassRoot::I8:
            return EtsClassRoot::BYTE;
        case ClassRoot::U16:
            return EtsClassRoot::CHAR;
        case ClassRoot::I16:
            return EtsClassRoot::SHORT;
        case ClassRoot::I32:
            return EtsClassRoot::INT;
        case ClassRoot::I64:
            return EtsClassRoot::LONG;
        case ClassRoot::F32:
            return EtsClassRoot::FLOAT;
        case ClassRoot::F64:
            return EtsClassRoot::DOUBLE;

        // Primitive arrays types
        case ClassRoot::ARRAY_U1:
            return EtsClassRoot::BOOLEAN_ARRAY;
        case ClassRoot::ARRAY_U8:
            return EtsClassRoot::BYTE_ARRAY;
        case ClassRoot::ARRAY_U16:
            return EtsClassRoot::CHAR_ARRAY;
        case ClassRoot::ARRAY_I16:
            return EtsClassRoot::SHORT_ARRAY;
        case ClassRoot::ARRAY_I32:
            return EtsClassRoot::INT_ARRAY;
        case ClassRoot::ARRAY_I64:
            return EtsClassRoot::LONG_ARRAY;
        case ClassRoot::ARRAY_F32:
            return EtsClassRoot::FLOAT_ARRAY;
        case ClassRoot::ARRAY_F64:
            return EtsClassRoot::DOUBLE_ARRAY;

        // Object types
        case ClassRoot::CLASS:
            return EtsClassRoot::CLASS;
        case ClassRoot::OBJECT:
            return EtsClassRoot::OBJECT;

        // Other types
        case ClassRoot::STRING:
            return EtsClassRoot::STRING;
        case ClassRoot::ARRAY_CLASS:
            return EtsClassRoot::STRING_ARRAY;
        default:
            LOG(FATAL, ETS) << "Unsupporeted class_root: " << helpers::ToUnderlying(classRoot);
    }

    UNREACHABLE();
}

}  // namespace ark::ets
