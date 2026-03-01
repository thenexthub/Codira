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
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17700
* @tc.name: testUrlHref001
* @tc.desc:  Gets and sets the serialized URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHref001() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	let result = params.href
 arktest.assertEQ(result, "http://username:password@host:8080/directory/file?query#fragment")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17800
* @tc.name: testUrlHref002
* @tc.desc:  Gets and sets the serialized URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHref002() {
	let params = url.URL.parseURL('http://host:8080/directory/file?query#fragment')
	let result = params.href
 arktest.assertEQ(result, "http://host:8080/directory/file?query#fragment")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_17900
* @tc.name: testUrlHref003
* @tc.desc:  Gets and sets the serialized URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHref003() {
	let params = url.URL.parseURL('http://username:password@host:8080')
	let result = params.href
 arktest.assertEQ(result, "http://username:password@host:8080/")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18000
* @tc.name: testUrlHref004
* @tc.desc:  Gets and sets the serialized URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHref004() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	params.href = 'http1://myhost/path?sss=1#ff';
	let result = params.href
 arktest.assertEQ(result, "http1://myhost/path?sss=1#ff")
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18100
* @tc.name: testUrlHref005
* @tc.desc:  Gets and sets the serialized URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHref005() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	params.href = 'http://myhost:99/path#ff';
	let result = params.href
 arktest.assertEQ(result, "http://myhost:99/path#ff")
}

//Sourced from Xts ActsUrlStaticTest#urlTest2#testUrlConstruction012
function testUrlHref006() {
    let params = url.URL.parseURL('htt1p://www.0902zy.cn/path?abc=123&def=456#yyyy')
    let params1 = url.URL.parseURL('//www.baidu.com/ssaa',params)
    const result = params1.href
	arktest.assertEQ(result, 'htt1p://www.baidu.com/ssaa')
}

//Sourced from Xts ActsUrlStaticTest#Escape_Url#testUrlParseUrl9Escape0021
function testUrlHref007() {
	let urlString1 = '\\b/path2';
	let urlString2 = '/\\b/path2';
	let url1 = url.URL.parseURL(urlString1, 'https://www.example.com/test/test1');
	let url2 = url.URL.parseURL(urlString2, 'https://www.example.com/test/test1');
	let temp1 = '\\b/path2';
	let rstUrl1 = 'https://www.example.com/test/' + temp1;
	let rstUrl2 = 'https://b/path2';
	arktest.assertEQ(url1.toString(), rstUrl1);
	arktest.assertEQ(url2.toString(), rstUrl2);
}

//Sourced from Xts ActsUrlStaticTest#urlTest2#testUrlparseURL0065
function testUrlHrefColonFinal() {
	let params = url.URL.parseURL('../h:', 'http://www.example.com');
    const result= params.href;
	arktest.assertEQ(result, 'http://www.example.com/h:');
}

//Sourced from Xts ActsUrlStaticTest#urlTest2#testUrlConstruction006
function testUrlDelete()
{
	const obj = url.URL.parseURL('https://sssw.dasf?key2=val&key');
	obj.params.sort();
	obj.params.delete('key2');
	obj.params.delete('key');
	obj.params.sort();
	arktest.assertEQ(obj.toString(),'https://sssw.dasf/');
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18200
* @tc.name: testUrlOrigin001
* @tc.desc:  Gets the read-only serialization of the URL's origin.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlOrigin001() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	let result = params.origin
 arktest.assertEQ(result, 'http://host:8080')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18300
* @tc.name: testUrlOrigin002
* @tc.desc:  Gets the read-only serialization of the URL's origin.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlOrigin002() {
	let params = url.URL.parseURL('http://username:password@host:11/directory/file?query#fragment')
	let result = params.origin
 arktest.assertEQ(result, 'http://host:11')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18400
* @tc.name: testUrlOrigin003
* @tc.desc:  Gets the read-only serialization of the URL's origin.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlOrigin003() {
	let params = url.URL.parseURL('http://username:password@host/directory/file?query#fragment')
	let result = params.origin
 arktest.assertEQ(result, 'http://host')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18500
* @tc.name: testUrlOrigin004
* @tc.desc:  Gets the read-only serialization of the URL's origin.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlOrigin004() {
	let params = url.URL.parseURL('http://username:password@aaaasshost:212/directory/file?query#fragment')
	let result = params.origin
 arktest.assertEQ(result, 'http://aaaasshost:212')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18600
* @tc.name: testUrlOrigin005
* @tc.desc:  Gets the read-only serialization of the URL's origin.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlOrigin005() {
	let params = url.URL.parseURL('http://username:password@host22:100#fragment')
	let result = params.origin
 arktest.assertEQ(result, 'http://host22:100')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18700
* @tc.name: testUrlProtocol001
* @tc.desc: Gets and sets the protocol portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlProtocol001() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	let result = params.protocol
 arktest.assertEQ(result, 'http:')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18800
* @tc.name: testUrlProtocol002
* @tc.desc: Gets and sets the protocol portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlProtocol002() {
	let params = url.URL.parseURL('http1://username:password@host:8080/directory/file?query#fragment')
	let result = params.protocol
 arktest.assertEQ(result, 'http1:')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_18900
* @tc.name: testUrlProtocol003
* @tc.desc: Gets and sets the protocol portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlProtocol003() {
	let params = url.URL.parseURL('https://username:password@host:8080/directory/file?query#fragment')
	let result = params.protocol
 arktest.assertEQ(result, 'https:')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19000
* @tc.name: testUrlProtocol004
* @tc.desc: Gets and sets the protocol portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlProtocol004() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	params.protocol = 'https:';
	let result = params.protocol
 arktest.assertEQ(result, 'https:')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19100
* @tc.name: testUrlProtocol005
* @tc.desc: Gets and sets the protocol portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlProtocol005() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	params.protocol = 'ftp:';
	let result = params.protocol
 arktest.assertEQ(result, 'ftp:')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19200
* @tc.name: testUrlUsername001
* @tc.desc: Gets and sets the username portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlUsername001() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	let result = params.username
 arktest.assertEQ(result, 'username')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19300
* @tc.name: testUrlUsername002
* @tc.desc: Gets and sets the username portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlUsername002() {
	let params = url.URL.parseURL('http://zhao:password@host:8080/directory/file?query#fragment')
	let result = params.username
 arktest.assertEQ(result, 'zhao')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19400
* @tc.name: testUrlUsername003
* @tc.desc: Gets and sets the username portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlUsername003() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	params.username = 'skk'
	let result = params.username
 arktest.assertEQ(result, 'skk')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19500
* @tc.name: testUrlUsername004
* @tc.desc: Gets and sets the username portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlUsername004() {
	let params = url.URL.parseURL('http://username:password@host:8080/directory/file?query#fragment')
	params.username = 'skp111'
	let result = params.username
 arktest.assertEQ(result, 'skp111')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19600
* @tc.name: testUrlUsername005
* @tc.desc: Gets and sets the username portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlUsername005() {
	let params = url.URL.parseURL('http://usme@host:8080/directory/file?query#fragment')
	let result = params.username
 arktest.assertEQ(result, 'usme')
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19700
* @tc.name: testUrlHostName001
* @tc.desc: Gets and sets the hostname portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHostName001() {
	let params = url.URL.parseURL('http://usme@host:8080/directory/file?query#fragment');
	let result = params.hostname;
 arktest.assertEQ(result, 'host');
    params.hostname = 'www.test.com';
    arktest.assertEQ(params.hostname, 'www.test.com');
    arktest.assertEQ(params.host, 'www.test.com:8080');
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19800
* @tc.name: testUrlHostName002
* @tc.desc: Gets and sets the hostname portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHostName002() {
	let params = url.URL.parseURL('http://usme@host1:8080/directory/file?query#fragment');
	let result = params.hostname;
 arktest.assertEQ(result, 'host1');
    params.hostname = 'www.test.com';
    arktest.assertEQ(params.host, 'www.test.com:8080');
    arktest.assertEQ(params.href, 'http://usme@www.test.com:8080/directory/file?query#fragment');
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_19900
* @tc.name: testUrlHost001
* @tc.desc: Gets and sets the host portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlHost001() {
	let params = url.URL.parseURL('http://usme@host:8080/directory/file?query#fragment');
	let result = params.host;
 arktest.assertEQ(result, 'host:8080');
    params.host = 'www.test:99';
    arktest.assertEQ(params.host, 'www.test:99');
    arktest.assertEQ(params.hostname, 'www.test');
    arktest.assertEQ(params.port, '99');
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_20000
* @tc.name: testUrlPort001
* @tc.desc: Gets and sets the port portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlPort001() {
	let params = url.URL.parseURL('http://usme@host:8080/directory/file?query#fragment');
	let result = params.port;
 arktest.assertEQ(result, '8080');
    params.port = '77';
    arktest.assertEQ(params.host, 'host:77');
}

/**
* @tc.number: SUB_COMMONLIBRARY_ETSUTILS_URL_20100
* @tc.name: testUrlGetHost001
* @tc.desc: Gets and sets the host portion of the URL.
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 2
*/
function testUrlGetHost001() {
	let params = url.URL.parseURL('http://usme@host:8080/directory/file?query#fragment');
	let result = params.host;
 arktest.assertEQ(result, 'host:8080');
    params.port = '77';
    params.hostname = 'www.test.com';
    arktest.assertEQ(params.host, 'www.test.com:77');
    arktest.assertEQ(params.toString(), 'http://usme@www.test.com:77/directory/file?query#fragment');
}

function testUrlConstruction013() {
	let urlStr = 'htt1p://www.0902zy.cn/path?abc=123&def=456#yyyy';
	let murl = url.URL.parseURL('', urlStr);
	arktest.assertEQ(murl.href, urlStr);
}

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testUrlHref001", testUrlHref001);
	suite.addTest("testUrlHref002", testUrlHref002);
	suite.addTest("testUrlHref003", testUrlHref003);
	suite.addTest("testUrlHref004", testUrlHref004);
	suite.addTest("testUrlHref005", testUrlHref005);
	suite.addTest("testUrlHref006", testUrlHref006);
	suite.addTest("testUrlHref007", testUrlHref007);
	suite.addTest("testUrlHrefColonFinal", testUrlHrefColonFinal);
	suite.addTest("testUrlDelete", testUrlDelete);
	suite.addTest("testUrlOrigin001", testUrlOrigin001);
	suite.addTest("testUrlOrigin002", testUrlOrigin002);
	suite.addTest("testUrlOrigin003", testUrlOrigin003);
	suite.addTest("testUrlOrigin004", testUrlOrigin004);
	suite.addTest("testUrlOrigin005", testUrlOrigin005);
	suite.addTest("testUrlProtocol001", testUrlProtocol001);
	suite.addTest("testUrlProtocol002", testUrlProtocol002);
	suite.addTest("testUrlProtocol003", testUrlProtocol003);
	suite.addTest("testUrlProtocol004", testUrlProtocol004);
	suite.addTest("testUrlProtocol005", testUrlProtocol005);
	suite.addTest("testUrlUsername001", testUrlUsername001);
	suite.addTest("testUrlUsername002", testUrlUsername002);
	suite.addTest("testUrlUsername003", testUrlUsername003);
	suite.addTest("testUrlUsername004", testUrlUsername004);
	suite.addTest("testUrlUsername005", testUrlUsername005);
    suite.addTest("testUrlHostName001", testUrlHostName001);
    suite.addTest("testUrlHostName002", testUrlHostName002);
    suite.addTest("testUrlHost001", testUrlHost001);
    suite.addTest("testUrlPort001", testUrlPort001);
    suite.addTest("testUrlGetHost001", testUrlGetHost001);
    suite.addTest("testUrlConstruction013", testUrlConstruction013);

	//passed
	return suite.run();
}
