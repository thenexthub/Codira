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
desc: The child class overrides the parent class method and uses the parent class's instance properties.
---*/

class Parent {
  greeting: string = "Hello from Parent";

  greet(): string {
    return this.greeting;
  }
}

class Child extends Parent {
  greet(): string {
    return super.greet() + " and Hello from Child";
  }
}

function main(): void {
  let child = new Child();
  arktest.assertEQ(child.greet(), "Hello from Parent and Hello from Child");
}