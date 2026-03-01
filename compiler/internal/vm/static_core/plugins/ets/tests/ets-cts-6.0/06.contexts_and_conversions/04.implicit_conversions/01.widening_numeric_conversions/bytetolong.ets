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
desc: Casting conversions byte to long.
---*/

function bytesToLong(bytes: Uint8Array): bigint {
    if (bytes.length !== 8) {
        throw new Error("Byte array must be exactly 8 bytes long");
    }

    const buffer = new ArrayBuffer(8);
    const dataView = new DataView(buffer);

    for (let i = 0; i < bytes.length; i++) {
        dataView.setUint8(i, bytes[i]);
    }

    return dataView.getBigInt64(0, true);
}

function main(): void {
    const bytes = new Uint8Array([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08]);
    try {
        const result = bytesToLong(bytes);
        console.log(result.toString());
    } catch (error) {
        console.error(error);
    }
}
