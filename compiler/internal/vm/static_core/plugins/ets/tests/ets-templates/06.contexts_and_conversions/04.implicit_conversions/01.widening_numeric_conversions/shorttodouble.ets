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
desc: Casting conversions short to double.
---*/

function isShort(value: number): boolean {
  return value >= -32768 && value <= 32767;
}

function shortToDouble(shortValue: number): number {
  if (!isShort(shortValue)) {
    throw new Error("no short type");
  }
  return shortValue;
}

function main(): void {
  let shortValue: number = 12345;
  let doubleValue: number = shortToDouble(shortValue);
  console.log(`raw value (short): ${shortValue}`);
  console.log(`Converted value (double): ${doubleValue}`);
  arktest.assertEQ(typeof doubleValue, "number")
}