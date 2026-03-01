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
desc: To verify that the parent class member variable (type Interface object array) is correctly initialized.
---*/

interface SomeInterface {
  name: string;
}

const obj1: SomeInterface = { name: "Object1" };
const obj2: SomeInterface = { name: "Object2" };

class Parent {
  public value: SomeInterface[];

  constructor(value: SomeInterface[]) {
    this.value = value;
  }
}

class Child extends Parent {
  constructor(value: SomeInterface[]) {
    super(value);
  }
}

function main(): void {
  const interface1: SomeInterface[] = [obj1, obj2];
  const myChild = new Child(interface1);
  arktest.assertEQ(myChild.value[0].name, "Object1");
  arktest.assertEQ(myChild.value[1].name, "Object2");
}