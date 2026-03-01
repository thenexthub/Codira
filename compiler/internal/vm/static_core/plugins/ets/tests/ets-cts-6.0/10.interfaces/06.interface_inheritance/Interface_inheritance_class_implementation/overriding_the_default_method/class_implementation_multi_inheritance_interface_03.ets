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
      Abstract class overrides a default method inherited by the subinterface,
      The default method returns the data type string.
---*/

interface Parent{
  speak(): string {
    return "speak";
  }
}

interface Child extends Parent{}

abstract class AbstractAnimal implements Child {
  speak(): string {
    return "speaking";
  }
}

class Animal extends AbstractAnimal {}

function main(): void {
  const animal = new Animal();
  arktest.assertEQ(animal.speak(), "speaking");
}
