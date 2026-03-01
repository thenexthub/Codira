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
desc: Casting conversions int to float.
---*/

function isInt(value: number): boolean {
  return value >= -2147483648 && value <= 2147483647;
}

function intToFloat(intValue: number): number {
  if (!isInt(intValue)) {
    throw new Error("no int type");
  }
  return intValue;
}

function main(): void {
  let intValue: number = 2147483647;
  let floatValue: number = intToFloat(intValue);
  console.log(`raw value (int): ${intValue}`);
  console.log(`Converted value (float): ${floatValue}`);
  arktest.assertEQ(typeof floatValue, "number")
}