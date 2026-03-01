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
desc: A subclass calls a method of its superclass, and the superclass method depends on the superclass's properties.
---*/

class Parent {
  private name: string;
  constructor(name: string) {
    this.name = name;
  }

  public greet(): string {
    return `Hello, my name is ${this.name}!`;
  }
}

class Child extends Parent {
  constructor(name: string) {
    super(name);
  }

  public callParentGreet(): string {
    return this.greet();
  }
}

function main(): void {
  const child = new Child("Alice");
  arktest.assertEQ(child.callParentGreet(), "Hello, my name is Alice!");
}