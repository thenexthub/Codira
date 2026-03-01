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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06800
* @tc.name: testParamsSort005
* @tc.desc: Sort all key/value pairs contained in this object in place and return undefined.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsSort005() {
	let params = new url.URLParams("a=value1&c=value2&4=key4&b=key2");
	params.sort();
	let res = params.toString();
 arktest.assertEQ(res, "4=key4&a=value1&b=key2&c=value2");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_06900
* @tc.name: testParamsValues001
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsValues001() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07000
* @tc.name: testParamsValues002
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsValues002() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07100
* @tc.name: testParamsValues003
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsValues003() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07200
* @tc.name: testParamsValues004
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsValues004() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07300
* @tc.name: testParamsValues005
* @tc.desc: Returns an iterator allowing to go through all values contained in this object.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsValues005() {
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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07400
* @tc.name: testParamsToString001
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsToString001() {
	let params = new url.URLParams("d=value1&c=value2&b=大")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=%E5%A4%A7")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07500
* @tc.name: testParamsToString002
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsToString002() {
	let params = new url.URLParams("d=value1&c=value2&b= 大")
	params.append("1 12", "QQQ")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=+%E5%A4%A7&1+12=QQQ")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07600
* @tc.name: testParamsToString003
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsToString003() {
	let params = new url.URLParams("￥=)")
	params.delete("5")
	let result = params.toString()
 arktest.assertEQ(result, "%EF%BF%A5=%29")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07700
* @tc.name: testParamsToString004
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsToString004() {
	let params = new url.URLParams("d=value1&c=value2&b=大&4=key4")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=%E5%A4%A7&4=key4")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07800
* @tc.name: testParamsToString005
* @tc.desc: Returns a query string suitable for use in a URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsToString005() {
	let params = new url.URLParams("d=value1&c=value2&b= 大&4=key4&5=key5")
	params.append("1 12", "QQQ")
	let result = params.toString()
 arktest.assertEQ(result, "d=value1&c=value2&b=+%E5%A4%A7&4=key4&5=key5&1+12=QQQ")
}

/**
  * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_07900
  * @tc.name: testParamsConstruction001
  * @tc.desc: A parameterized constructor used to create an URLParams instance.
  * @tc.size: MediumTest
  * @tc.type: Function
  * @tc.level: Level 2
  */
function testParamsConstruction001() {
	let params = new url.URLParams('?user=abc&query=xyz')
	let result = params.toString()
 arktest.assertEQ(result, "user=abc&query=xyz")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08000
* @tc.name: testParamsConstruction002
* @tc.desc: A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsConstruction002() {
	let rc: Record<string, string> = {
		'user': 'abc',
		'query': 'first'
	};
	let params = new url.URLParams(rc);
	let result = params.toString()
 arktest.assertEQ(result, "user=abc&query=first")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08100
* @tc.name: testParamsConstruction003
* @tc.desc: A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsConstruction003() {
	let params = new url.URLParams([
		['user', 'abc'],
		['query', 'first'],
		['query', 'second'],
	]);
	let result = params.toString()
 arktest.assertEQ(result, "user=abc&query=first&query=second")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08200
* @tc.name: testParamsConstruction004
* @tc.desc: A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
// function testParamsConstruction004() {
// const map = new Map();
// map.set('user', 'abc');
// map.set('query', 'xyz');
// let params = new url.URLParams(map)
// let result = params.toString()
// arktest.assertEQ(result,"user=abc&query=xyz")
// }

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08300
* @tc.name: testParamsConstruction005
* @tc.desc: A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
// function testParamsConstruction005() {
// function* getQueryPairs() {
//     yield ['user', 'abc'];
//     yield ['query', 'first'];
//     yield ['query', 'second'];
// }
// let params = new url.URLParams(getQueryPairs());
// let result = params.toString()
// arktest.assertEQ(result,"user=abc&query=first&query=second")
// }

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08400
* @tc.name: testParamsConstruction006
* @tc.desc: A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsConstruction006() {
	let params = new url.URLParams()
	params.append('abcde', 'fghki')
	let paramsResult = params.toString()
 arktest.assertEQ(paramsResult, 'abcde=fghki')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08500
* @tc.name: testParamsConstruction007
* @tc.desc: A parameterized constructor used to create an URLParams instance.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testParamsConstruction007() {
	let param = "";
	let params = new url.URLParams(param)
	params.append('abcde', 'fghki')
	let paramsResult = params.toString()
 arktest.assertEQ(paramsResult, 'abcde=fghki')
}



/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08700
* @tc.name: testParamsConstruction009
* @tc.desc: The type of init must be string two-dimensional array or object list throw error.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
// function testParamsConstruction009() {
// let param = [
// 	['user', 'abc', 'error'],
//   ];
// try {
//     let params = new url.URLParams(param);
// } catch(err) {
//     if (err instanceof BusinessError) {
//     arktest.assertEQ(err.toString(),`BusinessError: Parameter error. The type of ${param} must be string[][]`)
//     arktest.assertEQ(err.code,401)
//     arktest.assertEQ(err.message,`Parameter error. The type of ${param} must be string[][]`);
// }
// }
// }

function testParamsConstruction010() {
	let params = new url.URLParams('?user=abc&query=xyz')
	let params2 = new url.URLParams(params)
 arktest.assertEQ(params2.toString(), "user=abc&query=xyz")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08800
* @tc.name: testNewURLSearchParams001
* @tc.desc: The input parameter for New URLParams is undefined or null.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testNewURLSearchParams001() {
	let params = new url.URLParams(undefined);
	let result = params.toString()
 arktest.assertEQ(result, '')
	let params1 = new url.URLParams();
	let result2 = params1.toString()
 arktest.assertEQ(result2, '')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_08900
* @tc.name: testUrlAppend001
* @tc.desc: Appends a specified key/value pair as a new search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlAppend001() {
	let that = url.URL.parseURL('http://username:password@host:8080/directory/file?foo=1&bar=2');
	let params = new url.URLParams(that.search);
	params.append('ma', 'jk')
	let result = params.toString()
 arktest.assertEQ(result, 'foo=1&bar=2&ma=jk')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09000
* @tc.name: testUrlAppend002
* @tc.desc: Appends a specified key/value pair as a new search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlAppend002() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2')  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search)
	params.append("ma 大", "jk￥")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&ma+%E5%A4%A7=jk%EF%BF%A5")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09100
* @tc.name: testUrlAppend003
* @tc.desc: Appends a specified key/value pair as a new search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlAppend003() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2')  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search)
	params.append("foo~!@#$%^&*()_+-=", "jk")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&foo%7E%21%40%23%24%25%5E%26*%28%29_%2B-%3D=jk")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09200
* @tc.name: testUrlAppend004
* @tc.desc: Appends a specified key/value pair as a new search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlAppend004() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2')  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search)
	params.append("app", "par")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&app=par")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09300
* @tc.name: testUrlAppend005
* @tc.desc: Appends a specified key/value pair as a new search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlAppend005() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2')  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search)
	params.append("123", "456")
	let result = params.toString()
 arktest.assertEQ(result, "foo=1&bar=2&123=456")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09400
* @tc.name: testUrlAppend006
* @tc.desc: Appends a specified key/value pair as a new search parameter.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
// function testUrlAppend006() {
// let that = TestURL.parseURL('https://example.com?foo=1&bar=2')
// let params = new url.URLParams(that.search)
// try {
//     let a = 1;
//     params.append(a, "123")
// } catch(e) {
//     arktest.assertEQ(e.toString(),`BusinessError: Parameter error. The type of ${a} must be string`);
//     arktest.assertEQ(e.code,401)
//     arktest.assertEQ(e.message,`Parameter error. The type of ${a} must be string`);
// }
// }

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09500
* @tc.name: testUrlDelete001
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlDelete001() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2')  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search)
	params.delete("foo")
	let result = params.toString()
 arktest.assertEQ(result, "bar=2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09600
* @tc.name: testUrlDelete002
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlDelete002() {
	let that = url.URL.parseURL('https://example.com?foo大=1&bar=2');  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search);
	params.delete('foo');
	let result = params.toString();
 arktest.assertEQ(result, "foo%E5%A4%A7=1&bar=2");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09700
* @tc.name: testUrlDelete003
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlDelete003() {
	let that = url.URL.parseURL('https://example.com?foo大=1&bar=2');  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search);
	params.delete("foo大");
	let result = params.toString();
 arktest.assertEQ(result, "bar=2")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09800
* @tc.name: testUrlDelete004
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlDelete004() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2');  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search);
	params.delete('bar');
	let result = params.toString();
 arktest.assertEQ(result, "foo=1");
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_09900
* @tc.name: testUrlDelete005
* @tc.desc: Deletes the given search parameter and its associated value,from the list of all search parameters.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlDelete005() {
	let that = url.URL.parseURL('https://example.com?foo=1&bar=2');  // CC-OFF(G.OTH.05) test logic
	let params = new url.URLParams(that.search);
	params.delete("faa");
	let result = params.toString();
 arktest.assertEQ(result, "foo=1&bar=2")
}



function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testParamsSort005", testParamsSort005);
	suite.addTest("testParamsValues001", testParamsValues001);
	suite.addTest("testParamsValues002", testParamsValues002);
	suite.addTest("testParamsValues003", testParamsValues003);
	suite.addTest("testParamsValues004", testParamsValues004);
	suite.addTest("testParamsValues005", testParamsValues005);
	suite.addTest("testParamsToString001", testParamsToString001);
	suite.addTest("testParamsToString002", testParamsToString002);
	suite.addTest("testParamsToString003", testParamsToString003);
	suite.addTest("testParamsToString004", testParamsToString004);
	suite.addTest("testParamsToString005", testParamsToString005);
	suite.addTest("testParamsConstruction001", testParamsConstruction001);
	suite.addTest("testParamsConstruction002", testParamsConstruction002);
	suite.addTest("testParamsConstruction003", testParamsConstruction003);
	suite.addTest("testParamsConstruction006", testParamsConstruction006);
	suite.addTest("testParamsConstruction007", testParamsConstruction007);
	suite.addTest("testParamsConstruction010", testParamsConstruction010);
	suite.addTest("testNewURLSearchParams001", testNewURLSearchParams001);
	suite.addTest("testUrlAppend001", testUrlAppend001);
	suite.addTest("testUrlAppend002", testUrlAppend002);
	suite.addTest("testUrlAppend003", testUrlAppend003);
	suite.addTest("testUrlAppend004", testUrlAppend004);
	suite.addTest("testUrlAppend005", testUrlAppend005);
	suite.addTest("testUrlDelete001", testUrlDelete001);
	suite.addTest("testUrlDelete002", testUrlDelete002);
	suite.addTest("testUrlDelete003", testUrlDelete003);
	suite.addTest("testUrlDelete004", testUrlDelete004);
	suite.addTest("testUrlDelete005", testUrlDelete005);

	//passed
	return suite.run();
}
