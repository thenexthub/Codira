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
desc: Default method for child interface to override parent interface (the rewritten result should be output).
---*/

interface ParentInterface {
  parentMethod(): string {
    return "Parent method implementation";
  }
}

interface ChildInterface extends ParentInterface {
  parentMethod(): string;
  childMethod(): string;
}

class ChildClass implements ChildInterface {
  childMethod(): string {
    return "Child method implementation";
  }

  parentMethod(): string {
    return "Overridden implementation of parentMethod in ChildInterface";
  }
}

function main(): void {
  let instance: ChildInterface = new ChildClass();
  arktest.assertEQ(instance.parentMethod(), "Overridden implementation of parentMethod in ChildInterface");
  arktest.assertEQ(instance.childMethod(), "Child method implementation");
}
