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
    Multilevel inheritance, child interfaces inherit abstract methods from multiple parent interfaces
    (verify that the child interface inherits the abstract method2 of the first parent interface2).
---*/

interface ParentInterface1 {
  parentMethod1(): string;
}

interface ParentInterface2 {
  parentMethod2(): string;
}

interface ChildInterface extends ParentInterface1, ParentInterface2 {
  childMethod(): string;
}

class ChildClass implements ChildInterface {
  parentMethod1(): string {
    return "Parent method1 implementation";
  }

  parentMethod2(): string {
    return "Parent method2 implementation";
  }

  childMethod(): string {
    return "Child method implementation";
  }
}

function main(): void {
  let instance: ParentInterface2 = new ChildClass();
  arktest.assertEQ(instance.parentMethod1(), "Parent method1 implementation");
}