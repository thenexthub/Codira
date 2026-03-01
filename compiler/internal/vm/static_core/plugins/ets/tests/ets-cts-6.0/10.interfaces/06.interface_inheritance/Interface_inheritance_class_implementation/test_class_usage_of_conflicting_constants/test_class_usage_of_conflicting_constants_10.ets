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
    Verify that a class can distinguish the same-named constants
    between an interface constant and a class method parameter by local variable.
---*/

interface MyInterface {
  readonly CONSTANT: number;
}

class MyClass implements MyInterface {
  readonly CONSTANT: number = 10;

  someMethod(CONSTANT: number) {
    const localConstant = this.CONSTANT;
    console.log(`Interface/Class constant: ${localConstant}`);
    console.log(`Method parameter constant: ${CONSTANT}`);
    return localConstant !== CONSTANT;
  }
}

function assertEQ(condition: boolean) {
  if (!condition) {
    throw new Error('Assertion failed');
  }
}

function main(): void {
  const instance = new MyClass();
  const result = instance.someMethod(20);
  arktest.assertTrue(result);
}
