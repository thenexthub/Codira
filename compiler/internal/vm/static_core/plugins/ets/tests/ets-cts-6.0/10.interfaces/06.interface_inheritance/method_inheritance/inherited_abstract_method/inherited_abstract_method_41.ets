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
desc: Nested inheritance of interfaces (parentinterface1 call parentmethod1).
---*/

interface ParentInterface1 {
  parentMethod1(): string;
}

interface ParentInterface2 extends ParentInterface1 {
  parentMethod2(): string;
}

interface ChildInterface extends ParentInterface2 {
  childMethod(): string;
}

class ChildClass implements ChildInterface {
  childMethod(): string {
    return "Child method implementation";
  }

  parentMethod1(): string {
    return "Parent method1 implementation";
  }

  parentMethod2(): string {
    return "Parent method2 implementation";
  }
}

function main(): void {
  let instance: ParentInterface1 = new ChildClass();
  arktest.assertEQ(instance.parentMethod1(), "Parent method1 implementation");
}
