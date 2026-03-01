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

class A {
  foo1(v: String): String { return ('A:' + v) }
  foo2(v1: String, v2: String, v3: String): String { return ('A:' + v1 + '_' + v2 + '_' + v3) }
  foo3(v1: int, v2: int, v3: int, v4:int, v5:int): String { return 'A:' + (v1 + v2 + v3 + v4 + v5) }
}

class B {
  foo1(v: String): String { return ('B:' + v) }
  foo2(v1: String, v2: String, v3: String): String { return ('B:' + v1 + '_' + v2 + '_' + v3) }
  foo3(v1: int, v2: int, v3: int, v4:int, v5:int): String { return 'B:' + (v1 + v2 + v3 + v4 + v5) }
}

function test1(v: A | B): String {
  return v.foo1('abc')
}

function test2(v: A | B): String {
  return v.foo2('a', 'b', 'c')
}

function test3(v: A | B): String {
  return v.foo3(1, 2, 3, 4, 5)
}

function main() {
  let a = new A()
  let b = new B()
  arktest.assertEQ(test1(a), 'A:abc');
  arktest.assertEQ(test1(b), 'B:abc');
  arktest.assertEQ(test2(a), 'A:a_b_c');
  arktest.assertEQ(test2(b), 'B:a_b_c');
  arktest.assertEQ(test3(a), 'A:15');
  arktest.assertEQ(test3(b), 'B:15');
}
