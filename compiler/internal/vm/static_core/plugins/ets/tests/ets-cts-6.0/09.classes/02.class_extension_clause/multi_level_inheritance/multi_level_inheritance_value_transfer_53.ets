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
    This test scenario verifies that subclasses d and e correctly reinitialize the protected member variable
    _public_var to the number 20 in a multi-level inheritance chain.
---*/

class A {
  protected _publicVar: number;
}

class B extends A {
  constructor() {
    this._publicVar = 2;
  }
}

class C extends B {

}

class D extends C {
  constructor() {
    this._publicVar = 20;
  }
}

class E extends D {
  test() {
    arktest.assertEQ(this._publicVar, 20);
  }
}

function main(): void {
  let test: E = new E;
  test.test();
}