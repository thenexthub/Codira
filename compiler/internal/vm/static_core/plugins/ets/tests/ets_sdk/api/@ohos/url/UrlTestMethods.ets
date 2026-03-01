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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_13900
* @tc.name: testUrlSet003
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSet003() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	params.set("ma 大", "10￥");
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&ma+%E5%A4%A7=10%EF%BF%A5");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14000
* @tc.name: testUrlSet004
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSet004() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	// params.append("1","ACA");
	params.set("1", "CCC");
	let res = params.toString();
 arktest.assertEQ(res, "1=CCC&2=value2&key3=3");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14100
* @tc.name: testUrlSet005
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSet005() {
	let params = new url.URLParams("1=value1&2=value2&key3=3");
	params.set('12', 'BBB');
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=value2&key3=3&12=BBB");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14200
* @tc.name: testUrlSet006
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSet006() {
	// try {
	//     let params = new url.URLParams("1=value1&2=value2&key3=3");
	//     let a = 12;
	//     params.set(a, 'BBB');
	// } catch(err) {
	//     arktest.assertEQ(err.toString(),`BusinessError: Parameter error. The type of ${a} must be string`)
	//     arktest.assertEQ(err.code,401)
	//     arktest.assertEQ(err.message,`Parameter error. The type of ${a} must be string`);
	// }
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14300
* @tc.name: testUrlSet006
* @tc.desc: Sets the value associated with a given search parameter to the given value.
* If there were several matching values, this method deletes the others.
* If the search parameter doesn't exist, this method creates it.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSet007() {
	let params = new url.URLParams();
	params.set('name', 'value');
	let res = params.toString();
 arktest.assertEQ(res, "name=value");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14400
* @tc.name: testUrlSort001
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSort001() {
	let params = new url.URLParams("1=value1&3=value3&2=key2");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=key2&3=value3");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14500
* @tc.name: testUrlSort002
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSort002() {
	let params = new url.URLParams("a=value1&c=value2&b=key2");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "a=value1&b=key2&c=value2");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14600
* @tc.name: testUrlSort003
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSort003() {
	let params = new url.URLParams("d=value1&c=value2&b=33");
	params.append("a", "ACA");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "a=ACA&b=33&c=value2&d=value1");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14700
* @tc.name: testUrlSort004
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSort004() {
	let params = new url.URLParams("1=value1&3=value3&2=key2&4=key4");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "1=value1&2=key2&3=value3&4=key4");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14800
* @tc.name: testUrlSort005
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlSort005() {
	let params = new url.URLParams("a=value1&c=value2&4=key4&b=key2");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "4=key4&a=value1&b=key2&c=value2");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_14900
* @tc.name: testUrlValues001
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlValues001() {
	let params = new url.URLParams("d=value1&c=value2&b=大")
	let arr = new Array<string>();
	let i = 0
	for (let value of params.values()) {
		arr.push(value);
		i++
	}
 arktest.assertEQ(arr[0], "value1")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15000
* @tc.name: testUrlValues002
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlValues002() {
	let params = new url.URLParams("d=value1&c=value2&b=大")
	let arr = new Array<string>();
	let i = 0
	for (let value of params.values()) {
		arr.push(value);
		i++
	}
 arktest.assertEQ(arr[1], "value2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15100
* @tc.name: testUrlValues003
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlValues003() {
	let params = new url.URLParams("d=value1&c=value2&b=大")
	params.append("a", "ACA")
	let arr = new Array<string>();
	let i = 0
	for (let value of params.values()) {
		arr.push(value);
		i++
	}
 arktest.assertEQ(arr[3], "ACA")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15200
* @tc.name: testUrlValues004
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlValues004() {
	let params = new url.URLParams("d=value1&c=value2&b=大&4=key4")
	let arr = new Array<string>();
	let i = 0
	for (let value of params.values()) {
		arr.push(value);
		i++
	}
 arktest.assertEQ(arr[3], "key4")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_15300
* @tc.name: testUrlValues005
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlValues005() {
	let params = new url.URLParams("d=value1&c=value2&b=大&4=key4&5=key5")
	let arr = new Array<string>();
	let i = 0
	for (let value of params.values()) {
		arr.push(value);
		i++
	}
 arktest.assertEQ(arr[4], "key5")
}
/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_16700
* @tc.name: testNewUrl001
* @tc.desc:  The second parameter of Url construction is undefined or null.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testNewUrl001() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment', undefined)
	let result = params.toString()
 arktest.assertEQ(result, "http://username:password@host:8080/directory/file?query#fragment")
	let params1 = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	let result2 = params1.toString()
 arktest.assertEQ(result2, "http://username:password@host:8080/directory/file?query#fragment")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_16800
* @tc.name: testUrlToString001
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString001() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	let result = params.toString()
 arktest.assertEQ(result, "http://username:password@host:8080/directory/file?query#fragment")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_16900
* @tc.name: testUrlToString002
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString002() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file')
	let result = params.toString()
 arktest.assertEQ(result, "http://username:password@host:8080/directory/file")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17000
* @tc.name: testUrlToString003
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString003() {
	let params = url.URL.parseURL('http://username:password@host:8080#fragment')
	let result = params.toString()
 arktest.assertEQ(result, "http://username:password@host:8080/#fragment")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17100
* @tc.name: testUrlToString004
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString004() {
	let params = url.URL.parseURL('http1://host/directory/file?query#fragment')
	let result = params.toString()
 arktest.assertEQ(result, "http1://host/directory/file?query#fragment")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17200
* @tc.name: testUrlToString005
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString005() {
	let params = url.URL.parseURL('http:host:8080/directory/file?query#fragment')
	let result = params.toString()
 arktest.assertEQ(result, "http://host:8080/directory/file?query#fragment")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17300
* @tc.name: testUrlToString006
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString006() {
	let params = url.URL.parseURL('https://255.16581375')
	let result = params.toString()
 arktest.assertEQ(result, "https://255.253.2.255/")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17400
* @tc.name: testUrlToString007
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString007() {
	let params = url.URL.parseURL('https://192.1.')
	let result = params.toString()
 arktest.assertEQ(result, "https://192.0.0.1/")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17500
* @tc.name: testUrlToString008
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString008() {
	let params = url.URL.parseURL('https://192.1..')
	let result = params.toString()
 arktest.assertEQ(result, "https://192.1../")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17600
* @tc.name: testUrlToString009
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString009() {
	let params = url.URL.parseURL('https://192.95645654354.8.f')
	let result = params.toString()
 arktest.assertEQ(result, "https://192.95645654354.8.f/")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17700
* @tc.name: testUrlToString010
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString010() {
	let params = url.URL.parseURL('coap://?key=value#hash')
	let result = params.toString()
 arktest.assertEQ(result, "coap://?key=value#hash")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17800
* @tc.name: testUrlToString011
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString011() {
	let params = url.URL.parseURL('upcad://akls/casd/saqe/ss/?dalks=ccc')
	let result = params.toString()
 arktest.assertEQ(result, "upcad://akls/casd/saqe/ss/?dalks=ccc")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17900
* @tc.name: testUrlToString012
* @tc.desc:  Returns the serialized URL as a string.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlToString012() {
    let str = "ftp:!\"#%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ \\[]"
        + "^_`abcdefghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸"
        + "¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
	let params = url.URL.parseURL(str);
	let result = params.toString();
    const urlEncode = "ftp://!%22/#%&'()*+,-./0123456789:;%3C=%3E?@ABCDEFGHIJKLMNOPQRSTUVWXYZ%20%5C[]%5E_"
        + "`abcdefghijklmnopqrstuvwxyz{|}~%E2%82%AC%E2%80%9A%C6%92%E2%80%9E%E2%80%A6%E2%80%A0%E2%80%A1"
        + "%CB%86%E2%80%B0%C5%A0%E2%80%B9%C5%92%C5%BD%E2%80%98%E2%80%99%E2%80%9C%E2%80%9D%E2%80%A2%E2"
        + "%80%93%E2%80%94%CB%9C%E2%84%A2%C5%A1%E2%80%BA%C5%93%C5%BE%C5%B8%C2%A1%C2%A2%C2%A3%C2%A4%C2"
        + "%A5%C2%A6%C2%A7%C2%A8%C2%A9%C2%AA%C2%AB%C2%AC%C2%AD%C2%AE%C2%AF%C2%B0%C2%B1%C2%B2%C2%B3%C2"
        + "%B4%C2%B5%C2%B6%C2%B7%C2%B8%C2%B9%C2%BA%C2%BB%C2%BC%C2%BD%C2%BE%C2%BF%C3%80%C3%81%C3%82%C3"
        + "%83%C3%84%C3%85%C3%86%C3%87%C3%88%C3%89%C3%8A%C3%8B%C3%8C%C3%8D%C3%8E%C3%8F%C3%90%C3%91%C3"
        + "%92%C3%93%C3%94%C3%95%C3%96%C3%97%C3%98%C3%99%C3%9A%C3%9B%C3%9C%C3%9D%C3%9E%C3%9F%C3%A0%C3"
        + "%A1%C3%A2%C3%A3%C3%A4%C3%A5%C3%A6%C3%A7%C3%A8%C3%A9%C3%AA%C3%AB%C3%AC%C3%AD%C3%AE%C3%AF%C3"
        + "%B0%C3%B1%C3%B2%C3%B3%C3%B4%C3%B5%C3%B6%C3%B7%C3%B8%C3%B9%C3%BA%C3%BB%C3%BC%C3%BD%C3%BE%C3%BF";
 arktest.assertEQ(result, urlEncode);
}


function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testUrlSet003", testUrlSet003);
	suite.addTest("testUrlSet004", testUrlSet004);
	suite.addTest("testUrlSet005", testUrlSet005);
	suite.addTest("testUrlSet006", testUrlSet006);
	suite.addTest("testUrlSet007", testUrlSet007);
	suite.addTest("testUrlSort001", testUrlSort001);
	suite.addTest("testUrlSort002", testUrlSort002);
	suite.addTest("testUrlSort003", testUrlSort003);
	suite.addTest("testUrlSort004", testUrlSort004);
	suite.addTest("testUrlSort005", testUrlSort005);
	suite.addTest("testUrlValues001", testUrlValues001);
	suite.addTest("testUrlValues002", testUrlValues002);
	suite.addTest("testUrlValues003", testUrlValues003);
	suite.addTest("testUrlValues004", testUrlValues004);
	suite.addTest("testUrlValues005", testUrlValues005);
	suite.addTest("testNewUrl001", testNewUrl001);
	suite.addTest("testUrlToString001", testUrlToString001);
	suite.addTest("testUrlToString002", testUrlToString002);
	suite.addTest("testUrlToString003", testUrlToString003);
	suite.addTest("testUrlToString004", testUrlToString004);
	suite.addTest("testUrlToString005", testUrlToString005);
	suite.addTest("testUrlToString006", testUrlToString006);
	suite.addTest("testUrlToString007", testUrlToString007);
	suite.addTest("testUrlToString008", testUrlToString008);
	suite.addTest("testUrlToString009", testUrlToString009);
    suite.addTest("testUrlToString010", testUrlToString010);
    suite.addTest("testUrlToString011", testUrlToString011);
    suite.addTest("testUrlToString012", testUrlToString012);

	//passed
	return suite.run();
}
