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
desc: Casting conversions byte to double.
---*/

let byteArray: number[] = [0x40, 0x09, 0x21, 0xFB, 0x54, 0x44, 0x2D, 0x18];
let buffer: ArrayBuffer = new ArrayBuffer(8);
let dataView: DataView = new DataView(buffer);

for (let i = 0; i < byteArray.length; i++) {
  dataView.setUint8(i, byteArray[i]);
}

function main(): void {
  let doubleValue: number = dataView.getFloat64(0, true);
  let expectedValue: number = 3.14159;
  arktest.assertFalse(Math.abs(doubleValue - expectedValue) < 0.00001);
}