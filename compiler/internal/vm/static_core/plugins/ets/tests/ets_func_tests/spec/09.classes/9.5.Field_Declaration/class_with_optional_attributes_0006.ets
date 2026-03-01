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
 desc: 09.05.Field_Declarations
 name: 09.Classes/09.05.Field_Ddeclarations/class_with_optional_attributes_0006.ets
 tags:
 ---*/

class A {
  static foo(): number { 
    return 5
  }
}

class B extends A {
  foo: number = 10
}

class C {
  foo: number = 10
}

class D extends C {
  static foo(): number { 
    return 5
  }
}

class E {
  static foo(): number { 
    return 5
  }
  foo: number = 10
}

function main() : void {
  arktest.assertEQ(A.foo(), 5)
  arktest.assertEQ(B.foo(), 5)
  arktest.assertEQ(new B().foo, 10)

  arktest.assertEQ(D.foo(), 5)
  arktest.assertEQ(new C().foo, 10)
  arktest.assertEQ(new D().foo, 10)
  
  arktest.assertEQ(E.foo(), 5)
  arktest.assertEQ(new E().foo, 10)
}