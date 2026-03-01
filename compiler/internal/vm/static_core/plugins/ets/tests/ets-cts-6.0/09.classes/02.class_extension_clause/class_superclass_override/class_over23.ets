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
desc: Subclasses override the parent method and use the parent's generic method.
---*/

class Parent {
  greet<T>(message: T): string {
    return `Hello from Parent with message: ${message}`;
  }
}

class Child extends Parent {
  override greet<T>(message: T): string {
    return super.greet(message) + " and Hello from Child";
  }
}

function main(): void {
  const child = new Child();
  console.log(child.greet("Default Message"));
  arktest.assertEQ(
    child.greet("Test Message"),
    "Hello from Parent with message: Test Message and Hello from Child"
  );
}