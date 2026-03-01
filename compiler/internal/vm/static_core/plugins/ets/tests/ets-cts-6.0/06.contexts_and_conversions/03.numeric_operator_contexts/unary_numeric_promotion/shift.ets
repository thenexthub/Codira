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
    Each operand, separately, of a shift operator.
params: {{c.type}}
---*/

enum IntEnum {One = 1, Two = 0xFFFFFFFF, Three}

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
  {%- for r in rside %}
  let r{{loop.index}}: {{r.type}} = {{r.val}};
  arktest.assertEQ(foo(v << r{{loop.index}}), {{c.res}})
  arktest.assertEQ(foo(v >> r{{loop.index}}), {{c.res}})
  arktest.assertEQ(foo(v >>> r{{loop.index}}), {{c.res}})
  {% endfor %}
}
{% endfor %}
