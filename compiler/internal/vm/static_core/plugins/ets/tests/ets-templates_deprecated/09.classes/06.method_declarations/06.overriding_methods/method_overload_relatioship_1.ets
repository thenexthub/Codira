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
desc: Overload two methods with different throws clauses.
assert: There is no required relationship between the return types or between the throws or rethrows clauses of two methods with the same name, unless their signatures are override-equivalent.
---*/

class A {
  public foo(): int throws {
    return 1;
  }
 
  public foo(a: int): boolean {
    return true;
  }
}

function main(): void {
  let instance: A = new A();
  let a = 0;

  try {
      a = instance.foo();
  } catch (e) {
      a = 2;
  }

  arktest.assertEQ( a, 1 );
  arktest.assertEQ( instance.foo(2), true );
}
