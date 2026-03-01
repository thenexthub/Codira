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

const FLOAT_PI: float = 3.14f;

type L = StdDebug.Logger

async function byteFunc(): Promise<Object> {
    return -3 as byte as Object;
}

async function charFunc(): Promise<Object> {
    return c'x' as Object;
}

async function shortFunc(): Promise<Object> {
    return 32767 as short as Object;
}

async function intFunc(): Promise<Object> {
    return 5 as Object;
}

async function longFunc(): Promise<Object> {
    return 777 as long as Object;
}

async function floatFunc(): Promise<Object> {
    return FLOAT_PI as Object;
}

async function doubleFunc(): Promise<Object> {
    return 2.71 as Object;
}

let passed: boolean = false;
let errorMessage = '';

function reset(): void {
    passed = false;
    errorMessage = '';
}

function check(test: String): void {
    if (!passed) {
        L.log('Test ' + test + ' failed: ' + errorMessage);
        throw new Error();
    }
}

async function testByteFunc(): Promise<void> {
    reset();
    let byteP: Promise<Object> = byteFunc();
    let pThen = byteP.then<Object>((value: Object): Object => {
        if ((value as Byte) == -3) {
            passed = true;
        } else {
            errorMessage = 'Expected -3 but got ' + value;
        }
        return value
    });
    await byteP;
    await (pThen as Promise<Object>);
    check('byteFunc');
}

async function testCharFunc(): Promise<void> {
    reset();
    let charP: Promise<Object> = charFunc();
    let pThen = charP.then<Object>((value: Object): Object => {
        if ((value as Char) == c'x') {
            passed = true;
        } else {
            errorMessage = 'Expected x but got ' + value;
        }
        return value
    });
    await charP;
    await (pThen as Promise<Object>);
    check('charFunc');
}

async function testShortFunc(): Promise<void> {
    reset();
    let shortP: Promise<Object> = shortFunc();
    let pThen = shortP.then<Object>((value: Object): Object => {
        if ((value as Short) == 32767) {
            passed = true;
        } else {
            errorMessage = 'Expected 32767 but got ' + value;
        }
        return value
    });
    await shortP;
    await (pThen as Promise<Object>);
    check('shortFunc');
}

async function testIntFunc(): Promise<void> {
    reset();
    let intP: Promise<Object> = intFunc();
    let pThen = intP.then<Object>((value: Object): Object => {
        if ((value as Int) == 5) {
            passed = true;
        } else {
            errorMessage = 'Expected 5 but got ' + value;
        }
        return value
    });
    await intP;
    await (pThen as Promise<Object>);
    check('intFunc');
}

async function testLongFunc(): Promise<void> {
    reset();
    let longP: Promise<Object> = longFunc();
    let pThen = longP.then<Object>((value: Object): Object => {
        if ((value as Long) == 777) {
            passed = true;
        } else {
            errorMessage = 'Expected 777 but got ' + value;
            L.log(errorMessage);
        }
        return value
    });
    await longP;
    await (pThen as Promise<Object>);
    check('longFunc');
}

async function testFloatFunc(): Promise<void> {
    reset();
    let floatP: Promise<Object> = floatFunc();
    let pThen = floatP.then<Object>((value: Object): Object => {
        if ((value as Float) == FLOAT_PI) {
            passed = true;
        } else {
            errorMessage = 'Expected ' + FLOAT_PI + ' but got ' + value;
            L.log(errorMessage);
        }
        return value
    });
    await floatP;
    await (pThen as Promise<Object>);
    check('floatFunc');
}

async function testDoubleFunc(): Promise<void> {
    reset();
    let doubleP: Promise<Object> = doubleFunc();
    let pThen = doubleP.then<Object>((value: Object): Object => {
        if ((value as Double) == 2.71) {
            passed = true;
        } else {
            errorMessage = 'Expected 2.71 but got ' + value;
        }
        return value
    });
    await doubleP;
    await (pThen as Promise<Object>);
    check('doubleFunc');
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.launch_return');
    testSuite.addAsyncTest('testByteFunc', testByteFunc);
    testSuite.addAsyncTest('testCharFunc', testCharFunc);
    testSuite.addAsyncTest('testShortFunc', testShortFunc);
    testSuite.addAsyncTest('testIntFunc', testIntFunc);
    testSuite.addAsyncTest('testLongFunc', testLongFunc);
    testSuite.addAsyncTest('testFloatFunc', testFloatFunc);
    testSuite.addAsyncTest('testDoubleFunc', testDoubleFunc);
    let res = testSuite.run();
    return res;
}
