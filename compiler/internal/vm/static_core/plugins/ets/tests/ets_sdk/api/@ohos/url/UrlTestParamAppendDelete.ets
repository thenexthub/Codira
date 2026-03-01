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

function testParamsAppend001() {
	let that = url.URL.parseURL('http://username:password@host:8080/directory/file?foo=1&bar=2');
	let params = new url.URLParams(that.search);
	params.append('ma', 'jk')
	let result = params.toString()
 arktest.assertEQ(result, 'foo=1&bar=2&ma=jk');
}


function testParamsAppend002() {
	let that = url.URL.parseURL('http://username:password@host:8080/directory/file?foo=1&bar=2');
	let params = new url.URLParams(that.search);
	params.append("ma 大", "jk￥")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&ma+%E5%A4%A7=jk%EF%BF%A5")
}


function testParamsAppend003() {
	let that = url.URL.parseURL('http://username:password@host:8080/directory/file?foo=1&bar=2');
	let params = new url.URLParams(that.search);
	params.append("foo~!@#$%^&*()_+-=", "jk")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&foo%7E%21%40%23%24%25%5E%26*%28%29_%2B-%3D=jk")
}


function testParamsAppend004() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2');  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search);
	params.append("app", "par")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&app=par")
}

function testParamsAppend005() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2');  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search);
	params.append("123", "456")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&123=456")
}


/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_00800
* @tc.name: testParamsDelete001
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsDelete001() {
	let params = new url.URLParams('?foo=1&bar=2');
	params.delete("foo")
	let result = params.toString()
 arktest.assertEQ(result, "bar=2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_00900
* @tc.name: testParamsDelete002
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsDelete002() {
	let params = new url.URLParams('?foo大=1&bar=2');
	params.delete('foo');
	let result = params.toString();
 arktest.assertEQ(result, "foo%E5%A4%A7=1&bar=2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_01000
 * @tc.name: testParamsDelete003
 * @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
*/
function testParamsDelete003() {
	let params = new url.URLParams('?foo大=1&bar=2');
	params.delete("foo大");
	let result = params.toString();
 arktest.assertEQ(result, "bar=2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_01100
* @tc.name: testParamsDelete004
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsDelete004() {
	let params = new url.URLParams('?foo=1&bar=2');
	params.delete('bar');
	let result = params.toString();
 arktest.assertEQ(result, "foo=1");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_01200
* @tc.name: testParamsDelete005
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsDelete005() {
	let params = new url.URLParams('?foo=1&bar=2');
	params.delete("faa");
	let result = params.toString();
 arktest.assertEQ(result, "foo=1&bar=2")
}

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testParamsAppend001", testParamsAppend001);
	suite.addTest("testParamsAppend002", testParamsAppend002);
	suite.addTest("testParamsAppend003", testParamsAppend003);
	suite.addTest("testParamsAppend004", testParamsAppend004);
	suite.addTest("testParamsAppend005", testParamsAppend005);
	suite.addTest("testParamsDelete001", testParamsDelete001);
	suite.addTest("testParamsDelete002", testParamsDelete002);
	suite.addTest("testParamsDelete003", testParamsDelete003);
	suite.addTest("testParamsDelete004", testParamsDelete004);
	suite.addTest("testParamsDelete005", testParamsDelete005);

	//passed
	return suite.run();
}
