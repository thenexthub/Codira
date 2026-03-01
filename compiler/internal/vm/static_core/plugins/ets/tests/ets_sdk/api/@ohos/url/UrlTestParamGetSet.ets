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

function testParamsGet001() {
	let params = new url.URLParams("key1=value1&key2=value2");
 arktest.assertEQ(params.get("1"), undefined);
 arktest.assertEQ(params.get("key2"), "value2");
	params.append("5", "JKL");
 arktest.assertEQ(params.get("5"), "JKL");
}
function testParamsGet005() {
	let params = new url.URLParams("key1=value1&key2=value2");
	params.append("jss", "JL")
 arktest.assertEQ(params.get("jss"), "JL");
}
/**
  * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_03900
  * @tc.name: testParamsGetAll001
  * @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
  * @tc.size: MediumTest
  * @tc.type: Function
  * @tc.level: Level 2
  */
function testParamsGetAll001() {
	let params = new url.URLParams("key1=value1&key2=value2");
	params.append("key1", "AAA")
 arktest.assertEQ(params.getAll("key1").toString(), "value1,AAA");
}
/**
   * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04000
   * @tc.name: testParamsGetAll002
   * @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
   * @tc.size: MediumTest
   * @tc.type: Function
   * @tc.level: Level 2
   */
function testParamsGetAll002() {
	let params = new url.URLParams("key1=value1&8=DEF");
	params.append("8", "A8A")
 arktest.assertEQ(params.getAll("8").toString(), "DEF,A8A");
}
/**
   * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04100
   * @tc.name: testParamsGetAll003
   * @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
   * @tc.size: MediumTest
   * @tc.type: Function
   * @tc.level: Level 2
   */
function testParamsGetAll003() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=da");
	params.append("key3", "A3A")
 arktest.assertEQ(params.getAll("key3").toString(), "da,A3A");
}

/**
  * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04200
  * @tc.name: testParamsGetAll004
  * @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
  * @tc.size: MediumTest
  * @tc.type: Function
  * @tc.level: Level 2
  */
function testParamsGetAll004() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=大")
	params.append("key3", "A3A")
	let result = params.getAll("key4")
 arktest.assertEQ(result.toString(), '')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_04300
* @tc.name: testParamsGetAll005
* @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsGetAll005() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=大")
	params.append("key3", "A3A")
	let result = params.getAll("key2")
 arktest.assertEQ(result.toString(), "value2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05600
* @tc.name: testParamsSet001
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSet001() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	// params.append("11","ACA");
	params.set("11", "CCC");
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&11=CCC");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05700
* @tc.name: testParamsSet002
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSet002() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	params.set('10', 'BBB');
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&10=BBB");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05800
* @tc.name: testParamsSet003
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSet003() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	params.set("ma 大", "10￥");
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&ma+%E5%A4%A7=10%EF%BF%A5");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_05900
* @tc.name: testParamsSet004
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSet004() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	// params.append("1","ACA");
	params.set("1", "CCC");
	let res = params.toString();
 arktest.assertEQ(res, "1=CCC&2=value2&key3=3");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06000
* @tc.name: testParamsSet005
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSet005() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	params.set('12', 'BBB');
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&12=BBB");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06100
* @tc.name: testParamsSet006
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSet006() {
	let params = new url.URLParams('');
	params.set('12', 'BBB');
	let res = params.toString();
 arktest.assertEQ(res, "12=BBB");
}



function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testParamsGet001", testParamsGet001);
	suite.addTest("testParamsGet005", testParamsGet005);
	suite.addTest("testParamsGetAll001", testParamsGetAll001);
	suite.addTest("testParamsGetAll002", testParamsGetAll002);
	suite.addTest("testParamsGetAll003", testParamsGetAll003);
	suite.addTest("testParamsGetAll004", testParamsGetAll004);
	suite.addTest("testParamsGetAll005", testParamsGetAll005);
	suite.addTest("testParamsSet001", testParamsSet001);
	suite.addTest("testParamsSet002", testParamsSet002);
	suite.addTest("testParamsSet003", testParamsSet003);
	suite.addTest("testParamsSet004", testParamsSet004);
	suite.addTest("testParamsSet005", testParamsSet005);
	suite.addTest("testParamsSet006", testParamsSet006);

	//passed
	return suite.run();
}
