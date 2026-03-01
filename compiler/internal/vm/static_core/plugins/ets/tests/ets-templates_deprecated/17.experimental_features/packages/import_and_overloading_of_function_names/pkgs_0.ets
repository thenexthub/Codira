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
{%- for c in cases %}

/*---
desc: Different imported functions have the same name and similar parameters types, like int and Int, char and Char.
tags: [{{c.tags}}]
---*/

{{c.decl}}

function foo({{c.sig1}}): int {
    return 1
}
function foo({{c.sig2}}): int {
    return 2
}

function main(): void {
    let v1 = foo({{c.arg1}})
    let v2 = foo({{c.arg2}})
    arktest.assertEQ(v1, 1)
    arktest.assertEQ(v2, 2)
}
{%- endfor %}
