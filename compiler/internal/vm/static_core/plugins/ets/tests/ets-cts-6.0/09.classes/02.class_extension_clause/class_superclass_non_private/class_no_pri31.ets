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
    The child class calls the parent class method, and the private method overridden by the child class
    is used in the parent class method.
---*/

class Parent {
  private secret(): string {
    return "Secret from Parent!";
  }

  public reveal(): string {
    return this.secret();
  }
}

class Child extends Parent {
  private secret(): string {
    return "Secret from Child!";
  }

  public callParentReveal(): string {
    return this.reveal();
  }
}

function main(): void {
  const child = new Child();
  arktest.assertEQ(child.callParentReveal(), "Secret from Parent!");
}
