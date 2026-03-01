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
params: {{c.type}}
---*/

enum IntEnum {One = 1, Two, Three}

function foo(p: byte): int {
  return 1;
}

function foo(p: short): int {
  return 2;
}

function foo(p: int): int {
  return 3;
}

function foo(p: long): int {
  return 4;
}

function foo(p: char): int {
  return 5;
}

function foo(p: boolean): int {
  return 6;
}

function foo(p: float): int {
  return 7;
}

function foo(p: double): int {
  return 8;
}

function foo(p: Object): int {
  return 40;
}

function main(): void {
  let v: {{c.type}} = {{c.val}};
  let r: {{c.rt}} = {{c.rv}};
  let result: int = foo(v {{c.op}} r);
  arktest.assertEQ(result, {{c.res}})
}
{% endfor %}
