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
    To verify that class c indirectly inherits class a from class b and can correctly obtain the initial value
    of the public variable _publicVar defined by class a.
---*/

class A {
  public _publicVar: boolean;
  constructor() {
    this._publicVar = true;
  }
}

class B extends A {
  constructor() {
    super();
  }
}

class C extends B {
  constructor() {
    super();
  }

  test() {
  }
}

function main(): void {
  let test: C = new C();
  arktest.assertEQ(test._publicVar, true);
}