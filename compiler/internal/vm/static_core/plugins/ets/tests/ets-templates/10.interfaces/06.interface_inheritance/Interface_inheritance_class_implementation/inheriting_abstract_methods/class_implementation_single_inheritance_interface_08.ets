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

interface IAnimal {
  speak(): string;
  fly(): string;
}

abstract class AbstractFlyingBird implements IAnimal {
  abstract speak(): string;
    
  fly(): string {
    return "Bird is flying.";
  }
}

class Eagle extends AbstractFlyingBird {
  speak(): string {
    return "Eagle screeches.";
  }
}

function main(): void {
  let eagle = new Eagle();
  let result = eagle.speak();
  arktest.assertEQ(result, "Eagle screeches.");
}