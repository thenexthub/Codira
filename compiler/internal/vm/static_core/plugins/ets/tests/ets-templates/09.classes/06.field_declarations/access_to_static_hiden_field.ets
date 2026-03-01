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
desc: Access to a static hidden field.
assert: >-
    A hidden field can be accessed by using a qualified name if it is static, or by using a field access expression
    that contains the keyword super or a cast to a superclass type.
---*/

class A {
  public static x: int;
}

class B extends A {
  public x: boolean;

  public getX(): int {
    return A.x;
  }
}

function main(): void {
    let instance: B = new B();

    arktest.assertEQ( instance.getX(), 0 );
}
