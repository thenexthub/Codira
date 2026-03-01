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
    The child class calls the parent class method, and the asynchronous method overridden by the child class
    is used in the parent class method.
---*/

class Parent {
  public async fetchMessage(): Promise<string> {
    return "Hello from Parent!";
  }

  public async greet(): Promise<string> {
    return await this.fetchMessage();
  }
}

class Child extends Parent {
  public async fetchMessage(): Promise<string> {
    return "Hello from Child!";
  }

  public async callParentGreet(): Promise<string> {
    return await this.greet();
  }
}

function main(): void {
  (async () => {
    const child = new Child();
    arktest.assertEQ(child.callParentGreet(), "Hello from Child!");
  })();
}
