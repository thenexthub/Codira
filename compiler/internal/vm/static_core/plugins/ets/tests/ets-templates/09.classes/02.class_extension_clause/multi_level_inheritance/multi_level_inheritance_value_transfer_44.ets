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
    Whether the public member variable _public_var passed from class a to class f through a multi-level inheritance
    chain is correctly initialized to null.
---*/

class A {
  public _publicVar: null;

  constructor() {
    this._publicVar = null;
  }

}

class B extends A {

}

class C extends B {

}

class D extends C {

}

class E extends D {

}

class F extends E {
  test() {
    arktest.assertEQ(this._publicVar, null);
  }
}

function main(): void {
  let test:F = new F;
  test.test();
}