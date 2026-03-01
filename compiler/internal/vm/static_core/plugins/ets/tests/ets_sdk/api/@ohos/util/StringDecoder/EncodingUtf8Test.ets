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

import { util } from "@ohos.util";
import { BusinessError } from "@ohos.base";

function main() {
    const suite = new arktest.ArkTestsuite("StringDecoder API encoding utf-8 tests");

    suite.addTest("Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite001);
    suite.addTest("Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite002);
    suite.addTest("Handle exception and check code and name.", testwrite003);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend002);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend003);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend004);
    suite.addTest("Handle exception and check code and name.", testend005);
    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0001
 * @tc.name: testwrite001
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite001() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const decoded = decoder.write(input);
    arktest.assertEQ(decoded, '你好');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0002
 * @tc.name: testwrite002
 * @tc.desc: Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite002() {
    let decoder = new util.StringDecoder('utf-8');
    const decoded = decoder.write("abc");
    arktest.assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0004
 * @tc.name: testend002
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend002() {
    let decoder = new util.StringDecoder('utf-8');
    const decoded = decoder.end("abc");
    arktest.assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0005
 * @tc.name: testend003
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend003() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const decoded = decoder.end(input);
    arktest.assertEQ(decoded, '你好');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0006
 * @tc.name: testend004
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend004() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const decoded = decoder.write(input.slice(0, 5));
    const decodeend = decoder.end(input.slice(5));
    arktest.assertEQ(decoded, '你');
    arktest.assertEQ(decodeend, '好');
}

function testwrite003() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array(0);
    arktest.expectThrow(() => {
        decoder.write(input);
    }, (e: Error) => {
        arktest.assertTrue(e instanceof BusinessError);
        const err = e as BusinessError;
        arktest.assertEQ(err.code, 401);
        arktest.assertEQ(err.name, 'BusinessError');
        arktest.assertEQ(err.message, 'Error obtaining minimum number of input bytes');
        return true;
    });
}

function testend005() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const decoded = decoder.write(input.slice(0, 5));
    arktest.assertEQ(decoded, '你');
    arktest.expectThrow(() => {
        decoder.end(new Uint8Array(0));
    }, (e: Error) => {
        arktest.assertTrue(e instanceof BusinessError);
        const err = e as BusinessError;
        arktest.assertEQ(err.code, 401);
        arktest.assertEQ(err.name, 'BusinessError');
        arktest.assertEQ(err.message, 'Error obtaining minimum number of input bytes');
        return true;
    });
}
