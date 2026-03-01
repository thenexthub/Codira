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
  * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04500
  * @tc.name: testParamsHas001
  * @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
  * @tc.size: MediumTest
  * @tc.type: Function
  * @tc.level: Level 2
  */
function testParamsHas001() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=大")
	let result = params.has("2")
 arktest.assertEQ(result, false)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04600
* @tc.name: testParamsHas002
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsHas002() {
	let params = new url.URLParams("小=value1&key2=value2&key3=大")
	let result = params.has("小")
 arktest.assertEQ(result, true)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04700
* @tc.name: testParamsHas003
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsHas003() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大")
	params.append("￥", "ACA")
	let result = params.has("￥")
 arktest.assertEQ(result, true)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04800
* @tc.name: testParamsHas004
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsHas004() {
	let params = new url.URLParams("小=value1&key2=value2&key3=大")
	let result = params.has("无")
 arktest.assertEQ(result, false)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04900
* @tc.name: testParamsHas005
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsHas005() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大")
	params.append("￥", "ACA")
	let result = params.has("￥11")
 arktest.assertEQ(result, false)
}


/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05100
* @tc.name: testParamsKeys001
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsKeys001() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大");
	let arr = new Array<string>();
	let i = 0;
	for (let key of params.keys()) {
		arr.push(key);
		i++
	}
 arktest.assertEQ(arr[0], "小");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05200
* @tc.name: testParamsKeys002
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsKeys002() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大");
	let arr = new Array<string>();
	let i = 0;
	for (let key of params.keys()) {
		arr.push(key);
		i++
	}
 arktest.assertEQ(arr[1], "￥");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05300
* @tc.name: testParamsKeys003
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsKeys003() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大");
	let arr = new Array<string>();
	let i = 0;
	for (let key of params.keys()) {
		arr.push(key);
		i++
	}
 arktest.assertEQ(arr[2], "key3");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05400
* @tc.name: testParamsKeys004
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsKeys004() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大&key4=六");
	let arr = new Array<string>();
	let i = 0;
	for (let key of params.keys()) {
		arr.push(key);
		i++
	}
 arktest.assertEQ(arr[3], "key4");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05500
* @tc.name: testParamsKeys005
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsKeys005() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大&key4=六&key5=发");
	let arr = new Array<string>();
	let i = 0;
	for (let key of params.keys()) {
		arr.push(key);
		i++
	}
 arktest.assertEQ(arr[4], "key5");
}
 


/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06400
* @tc.name: testParamsSort001
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSort001() {
	let params = new url.URLParams("1=value1&3=value3&2=key2");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=key2&3=value3");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06500
* @tc.name: testParamsSort002
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSort002() {
	let params = new url.URLParams("a=value1&c=value2&b=key2");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "a=value1&b=key2&c=value2");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06600
* @tc.name: testParamsSort003
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSort003() {
	let params = new url.URLParams("d=value1&c=value2&b=33");
	params.append("a", "ACA");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "a=ACA&b=33&c=value2&d=value1");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06700
* @tc.name: testParamsSort004
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSort004() {
	let params = new url.URLParams("1=value1&3=value3&2=key2&4=key4");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=key2&3=value3&4=key4");
}
 


function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testParamsHas001", testParamsHas001);
	suite.addTest("testParamsHas002", testParamsHas002);
	suite.addTest("testParamsHas003", testParamsHas003);
	suite.addTest("testParamsHas004", testParamsHas004);
	suite.addTest("testParamsHas005", testParamsHas005);
	suite.addTest("testParamsKeys001", testParamsKeys001);
	suite.addTest("testParamsKeys002", testParamsKeys002);
	suite.addTest("testParamsKeys003", testParamsKeys003);
	suite.addTest("testParamsKeys004", testParamsKeys004);
	suite.addTest("testParamsKeys005", testParamsKeys005);
	suite.addTest("testParamsSort001", testParamsSort001);
	suite.addTest("testParamsSort002", testParamsSort002);
	suite.addTest("testParamsSort003", testParamsSort003);
	suite.addTest("testParamsSort004", testParamsSort004);

	//passed
	return suite.run();
}
