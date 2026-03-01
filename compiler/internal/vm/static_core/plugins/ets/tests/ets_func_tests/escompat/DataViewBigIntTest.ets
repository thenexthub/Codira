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

const BIG_UINT_MAX = new BigInt("18446744073709551615");
const BIG_UINT_MIN = 0n;
const BIG_INT_MAX = new BigInt("9223372036854775807");
const BIG_INT_MIN = new BigInt("-9223372036854775808");

const success : int = 0;
const fail    : int = 1;

function check(result: int, message: String): int {
    if(result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function main(): int {
    let failures: int = 0;
    failures += check(testDataViewBigIntMaxValue(), "testDataViewBigIntMaxValue");
    failures += check(testDataViewBigUnsingedIntMaxValue(), "testDataViewBigUnsingedIntMaxValue");
    failures += check(testDataViewBigIntSignedMinValue(), "testDataViewBigIntSignedMinValue")
    failures += check(testDataViewBigIntSignedMaxValue(), "testDataViewBigIntSignedMaxValue")
    failures += check(testDataViewBigUnsingedIntMinValue(), "testDataViewBigUnsingedIntMinValue")
    // NOTE(aleksander-sotov) Internal issue 21446
    // failures += check(testDataViewBigUnsingedIntMaxUValueOverflow(), "testDataViewBigUnsingedIntMaxUValueOverflow")
    return failures
}

function testDataViewBigIntMaxValue(): int {
    const buffer = new ArrayBuffer(64);
    const view = new DataView(buffer);
    
    view.setBigInt64(0, BIG_UINT_MAX);
    if (view.getBigInt64(0).getLong() != -1) {
        return fail;
    }
    return success;
}

function testDataViewBigUnsingedIntMaxValue(): int {
    const buffer = new ArrayBuffer(64);
    const view = new DataView(buffer);
    
    view.setBigInt64(0, BIG_UINT_MAX);
    if (view.getBigUint64(0) != BIG_UINT_MAX) {
        return fail;
    }
    return success;
}

function testDataViewBigUnsingedIntMaxUValueOverflow(): int {
    const buffer = new ArrayBuffer(64);
    const view = new DataView(buffer);
    
    view.setBigUint64(0, BIG_UINT_MAX + 1n);
    if (view.getBigUint64(0) != BIG_UINT_MIN) {
        return fail;
    }
    return success;
}

function testDataViewBigUnsingedIntMinValue(): int {
    const buffer = new ArrayBuffer(64);
    const view = new DataView(buffer);
    
    view.setBigInt64(0, BIG_UINT_MIN - 1n);
    if (view.getBigUint64(0) != BIG_UINT_MAX) {
        return fail;
    }
    return success;
}

function testDataViewBigIntSignedMinValue(): int {
    const buffer = new ArrayBuffer(64);
    const view = new DataView(buffer);
    
    view.setBigInt64(0, BIG_INT_MIN - 1n);
    if (view.getBigInt64(0) != BIG_INT_MAX) {
        return fail;
    }
    return success;
}

function testDataViewBigIntSignedMaxValue(): int {
    const buffer = new ArrayBuffer(64);
    const view = new DataView(buffer);
    
    view.setBigInt64(0, BIG_INT_MAX + 1n);
    if (view.getBigInt64(0) != BIG_INT_MIN) {
        return fail;
    }
    return success
}
