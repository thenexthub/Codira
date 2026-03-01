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

function main(): int {
    const suite = new arktest.ArkTestsuite("StringDecoder API tests");

    suite.addTest(" Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite003);
    suite.addTest(" Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite004);
    suite.addTest(" Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite005);
    suite.addTest("Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite006);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend001);
    suite.addTest("Test ENCODER and DOCODER.", test_Decode_Test_001)
    suite.addTest("Test ENCODER and DOCODER.", test_Decode_Test_002)
    suite.addTest("Test ENCODER and DOCODER.", test_Decode_Test_003)
    suite.addTest("Test ENCODER and DOCODER.", testArrayScenarios0600)
    suite.addTest("Test ENCODER and DOCODER.", testArrayScenarios0700)

    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_00013
 * @tc.name: testwrite003
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite003() {
    {
        let decoder = new util.StringDecoder('big5');
        let input = new Uint8Array([167, 65, 166, 110]);
        const decoded = decoder.write(input);
        arktest.assertEQ(decoded, '你好');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-4');
        let input = new Uint8Array([84,97,118,97,108,105,115,101,108,116,32,116,101,114,101,32,112,228,101,118,97,108]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Tavaliselt tere päeval');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-5');
        let input = new Uint8Array([183,212,224,208,210,225,226,210,227,217,226,213]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Здравствуйте');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-6');
        let input = new Uint8Array([229,209,205,200,199]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'مرحبا');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-7');
        let input = new Uint8Array([195,229,233,225,32,243,239,245]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Γεια σου');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-14');
        let input = new Uint8Array([68,105,111,108,99,104]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Diolch');
    }
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0012
 * @tc.name: testwrite004
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite004() {
    {
        let decoder = new util.StringDecoder('iso-8859-2');
        let input = new Uint8Array([0X61, 0X62, 0X63]);
        const decoded = decoder.write(input);
        arktest.assertEQ(decoded, 'abc');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-2');
        let input = new Uint8Array([83,122,105,97]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Szia');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-8');
        let input = new Uint8Array([249,236,229,237]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'שלום');
    }
    {
        let stringDecoder = new util.StringDecoder('macintosh');
        let input = new Uint8Array([142,163]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'é£');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-8-i');
        let input = new Uint8Array([249,236,229,237]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'שלום');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-10');
        let input = new Uint8Array([72,101,105]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Hei');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-13');
        let input = new Uint8Array([83,118,101,105,107,105]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Sveiki');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-8859-15');
        let input = new Uint8Array([72,111,108,97]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Hola');
    }
    {
        let stringDecoder = new util.StringDecoder('koi8-r');
        let input = new Uint8Array([240,210,201,215,197,212,44,32,205,201,210,33]);
        let data= stringDecoder.write(input);
        arktest.assertEQ(data, 'Привет, мир!');
    }
    {
        let stringDecoder = new util.StringDecoder('koi8-u');
        let input = new Uint8Array([240,210,201,215,166,212,44,32,211,215,166,212]);
        let data = stringDecoder.write(input);
        arktest.assertEQ(data, 'Привіт, світ');
    }
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0010
 * @tc.name: testwrite005
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite005() {
    {
        let decoder = new util.StringDecoder('koi8-r');
        let input = new Uint8Array([97, 98, 99]);
        const decoded = decoder.write(input);
        arktest.assertEQ(decoded, 'abc');
    }
    {
        let stringDecoder = new util.StringDecoder('utf-8');
        let uint8 = new Uint8Array([228,189,160,0,229,165,189]);
        let resStr = stringDecoder.write(uint8);
        arktest.assertEQ(resStr, '你\u0000好');
        arktest.assertEQ(resStr.length, 3);
    }
    {

        let stringDecoder = new util.StringDecoder('gb18030');
        let uint8 = new Uint8Array([97,98,99,236,218,236,218]);
        let res = stringDecoder.write(uint8);
        arktest.assertEQ(res, 'abc熠熠');
    }
    {
        let stringDecoder = new util.StringDecoder('big5');
        let uint8 = new Uint8Array([97,98,99,230,102,230,102]);
        let res = stringDecoder.write(uint8);
        arktest.assertEQ(res, 'abc熠熠');
    }
    {
        let stringDecoder = new util.StringDecoder('shift_jis');
        let uint8 = new Uint8Array([130,177,130,241,130,201,130,191,130,205]);
        let res = stringDecoder.write(uint8);
        arktest.assertEQ(res, 'こんにちは');
    }
    {
        let stringDecoder = new util.StringDecoder('iso-2022-jp');
        let uint8 = new Uint8Array([27,36,66,36,51,36,115,36,75,36,65,36,79,27,40,66]);
        let res = stringDecoder.write(uint8);
        arktest.assertEQ(res, 'こんにちは');
    }
    {
        let stringDecoder = new util.StringDecoder('ibm866');
        let uint8 = new Uint8Array([135,164,224,160,162,225,226,162,227,169,226,165]);
        let res = stringDecoder.write(uint8);
        arktest.assertEQ(res, 'Здравствуйте');
    }
    {
        let stringDecoder = new util.StringDecoder('GBK');
        let uint8 = new Uint8Array([97,98,99,236,218,236,218]);
        let data= stringDecoder.write(uint8);
        arktest.assertEQ(data, 'abc熠熠');
    }
    {
        let stringDecoder = new util.StringDecoder('GB2312');
        let uint8 = new Uint8Array([97,98,99,236,218,236,218]);
        let data= stringDecoder.write(uint8);
        arktest.assertEQ(data, 'abc熠熠');
    }
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0011
 * @tc.name: testwrite006
 * @tc.desc: Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite006() {
    let decoder = new util.StringDecoder('windows-1250');
    let input = new Uint8Array([0X61, 0X62, 0X63]);
    const decoded = decoder.write(input);
    arktest.assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0003
 * @tc.name: testend001
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend001() {
    {
        let decoder = new util.StringDecoder('macintosh');
        let input = new Uint8Array([0X61]);
        const decoded = decoder.end(input);
        arktest.assertEQ(decoded, 'a');
    }
    {
        let stringDecoder = new util.StringDecoder('utf-8');
        let uint8 = new Uint8Array([0,228,189,160,229]);
        let retStr = stringDecoder.write(uint8);
        arktest.assertEQ(retStr, '\u0000你');
        let uint8arr = new Uint8Array([165,189]);
        let resStr = stringDecoder.end(uint8arr);
        arktest.assertEQ(resStr, '好');
    }
    {
        let stringDecoder = new util.StringDecoder('utf-8');
        let uint8 = new Uint8Array([228,189,160,0,229,165,189]);
        let resStr = stringDecoder.write(uint8);
        arktest.assertEQ(resStr, '你\u0000好');
        arktest.assertEQ(resStr.length, 3);
    }
}


/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0100
 * @tc.name       : test_Decode_Test_001
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 0
 */
function test_Decode_Test_001() {
    const knownEncodings = ['utf-8', 'ibm866', 'iso-8859-2', 'iso-8859-3', 'iso-8859-4', 'iso-8859-5', 'iso-8859-6',
        'iso-8859-7', 'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13', 'iso-8859-14', 'iso-8859-15',
        'koi8-r', 'koi8-u', 'macintosh', 'windows-874', 'windows-1250', 'windows-1251', 'windows-1252', 'windows-1253',
        'windows-1254', 'windows-1255', 'windows-1256', 'windows-1257', 'windows-1258', 'x-mac-cyrillic', 'gbk',
        'gb18030', 'big5', 'euc-jp', 'iso-2022-jp', 'shift_jis', 'euc-kr'];

    for(let item of knownEncodings) {
        let decoder = new util.StringDecoder(item)
        let input1 = new Uint8Array([72, 101, 108, 108, 111, 87, 111, 114, 108, 100, 49, 50, 51, 63])
        let input2 = 'HelloWorld123?HelloWorld123?'
        const decoded1 = decoder.write(input1)
        const decoded2 = decoder.write(input2)
        if (item === 'utf-16be') {
            let input2 = 'HelloWorld123?HelloWorld123?'
        }
        arktest.assertEQ(decoded1, 'HelloWorld123?')
        arktest.assertEQ(decoded2, 'HelloWorld123?HelloWorld123?')

        let input3 = input1.slice(0, 5)
        let input33 = input1.slice(5)
        const decoded3 = decoder.write(input3)
        const decodedend3 = decoder.write(input33)
        arktest.assertEQ(decoded3, 'Hello')
        arktest.assertEQ(decodedend3, 'World123?')

        let input4 = input1.slice(0, 1)
        let input44 = input1.slice(1)
        const decoded4 = decoder.write(input4)
        const decodedend4 = decoder.write(input44)
        arktest.assertEQ(decoded4, 'H')
        arktest.assertEQ(decodedend4, 'elloWorld123?')
    }
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0200
 * @tc.name       : test_Decode_Test_002
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 0
 */
function test_Decode_Test_002() {
    {
        let that = new util.TextEncoder('utf-16be')
        let result = that.encodeInto('HelLoWorLd123?')
        let decoder = new util.StringDecoder('utf-16be')
        const decoded1 = decoder.write(result)
        arktest.assertEQ(decoded1, 'HelLoWorLd123?')
    }
    {
        let textEncoder = new util.TextEncoder();
        let buffer = new ArrayBuffer(4);
        let dest = new Uint8Array(buffer);
        textEncoder.encodeIntoUint8Array('😃', dest);
        let stringDecoder = new util.StringDecoder();
        let result = stringDecoder.write(dest);
        arktest.assertEQ(result, '😃');
    }
    {
        let textEncoder = new util.TextEncoder();
        let buffer = new ArrayBuffer(10);
        let dest = new Uint8Array(buffer);
        textEncoder.encodeIntoUint8Array('aé中😃', dest);
        let stringDecoder = new util.StringDecoder();
        let result = stringDecoder.write(dest);
        arktest.assertEQ(result, 'aé中😃');
    }
    {
        let textEncoder = new util.TextEncoder();
        let buffer = new ArrayBuffer(4);
        let dest = new Uint8Array(buffer);
        textEncoder.encodeIntoUint8Array('😃🤞', dest);
        let stringDecoder = new util.StringDecoder();
        let result = stringDecoder.write(dest);
        arktest.assertEQ(result, '😃');
    }
    {
        let textEncoder = new util.TextEncoder();
        let buffer = new ArrayBuffer(8);
        let dest = new Uint8Array(buffer);
        textEncoder.encodeIntoUint8Array('😃🤞', dest);
        let stringDecoder = new util.StringDecoder();
        let result = stringDecoder.write(dest);
        arktest.assertEQ(result, '😃🤞');
    }
    {
        let textEncoder = new util.TextEncoder();
        let buffer = new ArrayBuffer(10);
        let dest = new Uint8Array(buffer);
        textEncoder.encodeIntoUint8Array('a😃🤞b', dest);
        let stringDecoder = new util.StringDecoder();
        let result = stringDecoder.write(dest);
        arktest.assertEQ(result, 'a😃🤞b');
    }
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0300
 * @tc.name       : test_Decode_Test_003
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 1
 */
function test_Decode_Test_003() {
    let that = new util.TextEncoder('utf-16le')
    let result = that.encodeInto('HelLoWorLd123?')
    let decoder = new util.StringDecoder('utf-16le')
    const decoded1 = decoder.write(result)
    arktest.assertEQ(decoded1, 'HelLoWorLd123?')
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0900
 * @tc.name       : testArrayScenarios0600
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 1
 */
function testArrayScenarios0600() {
    const TAG = 'testArrayScenarios0600 == '
    let encodingArr =
    ['utf-8', 'UTF-8', 'GBK', 'GB2312', 'GB18030', 'gb18030', 'ibm866', 'iso-8859-1', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13', 'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u',
        'macintosh', 'windows-874', 'windows-1250', 'windows-1251', 'windows-1252', 'windows-1253', 'windows-1254',
        'windows-1255', 'windows-1256', 'windows-1257', 'windows-1258', 'x-mac-cyrillic', 'gbk', 'big5',
        'euc-jp', 'iso-2022-jp', 'shift_jis', 'euc-kr']
    let uint8Array = new Uint8Array([0, 0, 0, 0, 0, 72, 101, 108, 108, 111, 87, 111, 114, 108, 100, 49, 50, 51, 63])

    for(let item of encodingArr) {
        let decoder = new util.StringDecoder(item)
        let decoded1 = decoder.write(uint8Array)
        let urlcode = encodeURI(decoded1)
        arktest.assertEQ(urlcode, '%00%00%00%00%00HelloWorld123?')
    }

    let decoder1 = new util.StringDecoder('utf-16be')
    let decodedBe = decoder1.write(uint8Array)
    let urlcodeBe = encodeURI(decodedBe)
    arktest.assertEQ(urlcodeBe, '%00%00H%E6%95%AC%E6%B1%AF%E5%9D%AF%E7%89%AC%E6%90%B1%E3%88%B3')

    let decoder2 = new util.StringDecoder('utf-16le')
    let decodedLe = decoder2.write(uint8Array)
    let urlcodeLe = encodeURI(decodedLe)
    arktest.assertEQ(urlcodeLe, '%00%00%E4%A0%80%E6%B1%A5%E6%BD%AC%E6%BD%97%E6%B1%B2%E3%85%A4%E3%8C%B2')
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_1000
 * @tc.name       : testArrayScenarios0700
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 1
 */
function testArrayScenarios0700() {
    const TAG = 'testArrayScenarios0700 == '
    let encodingArr =
    ['utf-8', 'UTF-8', 'GBK', 'GB2312', 'GB18030', 'gb18030', 'ibm866', 'iso-8859-1', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13', 'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u',
        'macintosh', 'windows-874', 'windows-1250', 'windows-1251', 'windows-1252', 'windows-1253', 'windows-1254',
        'windows-1255', 'windows-1256', 'windows-1257', 'windows-1258', 'x-mac-cyrillic', 'gbk', 'big5',
        'euc-jp', 'iso-2022-jp', 'shift_jis', 'euc-kr']
    let uint8Array = new Uint8Array([72, 101, 108, 108, 111, 87, 111, 114, 108, 100, 49, 50, 51, 63, 0, 0, 0, 0, 0])

    for(let item of encodingArr) {
        let decoder = new util.StringDecoder(item)
        let decoded1 = decoder.end(uint8Array.slice(14))
        let urlcode = encodeURI(decoded1)
        arktest.assertEQ(urlcode, '%00%00%00%00%00')
    }

    let decoder1 = new util.StringDecoder('utf-16be')
    let decodedBe = decoder1.end(uint8Array.slice(14))
    let urlcodeBe = encodeURI(decodedBe)
    arktest.assertEQ(urlcodeBe, '%00%00%EF%BF%BD')

    let decoder2 = new util.StringDecoder('utf-16le')
    let decodedLe = decoder2.end(uint8Array.slice(14))
    let urlcodeLe = encodeURI(decodedLe)
    arktest.assertEQ(urlcodeLe, '%00%00%EF%BF%BD')
}
