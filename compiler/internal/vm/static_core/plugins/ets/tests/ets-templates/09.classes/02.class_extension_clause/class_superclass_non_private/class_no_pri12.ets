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
desc: >-
    The child class calls the parent class method, and the chained method overridden by the child class
    is used in the parent class method.
---*/

class Parent {
  public step1(): string {
    return "Step 1 from Parent";
  }

  public step2(): string {
    return this.step1() + " -> Step 2 from Parent";
  }
}

class Child extends Parent {
  public step1(): string {
    return "Step 1 from Child";
  }

  public callParentChain(): string {
    return this.step2();
  }
}

function main(): void {
  const child = new Child();
  arktest.assertEQ(child.callParentChain(), "Step 1 from Child -> Step 2 from Parent");
}
