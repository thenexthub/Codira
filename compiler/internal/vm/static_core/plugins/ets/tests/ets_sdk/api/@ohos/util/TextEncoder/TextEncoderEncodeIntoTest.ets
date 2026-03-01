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

function main() {
    const suite = new arktest.ArkTestsuite("TextEncoder API tests");

    suite.addTest("test_create_with_uft8", testCreateUft8);
    suite.addTest("test_create_with_iso-8859-2", testCreateIso88592);
    suite.addTest("test_create_with_euc-kr", testCreateEuckr);
    suite.addTest("test_create_with_ibm866", testCreateIbm866);
    suite.addTest("test_create_with_x-mac-cyrillic", testCreateXmaccyrillic);
    suite.addTest("test_EncodeInto_with_utf8_001", testEncodeIntoUtf8001);
    suite.addTest("test_EncodeInto_with_utf8_002", testEncodeIntoUtf8002);
    suite.addTest("test_EncodeInto_with_utf8_003", testEncodeIntoUtf8003);
    suite.addTest("test_EncodeInto_with_utf8_004", testEncodeIntoUtf8004);
    suite.addTest("test_EncodeInto_with_utf8_005", testEncodeIntoUtf8005);
    suite.addTest("test_EncodeInto_with_utf8_006", testEncodeIntoUtf8006);
    suite.addTest("test_EncodeInto_with_utf8_007", testEncodeIntoUtf8007);
    return suite.run()
}

function testCreateUft8() {
    let that = util.TextEncoder.create()
    let str = that.encoding
    arktest.assertEQ(str, 'utf-8')
}

function testCreateIso88592() {
    let that = util.TextEncoder.create('iso-8859-2')
    let str = that.encoding
    arktest.assertEQ(str, 'iso-8859-2')
}

function testCreateEuckr() {
    let that = util.TextEncoder.create('euc-kr')
    let str = that.encoding
    arktest.assertEQ(str, 'euc-kr')
}

function testCreateIbm866() {
    let that = util.TextEncoder.create('ibm866')
    let str = that.encoding
    arktest.assertEQ(str, 'ibm866')
}

function testCreateXmaccyrillic() {
    let that = util.TextEncoder.create('x-mac-cyrillic')
    let str = that.encoding
    arktest.assertEQ(str, 'x-mac-cyrillic')
}

function testEncodeIntoUtf8001() {
    let that = new util.TextEncoder()
    let result = that.encodeInto('\uD800楼楼')
    arktest.assertEQ(result[5], 188)
}

function testEncodeIntoUtf8002() {
    let that = new util.TextEncoder()
    let result = that.encodeInto('a\uD800楼楼')
    arktest.assertEQ(result[0], 0x61)
}

function testEncodeIntoUtf8003() {
    let that = new util.TextEncoder()
    let result = that.encodeInto('abc\uD800楼楼')
    arktest.assertEQ(result[1], 0x62)
}

function testEncodeIntoUtf8004() {
    let that = new util.TextEncoder()
    let result = that.encodeInto('123\uD800楼楼')
    arktest.assertEQ(result[0], 49)
    arktest.assertEQ(result[9], 230)
}

function testEncodeIntoUtf8005() {
    let that = new util.TextEncoder()
    let result = that.encodeInto('123¥\uD800楼')
    arktest.assertEQ(result[10], 0xbc)
}

function testEncodeIntoUtf8006() {
    let that = new util.TextEncoder()
    let result = that.encodeInto('¥¥')
    arktest.assertEQ(result[0], 0xc2)
}

function testEncodeIntoUtf8007() {
    let that = new util.TextEncoder()
    let result = that.encodeInto('$$')
    arktest.assertEQ(result[0], 0x24)
}
