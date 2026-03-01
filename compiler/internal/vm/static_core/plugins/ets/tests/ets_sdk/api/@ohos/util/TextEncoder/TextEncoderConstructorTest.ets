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

function CharCodeEqual(resStr: String, expect: number)
{
    return resStr.charCodeAt(0) === expect;
}

function main() {
    const suite = new arktest.ArkTestsuite("TextEncoder API tests");

    suite.addTest("test_encoder_utf8", test_encoder_utf8);
    suite.addTest("test_encoder_u16le_helloworld", test_encoder_u16le_helloworld);
    suite.addTest("test_encoder_u16le_he", test_encoder_u16le_he);
    suite.addTest("test_encoder_u16be_helloworld", test_encoder_u16be_helloworld);
    suite.addTest("test_encoder_u16be_he", test_encoder_u16be_he);
    suite.addTest("test_encoder_u16be_cnstr", test_encoder_u16be_cnstr);
    suite.addTest("test_encoder_u16le_cnstr", test_encoder_u16le_cnstr);
    suite.addTest("test_encoder_default_abc", test_encoder_default_abc);
    suite.addTest("test_encoder_default_cnstr", test_encoder_default_cnstr);
    suite.addTest("test_encoder_default_empty", test_encoder_default_empty);
    suite.addTest("test_encoder_encode_into_uint8array_escape", test_encoder_encode_into_uint8array_escape);
    suite.addTest("test_encoder_encode_into_uint8array_enstr", test_encoder_encode_into_uint8array_enstr);
    suite.addTest("test_encoder_encode_into_uint8array_enstrbe", test_encoder_encode_into_uint8array_enstrbe);
    suite.addTest("test_encoding_utf8", test_encoding_utf8);
    suite.addTest("test_encoding_gb18030", test_encoding_gb18030);
    suite.addTest("test_encoding_gbk", test_encoding_gbk);
    suite.addTest("test_encoding_gb2312", test_encoding_gb2312);
    suite.addTest("test_encoding_utf16le", test_encoding_utf16le);
    suite.addTest("test_encoding_iso88593", test_encoding_iso88593);
    suite.addTest("test_encoding_u16le_h", test_encoding_u16le_h);
    return suite.run()
}

function test_encoder_utf8() {
    const enc = new util.TextEncoder('utf-8');
    let str = 'Hello world';
    const bytes = enc.encodeInto(str);
    for (let i = 0; i < str.length; i++) {
        arktest.assertTrue(CharCodeEqual(str[i], bytes[i]), "expected equality failed: " +
                                       str[i].charCodeAt(0) + "===" + bytes[i]);
    }
}

function test_encoder_u16le_helloworld() {
    const enc = new util.TextEncoder('utf-16le');
    let str = 'Hello world';
    const bytes = enc.encodeInto(str);
    arktest.assertEQ(bytes[0], 72);
    arktest.assertEQ(bytes[3], 0);
}

function test_encoder_u16le_he() {
    const enc = new util.TextEncoder('utf-16le');
    const bytes = enc.encodeInto('He');
    arktest.assertEQ(bytes[0], 72);
    arktest.assertEQ(bytes[3], 0);
}

function test_encoder_u16be_helloworld() {
    const enc = new util.TextEncoder('utf-16be');
    const bytes = enc.encodeInto('Hello world');
    arktest.assertEQ(bytes[0], 0);
    arktest.assertEQ(bytes[1], 72);
}

function test_encoder_u16be_he() {
    const enc = new util.TextEncoder('utf-16be');
    const bytes = enc.encodeInto('He');
    arktest.assertEQ(bytes[0], 0);
    arktest.assertEQ(bytes[1], 72);
}

function test_encoder_GB2312() {
    let enc = util.TextEncoder.create('GB2312');
    let result = enc.encodeInto('abc哈熠');
    let typedArray = new Uint8Array([97,98,99,185,254,236,218]);
    for (let i = 0; i < typedArray.length; i++) {
        arktest.assertEQ(result[i], typedArray[i]);
    }
}

function test_encoder_u16be_cnstr() {
    let enc= new util.TextEncoder('utf-16be');
    let result = enc.encodeInto('abc哈熠');
    let res = new Uint8Array([0,97,0,98,0,99,84,200,113,160]);
    arktest.assertEQ(result[0], 0);
    arktest.assertEQ(result[1], 97);
}

function test_encoder_u16le_cnstr() {
    let enc = new util.TextEncoder('utf-16le');
    let result = enc.encodeInto('abc哈熠');
    let res3 = new Uint8Array([97,0,98,0,99,0,200,84,160,113]);
    arktest.assertEQ(result[0], 97);
    arktest.assertEQ(result[1], 0);
}

function test_encoder_default_abc() {
    let enc = new util.TextEncoder();
    let result = enc.encodeInto('abc')
    arktest.assertEQ(result[0], 0x61);
}

function test_encoder_default_cnstr() {
    let enc = new util.TextEncoder()
    let result = enc.encodeInto('\uD800楼楼')
    arktest.assertEQ(result[5], 188);
}

function test_encoder_default_empty() {
    let enc = new util.TextEncoder();
    let result = enc.encodeInto('');
    arktest.assertEQ(result.length, 0);
}

function test_encoder_encode_into_uint8array_escape() {
    let enc = new util.TextEncoder()
    let buffer = new ArrayBuffer(10)
    let typedArray = new Uint8Array(buffer)
    let encodeIntoUint8ArrayInfo = enc.encodeIntoUint8Array('@1h-+=&^', typedArray);

    const encc = new util.TextEncoder('utf-8');
    let str = '@1h-+=&^';
    const bytes = encc.encodeInto(str);

    arktest.assertEQ(encodeIntoUint8ArrayInfo.read, 8);
    arktest.assertEQ(encodeIntoUint8ArrayInfo.written, 8);
    for (let i = 0; i < bytes.length; i++) {
         arktest.assertEQ(bytes[i], typedArray[i]);
    }
}

function test_encoder_encode_into_uint8array_enstr() {
    let enc = new util.TextEncoder('utf-16le');
    let buffer = new ArrayBuffer(20);
    let typedArray = new Uint8Array(buffer);
    let encodeIntoUint8ArrayInfo = enc.encodeIntoUint8Array('abcdefg', typedArray)

    const encc = new util.TextEncoder('utf-16le');
    let str = 'abcdefg';
    const bytes = encc.encodeInto(str);

    for (let i = 0; i < bytes.length; i++) {
         arktest.assertEQ(bytes[i], typedArray[i]);
    }
}

function test_encoder_encode_into_uint8array_enstrbe() {
    let enc = new util.TextEncoder('utf-16be');
    let buffer = new ArrayBuffer(20);
    let typedArray = new Uint8Array(buffer);
    let encodeIntoUint8ArrayInfo = enc.encodeIntoUint8Array('abcdefg', typedArray)

    const encc = new util.TextEncoder('utf-16be');
    let str = 'abcdefg';
    const bytes = encc.encodeInto(str);

    for (let i = 0; i < bytes.length; i++) {
         arktest.assertEQ(bytes[i], typedArray[i]);
    }
}

function test_encoding_utf8() {
    let enc = new util.TextEncoder();
    let str = enc.encoding;
    arktest.assertEQ(str, 'utf-8');
}

function test_encoding_gb18030() {
    let enc = new util.TextEncoder('gb18030');
    let str = enc.encoding;
    arktest.assertEQ(str, 'gb18030');
    enc = new util.TextEncoder('GB18030');
    str = enc.encoding;
    arktest.assertEQ(str, 'GB18030');
}

function test_encoding_gbk() {
    let enc = new util.TextEncoder('gbk');
    let str = enc.encoding;
    arktest.assertEQ(str, 'gbk');
    enc = new util.TextEncoder('GBK');
    str = enc.encoding;
    arktest.assertEQ(str, 'GBK');
    enc = new util.TextEncoder('GbK');
    str = enc.encoding;
    arktest.assertEQ(str, 'GbK');
}

function test_encoding_gb2312() {
    let enc = new util.TextEncoder('gb2312');
    let str = enc.encoding;
    arktest.assertEQ(str, 'gb2312');
    enc = new util.TextEncoder('GB2312');
    str = enc.encoding;
    arktest.assertEQ(str, 'GB2312');
    enc = new util.TextEncoder('gB2312');
    str = enc.encoding;
    arktest.assertEQ(str, 'gB2312');
}

function test_encoding_utf16le() {
    let enc = new util.TextEncoder('utf-16le');
    let str = enc.encoding;
    arktest.assertEQ(str, 'utf-16le');
    enc = new util.TextEncoder('Utf-16le');
    str = enc.encoding;
    arktest.assertEQ(str, 'Utf-16le');
}

function test_encoding_iso88593() {
    let enc = new util.TextEncoder('ISO-8859-3');
    let str = enc.encoding;
    arktest.assertEQ(str, 'ISO-8859-3');
    enc = new util.TextEncoder('IBM866');
    str = enc.encoding;
    arktest.assertEQ(str, 'IBM866');
}

function test_encoding_u16le_h() {
    let enc = new util.TextEncoder('utf-16le');
    const bytes = enc.encodeInto('H');
    arktest.assertEQ(bytes[0], 72);
    arktest.assertEQ(bytes[1], 0);
}