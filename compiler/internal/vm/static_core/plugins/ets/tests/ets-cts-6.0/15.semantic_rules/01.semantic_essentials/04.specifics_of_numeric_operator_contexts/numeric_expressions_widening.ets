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

/*---
desc: >-
    Operands of unary and binary numeric expressions are widened to a larger numeric type.
    The minimum type is int.
---*/

function main() {
    let b: byte = 1
    let s: Short = 2
    let i: int = 3
    let l: Long = 4

{%- for c in binary %}
    let x1_{{loop.index}} = b {{c}} b
    arktest.assertTrue(x1_{{loop.index}} instanceof int)
    let x2_{{loop.index}} = b {{c}} s
    arktest.assertTrue(x2_{{loop.index}} instanceof int)
    let x3_{{loop.index}} = s {{c}} s
    arktest.assertTrue(x3_{{loop.index}} instanceof int)
    let x4_{{loop.index}} = b {{c}} i
    arktest.assertTrue(x4_{{loop.index}} instanceof int)
    let x5_{{loop.index}} = i {{c}} l
    arktest.assertTrue(x5_{{loop.index}} instanceof long)
{%- endfor -%}

{%- for c in unary_prefix %}
    let y1_{{loop.index}} = {{c}}b
    arktest.assertTrue(y1_{{loop.index}} instanceof int)
    let y2_{{loop.index}} = {{c}}s
    arktest.assertTrue(y2_{{loop.index}} instanceof int)
    let y3_{{loop.index}} = {{c}}i
    arktest.assertTrue(y3_{{loop.index}} instanceof int)
    let y4_{{loop.index}} = {{c}}l
    arktest.assertTrue(y4_{{loop.index}} instanceof long)
{%- endfor -%}
}