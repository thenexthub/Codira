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
desc: A subclass calls a method of its superclass, and the superclass method returns an object.
---*/

class KeyValuePair {
  key: string = '';
  value: number = 0;
  constructor(key: string, value: number) {
    this.value = value;
    this.key = key;
  }
}

class Parent {
  public createObject(): KeyValuePair {
    return new KeyValuePair("example", 42);
  }
}

class Child extends Parent {
  public callParentCreateObject(): KeyValuePair {
    return this.createObject();
  }
}

function main(): void {
  const child = new Child();
  const result = child.callParentCreateObject();
  arktest.assertEQ(result.key, "example");
  arktest.assertEQ(result.value, 42);
}
