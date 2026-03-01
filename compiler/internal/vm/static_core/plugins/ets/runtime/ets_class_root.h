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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_ROOT_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_ROOT_H

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/type_helpers.h"
#include "runtime/include/class_root.h"

namespace ark::ets {

enum class EtsClassRoot {
    VOID = helpers::ToUnderlying(ClassRoot::V),
    BOOLEAN = helpers::ToUnderlying(ClassRoot::U1),
    BYTE = helpers::ToUnderlying(ClassRoot::I8),
    CHAR = helpers::ToUnderlying(ClassRoot::U16),
    SHORT = helpers::ToUnderlying(ClassRoot::I16),
    INT = helpers::ToUnderlying(ClassRoot::I32),
    LONG = helpers::ToUnderlying(ClassRoot::I64),
    FLOAT = helpers::ToUnderlying(ClassRoot::F32),
    DOUBLE = helpers::ToUnderlying(ClassRoot::F64),

    BOOLEAN_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_U1),
    BYTE_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_I8),
    CHAR_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_U16),
    SHORT_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_I16),
    UINT_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_U32),
    INT_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_I32),
    ULONG_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_U64),
    LONG_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_I64),
    FLOAT_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_F32),
    DOUBLE_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_F64),

    CLASS = helpers::ToUnderlying(ClassRoot::CLASS),
    OBJECT = helpers::ToUnderlying(ClassRoot::OBJECT),
    STRING = helpers::ToUnderlying(ClassRoot::STRING),
    LINE_STRING = helpers::ToUnderlying(ClassRoot::LINE_STRING),
    SLICED_STRING = helpers::ToUnderlying(ClassRoot::SLICED_STRING),
    TREE_STRING = helpers::ToUnderlying(ClassRoot::TREE_STRING),
    STRING_ARRAY = helpers::ToUnderlying(ClassRoot::ARRAY_STRING),
};

inline ClassRoot ToCoreClassRoot(EtsClassRoot etsClassRoot)
{
    return static_cast<ClassRoot>(etsClassRoot);
}

EtsClassRoot ToEtsClassRoot(ClassRoot classRoot);

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_ROOT_H
