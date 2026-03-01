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
desc: The method with the same name has different parameter types and quantities, but the return type is the same.
---*/

let res: int = 0

interface Parent1 {
  log(message: string): void;
}

interface Parent2 {
  log(message: string, level: number): void;
}

interface Child extends Parent1, Parent2 {}

class ChildClass implements Child {
  log(...p: Any[]): Any {
    if (p.length == 1)
      res = 1
    else
      res = 2
  }
}

function main() {
  let c: ChildClass = new ChildClass()
  c.log("str")
  arkts.assertEQ(res, 1)
  c.log("str", 2)
  arkts.assertEQ(res, 2)
}
