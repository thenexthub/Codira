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

{% for c in cases %}
/*---
desc: |-
    When an operator applies binary numeric promotion to a pair of operands,
    each of which must denote a value that is convertible to a numeric type,
    the following rules apply, in order:
    1. If any operand is of a reference type, it is subjected to unboxing conversion (link to conv).
    2. Widening primitive conversion (link to conv) is applied to convert either or both operands as specified by the following rules:
      •	If either operand is of type double, the other is converted to double.
      •	Otherwise, if either operand is of type float, the other is converted to float.
      •	Otherwise, if either operand is of type long, the other is converted to long.
      •	Otherwise, both operands are converted to type int.
params: {{c.type}}, cannot promote
tags: [compile-only, negative]
---*/

function main(): void {
  let v: {{c.type}} = {{c.val}};
  let r: {{c.rt}} = {{c.rv}};
  let result: int = v {{c.op}} r ? 0 : 1;
  arktest.assertEQ(result, 0)
}
{% endfor %}
