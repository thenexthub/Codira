/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { url } from "@ohos.url";
 
 
/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11600
* @tc.name: testUrlGet001
* @tc.desc: Returns the first value associated to the given search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGet001() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let result = params.get("1")
 arktest.assertEQ(result, undefined)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11700
* @tc.name: testUrlGet002
* @tc.desc: Returns the first value associated to the given search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGet002() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let result = params.get("key2")
 arktest.assertEQ(result, "value2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11800
* @tc.name: testUrlGet003
* @tc.desc: Returns the first value associated to the given search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGet003() {
	let params = new url.URLParams("key1=value1&key2=value2")
	params.append("5", "JKL")
	let result = params.get("5")
 arktest.assertEQ(result, "JKL")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_11900
* @tc.name: testUrlGet004
* @tc.desc: Returns the first value associated to the given search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGet004() {
	let params = new url.URLParams("key1=value1&key2=value2")
	let result = params.get("key1")
 arktest.assertEQ(result, "value1")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12000
* @tc.name: testUrlGet005
* @tc.desc: Returns the first value associated to the given search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGet005() {
	let params = new url.URLParams("key1=value1&key2=value2")
	params.append("jss", "JL")
	let result = params.get("jss")
 arktest.assertEQ(result, "JL")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12100
* @tc.name: testUrlGetAll001
* @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGetAll001() {
	let params = new url.URLParams("key1=value1&key2=value2")
	params.append("key1", "AAA")
	let result = params.getAll("key1")
 arktest.assertEQ(result.toString(), "value1,AAA")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12200
* @tc.name: testUrlGetAll002
* @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGetAll002() {
	let params = new url.URLParams("key1=value1&8=DEF")
	params.append("8", "A8A")
	let result = params.getAll("8")
 arktest.assertEQ(result.toString(), "DEF,A8A")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12300
* @tc.name: testUrlGetAll003
* @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGetAll003() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=da")
	params.append("key3", "A3A")
	let result = params.getAll("key3")
 arktest.assertEQ(result.toString(), "da,A3A")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12400
* @tc.name: testUrlGetAll004
* @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGetAll004() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=大")
	params.append("key3", "A3A")
	let result = params.getAll("key4")
 arktest.assertEQ(result.toString(), '')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12500
* @tc.name: testUrlGetAll005
* @tc.desc: Returns all key-value pairs associated with a given search parameter as an array.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGetAll005() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=大")
	params.append("key3", "A3A")
	let result = params.getAll("key2")
 arktest.assertEQ(result.toString(), "value2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12600
* @tc.name: testUrlHas001
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHas001() {
	let params = new url.URLParams("key1=value1&key2=value2&key3=大")
	let result = params.has("2")
 arktest.assertEQ(result, false)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12700
* @tc.name: testUrlHas002
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHas002() {
	let params = new url.URLParams("小=value1&key2=value2&key3=大")
	let result = params.has("小")
 arktest.assertEQ(result, true)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12800
* @tc.name: testUrlHas003
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHas003() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大")
	params.append("￥", "ACA")
	let result = params.has("￥")
 arktest.assertEQ(result, true)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_12900
* @tc.name: testUrlHas004
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHas004() {
	let params = new url.URLParams("小=value1&key2=value2&key3=大")
	let result = params.has("无")
 arktest.assertEQ(result, false)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13000
* @tc.name: testUrlHas005
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHas005() {
	let params = new url.URLParams("小=value1&￥=value2&key3=大")
	params.append("￥", "ACA")
	let result = params.has("￥11")
 arktest.assertEQ(result, false)
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13100
* @tc.name: testUrlHas006
* @tc.desc: Returns a Boolean that indicates whether a parameter with the specified name exists.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHas006() {
	// let params = new url.URLParams("小=value1&￥=value2&key3=大")
	// params.append("￥","ACA")
	// try {
	//     let a = 1;
	//     params.has(a);
	// } catch(e) {
	//     arktest.assertEQ(e.toString(),`BusinessError: Parameter error. The type of ${a} must be string`);
	//     arktest.assertEQ(e.code,401)
	//     arktest.assertEQ(e.message,`Parameter error. The type of ${a} must be string`);
	// }
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13200
* @tc.name: testUrlKeys001
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlKeys001() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13300
* @tc.name: testUrlKeys002
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlKeys002() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13400
* @tc.name: testUrlKeys003
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlKeys003() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13500
* @tc.name: testUrlKeys004
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlKeys004() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13600
* @tc.name: testUrlKeys005
* @tc.desc: Returns an iterator allowing to go through all keys contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlKeys005() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13700
* @tc.name: testUrlSet001
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSet001() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	// params.append("11","ACA");
	params.set("11", "CCC");
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&11=CCC");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13800
* @tc.name: testUrlSet002
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSet002() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	params.set('10', 'BBB');
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&10=BBB");
}


function testUrlStringify001() {
	let str ='https://admin:SecurePass123!@api.example.com:443/v2/resources/finance/transactions/2025Q4?id=10086&type=bond&region=CN&sort=amount_desc#payment-detail-789'
    const uriObj3 = url.URL.parseURL(str);
	let jsonStr = JSON.stringify(uriObj3.params);
	let jsonElement = JSON.parseJsonElement(jsonStr);
	arktest.assertEQ(jsonElement.getString("parentUrl"), 'https://admin:SecurePass123!@api.example.com/v2/resources/finance/transactions/2025Q4?id=10086&type=bond&region=CN&sort=amount_desc#payment-detail-789')
}

class A {
	a:string;
	b:int;
	c:url.URL
	constructor(aa:string, bb:int, cc:url.URL) {
		this.a = aa;
		this.b = bb;
		this.c = cc;
	}
}
function testUrlStringify002() {
	let str ='https://admin:SecurePass123!@api.example.com:443/v2/resources/finance/transactions/2025Q4?id=10086&type=bond&region=CN&sort=amount_desc#payment-detail-789'
    const uriObj3 = url.URL.parseURL(str);
	let obj: A = new A("1", 1, uriObj3);
	let jsonStr = JSON.stringify(obj);
	let jsonElement = JSON.parseJsonElement(jsonStr);
	arktest.assertEQ(jsonElement.getString("a"), '1')
	arktest.assertEQ(jsonElement.getInteger("b"), 1)
	arktest.assertEQ(jsonElement.getString("c"), 'https://admin:SecurePass123!@api.example.com/v2/resources/finance/transactions/2025Q4?id=10086&type=bond&region=CN&sort=amount_desc#payment-detail-789')
}

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testUrlGet001", testUrlGet001);
	suite.addTest("testUrlGet002", testUrlGet002);
	suite.addTest("testUrlGet003", testUrlGet003);
	suite.addTest("testUrlGet004", testUrlGet004);
	suite.addTest("testUrlGet005", testUrlGet005);
	suite.addTest("testUrlGetAll001", testUrlGetAll001);
	suite.addTest("testUrlGetAll002", testUrlGetAll002);
	suite.addTest("testUrlGetAll003", testUrlGetAll003);
	suite.addTest("testUrlGetAll004", testUrlGetAll004);
	suite.addTest("testUrlGetAll005", testUrlGetAll005);
	suite.addTest("testUrlHas001", testUrlHas001);
	suite.addTest("testUrlHas002", testUrlHas002);
	suite.addTest("testUrlHas003", testUrlHas003);
	suite.addTest("testUrlHas004", testUrlHas004);
	suite.addTest("testUrlHas005", testUrlHas005);
	suite.addTest("testUrlHas006", testUrlHas006);
	suite.addTest("testUrlKeys001", testUrlKeys001);
	suite.addTest("testUrlKeys002", testUrlKeys002);
	suite.addTest("testUrlKeys003", testUrlKeys003);
	suite.addTest("testUrlKeys004", testUrlKeys004);
	suite.addTest("testUrlKeys005", testUrlKeys005);
	suite.addTest("testUrlSet001", testUrlSet001);
	suite.addTest("testUrlSet002", testUrlSet002);
	suite.addTest("testUrlStringify001", testUrlStringify001);

	//passed
	return suite.run();
}
