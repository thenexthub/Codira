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
{%- for v in  vals %}
/*---
desc: >-
    ArkTS provides a number of operators to act on character values:
    Character operators that produce a value of type char.
---*/

function minus(p: Char): Char {
    // Widening to int happens
    return (-p).toChar()
}

function plus(p: Char): Char {
    // Widening to int happens
    return (+p).toChar()
}

function main() {
    const v1 = new Char({{v.v1}})

    arktest.assertEQ(({{v.exp}}), {{v.r}})
}
{%- endfor %}
