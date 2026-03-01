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
desc: Test an array with a union type of different TypedArrays and number.
---*/

interface GenericInterface<T> {
  process(data: T): T;
}

class GenericImplementation<T> implements GenericInterface<T> {
  process(data: T): T {
    return data;
  }
}

function main(): void {
  const int8Array = new Int8Array([1, 2, 3]);
  const uint16Array = new Uint16Array([4, 5, 6]);
  let arrImplementation: GenericInterface<(Int8Array | Uint16Array | number)[]> = new GenericImplementation<(Int8Array | Uint16Array | number)[]>();
  let inputArray: (Int8Array | Uint16Array | number)[] = [int8Array, 10, uint16Array, 20];
  let result = arrImplementation.process(inputArray);
  arktest.assertEQ(result, inputArray);
}