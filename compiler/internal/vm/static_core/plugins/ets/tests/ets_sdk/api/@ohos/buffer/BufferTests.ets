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

import buffer from "@ohos.buffer"

class Foo {
    valueOf1() {
      return 'this is a test';
    }

    toString1() {
      return 'this is a test';
    }
}

class Person {
    name: string;
    age: number;
    constructor(name: string, age: number) {
      this.name = name;
      this.age = age;
    }
}

function testToString() {
    {
        let buf1 = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf1.writeInt8(i + 97, i);
        }
        const res = buf1.toString('base64'); // YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=
        arktest.assertEQ(res, "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=");
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 30); // '';
        arktest.assertEQ(str, '')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 5, 3); // ''
        arktest.assertEQ(str, '');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', -1); // abcdefghijklmnopqrstuvwxyz
        arktest.assertEQ(str, 'abcdefghijklmnopqrstuvwxyz');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', Infinity.toInt()); // ''
        arktest.assertEQ(str, '');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', -Infinity.toInt()); // abcdefghijklmnopqrstuvwxyz
        arktest.assertEQ(str, 'abcdefghijklmnopqrstuvwxyz')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', undefined, Infinity.toInt()) // abcdefghijklmnopqrstuvwxyz
        arktest.assertEQ(str, 'abcdefghijklmnopqrstuvwxyz')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 0, -Infinity.toInt()); // ''
        arktest.assertEQ(str, '')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', undefined, -1); // ''
        arktest.assertEQ(str, '')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, -1); // ''
        arktest.assertEQ(res, '')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, Infinity.toInt()); // aGVsbG8=
        arktest.assertEQ(res, 'aGVsbG8=')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, -Infinity.toInt()); // ''
        arktest.assertEQ(res, '')
    }
    {
        let array = new Uint8Array([252, 11, 3, 67, 237, 118, 91, 177, 43]);
        let buf = buffer.from(array);
        let strUrl = buf.toString("base64url");
        let str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_AsDQ-12W7Er");
        arktest.assertEQ(str, "/AsDQ+12W7Er");

        array = new Uint8Array([2, 192, 254, 253, 5, 132, 69]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(String(strUrl), 'AsD-_QWERQ');
        arktest.assertEQ(str, "AsD+/QWERQ==");

        array = new Uint8Array([215, 109, 211, 97, 72, 142, 167, 241]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "123TYUiOp_E");
        arktest.assertEQ(str, "123TYUiOp/E=");

        array = new Uint8Array([252]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_A");
        arktest.assertEQ(str, "/A==");

        array = new Uint8Array([252, 97]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_GE");
        arktest.assertEQ(str, "/GE=");

        array = new Uint8Array([252, 97, 142]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_GGO");
        arktest.assertEQ(str, "/GGO");
    }
}

function testWriteUInt8Error() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt8(0x0304, 1);}, 'The value of "value" is out of range. It must be >= 0 and <= 255. Received value is: 772');
}

function testWriteInt32BEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeInt32BE(3405691582, 0);}, 'The value of "value" is out of range. It must be >= -2147483648 and <= 2147483647. Received value is: 3405691582');
}

function testWriteInt32LEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeInt32LE(3405691582, 0);}, 'The value of "value" is out of range. It must be >= -2147483648 and <= 2147483647. Received value is: 3405691582');
}

function testWriteInt16LEError() {
    let buf = buffer.alloc(5);
    arktest.expectError(() => {buf.writeInt16LE(48879, 0);}, 'The value of "value" is out of range. It must be >= -32768 and <= 32767. Received value is: 48879');
}

function testWriteInt16BEError() {
    let buf = buffer.alloc(5);
    arktest.expectError(() => {buf.writeInt16BE(48879, 0);}, 'The value of "value" is out of range. It must be >= -32768 and <= 32767. Received value is: 48879');
}

function testWriteInt8Error() {
    let buf = buffer.alloc(5);
    arktest.expectError(() => {buf.writeInt8(0x0304, 0);}, 'The value of "value" is out of range. It must be >= -128 and <= 127. Received value is: 772');
}

function testWriteUInt32BEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt32BE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 4294967295. Received value is: -2');
}

function testWriteUInt32LEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt32LE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 4294967295. Received value is: -2');
}

function testWriteUInt16BEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt16BE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 65535. Received value is: -2');
}

function testWriteUInt16LEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt16LE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 65535. Received value is: -2');
}

function testAlloc0016() {
    let str = 'Parameter error. The type of "size" must be number and the value cannot be ' +
             'negative. Received value is: -5';
    arktest.expectError(() => {let buf = buffer.alloc(-5);}, str);
}

function testAllocUninitializedFromPool0024() {
    let encodeArr: Array< String > = new Array < String > ('utf8', 'utf-8', 'ucs2', 'ucs-2', 'ascii', 'latin1', 'binary', 'utf16le', 'base64', 'base64url', 'hex')
    encodeArr.forEach((encode )=> {
      let buf = buffer.alloc(10).fill("ab$#", 0, undefined, encode as buffer.BufferEncoding);
      arktest.assertEQ(buf.length, 10)
    });
}

function testAllocUninitializedFromPool0026() {
    let str = 'Parameter error. The type of "size" must be number and the value cannot be ' +
              'negative. Received value is: -5'
    arktest.expectError(() => {let buf = buffer.allocUninitializedFromPool(-5);}, str);
}

function testByteLength0033() {
    let encodeArr: Array< String > = new Array < String > ('utf8', 'utf-8', 'ucs2', 'ucs-2', 'ascii', 'latin1', 'binary', 'utf16le', 'base64', 'base64url', 'hex')
    let result:Array<number> = new Array<number>(4, 4, 8, 8, 4, 4, 4, 8, 3, 3, 2)
    for (let i = 0, len = encodeArr.length; i< len; i++) {
      let byteLen = buffer.byteLength("abcd", encodeArr[i] as buffer.BufferEncoding);
      arktest.assertEQ(byteLen, result[i])
    }
}

function testConcat0074() {
    let str = 'The value of "totalLength" is out of range. It must be >= 0 and <= 2147483647. Received value is: -1';
    let buf1 = buffer.from("123$");
    let buf2 = buffer.from("*35");
    arktest.expectError(() => {let buf = buffer.concat([buf1, buf2], -1);}, str);
}

function testFill0093() {
    let str = 'The value of "offset" is out of range. It must be >= 0 and <= 2147483647. Received value is: -1';
    arktest.expectError(() => {let buf = buffer.alloc(3).fill("$*$", -1);}, str);
}

function testFill0094() {
    let str = 'The value of "end" is out of range. It must be >= 0 and <= 3. Received value is: 5';
    arktest.expectError(() => {let buf = buffer.alloc(3).fill("$*$", 0, 5);}, str);
}

function testWrite0105() {
    let str = 'The value of "offset" is out of range. It must be >= 0 and <= 7. Received value is: -1';
    let buf = buffer.alloc(8);
    arktest.expectError(() => {let offset = buf.write("abcde", -1);}, str);
}

function testCopy0144() {
    let str = 'The value of "targetStart" is out of range. It must be >= 0. Received value is: -1';
    let buf1 = buffer.from("123656");
    let buf2 = buffer.from("1235");
    arktest.expectError(() => {let num = buf1.copy(buf2, -1);}, str);
}

function testSwap160201() {
    let str = 'The buffer size must be a multiple of 16-bits';
    let buf1 = buffer.from("132");
    arktest.expectError(() => {buf1.swap16();}, str);
}

function testswap320211() {
    let str = 'The buffer size must be a multiple of 32-bits';
    let buf1 = buffer.from("132");
    arktest.expectError(() => {buf1.swap32();}, str);
}

function testSwap640221() {
    let str = 'The buffer size must be a multiple of 64-bits';
    let buf1 = buffer.from("1234567");
    arktest.expectError(() => {buf1.swap64();}, str);
}

function testAllocUninitialized0782() {
    let str = 'Parameter error. The type of "size" must be number and the value cannot be ' +
          'negative. Received value is: -5'
    arktest.expectError(() => {let buf = buffer.allocUninitialized(-5);}, str);
}

function testCopy0148() {
    let uint8array = new Uint8Array(10).fill(0);
    let buf = buffer.from([1, 2, 3, 4, 5, 6, 7]);
    let num = buf.copy(uint8array, 0, 3);
    arktest.assertEQ(num, 4);
    let str = uint8array.toString();
    arktest.assertEQ(str, '4,5,6,7,0,0,0,0,0,0');
}

function testCopy0149() {
    let uint8array = new Uint8Array(10).fill(0);
    let buf = buffer.from([1, 2, 3, 4, 5, 6, 7]);
    let num = buf.copy(uint8array, 0, 3, 255);
    arktest.assertEQ(num , 4);
    let str = uint8array.toString();
    arktest.assertEQ(str, '4,5,6,7,0,0,0,0,0,0');
}

function testReadUIntBE0631() {
    let str = 'The value of "offset" is out of range. It must be >= 0 and <= 1. Received value is: 2';
    let buf = buffer.allocUninitializedFromPool(4);
    buf.writeUIntBE(0x13141516, 0, 4);
    arktest.expectError(() => {let ref = Double(buf.readUIntBE(2, 3)).toString(16);}, str);
}

function testReadUIntLE0641() {
    let str = 'The value of "offset" is out of range. It must be >= 0 and <= 1. Received value is: 2';
    let buf = buffer.allocUninitializedFromPool(4);
    buf.writeUIntLE(0x13141516, 0, 4);
    arktest.expectError(() => {let ref = Double(buf.readUIntLE(2, 3)).toString(16);}, str);
}

function testReadIntBE0651() {
    let str = 'The value of "offset" is out of range. It must be >= 0 and <= 1. Received value is: 2';
    let buf = buffer.allocUninitializedFromPool(6);
    buf.writeIntBE(0x123456789011, 0, 6);
    arktest.expectError(() => {let ref = Double(buf.readIntBE(2, 5)).toString(16);}, str);
}

function testReadIntLE0661() {
    let str = 'The value of "offset" is out of range. It must be >= 0 and <= 1. Received value is: 2';
    let buf = buffer.allocUninitializedFromPool(6);
    buf.writeIntLE(0x123456789011, 0, 6);
    arktest.expectError(() => {let ref = Double(buf.readIntLE(2, 5)).toString(16);}, str);
}

function testBufferCopy0012() {
    let str = 'The value of "sourceStart" is out of range. It must be >= 0. Received value is: -2147483647';
    let buf1 = buffer.allocUninitializedFromPool(26);
    let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
    for (let i = 0; i < 26; i++) {
        buf1.writeInt8(i + 97, i);
    }
    arktest.expectError(() => {buf1.copy(buf2, undefined, -Infinity.toInt());}, str);
}

function testBufferCopy0018() {
    let str = 'The value of "sourceEnd" is out of range. It must be >= 0. Received value is: -2147483647';
    let buf1 = buffer.allocUninitializedFromPool(26);
    let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
    for (let i = 0; i < 26; i++) {
        buf1.writeInt8(i + 97, i);
    }
    arktest.expectError(() => {buf1.copy(buf2, undefined, undefined, -Infinity.toInt());}, str);
}

function testBufferReadBigInt64BE0004() {
    let str = 'The value of "offset" is out of range. It must be >= 0 and <= 0. Received value is: -1';
    let buf = buffer.alloc(8)
    arktest.expectError(() => {buf.readBigInt64BE(-1);}, str);
}

function testIsBuffer0002() {
    let obj = new Object();
    let flag = buffer.isBuffer(obj);
    arktest.assertEQ(String(flag), 'false');
}

function testIsEncoding0001() {
    let encodeArr = ['utf8', 'utf-8', 'ucs2', 'ucs-2', 'ascii', 'latin1', 'binary',
        'utf16le', 'base64', 'base64url', 'hex'];
    for (const encode of encodeArr) {
        let flag = buffer.isEncoding(encode);
        arktest.assertEQ(String(flag), 'true');
    }
}

function testSlice0002() {
    let blob: buffer.Blob = new buffer.Blob(['a', 'b', 'c']);
    let result = blob.slice(1);
    arktest.assertEQ(result.size, 2);
}

function testSlice0003() {
    let blob: buffer.Blob = new buffer.Blob(['a', 'b', 'c', "d"]);
    let result = blob.slice(1, 2);
    arktest.assertEQ(result.size, 1);
}

function testSlice0004() {
    let blob: buffer.Blob = new buffer.Blob(['a', 'b', 'c']);
    let result = blob.slice(1, 2, "Number");
    arktest.assertEQ(result.size, 1);
}

function testBufferBufferCompare0004() {
    let str = 'The value of "targetStart" is out of range. It must be >= 0 and <= 2147483647. Received value is: -1';
    const buf1 = buffer.from([1, 2, 3]);
    const buf2 = buffer.from([3, 2, 1]);
    arktest.expectError(() => {const result = buf1.compare(buf2, -1);}, str);
}

function testBufferEquals0002() {
    let buf = buffer.from('你好');
    arktest.assertEQ(buf.includes('你好', undefined, 'utf-8'),  true);
    arktest.assertEQ(buf.includes('hello', undefined, 'utf-8'),  false);
}

function testBufferCopy0005() {
    let buf1 = buffer.allocUninitializedFromPool(26);
    let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
    for (let i = 0; i < 26; i++) {
      buf1.writeInt8(i + 97, i);
    }
    buf1.copy(buf2, Infinity.toInt());
    arktest.assertEQ(buf2.toString(),  '!!!!!!!!!!!!!!!!!!!!!!!!!!');
}

function testAlloc30040() {
    let buf = buffer.alloc(10, '');
    arktest.assertEQ(buf.length, 10);
}

function testAlloc30050() {
    let buf = buffer.alloc(20).fill('');
    arktest.assertEQ(buf.length, 20);
}

function testBufferAlloc0009() {
    let buf9 = buffer.alloc(3, '');
    arktest.assertEQ(buf9.length, 3);
    let pair = buf9.entries();
    let next = pair.next();
    let isOk = true;
    while (!next.done) {
        String()
        isOk = Boolean(isOk&&(String(next?.value?.[1]).localeCompare('')==0));
        next = pair.next();
    }
    arktest.assertEQ(isOk, false);
}

function testfrom0266() {
    let buf12 = buffer.from("AsD-_QWEr", "base64url")
    let str = buf12.toString("hex");
    arktest.assertEQ(str, '02c0fefd0584');
}


function testfrom026601() {
    let buf14 = buffer.from("_AsDQ-12W7Er_", "base64url")
    let str = buf14.toString("hex");
    arktest.assertEQ(str, 'fc0b0343ed765bb12b');
}

function testIndexOf0008() {
    let buf = buffer.from("123");
    let index = buf.indexOf("");
    arktest.assertEQ(index, 0);
}

function testfrom0789() {
    let sharedArr = new ArrayBuffer(10)
    let uint8Arr = new Uint8Array(sharedArr);
    uint8Arr[9] = 9;
    let bufObj = buffer.from(sharedArr, 5, 5);
    uint8Arr[1] = 10;
    arktest.assertEQ(bufObj.length, 5);
}
function testfrom0790() {
    let sharedArr = new ArrayBuffer(1000)
    let bufObj = buffer.from(sharedArr, 5, 1);
    arktest.assertEQ(bufObj.length, 1);
}

function testfrom0791() {
    let sharedArr = new ArrayBuffer(10)
    let bufObj = buffer.from(sharedArr, 3, 6);
    arktest.assertEQ(bufObj.length, 6);
}

function testFrom0013() {
    let str = 'The value of "byteOffset" is out of range. It must be >= 0 and <= 5. Received value is: 6';
    let arrayBuffer = new ArrayBuffer(5);
    let array = new Int8Array(arrayBuffer);
    array[0] = 1;
    array[1] = 2;
    array[2] = 3;
    array[3] = 4;
    array[4] = 5;
    arktest.expectError(() => {buffer.from(arrayBuffer, 6, 1);}, str);
}

function testBufferFrom10008() {
    const ab1 = new ArrayBuffer(8);
    const res = buffer.from(ab1, 4, 0);
    arktest.assertEQ(res.length, 0);
}

function testBufferFrom10009() {
    let str = 'The value of "length" is out of range. It must be >= 0 and <= 8. Received value is: -1';
    const ab1 = new ArrayBuffer(8);
    arktest.expectError(() => {buffer.from(ab1, 0, -1);}, str);
}

function testBufferFrom10010() {
    let str = 'The value of "length" is out of range. It must be >= 0 and <= 8. Received value is: 2147483647';
    const ab1 = new ArrayBuffer(8);
    arktest.expectError(() => {buffer.from(ab1, 0, Infinity.toInt());}, str);
}

function testWrite0009() {
    let str = 'The buffer length is 0, and writing data is not allowed';
    let buf = buffer.alloc(0);
    arktest.expectError(() => {let offset = buf.write("");}, str);
}

function testfrom0792() {
    let ab = new ArrayBuffer(100);
    let buf = buffer.from(ab, (1.2).toInt(), (1.2).toInt());
    let res = buf.toString('hex');
    arktest.assertEQ(res, '00');
}

function testfrom0793() {
    let ab = new ArrayBuffer(100);
    let buf = buffer.from(ab, 1, (1.2).toInt());
    let res = buf.toString('hex');
    arktest.assertEQ(res, '00');
}

function testfrom0794() {
    let ab = new ArrayBuffer(100);
    let buf = buffer.from(ab, (1.2).toInt(), 1);
    let res = buf.toString('hex');
    arktest.assertEQ(res, '00');
}

function testfrom0795() {
    let ab = new ArrayBuffer(100);
    let buf = buffer.from(ab, (9.9).toInt(), (9.9).toInt());
    let res = buf.toString('hex');
    arktest.assertEQ(res, '000000000000000000');
}

function testfrom0796() {
    let ab = new ArrayBuffer(100);
    let buf = buffer.from(ab, (2.9).toInt() , (2.9).toInt());
    let res = buf.toString('hex');
    arktest.assertEQ(res, '0000');
}

function testTranscode0084() {
    let str = 'Parameter error. The type of "toEnc" must be BufferEncoding. the toEnc 0 is unknown';
    let buf1 = buffer.from("测试");
    arktest.expectError(() => {let buf = buffer.transcode(buf1, "utf8", '0');}, str);
}

function testfrom0783() {
    const buf = buffer.from([10]);
    arktest.assertEQ(buf.length, 1);
}

function testFrom30140() {
    let str = 'Parameter error. The type of "input" must be Buffer or ArrayBuffer, Array, Array-like';
    let obj =  new Map<string, string>()
    arktest.expectError(() => {let buf1 = buffer.from(obj, 0, 5);}, str);
}

function testBufferFrom30009() {
    let str = 'Parameter error. The type of "input" must be Buffer or ArrayBuffer, Array, Array-like';
    const person = new Person("Alice", 30);
    arktest.expectError(() => {
        buffer.from(person, 11, 0);
        buffer.from(person, 11, -1);
        buffer.from(person, 11, Infinity.toInt());
        buffer.from(person, 11, -Infinity.toInt());
    }, str);
}

function testFrom0015() {
    let buf = new Uint8Array([97, 98]);
    const buf1 = buffer.from(buf, -1, -1);
    let str = buf1.toString();
    arktest.assertEQ(str, 'ab');
}

function testBufferFrom30005() {
    let str = 'Parameter error. The type of "encoding" must be BufferEncoding. the encoding test is unknown';
    arktest.expectError(() => {let buf = buffer.from(new String('hello world'), 'test', 1);}, str);
}

function testBufferBufferCompare0005() {
    const buf1 = buffer.from([1, 2, 3]);
    const buf2 = buffer.from([3, 2, 1]);
    const result = buf1.compare(buf2, Infinity.toInt());
    arktest.assertEQ(result, 1);

}

function testBufferBufferCompare0017() {
    const buf1 = buffer.from([1, 2, 3]);
    const buf2 = buffer.from([1, 2, 3]);
    const result = buf1.compare(buf2, undefined, undefined, Infinity.toInt());
    arktest.assertEQ(result, -1);
}

function testFrom0009() {
    let str = 'Parameter error. The type of "input" must be Buffer or ArrayBuffer, Array, Array-like';
    let foo = new Foo();
    arktest.expectError(() => {const buf = buffer.from(foo, 'utf8', 0);}, str);
}

function testFrom0014() {
    let buf = new Uint8Array([97, 98]);
    const buf1 = buffer.from(buf);
    let str = buf1.toString();
    arktest.assertEQ(str, 'ab');
}

function testKeys() {
    const arr = new Uint8Array(4);
    arr[0] = 97;
    arr[1] = 98;
    arr[2] = 99;
    arr[3] = 100;
    let buf = buffer.from(arr.buffer, 1);
    let keys = buf.keys();
    let tmp = 0;
    for (const key of keys) {
        arktest.assertEQ(String(key),String(tmp));
        tmp++;
    }
}

function testValues() {
    const arr = new Uint8Array(4);
    arr[0] = 97;
    arr[1] = 98;
    arr[2] = 99;
    arr[3] = 100;
    let buf = buffer.from(arr.buffer, 1);
    let values = buf.values();
    let tmp = arr[1];
    for (const value of values) {
        arktest.assertEQ(String(value),String(tmp));
        tmp++;
    }
}


function main(): int {
    const suite = new arktest.ArkTestsuite("Buffer API tests")
    suite.addTest("Test buffer toString", testToString)
    suite.addTest("Test buffer WriteUInt8 throw value error", testWriteUInt8Error)
    suite.addTest("Test buffer WriteInt32BE throw value error", testWriteInt32BEError)
    suite.addTest("Test buffer WriteInt32LE throw value error", testWriteInt32LEError)
    suite.addTest("Test buffer WriteInt16LE throw value error", testWriteInt16LEError)
    suite.addTest("Test buffer WriteInt16BE throw value error", testWriteInt16BEError)

    suite.addTest("Test buffer WriteInt8 throw value error", testWriteInt8Error)
    suite.addTest("Test buffer WriteUInt32BE throw value error", testWriteUInt32BEError)
    suite.addTest("Test buffer WriteUInt32LE throw value error", testWriteUInt32LEError)
    suite.addTest("Test buffer WriteUInt16BE throw value error", testWriteUInt16BEError)
    suite.addTest("Test buffer WriteUInt16LE throw value error", testWriteUInt16LEError)
    suite.addTest("Test buffer testAlloc0016", testAlloc0016)
    suite.addTest("Test buffer testAllocUninitializedFromPool0024", testAllocUninitializedFromPool0024)
    suite.addTest("Test buffer testAllocUninitializedFromPool0026", testAllocUninitializedFromPool0026)
    suite.addTest("Test buffer testByteLength0033", testByteLength0033)
    suite.addTest("Test buffer testConcat0074", testConcat0074)
    suite.addTest("Test buffer testFill0093", testFill0093)
    suite.addTest("Test buffer testFill0094", testFill0094)
    suite.addTest("Test buffer testWrite0105", testWrite0105)
    suite.addTest("Test buffer testCopy0144", testCopy0144)
    suite.addTest("Test buffer testSwap160201", testSwap160201)
    suite.addTest("Test buffer testswap320211", testswap320211)
    suite.addTest("Test buffer testSwap640221", testSwap640221)
    suite.addTest("Test buffer testAllocUninitialized0782", testAllocUninitialized0782)
    suite.addTest("Test buffer testCopy0148", testCopy0148)
    suite.addTest("Test buffer testCopy0149", testCopy0149)
    suite.addTest("Test buffer testBufferCopy0012", testBufferCopy0012)
    suite.addTest("Test buffer testBufferCopy0018", testBufferCopy0018)
    suite.addTest("Test buffer testBufferReadBigInt64BE0004", testBufferReadBigInt64BE0004)
    suite.addTest("Test buffer testIsBuffer0002", testIsBuffer0002)
    suite.addTest("Test buffer testIsEncoding0001", testIsEncoding0001)
    suite.addTest("Test buffer testReadUIntBE0631", testReadUIntBE0631)
    suite.addTest("Test buffer testReadUIntLE0641", testReadUIntLE0641)
    suite.addTest("Test buffer testReadIntBE0651", testReadIntBE0651)
    suite.addTest("Test buffer testReadIntLE0661", testReadIntLE0661)
    suite.addTest("Test buffer testSlice0002", testSlice0002)
    suite.addTest("Test buffer testSlice0003", testSlice0003)
    suite.addTest("Test buffer testSlice0004", testSlice0004)
    suite.addTest("Test buffer testBufferBufferCompare0004", testBufferBufferCompare0004)
    suite.addTest("Test buffer testBufferEquals0002", testBufferEquals0002)
    suite.addTest("Test buffer testBufferCopy0005", testBufferCopy0005)
    suite.addTest("Test buffer testAlloc30040", testAlloc30040)
    suite.addTest("Test buffer testAlloc30050", testAlloc30050)
    suite.addTest("Test buffer testBufferAlloc0009", testBufferAlloc0009)
    suite.addTest("Test buffer testfrom0266", testfrom0266)
    suite.addTest("Test buffer testfrom026601", testfrom026601)
    suite.addTest("Test buffer testIndexOf0008", testIndexOf0008)
    suite.addTest("Test buffer testfrom0789", testfrom0789)
    suite.addTest("Test buffer testfrom0790", testfrom0790)
    suite.addTest("Test buffer testfrom0791", testfrom0791)
    suite.addTest("Test buffer testFrom0013", testFrom0013)
    suite.addTest("Test buffer testBufferFrom10008", testBufferFrom10008)
    suite.addTest("Test buffer testBufferFrom10009", testBufferFrom10009)
    suite.addTest("Test buffer testBufferFrom10010", testBufferFrom10010)
    suite.addTest("Test buffer testWrite0009", testWrite0009)
    suite.addTest("Test buffer testfrom0792", testfrom0792)
    suite.addTest("Test buffer testfrom0793", testfrom0793)
    suite.addTest("Test buffer testfrom0794", testfrom0794)
    suite.addTest("Test buffer testfrom0795", testfrom0795)
    suite.addTest("Test buffer testfrom0796", testfrom0796)
    suite.addTest("Test buffer testTranscode0084", testTranscode0084)
    suite.addTest("Test buffer testfrom0783", testfrom0783)
    suite.addTest("Test buffer testFrom30140", testFrom30140)
    suite.addTest("Test buffer testFrom0015", testFrom0015)
    suite.addTest("Test buffer testBufferFrom30005", testBufferFrom30005)
    suite.addTest("Test buffer testBufferFrom30009", testBufferFrom30009)
    suite.addTest("Test buffer testBufferBufferCompare0005", testBufferBufferCompare0005)
    suite.addTest("Test buffer testBufferBufferCompare0017", testBufferBufferCompare0017)
    suite.addTest("Test buffer testFrom0009", testFrom0009)
    suite.addTest("Test buffer testFrom0014", testFrom0014)
    suite.addTest("Test buffer testKeys", testKeys)
    suite.addTest("Test buffer testValues", testValues)
    return suite.run()
}