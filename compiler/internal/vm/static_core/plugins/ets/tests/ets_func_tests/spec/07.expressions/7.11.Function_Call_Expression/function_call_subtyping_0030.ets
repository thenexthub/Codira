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
desc: Function Call Expression Subtyping
name: function_call_subtyping_0000.ets
spec: 7.11
---*/

interface I {
    foo(): int {return 6}
}

class A implements I {
  countA : int;

  foo() : int {return this.countA + this.countA}
  
  constructor() {
    this.countA = 6
  }
}

class B extends A {
  countB : int;

  foo() : int {return this.countB + this.countB}
    
  constructor() {
    super()
    this.countB = 28
  }
}

class D extends B {
  countD : int;
  foo() : int {return this.countD + this.countD}

  constructor() {
    super()
    this.countD = 486
  }
}

function f(): D {
  return new D();
}

function main() {
  let x : I = f()
  
  arktest.assertEQ(x.foo(), 486 + 486)
}
