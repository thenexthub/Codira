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
desc: Members of the triple inheritance number type.
---*/

class A {
  protected _protectedVar: number;
  public _publicVar: number;

  constructor() {
    this._protectedVar = 2;
    this._publicVar = 3;
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

  test(): void {

  }
}

function main(): void {
  let test: C = new C();
  arktest.assertEQ(test._publicVar, 3);
}