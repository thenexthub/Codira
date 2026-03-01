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
    Character operators are identical to integer operators, so produce a value of type int.
---*/

function add(p: char, q: char): char {
    return (p + q).toChar()
}

function main() {
    const v1 = {{v.v1}}
    const v2 = {{v.v2}}

    arktest.assertEQ(({{v.exp}}), {{v.r}})
}
{%- endfor %}
