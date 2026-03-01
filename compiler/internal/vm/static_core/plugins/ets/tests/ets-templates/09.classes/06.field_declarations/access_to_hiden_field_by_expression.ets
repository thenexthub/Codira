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
desc: Access to a hidden field by access expression with 'super' keyword.
assert: >-
    A hidden field can be accessed by using a qualified name if it is static, or by using a field access expression
    that contains the keyword super or a cast to a superclass type.
---*/

class A {
  private _x: int;

  public get x(): int {
    return this._x;
  }
}

class B extends A {

  public getX(): int {
    return super.x;
  }
}

function main(): void {
    let instance: B = new B();

    arktest.assertTrue(instance.getX() instanceof int);
    arktest.assertEQ(instance.getX(), 0);
    arktest.assertTrue(instance.x instanceof int);
    arktest.assertEQ(instance.x, 0);
}
