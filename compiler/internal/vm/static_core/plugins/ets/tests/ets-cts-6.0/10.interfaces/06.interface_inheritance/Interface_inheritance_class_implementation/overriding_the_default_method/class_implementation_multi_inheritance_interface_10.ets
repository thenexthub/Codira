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
      The parent class overrides the default methods of the part inherited by
      the subinterface implemented by the subinterface of the child class.
---*/

interface Parent{
  speak(): number[] {
    return [1, 2, 3];
  }

  move(): number[] {
    return [4, 5, 6];
  }
}

interface Child extends Parent{}

class Vehicle implements Child {}

class Car extends Vehicle {
  speak(): number[] {
    return [4, 5, 6];
  }
}

function main(): void {
  const car = new Car();
  arktest.assertEQ(car.speak()[1], 5);
  arktest.assertEQ(car.move()[1], 5);
}
