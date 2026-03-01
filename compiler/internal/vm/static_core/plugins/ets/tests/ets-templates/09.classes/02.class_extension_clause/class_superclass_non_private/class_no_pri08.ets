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
    The child class calls the method of the parent class, and the new method of the child class is used
    in the parent class method.
---*/

class Parent {
  public greet(): string {
    return "Hello from Parent!";
  }

  public farewell(): string {
    return this.greet() + " Goodbye from Parent!";
  }
}

class Child extends Parent {
  public customGreet(): string {
    return "Custom Hello from Child!";
  }

  public greet(): string {
    return this.customGreet();
  }

  public callParentFarewell(): string {
    return this.farewell();
  }
}

function main(): void {
  const child = new Child();
  arktest.assertEQ(child.callParentFarewell(), "Custom Hello from Child! Goodbye from Parent!");
}
