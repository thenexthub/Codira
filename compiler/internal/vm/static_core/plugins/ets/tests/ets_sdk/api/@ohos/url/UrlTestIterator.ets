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
import { url } from "@ohos.url";
 

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10000
* @tc.name: testUrlEntries001
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlEntries001() {
	let params = new url.URLParams("key1=value1&key2=value2");

	let i = 0;
	let arr = new Array<string>();
	for (let pair of params.entries()) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[1], "value1");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10100
* @tc.name: testUrlEntries002
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlEntries002() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params.entries()) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[2], "key2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10200
* @tc.name: testUrlEntries003
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlEntries003() {
	let params = new url.URLParams("key1=value1&key2=value2")
	params.append("foo", "jk")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params.entries()) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[5], "jk")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10300
* @tc.name: testUrlEntries004
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlEntries004() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params.entries()) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[3], "value2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10400
* @tc.name: testUrlEntries005
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlEntries005() {
	let params = new url.URLParams("key1=value1&key2=value2")
	params.append("jss", "txt")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params.entries()) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[4], "jss")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10500
* @tc.name: testUrl[Symbol.iterator]001
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlIterator001() {
	let params = new url.URLParams("key1=value1&key2=value2");
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[1], "value1");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10600
* @tc.name: testUrl[Symbol.iterator]002
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlIterator002() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[2], "key2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10700
* @tc.name: testUrl[Symbol.iterator]003
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlIterator003() {
	let params = new url.URLParams("key1=value1&key2=value2")
	params.append("foo", "jk")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[5], "jk")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10800
* @tc.name: testUrl[Symbol.iterator]004
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlIterator004() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[3], "value2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_10900
* @tc.name: testUrl[Symbol.iterator]005
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlIterator005() {
	let params = new url.URLParams("key1=value1&key2=value2")
	params.append("jss", "txt")
	let i = 0;
	let arr = new Array<string>();
	for (let pair of params) {
		arr.push(pair[0]);
		i++;
		arr.push(pair[1]);
		i++;
	}
 arktest.assertEQ(arr[4], "jss")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11000
* @tc.name: testUrlForEach001
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlForEach001() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let arr = new Array<string>();
	let i = 0;

	params.forEach((value, key, searchParams:url.URLParams) => {
		arr.push(value + " " + key + " " + (params == searchParams));
		i++
	});
 arktest.assertEQ(arr[0], "value1 key1 true");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11100
* @tc.name: testUrlForEach002
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlForEach002() {
	let params = new url.URLParams("key1=value1&key2=value2");
	let arr = new Array<string>();
	let i = 0;

	params.forEach((value, key, searchParams:url.URLParams) => {
		arr.push(value + " " + key + " " + (params == searchParams));
		i++
	});
 arktest.assertEQ(arr[1], "value2 key2 true");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11200
* @tc.name: testUrlForEach003
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlForEach003() {
	let params = new url.URLParams("key1=value1&key2=value2");
	params.append("foo", "jk");
	let arr = new Array<string>();
	let i = 0;

	params.forEach((value, key, searchParams:url.URLParams) => {
		arr.push(value + " " + key + " " + (params == searchParams));
		i++
	});
 arktest.assertEQ(arr[2], "jk foo true");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11300
* @tc.name: testUrlForEach004
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlForEach004() {
	let params = new url.URLParams("foo=bar&jss=txt");
	let arr = new Array<string>();
	let i = 0;

	params.forEach((value, key, searchParams:url.URLParams) => {
		arr.push(value + " " + key + " " + (params == searchParams));
		i++
	});
 arktest.assertEQ(arr[1], "txt jss true");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11400
* @tc.name: testUrlForEach005
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlForEach005() {
	let params = new url.URLParams("foo=bar&jss=txt");
	params.append("foo", "jk");
	let arr = new Array<string>();
	let i = 0;

	params.forEach((value, key, searchParams:url.URLParams) => {
		arr.push(value + " " + key + " " + (params == searchParams));
		i++
	});
 arktest.assertEQ(arr[0], "bar foo true");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11500
* @tc.name: testUrlForEach006
* @tc.desc: The second parameter of forEach is undefined or null.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlForEach006() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let arr = new Array<string>();
	let i = 0;
	params.forEach((value, key, searchParams: url.URLParams) => {
		arr.push(value + " " + key + " " + (params == searchParams));
		i++
	});
 arktest.assertEQ(arr[0], "value1 key1 true");
}
 

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testUrlEntries001", testUrlEntries001);
	suite.addTest("testUrlEntries002", testUrlEntries002);
	suite.addTest("testUrlEntries003", testUrlEntries003);
	suite.addTest("testUrlEntries004", testUrlEntries004);
	suite.addTest("testUrlEntries005", testUrlEntries005);
	suite.addTest("testUrlIterator001", testUrlIterator001);
	suite.addTest("testUrlIterator002", testUrlIterator002);
	suite.addTest("testUrlIterator003", testUrlIterator003);
	suite.addTest("testUrlIterator004", testUrlIterator004);
	suite.addTest("testUrlIterator005", testUrlIterator005);
	suite.addTest("testUrlForEach001", testUrlForEach001);
	suite.addTest("testUrlForEach002", testUrlForEach002);
	suite.addTest("testUrlForEach003", testUrlForEach003);
	suite.addTest("testUrlForEach004", testUrlForEach004);
	suite.addTest("testUrlForEach005", testUrlForEach005);
	suite.addTest("testUrlForEach006", testUrlForEach006);

	//passed
	return suite.run();
}
