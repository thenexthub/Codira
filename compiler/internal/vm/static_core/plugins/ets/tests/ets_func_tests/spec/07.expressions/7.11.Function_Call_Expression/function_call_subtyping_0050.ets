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




class A<T>  {
    countA : T;

    foo() : T {
    return this.countA
    }
  
    constructor(p: T) {
    this.countA = p
  }
}

class B<T> extends A<T> {
  countB : T;

    foo() : T {
    return this.countB
    }
    
    constructor(p: T) {
    super(p)
    this.countB = p
    }
}


function f(): B<Int> {
  return new B<Int>(6);
}

function main() {
  let x : A<Int> = f()
  
  arktest.assertEQ(x.foo(),  6)
}

