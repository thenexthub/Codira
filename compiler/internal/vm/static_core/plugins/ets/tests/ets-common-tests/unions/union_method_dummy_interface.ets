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

interface I1 {}
interface I2 {}
interface I3 extends I1, I2 {}

class Dummy {
  m(p: I3): String { return ('Dummy') }
}

class A1 {
  m(p: I3): String { return ('A1') }
}

class A2 extends A1 {
  m(p: I1): String { return ('A2') }
}

class A3 extends A2 {
  m(p: I2): String { return ('A3') }
}

function test(c: A1 | Dummy, a: I3): String { return c.m(a) }

function main() {
  arktest.assertEQ(test(new A1(), {}), 'A1');
  arktest.assertEQ(test(new A2(), {}), 'A2');
  arktest.assertEQ(test(new A3(), {}), 'A2');
}
