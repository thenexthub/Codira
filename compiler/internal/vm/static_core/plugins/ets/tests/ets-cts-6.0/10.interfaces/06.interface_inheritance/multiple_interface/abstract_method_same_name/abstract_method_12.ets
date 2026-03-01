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
    The method with the same name has the different parameters parameter type
    but same return types, valid overloading.
---*/

interface Parent1 {
  combine(a: string, b: number): string;
}

interface Parent2 {
  combine(b: number, a: string): string;
}

// Parent1 and Parent2 combine methods with different parameter orders
interface Child extends Parent1, Parent2 {}

class ChildClass implements Child {
  combine(...p: Any[]): Any {
    if (typeof p[0] === "string")
      return "str1"
    return "str2"
  }
}

function main() {
  let c: ChildClass = new ChildClass()
  arktest.assertEQ(c.combine("str", 1),"str1")
  arktest.assertEQ(c.combine(2, "str"),"str2")
}
