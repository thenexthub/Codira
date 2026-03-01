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
desc: Override method declaration.
assert: >-
    A compile-time error occurs if a method marked with the modifier override
    does not override a method from a superclass
tags: [compile-only, negative]
---*/

class A {
}

final class B extends A {
  override foo/* @ label*/(): int {
    return 5;
  }
}

function main(): void {
  let instance: B = new B();

  arktest.assertEQ( instance.foo(), 5 );
}

/* @@@ label Error TypeError: Method foo(): Int in B not overriding any method */
