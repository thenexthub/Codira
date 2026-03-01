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
    The method with the same name has the same parameter type but different return types,
    redefine the calculated method.
---*/

interface Parent1 {
  calculate(value: number): Object;
}

interface Parent2 extends Parent1 {
  calculate(value: number): number|string;
}

class ChildClass implements Parent2 {
  calculate(value: number): number | string {
    if (value > 10) {
      return value*2
    } else {
      return ("result " + value);
    }
  }
}

function main(): void {
  const child = new ChildClass();
  arktest.assertEQ(child.calculate(5), "result 5");
  arktest.assertEQ(child.calculate(15), 30);
}
