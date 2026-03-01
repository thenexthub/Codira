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
desc: Multiple interface implementations with different parameters and return values.
---*/

interface ICalculator {
  add(a: number, b: string): string;
  subtract(a: number, b: string): string;
}

interface IStringManipulator {
  reverse(str: string): string;
  capitalize(str: string): string;
}

class Utility implements ICalculator, IStringManipulator {
  add(a: number, b: string): string {
    return a + b;
  }

  subtract(a: number, b: string): string {
    return a + b;
  }

  reverse(str: string): string {
    return str;
  }

  capitalize(str: string): string {
    return str;
  }
}

function main(): void {
  const util = new Utility();
  const result = util.add(3, "str");
  const result1 = util.subtract(4, "aa");
  const result2 = util.reverse('hello');
  const result3 = util.capitalize('world');
  arktest.assertEQ(result, "3str");
  arktest.assertEQ(result1, "4aa");
  arktest.assertEQ(result2, "hello");
  arktest.assertEQ(result3, 'world');
}