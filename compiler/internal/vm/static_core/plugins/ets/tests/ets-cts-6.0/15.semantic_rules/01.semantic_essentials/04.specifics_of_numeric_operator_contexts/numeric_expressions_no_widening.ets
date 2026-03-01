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
    The postfix and prefix increment and decrement operators evaluate byte and short operands without
    widening. Same is true also for an assignment operator (considering assignment as a binary operator).
---*/

function main() {
{%- for t in type %}
    let y_{{t}}: {{t}} = 1

{%- for op in unaryop %}
    let x_{{t}}_prefix_{{loop.index}} = {{op}}y_{{t}}
    arktest.assertTrue(x_{{t}}_prefix_{{loop.index}} instanceof {{t}})
    let x_{{t}}_postfix_{{loop.index}} = y_{{t}}{{op}}
    arktest.assertTrue(x_{{t}}_postfix_{{loop.index}} instanceof {{t}})
{%- endfor %}
    let x_{{t}}_assignment = y_{{t}}
    arktest.assertTrue(x_{{t}}_assignment instanceof {{t}})
{%- endfor %}
}