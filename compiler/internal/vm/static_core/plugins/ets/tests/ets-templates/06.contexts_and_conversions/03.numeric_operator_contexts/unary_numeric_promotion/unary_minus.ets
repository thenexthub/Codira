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
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    The operand of a unary minus operator.
params: {{c.type}}
---*/

enum IntEnum {One = 1, Two = 0x7FFFFFFF, Three}

function foo(a: byte): int {
  return 2;
}

function foo(a: char): int {
  return 3;
}

function foo(a: short): int {
  return 4;
}

function foo(a: int): int {
  return 5;
}

function foo(a: long): int {
  return 6;
}

function foo(a: float): int {
  return 7;
}

function foo(a: double): int {
  return 8;
}

function foo(a: Object): int {
  return 1;
}

function main(): void {
  let v: {{c.type}} = {{c.val}};
  let r0 = foo(-v);
  let r1 = foo(- -v);
  let r2 = foo(- - -v);
  let r3 = -v;
  let r4 = - - v;
  let r5 = - - - v;
  arktest.assertEQ(r0, {{c.r[0]}});
  arktest.assertEQ(r1, {{c.r[1]}});
  arktest.assertEQ(r2, {{c.r[2]}});
  arktest.assertEQ(r3, {{c.r[3]}});
  arktest.assertEQ(r4, {{c.r[4]}});
  arktest.assertEQ(r5, {{c.r[5]}});
}
{% endfor %}
