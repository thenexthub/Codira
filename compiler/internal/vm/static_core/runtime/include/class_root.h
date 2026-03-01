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
#ifndef PANDA_RUNTIME_CLASS_ROOT_H
#define PANDA_RUNTIME_CLASS_ROOT_H

namespace ark {

enum class ClassRoot {
    V = 0,
    U1,
    I8,
    U8,
    I16,
    U16,
    I32,
    U32,
    I64,
    U64,
    F32,
    F64,
    ANY,
    NEVER,
    TAGGED,
    ARRAY_U1,
    ARRAY_I8,
    ARRAY_U8,
    ARRAY_I16,
    ARRAY_U16,
    ARRAY_I32,
    ARRAY_U32,
    ARRAY_I64,
    ARRAY_U64,
    ARRAY_F32,
    ARRAY_F64,
    ARRAY_TAGGED,
    CLASS,
    OBJECT,
    STRING,         // -> STRING_CLASS
    LINE_STRING,    // -> LINE_STRING_CLASS
    SLICED_STRING,  // -> SLICED_STRING_CLASS
    TREE_STRING,    // -> TREE_STRING_CLASS
    ARRAY_CLASS,
    ARRAY_STRING,
    LAST_CLASS_ROOT_ENTRY = ARRAY_STRING  // Must be the last in this enum
};

}  // namespace ark

#endif  // PANDA_RUNTIME_CLASS_ROOT_H
