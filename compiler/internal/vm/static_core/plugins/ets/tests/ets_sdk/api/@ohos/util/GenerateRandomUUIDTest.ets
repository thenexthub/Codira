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

import {util} from "@ohos.util";

const success: int = 0;
const fail: int = 1;

function check(result: int, message: string): int {
    if (result == 0) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function main(): int {
    let failures: int = 0
    failures += check(testGenerateRandomUUID(), "Test testGenerateRandomUUID() for RFC4122 compliance")
    failures += check(testIsUniqueUUID(), "Test generated UUID's by generateRandomUUID() are unique")
    failures += check(testGetNonCachedUUID(), "Test get non-cached UUID")
    return failures
}

function testGenerateRandomUUID(): int {
    let id = util.generateRandomUUID()
    arktest.assertEQ(id.length, 36)
    let delimiterIdx = new Array<int>(8, 13, 18, 23)
    const hexDigits = "0123456789abcdef"
    for (let i = 0; i < 36; ++i) {
       if (delimiterIdx.includes(i)) {
           arktest.assertEQ(id.charAt(i), c'-')
       } else {
           arktest.assertTrue(hexDigits.includes(new Char(id.charAt(i)).toString()))
       }
    }

    const versionByte = 14
    arktest.assertEQ(id.charAt(versionByte), c'4', "Version byte has to be 0x4")
    const withReservedBits = 19
    arktest.assertTrue(id.charAt(withReservedBits) >= c'8' && id.charAt(withReservedBits) <= c'b',
        id.charAt(withReservedBits) + " are not valid reserved bits from range 0x8..0xb!")
    return success
}

function testIsUniqueUUID(): int {
    let idSet: Set<string> = new Set<string>()
    for (let i = 0; i < 1000; ++i) {
        let id = util.generateRandomUUID()
        arktest.assertFalse(idSet.has(id))
        idSet.add(id)
    }
    return success
}

function testGetNonCachedUUID(): int {
    let idSet: Set<string> = new Set<string>()
    for (let i = 0; i < 1000; ++i) {
        let id = util.generateRandomUUID(false)
        arktest.assertFalse(idSet.has(id))
        idSet.add(id)
    }
    return success
}
