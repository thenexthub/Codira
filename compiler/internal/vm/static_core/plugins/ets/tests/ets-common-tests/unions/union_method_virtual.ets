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
  meth(p: number): number { return p + 11.0 }
}

class B {
  meth(p: number): number { return p + 22.0 }
}

class C extends A {
  meth(p: number): number { return p + 33.0 }
}

class D extends B {
  meth(p: number): number { return p + 44.0 }
}

class E extends C {
}

function test(v: A | B): number {
  return v.meth(10.0)
}

function main() {
  arktest.assertEQ(test(new A()), 21.0);
  arktest.assertEQ(test(new B()), 32.0);
  arktest.assertEQ(test(new C()), 43.0);
  arktest.assertEQ(test(new D()), 54.0);
  arktest.assertEQ(test(new E()), 43.0);
}
