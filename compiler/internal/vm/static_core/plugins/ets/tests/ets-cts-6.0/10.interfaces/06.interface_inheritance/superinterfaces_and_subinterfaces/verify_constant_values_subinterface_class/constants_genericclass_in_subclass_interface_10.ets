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
desc: Verify whether the constant values generic class<string[]> remain consistent in the subclass interface.
---*/

class GenericClass<T> {
  public value: T;

  constructor(value: T) {
    this.value = value;
  }
}

const temp_value: string[] = ["a", "b", "c"];

interface ParentInterface {
  readonly CONSTANT_VALUE: GenericClass<string[]>;
}

interface SubInterface extends ParentInterface {}

class MyClass implements SubInterface {
  readonly CONSTANT_VALUE: GenericClass<string[]> = new GenericClass<string[]>(temp_value);
}

class SubClass extends MyClass {}

function main(): void {
  const sub = new SubClass();
  arktest.assertEQ(sub.CONSTANT_VALUE.value[0], "a");
}
