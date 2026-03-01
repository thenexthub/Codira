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
desc: To verify that the ostrich class implements the ianimal interface.
---*/

interface IAnimal {
  speak(): string;
  move(): string;
}

abstract class AbstractBird implements IAnimal {
  abstract move(): string;
  abstract speak(): string;
}

class Ostrich extends AbstractBird {
  speak(): string {
    return "Ostrich booms.";
  }

  move(): string {
    return "Bird is flying.";
  }
}

function main(): void {
  let ostrich = new Ostrich();
  const speakOutput = ostrich.speak();
  const moveOutput = ostrich.move();
  arktest.assertEQ(speakOutput, "Ostrich booms.");
  arktest.assertEQ(moveOutput, "Bird is flying.");
}