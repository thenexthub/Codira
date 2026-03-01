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
import {url} from "@ohos.url"

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_01600
* @tc.name: testParamsEntries001
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsEntries001() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_01700
* @tc.name: testParamsEntries002
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsEntries002() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_01800
* @tc.name: testParamsEntries003
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsEntries003() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_01900
* @tc.name: testParamsEntries004
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsEntries004() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02000
* @tc.name: testParamsEntries005
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsEntries005() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02100
* @tc.name: testParams[Symbol.iterator]001
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsIterator001() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02200
* @tc.name: testParams[Symbol.iterator]002
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsIterator002() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02300
* @tc.name: testParams[Symbol.iterator]003
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsIterator003() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02400
* @tc.name: testParams[Symbol.iterator]004
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsIterator004() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02500
* @tc.name: testParams[Symbol.iterator]005
* @tc.desc: Returns an ES6 iterator. Each item of the iterator is a JavaScript Array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsIterator005() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02600
* @tc.name: testParamsForEach001
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsForEach001() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02700
* @tc.name: testParamsForEach002
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsForEach002() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02800
* @tc.name: testParamsForEach003
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsForEach003() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_02900
* @tc.name: testParamsForEach004
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsForEach004() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_03000
* @tc.name: testParamsForEach005
* @tc.desc: Callback functions are used to traverse key-value pairs on the URLParams instance object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsForEach005() {
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
function testParamsForEach007() {
	let params = new url.URLParams("key1=value1&key2=value2");
	let arr = new Array<string>();
	let i = 0;

	params.forEach((value, key, searchParams:url.URLParams) => {
		arr.push(value + " " + key + " " + (params == searchParams));
		i++
	});
 arktest.assertEQ(arr[0], "value1 key1 true");
}

function testParamsArrayFrom() {
    let params = new url.URLParams("key1=\b&\b=val2&key3=val3");
    let arrayFromEntries = Array.from(params.entries());
    let arrayFromKeys = Array.from(params.keys());
    let arrayFromValues = Array.from(params.values());
    let arrayFromIterator = Array.from(params.$_iterator());
    arktest.assertEQ(arrayFromEntries.toString(), "key1,\b,\b,val2,key3,val3");
    arktest.assertEQ(arrayFromIterator.toString(), "key1,\b,\b,val2,key3,val3");
    arktest.assertEQ(arrayFromKeys.toString(), "key1,\b,key3");
    arktest.assertEQ(arrayFromValues.toString(), "\b,val2,val3");
}

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testParamsEntries001", testParamsEntries001);
	suite.addTest("testParamsEntries002", testParamsEntries002);
	suite.addTest("testParamsEntries003", testParamsEntries003);
	suite.addTest("testParamsEntries004", testParamsEntries004);
	suite.addTest("testParamsEntries005", testParamsEntries005);
	suite.addTest("testParamsIterator001", testParamsIterator001);
	suite.addTest("testParamsIterator002", testParamsIterator002);
	suite.addTest("testParamsIterator003", testParamsIterator003);
	suite.addTest("testParamsIterator004", testParamsIterator004);
	suite.addTest("testParamsIterator005", testParamsIterator005);
	suite.addTest("testParamsForEach001", testParamsForEach001);
	suite.addTest("testParamsForEach002", testParamsForEach002);
	suite.addTest("testParamsForEach003", testParamsForEach003);
	suite.addTest("testParamsForEach004", testParamsForEach004);
	suite.addTest("testParamsForEach005", testParamsForEach005);
	suite.addTest("testParamsForEach007", testParamsForEach007);
    suite.addTest("testParamsArrayFrom", testParamsArrayFrom);

	//passed
	return suite.run();
}
