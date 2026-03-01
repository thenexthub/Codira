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

export class CRC32 {

    table = new int[256];

    public setup(): void {
        let c: int = 0;
        for (let n: int = 0; n < 256; n++) {
            c = n;
            for (let k: int = 0; k < 8; k++) {
                c = ((c&1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
            }
            this.table[n] = c;
        }
    }

    crc32(str: String): int {
        let crc: int = 0 ^ (-1);
        for (let i = 0; i < str.length; i++ ) {
            crc = (crc >>> 8) ^ this.table[(crc ^ (str.charAt(i) as int)) & 0xFF];
        }
        return (crc ^ (-1)) >>> 0;
    }

    test(): int {
        let s = "Lorem ipsum dolor sit amet, consectetur adipiscing elit";
        return this.crc32(s);
    }

}

