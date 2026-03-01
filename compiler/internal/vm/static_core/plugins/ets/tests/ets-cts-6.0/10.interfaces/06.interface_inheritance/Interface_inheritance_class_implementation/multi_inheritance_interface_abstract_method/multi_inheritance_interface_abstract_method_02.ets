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
desc: Add the number of interface methods and parameters.
---*/

interface Speaker {
  speak(message: string): string;
}

interface Runner {
  run(speed: number): number;
}

class Human implements Speaker, Runner {
  speak(message: string): string {
    return `Human says: ${message}`;
  }
  
  run(speed: number): number {
    return speed;
  }
}

function main(): void {
  const human = new Human();
  const result = human.speak('Hello!');
  const result1 = human.run(10);
  arktest.assertEQ(result, "Human says: Hello!");
  arktest.assertEQ(result1, 10);
}