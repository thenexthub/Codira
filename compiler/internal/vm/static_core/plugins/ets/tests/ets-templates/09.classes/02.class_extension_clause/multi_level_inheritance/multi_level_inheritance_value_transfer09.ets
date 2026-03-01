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
    Verify that the instance of class b inherits from class a can correctly obtain the initial string value
    of the public variable _publicVar in class a.
---*/

class A {
  public _publicVar: string;

  constructor() {
    this._publicVar = 'Multi-level inheritance';
  }
}

class B extends A {
  constructor() {
    super();
  }

  test() {
  }
}

function main(): void {
  let test: B = new B();
  arktest.assertEQ(test._publicVar, 'Multi-level inheritance');
}