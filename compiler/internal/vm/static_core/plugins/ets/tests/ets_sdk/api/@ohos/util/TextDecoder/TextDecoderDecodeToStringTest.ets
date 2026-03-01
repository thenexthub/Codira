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
    const suite = new arktest.ArkTestsuite("TextDecoder API tests");

    suite.addTest("test_decodeToString_gb18030", testDecodeToStringWithGb18030);
    suite.addTest("test_decodeToString_big5", testDecodeToStringWithBig5);
    suite.addTest("test_decodeToString_shift_jis", testDecodeToStringWithShiftJis);
    suite.addTest("test_decodeToString_iso-2022-jp", testDecodeToStringWithIso2022Jp);
    suite.addTest("test_decodeToString_ibm866", testDecodeToStringWithIbm866);
    suite.addTest("test_decodeToString_gbk", testDecodeToStringWithGbk);
    suite.addTest("test_decodeToString_gb2312", testDecodeToStringWithGb2312);
    suite.addTest("test_decodeToString_iso-8859-2", testDecodeToStringWithIso88592);
    suite.addTest("test_decodeToString_iso-8859-4", testDecodeToStringWithIso88594);
    suite.addTest("test_decodeToString_iso-8859-5", testDecodeToStringWithIso88595);
    suite.addTest("test_decodeToString_iso-8859-6", testDecodeToStringWithIso88596);
    suite.addTest("test_decodeToString_iso-8859-7", testDecodeToStringWithIso88597);
    suite.addTest("test_decodeToString_iso-8859-8", testDecodeToStringWithIso88598);
    suite.addTest("test_decodeToString_iso-8859-8-i", testDecodeToStringWithIso88598i);
    suite.addTest("test_decodeToString_iso-8859-10", testDecodeToStringWithIso885910);
    suite.addTest("test_decodeToString_iso-8859-13", testDecodeToStringWithIso885913);
    suite.addTest("test_decodeToString_iso-8859-14", testDecodeToStringWithIso885914);
    suite.addTest("test_decodeToString_iso-8859-15", testDecodeToStringWithIso885915);
    suite.addTest("test_decodeToString_iso-koi8-r", testDecodeToStringWithKoi8r);
    suite.addTest("test_decodeToString_iso-koi8-u", testDecodeToStringWithKoi8u);

    return suite.run()
}

function testDecodeToStringWithGb18030() {
    let result = new Uint8Array([97,98,99,185,254,185,254,236,218,236,218]);
    let textDecode = util.TextDecoder.create('gb18030', {ignoreBOM : true});
    let res = textDecode.decodeToString(result);
    arktest.assertEQ(res, 'abc哈哈熠熠')
}

function testDecodeToStringWithBig5() {
    let result = new Uint8Array([97,98,99,171,162,171,162,230,102,230,102]);
    let textDecode = util.TextDecoder.create('big5', {ignoreBOM : true});
    let res = textDecode.decodeToString(result);
    arktest.assertEQ(res, 'abc哈哈熠熠')
}

function testDecodeToStringWithShiftJis() {
    let result = new Uint8Array([130,177,130,241,130,201,130,191,130,205]);
    let textDecode = util.TextDecoder.create('shift_jis');
    let res = textDecode.decodeToString(result);
    arktest.assertEQ(res, 'こんにちは')
}

function testDecodeToStringWithIso2022Jp() {
    let result = new Uint8Array([27,36,66,36,51,36,115,36,75,36,65,36,79,27,40,66]);
    let textDecode = util.TextDecoder.create('iso-2022-jp', {ignoreBOM : true});
    let res = textDecode.decodeToString(result);
    arktest.assertEQ(res, 'こんにちは')
}

function testDecodeToStringWithIbm866() {
    let result = new Uint8Array([135,164,224,160,162,225,226,162,227,169,226,165]);
    let textDecode = util.TextDecoder.create('ibm866');
    let res = textDecode.decodeToString(result);
    arktest.assertEQ(res, 'Здравствуйте');
}

function testDecodeToStringWithGbk() {
    let result = new Uint8Array([97,98,99,185,254,236,218]);
    let textDecode = util.TextDecoder.create('GBK', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'abc哈熠');
}

function testDecodeToStringWithGb2312() {
    let result = new Uint8Array([97,98,99,185,254,236,218]);
    let textDecode = util.TextDecoder.create('GB2312', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'abc哈熠');
}

function testDecodeToStringWithIso88592() {
    let result = new Uint8Array([83,122,105,97]);
    let textDecode = util.TextDecoder.create('iso-8859-2');
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Szia');
}

function testDecodeToStringWithIso88594() {
    let result = new Uint8Array([84,97,118,97,108,105,115,101,108,116,32,116,101,114,101,32,112,228,101,118,97,108]);
    let textDecode = util.TextDecoder.create('iso-8859-4', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Tavaliselt tere päeval');
}

function testDecodeToStringWithIso88595() {
    let result = new Uint8Array([183,212,224,208,210,225,226,210,227,217,226,213]);
    let textDecode = util.TextDecoder.create('iso-8859-5', {ignoreBOM : false});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Здравствуйте');
}

function testDecodeToStringWithIso88596() {
    let result = new Uint8Array([229,209,205,200,199]);
    let textDecode = util.TextDecoder.create('iso-8859-6');
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'مرحبا');
}

function testDecodeToStringWithIso88597() {
    let result = new Uint8Array([195,229,233,225,32,243,239,245]);
    let textDecode = util.TextDecoder.create('iso-8859-7');
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Γεια σου');
}

function testDecodeToStringWithIso88598() {
    let result = new Uint8Array([249,236,229,237]);
    let textDecode = util.TextDecoder.create('iso-8859-8', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'שלום');
}

function testDecodeToStringWithIso88598i() {
    let result = new Uint8Array([249,236,229,237]);
    let textDecode = util.TextDecoder.create('iso-8859-8-i', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'שלום');
}

function testDecodeToStringWithIso885910() {
    let result = new Uint8Array([72,101,105]);
    let textDecode = util.TextDecoder.create('iso-8859-10', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Hei');
}

function testDecodeToStringWithIso885913() {
    let result = new Uint8Array([83,118,101,105,107,105]);
    let textDecode = util.TextDecoder.create('iso-8859-13');
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Sveiki');
}

function testDecodeToStringWithIso885914() {
    let result = new Uint8Array([68,105,111,108,99,104]);
    let textDecode = util.TextDecoder.create('iso-8859-14', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Diolch');
}

function testDecodeToStringWithIso885915() {
    let result = new Uint8Array([72,111,108,97]);
    let textDecode = util.TextDecoder.create('iso-8859-15');
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Hola');
}

function testDecodeToStringWithKoi8r() {
    let result = new Uint8Array([240,210,201,215,197,212,44,32,205,201,210,33]);
    let textDecode = util.TextDecoder.create('koi8-r', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Привет, мир!');
}

function testDecodeToStringWithKoi8u() {
    let result = new Uint8Array([240,210,201,215,166,212,44,32,211,215,166,212]);
    let textDecode = util.TextDecoder.create('koi8-u', {ignoreBOM : true});
    let data= textDecode.decodeToString(result);
    arktest.assertEQ(data, 'Привіт, світ');
}
