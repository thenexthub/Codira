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
desc: 09.1 - 09.4 Class Definition
tags: []
---*/

abstract class A {
  s: string = ""
}

abstract class B1 extends A {
  n: int
}

abstract class B2 extends A {
  n: long
}

abstract class C1 extends B1 {
  public getString(): string {
    return this.s;
  }

  public getNumber(): int {
    return this.n;
  }
}

abstract class C2 extends B2 {
  public getString(): string {
    return this.s;
  }

  public getNumber(): long {
    return this.n;
  }
}

class C11 extends C1 {
  constructor(s: string, n: int ) {
    this.s = s;
    this.n = n;
  }
}

class C21 extends C2 {
  constructor(s: string, n: long) {
    this.s = s;
    this.n = n;
  }
}

function main() {
  let c11: C11 = new C11("c11", 100);
  let c21: C21 = new C21("c21", 200);

  arktest.assertEQ( c11.getString(), "c11")
  arktest.assertEQ( c21.getString(), "c21")
  arktest.assertEQ( c11.getNumber(), 100)
  arktest.assertEQ( c21.getNumber(), 200)
}
