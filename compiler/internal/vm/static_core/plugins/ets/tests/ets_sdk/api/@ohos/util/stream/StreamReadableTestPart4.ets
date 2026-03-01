/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

    suite.addAsyncTest("Readable does not implement DoRead.", testReadableErrorOnUnimplementedDoRead);
    suite.addTest("Reads a buffer of a specified size from the buffer001.", testReadableSetEncodingUtf8);
    suite.addTest("Reads a buffer of a specified size from the buffer002.", testReadableSetEncodingUtf8Alias);
    suite.addTest("Reads a buffer of a specified size from the buffer003.", testReadablePushAndReadWithInvalidEncoding);
    suite.addTest("Reads a buffer of a specified size from the buffer004.", testReadablePushAndReadUtf8);
    suite.addTest("Reads a buffer of a specified size from the buffer005.",
        testReadablePushAndReadWithUtf8EncodingOption);
    suite.addTest("Reads a buffer of a specified size from the buffer006.",
        testReadablePushAndReadWithWindows1250EncodingOption);
    suite.addTest("Reads a buffer of a specified size from the buffer007.", testReadableEncodingPropertyWindows1250);
    suite.addTest("Reads a buffer of a specified size from the buffer008.", testReadableEncodingPropertyDefaultUtf8);
    suite.addTest("Reads a buffer of a specified size from the buffer009.", testReadablePushAndReadDefaultUtf8);

    return suite.run();
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

// Test cases ported from ArkTS 1.0:testStreamReadableWithUnimplementedDoRead001
async function testReadableErrorOnUnimplementedDoRead() {
    let readable = new stream.Readable();
    readable.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.name, 'BusinessError');
        arktest.assertEQ(error.message, 'The doRead() method is not implemented.');
    }));
    readable.on('close', mustCall());
    readable.read();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamReadableread001
function testReadableSetEncodingUtf8() {
    const stringdecoder = new StringDecoderReadable();
    stringdecoder.setEncoding("utf-8");
    arktest.assertEQ(stringdecoder.readableEncoding, "utf-8");
}

// Test cases ported from ArkTS 1.0:testStreamReadableread002
function testReadableSetEncodingUtf8Alias() {
    const stringdecoder = new StringDecoderReadable();
    stringdecoder.setEncoding("utf8");
    arktest.assertEQ(stringdecoder.readableEncoding, "utf-8");
}

// Test cases ported from ArkTS 1.0:testStreamReadableread003
function testReadablePushAndReadWithInvalidEncoding() {
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const stringdecoder = new StringDecoderReadable();
    stringdecoder.push(input);
    stringdecoder.setEncoding("utf-8")
    let res = stringdecoder.read(2);
    arktest.assertEQ(res, '你好');
}

// Test cases ported from ArkTS 1.0:testStreamReadableread004
function testReadablePushAndReadUtf8() {
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const stringdecoder = new StringDecoderReadable();
    stringdecoder.push(input);
    stringdecoder.setEncoding('utf-8');
    let res = stringdecoder.read(4);
    arktest.assertEQ(res, null);
}

// Test cases ported from ArkTS 1.0:testStreamReadableread005
function testReadablePushAndReadWithUtf8EncodingOption() {
    let enc: stream.ReadableOptions = {
        encoding: "utf-8"
    };
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const stringdecoder = new StringDecoderReadable(enc);
    stringdecoder.push(input);
    let res = stringdecoder.read(input.length);
    arktest.assertEQ(res, null);
}

// Test cases ported from ArkTS 1.0:testStreamReadableread006
function testReadablePushAndReadWithWindows1250EncodingOption() {
    let enc: stream.ReadableOptions = {
        encoding: "windows-1250"
    };
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const stringdecoder = new StringDecoderReadable(enc);
    let ss = stringdecoder.setEncoding();
    stringdecoder.push(input);
    let res = stringdecoder.read();
    arktest.assertEQ(res, '你好');
}

// Test cases ported from ArkTS 1.0:testStreamReadableread007
function testReadableEncodingPropertyWindows1250() {
    let enc: stream.ReadableOptions = {
        encoding: "windows-1250"
    };
    const stringdecoder = new StringDecoderReadable(enc);
    let res = stringdecoder.readableEncoding;
    arktest.assertEQ(res, 'windows-1250');
}

// Test cases ported from ArkTS 1.0:testStreamReadableread008
function testReadableEncodingPropertyDefaultUtf8() {
    const stringdecoder = new StringDecoderReadable();
    let res = stringdecoder.readableEncoding;
    arktest.assertEQ(res, null);
}

// Test cases ported from ArkTS 1.0:testStreamReadableread009
function testReadablePushAndReadDefaultUtf8() {
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const stringdecoder = new StringDecoderReadable();
    stringdecoder.push(input);
    let res = stringdecoder.read();

    arktest.assertEQ((res as Object).toString(), '你好');
}

function testReadableEmitClose() {
    const stringdecoder = new StringDecoderReadable();
    stringdecoder.on('close', mustCall());
    stringdecoder.push("test");
    stringdecoder.push(null);
}

class StringDecoderReadable extends stream.Readable {
    constructor() {
        super();
    }

    constructor(opt: stream.ReadableOptions) {
        super(opt);
    }

    doRead(size: int) {
    }
}
