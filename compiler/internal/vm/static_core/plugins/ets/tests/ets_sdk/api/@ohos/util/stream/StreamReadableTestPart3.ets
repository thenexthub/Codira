/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError } from "@ohos.base";
import { stream } from "@ohos.util.stream";

const mustChecks: Array<Function> = new Array<Function>();

function main(): int {
    const suite = new arktest.ArkTestsuite("Stream API tests");

    suite.addTest("Pause and Resume in Readable Stream.", testReadablePauseAndResume)
    suite.addAsyncTest("Readable property in Readable.", testReadablePropertyReadable)
    suite.addTest("Set encoding for Readable.", testReadableSetEncoding)

    return suite.run();
}

function mustNotCall() {
    let fn: Function = (...params: Object[]) => { }
    return mustCall(fn, 0);
}

async function mustCallCheckAll() {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => {
            try {
                mustChecks.forEach(check => {
                    check.unsafeCall();
                });
                mustChecks.length = 0;
                resolve(undefined);
            } catch (e) {
                mustChecks.length = 0;
                reject(e)
            }
        }, 2000);
    })
}

function mustCall(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (err: Error | undefined | null): void => {
        callCount++;
        fn?.unsafeCall(err);
    }

    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

class MyRead1 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamReadablePauseAndResume001
function testReadablePauseAndResume() {
    let readable = new MyRead1();
    readable.setEncoding('utf8');
    readable.on('readable', (): void => {
    });
    readable.off('readable');
    readable.pause();
    setTimeout(() => {
        arktest.assertTrue(readable.isPaused());
    }, 0);
}

class MyRead2 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamReadableReadable001
async function testReadablePropertyReadable() {
    let readable = new MyRead2();
    let testMustNotCall = mustNotCall();
    arktest.assertEQ(readable.readable, true);
    readable.on('end', testMustNotCall);
    readable.resume();
    readable.push(null);
    arktest.assertEQ(readable.readable, true);
    readable.off('end');
    readable.on('end', mustCall((): void => {
        arktest.assertEQ(readable.readable, false);
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamReadableSetEncoding001
function testReadableSetEncoding() {
    let readable = new stream.Readable();
    readable.setEncoding();
    arktest.assertEQ(readable.readableEncoding, 'utf-8');
}
