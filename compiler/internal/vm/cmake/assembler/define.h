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

#ifndef ASSEMBLER_DEFINE_H
#define ASSEMBLER_DEFINE_H

/* Implementation-specific defines */

constexpr char PARSE_COMMENT_MARKER = '#';

constexpr char PARSE_AREA_MARKER = '.';

#define PANDA_ASSEMBLER_TYPES(_) \
    _("void", VOID)              \
    _("u1", U1)                  \
    _("u8", U8)                  \
    _("i8", I8)                  \
    _("u16", U16)                \
    _("i16", I16)                \
    _("u32", U32)                \
    _("i32", I32)                \
    _("u64", U64)                \
    _("i64", I64)                \
    _("f32", F32)                \
    _("f64", F64)                \
    _("any", TAGGED)

#define KEYWORDS_LIST(_)     \
    _(".catch", CATCH)       \
    _(".catchall", CATCHALL) \
    _(".language", LANG)     \
    _(".function", FUN)      \
    _(".record", REC)        \
    _(".array", ARR)         \
    _(".field", FLD)

#endif  // ASSEMBLER_DEFINE_H
