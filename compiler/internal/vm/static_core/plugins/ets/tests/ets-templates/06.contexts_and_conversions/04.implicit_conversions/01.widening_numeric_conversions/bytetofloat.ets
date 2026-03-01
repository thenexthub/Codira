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
desc: Casting conversions byte to float.
---*/

function bytesToFloat(bytes: Uint8Array): number {
  if (bytes.length !== 4) {
    throw new Error("Byte array must be exactly 4 bytes long");
  }

  const buffer = new ArrayBuffer(4);
  const dataView = new DataView(buffer);

  for (let i = 0; i < bytes.length; i++) {
    dataView.setUint8(i, bytes[i]);
  }

  return dataView.getFloat32(0, true); 
}

function main(): void {
  const bytes1 = new Uint8Array([0x40, 0x49, 0x0F, 0xDB]);
  const result1 = bytesToFloat(bytes1);
  console.log(`Test Case 1: ${result1}`);

  const bytes2 = new Uint8Array([0x3F, 0x80, 0x00, 0x00]);
  const result2 = bytesToFloat(bytes2);
  console.log(`Test Case 2: ${result2}`);

  const bytes3 = new Uint8Array([0xBF, 0x80, 0x00, 0x00]);
  const result3 = bytesToFloat(bytes3);
  console.log(`Test Case 3: ${result3}`);

  const bytes4 = new Uint8Array([0x00, 0x00, 0x00, 0x00]);
  const result4 = bytesToFloat(bytes4);
  console.log(`Test Case 4: ${result4}`);
}