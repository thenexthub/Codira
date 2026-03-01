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
desc: Methods with the same name have different return types，compatible with two return types.
---*/

interface Parent1 {
  getStringValue(): string; 
}

interface Parent2 {
  getNumberValue(): number; 
}

interface Child extends Parent1, Parent2 {
  getValue(): string | number; 
}

class ChildClass implements Child {
  getStringValue(): string {
    return "Hello";
  }

  getNumberValue(): number {
    return 42;
  }

  getValue(): string | number {
    return Math.random() > 0.5 ? this.getStringValue() : this.getNumberValue();
  }
}

function main(): void {
  const child = new ChildClass();
  const value = child.getValue();

  if (typeof value === "string") {
    arktest.assertEQ(value, "Hello");
  } else {
    arktest.assertEQ(value, 42);
  }
}