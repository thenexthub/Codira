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
      The parent class and the method with the same name in the interface,
      the child class chooses to call the parent class implementation.
---*/

let result: int = 0;

abstract class Animal {
  makeSound(): void {
    result += 1
  }
}

interface Machine {
  makeSound(): void {
    result += 2
  }
}

class Robot extends Animal implements Machine {
  makeSound(): void {
    super.makeSound(); 
    result += 3;
  }
}

function main(): void {
  result = 0
  const robot = new Robot();
  robot.makeSound();
  arktest.assertEQ(result, 4);
}
