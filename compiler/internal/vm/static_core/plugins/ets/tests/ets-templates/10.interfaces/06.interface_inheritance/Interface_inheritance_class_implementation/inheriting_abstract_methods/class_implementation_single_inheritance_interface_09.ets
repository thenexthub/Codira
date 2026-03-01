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
desc: The subclass can correctly inherit and implement the parent class methods.
---*/

abstract class IAnimal {
  abstract speak(): string;

  move(): string {
    return "moveing.";
  }
}

class Dog extends IAnimal {
  speak(): string {
    return "speaking.";
  }
}

function main(): void {
  let dog = new Dog();
  let result = dog.speak();
  let result1 = dog.move();
  arktest.assertEQ(result, "speaking.");
  arktest.assertEQ(result1, "moveing.");
}