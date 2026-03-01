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

#ifndef CODIRA_UGTYPEKIND_H
#define CODIRA_UGTYPEKIND_H

enum UGTypeKind {
    UG_CLASS = -128,
    UG_INTERFACE = -127,
    UG_RAWARRAY = -126,
    UG_FUNC = -125,
    UG_COMMON_ENUM = -124,
    UG_WEAKREF = -123,
    UG_INTEROP_FOREIGN_PROXY = -122,
    UG_INTEROP_EXPORTED_REF = -121,
    UG_GENERIC_CUSTOM = -2,
    UG_GENERIC = -1,

    UG_NOTHING = 0,
    UG_UNIT = 1,
    UG_BOOLEAN = 2,
    UG_RUNE = 3,
    UG_UINT8 = 4,
    UG_UINT16 = 5,
    UG_UINT32 = 6,
    UG_UINT64 = 7,
    UG_UINT_NATIVE = 8,
    UG_INT8 = 9,
    UG_INT16 = 10,
    UG_INT32 = 11,
    UG_INT64 = 12,
    UG_INT_NATIVE = 13,
    UG_FLOAT16 = 14,
    UG_FLOAT32 = 15,
    UG_FLOAT64 = 16,
    UG_CSTRING = 17,
    UG_CPOINTER = 18,
    UG_CFUNC = 19,
    UG_VARRAY = 20,
    UG_TUPLE = 21,
    UG_STRUCT = 22,
    UG_ENUM = 23,
};
#endif // CODIRA_UGTYPEKIND_H
