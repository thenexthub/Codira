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
import buffer from "@ohos.buffer";
import { BusinessError } from "@ohos.base";

function testCopy0160() {
	let buf = buffer.alloc(10);
	for (let i = 0; i < 10; i++) {
		buf[i] = i + 97;
	}
	let len = buf.copy(buf, 2);
	arktest.assertEQ(String(len), '8');
	arktest.assertEQ(buf.toString(), 'ababcdefgh');
};

function testCopy0161() {
	let buf = buffer.alloc(10);
	for (let i = 0; i < 10; i++) {
		buf[i] = i + 97;
	}
	let len = buf.copy(buf, 8);
	arktest.assertEQ(String(len), '2');
	arktest.assertEQ(buf.toString(), 'abcdefghab');
};

function testBufferCopy0002() {
	const buf1 = buffer.alloc(26);
	for (let i = 0; i < 26; i++) {
		buf1[i] = i + 97;
	}
	const buf2 = new Uint8Array(26);
	buf1.copy(buf2);
	for (let i = 0; i < 26; i++) {
		arktest.assertEQ(buf2[i], 97 + i);
	}
};

function testBufferCopy0006() {
	let buf1 = buffer.allocUninitializedFromPool(26);
	let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
	for (let i = 0; i < 26; i++) {
		buf1.writeInt8(i + 97, i);
	}
	arktest.expectError(() => {
		buf1.copy(buf2, (-Infinity).toInt());
	}, 'The value of "targetStart" is out of range. It must be >= 0. Received value is: -2147483648');
}

function testBufferCopy0012() {
	let buf1 = buffer.allocUninitializedFromPool(26);
	let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
	for (let i = 0; i < 26; i++) {
		buf1.writeInt8(i + 97, i);
	}
	arktest.expectError(() => {
		buf1.copy(buf2, undefined, (-Infinity).toInt());
	}, 'The value of "sourceStart" is out of range. It must be >= 0. Received value is: -2147483648');
}
function testCopy0148() {
	let uint8array = new Uint8Array(10).fill(0);
	let buf = buffer.from([1, 2, 3, 4, 5, 6, 7]);
	let num = buf.copy(uint8array, 0, 3);
	arktest.assertEQ(String(num), '4');
	let str = uint8array.toString();
	arktest.assertEQ(str, "4,5,6,7,0,0,0,0,0,0");
}
function testCopy0149() {
	let uint8array = new Uint8Array(10).fill(0);
	let buf = buffer.from([1, 2, 3, 4, 5, 6, 7]);
	let num = buf.copy(uint8array, 0, 3, 255);
	arktest.assertEQ(String(num), '4');
	let str = uint8array.toString();
	arktest.assertEQ(str, "4,5,6,7,0,0,0,0,0,0");
}

function testBufferCopy0018() {
	let buf1 = buffer.allocUninitializedFromPool(26);
	let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
	for (let i = 0; i < 26; i++) {
		buf1.writeInt8(i + 97, i);
	}

	arktest.expectError(() => {
		buf1.copy(buf2, undefined, undefined, (-Infinity).toInt());
	}, 'The value of "sourceEnd" is out of range. It must be >= 0. Received value is: -2147483648');
}


function testBufferWrite0004() {
	arktest.expectError(() => {
		let buf = buffer.alloc(5);
		let len = buf.write("abcd", -1);
	}, 'The value of "offset" is out of range. It must be >= 0 and <= 4. Received value is: -1');
}

function testBufferWrite0005() {
	arktest.expectError(() => {
		let buf = buffer.alloc(10);
		const result = buf.write("test", Infinity.toInt());
	}, 'The value of "offset" is out of range. It must be >= 0 and <= 9. Received value is: 2147483647');
}

function testBufferWrite0006() {
	arktest.expectError(() => {
		let buf = buffer.alloc(10);
		const result = buf.write("test", (-Infinity).toInt());
	}, 'The value of "offset" is out of range. It must be >= 0 and <= 9. Received value is: -2147483648');


}
function testBufferWrite0020() {
	arktest.expectError(() => {
		let buf = buffer.alloc(5);
		const res = buf.write("abc", 6, 1, 'utf8');
	}, 'The value of "offset" is out of range. It must be >= 0 and <= 4. Received value is: 6');
}

function testBufferWrite0024() {
	arktest.expectError(() => {
		let buf = buffer.alloc(10);
		const res = buf.write("abc", Infinity.toInt(), Infinity.toInt(), 'utf8');
	}, 'The value of "offset" is out of range. It must be >= 0 and <= 9. Received value is: 2147483647');
}

function testBufferWrite0026() {
	arktest.expectError(() => {
		let buf = buffer.alloc(10);
		const res = buf.write("abc", (-Infinity).toInt(), (-Infinity).toInt(), 'utf8');
	}, 'The value of "offset" is out of range. It must be >= 0 and <= 9. Received value is: -2147483648');
}

function testBufferConcat0005() {
	let buf1 = buffer.from("1234");
	let buf2 = buffer.from("abcd");
	arktest.expectError(() => {
		buffer.concat([buf1, buf2], -1);
	}, 'The value of "totalLength" is out of range. It must be >= 0 and <= 2147483647. Received value is: -1');
}

function testBufferConcat0006() {
	let buf1 = buffer.from("1234");
	let buf2 = buffer.from("abcd");
	arktest.expectError(() => {
		buffer.concat([buf1, buf2], Infinity.toInt() + 1);
	}, 'The value of "totalLength" is out of range. It must be >= 0 and <= 2147483647. Received value is: -2147483648');
}

function testBufferConcat0007() {
	let buf1 = buffer.from("1234");
	let buf2 = buffer.from("abcd");
	arktest.expectError(() => {
		buffer.concat([buf1, buf2], (-Infinity).toInt());
	}, 'The value of "totalLength" is out of range. It must be >= 0 and <= 2147483647. Received value is: -2147483648');
}
function testBufferConcat0009() {
	const arr1 = new Uint8Array([104, 101, 108, 108, 111]);
	const arr2 = new Uint8Array([119, 111, 114, 108, 100]);
	const buf1 = buffer.from(arr1);
	const buf2 = buffer.from(arr2);

	arktest.expectError(() => {
		const concatBuf = buffer.concat([buf1, buf2], -1);
	}, 'The value of "totalLength" is out of range. It must be >= 0 and <= 2147483647. Received value is: -1');
}


function testBufferFill0007() {
	const buf = buffer.alloc(6);
	arktest.expectError(() => {
		buf.fill('D', -1);
	}, 'The value of "offset" is out of range. It must be >= 0 and <= 2147483647. Received value is: -1');
}

function testBufferFill0016() {
	const buf = buffer.alloc(5);
	buf.fill('A', 0, 5, 'base64');
	arktest.assertEQ(buf.toString('base64'), 'AAAAAAA=');
}

function testBufferFill0017() {
	const buf = buffer.alloc(5);
	buf.fill('a', 0, 5, 'utf8');
	arktest.assertEQ(buf.toString('utf8'), 'aaaaa');
}

function testBufferFill0018() {
	const arr1 = new Uint8Array([97, 98, 99, 100, 101, 102, 103])
  	let buf = buffer.from(arr1.buffer, 1);
	buf.fill('0', 2, 4, 'utf8');
	arktest.assertEQ(buf.toString('utf8'), 'bc00fg');
}

function testFill0091() {
	const buf1 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'hex');
	let str = buf1.toString('hex');
	arktest.assertEQ(str, "f1f1f1f1f1f1f1f1f1f1");

	let buf2 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'ascii');
	str = buf2.toString("hex");
	arktest.assertEQ(str, "46311846474631184647");

	let buf3 = buffer.alloc(10).fill('F1FG', 0, 10, 'base64');
	str = buf3.toString("hex");
	arktest.assertEQ(str, "17514617514617514617");

	let buf4 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'binary');
	str = buf4.toString("hex");
	arktest.assertEQ(str, "46311846474631184647");

	let buf5 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'latin1');
	str = buf5.toString("hex");
	arktest.assertEQ(str, "46311846474631184647");

	let buf6 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'ucs2');
	str = buf6.toString("hex");
	arktest.assertEQ(str, "46003100185246004700");

	let buf7 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'utf8');
	str = buf7.toString("hex");
	arktest.assertEQ(str, "4631e5889846474631e5");
}


function testBufferTranscode0003() {
	let buf = buffer.from('hello world');
	let newBuf = buffer.transcode(buf, 'utf-8', 'base64');
	arktest.assertEQ(newBuf.length, 8);
}

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testCopy0160", testCopy0160);
	suite.addTest("testCopy0161", testCopy0161);
	suite.addTest("testBufferCopy0002", testBufferCopy0002);
	suite.addTest("testBufferCopy0006", testBufferCopy0006);
	suite.addTest("testBufferCopy0012", testBufferCopy0012);
	suite.addTest("testCopy0148", testCopy0148);
	suite.addTest("testCopy0149", testCopy0149);
	suite.addTest("testBufferCopy0018", testBufferCopy0018);
	suite.addTest("testBufferWrite0004", testBufferWrite0004);
	suite.addTest("testBufferWrite0005", testBufferWrite0005);
	suite.addTest("testBufferWrite0006", testBufferWrite0006);
	suite.addTest("testBufferWrite0020", testBufferWrite0020);
	suite.addTest("testBufferWrite0024", testBufferWrite0024);
	suite.addTest("testBufferWrite0026", testBufferWrite0026);
	suite.addTest("testBufferConcat0005", testBufferConcat0005);
	suite.addTest("testBufferConcat0006", testBufferConcat0006);
	suite.addTest("testBufferConcat0007", testBufferConcat0007);
	suite.addTest("testBufferConcat0009", testBufferConcat0009);
	suite.addTest("testBufferFill0007", testBufferFill0007);
	suite.addTest("testBufferFill0016", testBufferFill0016);
	suite.addTest("testBufferFill0017", testBufferFill0017);
	suite.addTest("testBufferFill0018", testBufferFill0018);
	suite.addTest("testFill0091", testFill0091);
	suite.addTest("testBufferTranscode0003", testBufferTranscode0003);

	return suite.run();
}
