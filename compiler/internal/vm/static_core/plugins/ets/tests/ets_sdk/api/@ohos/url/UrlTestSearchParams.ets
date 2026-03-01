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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15400
* @tc.name: testUrlSearchParamsToString001
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsToString001() {
	let params = new url.URLParams("d=value1&c=value2&b=大")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=%E5%A4%A7")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15500
* @tc.name: testUrlSearchParamsToString002
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsToString002() {
	let params = new url.URLParams("d=value1&c=value2&b= 大")
	params.append("1 12", "QQQ")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=+%E5%A4%A7&1+12=QQQ")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15600
* @tc.name: testUrlSearchParamsToString003
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsToString003() {
	let params = new url.URLParams("￥=)")
	params.delete("5")
	let result = params.toString()
 arktest.assertEQ(result, "%EF%BF%A5=%29")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15700
* @tc.name: testUrlSearchParamsToString004
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsToString004() {
	let params = new url.URLParams("d=value1&c=value2&b=大&4=key4")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=%E5%A4%A7&4=key4")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15800
* @tc.name: testUrlSearchParamsToString005
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsToString005() {
	let params = new url.URLParams("d=value1&c=value2&b= 大&4=key4&5=key5")
	params.append("1 12", "QQQ")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=+%E5%A4%A7&4=key4&5=key5&1+12=QQQ")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15900
* @tc.name: testUrlSearchParamsConstruction001
* @tc.desc:  A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsConstruction001() {
	let params = new url.URLParams('?user=abc&query=xyz')
	let result = params.toString()
 arktest.assertEQ(result, "user=abc&query=xyz")
}


/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_16100
* @tc.name: testUrlSearchParamsConstruction003
* @tc.desc:  A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsConstruction003() {
	let params = new url.URLParams([
		['user', 'abc'],
		['query', 'first'],
		['query', 'second'],
	]);
	let result = params.toString()
 arktest.assertEQ(result, "user=abc&query=first&query=second")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_16400
* @tc.name: testUrlSearchParamsConstruction006
* @tc.desc:  A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsConstruction006() {
	let params = new url.URLParams()
	params.append('abcde', 'fghki')
	let paramsResult = params.toString()
 arktest.assertEQ(paramsResult, 'abcde=fghki')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_16500
* @tc.name: testUrlSearchParamsConstruction007
* @tc.desc:  A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsConstruction007() {
	let param = "";
	let params = new url.URLParams(param)
	params.append('abcde', 'fghki')
	let paramsResult = params.toString()
 arktest.assertEQ(paramsResult, 'abcde=fghki')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_16600
* @tc.name: testUrlSearchParamsConstruction008
* @tc.desc:  A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSearchParamsConstruction008() {
	let params = new url.URLParams()
	params.append('abcde', 'fghki')
	let paramsResult = params.toString()
 arktest.assertEQ(paramsResult, 'abcde=fghki')
}

function testUrlSearchParamsToUSVStringToString() {
  let params = new url.URLParams()
  params.append('\uD800', 'abc');
  let paramsString = params.toString();
  console.log("YaoTest toStrng: " + paramsString);
  arktest.assertEQ(paramsString, '%EF%BF%BD=abc');
}

function testUrlSearchParamsToUSVStringHas() {
  let params = new url.URLParams()
  params.append('\uD800', 'abc');
  let res1 = params.has('\uD800');
  arktest.assertTrue(res1);

  let res2 = params.has('�');
  arktest.assertFalse(res2);
}

function testUrlSearchParamsToUSVStringGet() {
  let params = new url.URLParams()
  params.append('\uD800', 'abc');
  let res1 = params.get('\uD800');
  arktest.assertEQ(res1, 'abc');

  let res2 = params.get('�');
  arktest.assertEQ(res2, undefined);
}

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testUrlSearchParamsToString001", testUrlSearchParamsToString001);
	suite.addTest("testUrlSearchParamsToString002", testUrlSearchParamsToString002);
	suite.addTest("testUrlSearchParamsToString003", testUrlSearchParamsToString003);
	suite.addTest("testUrlSearchParamsToString004", testUrlSearchParamsToString004);
	suite.addTest("testUrlSearchParamsToString005", testUrlSearchParamsToString005);
	suite.addTest("testUrlSearchParamsConstruction001", testUrlSearchParamsConstruction001);
	suite.addTest("testUrlSearchParamsConstruction003", testUrlSearchParamsConstruction003);
	suite.addTest("testUrlSearchParamsConstruction006", testUrlSearchParamsConstruction006);
	suite.addTest("testUrlSearchParamsConstruction007", testUrlSearchParamsConstruction007);
	suite.addTest("testUrlSearchParamsConstruction008", testUrlSearchParamsConstruction008);
	suite.addTest("testUrlSearchParamsToUSVStringToString", testUrlSearchParamsToUSVStringToString);
	suite.addTest("testUrlSearchParamsToUSVStringHas", testUrlSearchParamsToUSVStringHas);
	suite.addTest("testUrlSearchParamsToUSVStringGet", testUrlSearchParamsToUSVStringGet);

	//passed
	return suite.run();
}
