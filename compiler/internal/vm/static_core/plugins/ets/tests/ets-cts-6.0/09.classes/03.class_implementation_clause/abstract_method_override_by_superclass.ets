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
desc: Class overrides abstract methods from superinterface by inheritance them from superclass.
assert: >-
    Unless the class being declared is abstract, all the abstract member methods of each direct superinterface
    must be implemented either by a declaration in this class or by an existing method declaration inherited
    from the direct superclass or a direct superinterface, because a class that is not abstract is not permitted
    to have abstract methods.
---*/

class A {
  final foo(): int {
    return 5;
  }

  final bar(): int {
    return 10;
  }
}

final class D extends A implements B, C {
}

interface B {
  bar(): int;
}

interface C {
  foo(): int;
}

function main(): void {
  let c: D = new D();
  let b: D = new D();

  arktest.assertEQ(c.foo(), 5);
  arktest.assertEQ(b.bar(), 10);
}
