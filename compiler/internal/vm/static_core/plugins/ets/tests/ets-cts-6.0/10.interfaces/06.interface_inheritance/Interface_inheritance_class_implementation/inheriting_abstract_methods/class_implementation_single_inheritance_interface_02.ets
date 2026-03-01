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
desc: To verify that the cat class implements the ianimal interface.
---*/

interface IAnimal {
  speak(): string;
  move(): string;
}

abstract class AbstractMammal implements IAnimal {
  move(): string {
    return "Mammal is walking.";
  }

  abstract speak(): string;
}

class Cat extends AbstractMammal {
  speak(): string {
    return "Cat meows.";
  }
}

function main(): void {
  let cat = new Cat();
  const speakOutput = cat.speak();
  const moveOutput = cat.move();
  arktest.assertEQ(speakOutput, "Cat meows.");
  arktest.assertEQ(moveOutput, "Mammal is walking.");
}