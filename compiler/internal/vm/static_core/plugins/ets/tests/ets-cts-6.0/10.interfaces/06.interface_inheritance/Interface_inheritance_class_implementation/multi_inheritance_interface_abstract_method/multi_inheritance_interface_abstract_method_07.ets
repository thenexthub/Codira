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
desc: Basic multi-interface multi-method implementation.
---*/

interface IAnimal {
  eat(): string;
  sleep(): string;
}

interface IFlyable {
  fly(): string;
  land(): string;
}

class Bird implements IAnimal, IFlyable {
  eat() {
    return 'Bird is eating.';
  }

  sleep() {
    return 'Bird is sleeping.';
  }

  fly() {
    return 'Bird is flying.';
  }

  land() {
    return 'Bird has landed.';
  }
}

function main(): void {
  const bird = new Bird();
  const result = bird.eat();
  const result1 = bird.sleep();
  const result2 = bird.fly();
  const result3 = bird.land();
  arktest.assertEQ(result, 'Bird is eating.');
  arktest.assertEQ(result1, 'Bird is sleeping.');
  arktest.assertEQ(result2, 'Bird is flying.');
  arktest.assertEQ(result3, 'Bird has landed.');
}