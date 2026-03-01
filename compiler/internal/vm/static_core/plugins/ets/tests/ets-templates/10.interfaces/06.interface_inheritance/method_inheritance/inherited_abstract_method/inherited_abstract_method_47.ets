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
desc: Union type methods in interfaces.
---*/

interface ParentInterface {
  parentMethod(value: string | number): string;
}

interface ChildInterface extends ParentInterface {
  childMethod(): string;
}

class ChildClass implements ChildInterface {
  childMethod(): string {
    return "Child method implementation";
  }

  parentMethod(value: string | number): string {
    if (value === "str") {
      return "Parent stringmethod implementation";
    }
    return "Parent numbermethod implementation";
  }
}

function main(): void {
  let instance: ParentInterface = new ChildClass();
  arktest.assertEQ(instance.parentMethod("str"), "Parent stringmethod implementation");
}