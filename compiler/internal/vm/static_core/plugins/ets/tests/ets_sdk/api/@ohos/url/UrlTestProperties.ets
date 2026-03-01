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

function testUrlTest068() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?ppp9875=77#fragment");
 arktest.assertEQ(murl.search, "?ppp9875=77");
	console.log('-------------------------68 done')
}

function testUrlTest069() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?ppp9875=77#fragment");
 arktest.assertEQ(murl.search, "?ppp9875=77");
	murl.search = "";
 arktest.assertEQ(murl.search, "");
	console.log('-------------------------69 done')
}

function testUrlTest070() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?ppp9875=77#fragment");
 arktest.assertEQ(murl.search, "?ppp9875=77");
	murl.search = "?444=555";
 arktest.assertEQ(murl.search, "?444=555");
	console.log('-------------------------70 done')
}

function testUrlTest071() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?ppp9875=77#fragment");
 arktest.assertEQ(murl.search, "?ppp9875=77");
	murl.search = "444=555";
 arktest.assertEQ(murl.search, "?444=555");
	console.log('-------------------------71 done')
}

function testUrlTest072() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?ppp9875=77#fragment");
 arktest.assertEQ(murl.search, "?ppp9875=77");
	murl.search = "?##44=55";
 arktest.assertEQ(murl.search, "?%23%2344=55");
	console.log('-------------------------72 done')
}

function testUrlTest073() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#fragment");
 arktest.assertEQ(murl.hash, "#fragment");
	console.log('-------------------------73 done')
}

function testUrlTest074() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
 arktest.assertEQ(murl.hash, "#123456");
	console.log('-------------------------74 done')
}

function testUrlTest075() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
 arktest.assertEQ(murl.hash, "#123456");
	murl.hash = "";
 arktest.assertEQ(murl.hash, "");
	console.log('-------------------------75 done')
}

function testUrlTest076() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
 arktest.assertEQ(murl.hash, "#123456");
	murl.hash = "#445566";
 arktest.assertEQ(murl.hash, "#445566");
	console.log('-------------------------76 done')
}

function testUrlTest077() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
 arktest.assertEQ(murl.hash, "#123456");
	murl.hash = "445566";
 arktest.assertEQ(murl.hash, "#445566");
	console.log('-------------------------77 done')
}

function testUrlTest078() {
	let murl = url.URL.parseURL("http://[1080:0:0:0:8:800:200C:417A]/index.html");
 arktest.assertEQ(murl.urlInner.flags[0], false);
	console.log('-------------------------78 done')
}

function testUrlTest079() {
	let murl = url.URL.parseURL("http://0377.0xff.255.1:80/index.html");
 arktest.assertEQ(murl.urlInner.flags[0], false);
	console.log('-------------------------79 done')
}

function testUrlTest080() {
	try {
		let murl = url.URL.parseURL("http://[1080:0:0:0:8:800:200C.417A]/index.html");
	 arktest.assertEQ(murl.urlInner.flags[0], true);
	} catch (err) {
		console.log('-------------------------80 done ' + err)
	}
}

function testUrlTest081() {
	let murl = url.URL.parseURL("http://0377.0xff.255.1:80/index.html");
 arktest.assertEQ(murl.urlInner.flags[0], false);
	console.log('-------------------------81 done')
}

function testUrlTest082() {
	try {
		let murl = url.URL.parseURL("http://192.168.259.551:80/index.html");
	 arktest.assertEQ(murl.urlInner.flags[0], true);
	} catch (err) {
		console.log('-------------------------82 done ' + err)
	}
}

function testUrlTest083() {
	let murl = url.URL.parseURL("http://0377.0xff.255.1:80/index.html");
	murl.hostname = "host";
 arktest.assertEQ(murl.hostname, "host");
	console.log('-------------------------83 done')
}

function testUrlTest084() {
	let murl = url.URL.parseURL("http://0377.0xff.255.1:80/index.html");
	murl.hostname = "";
 arktest.assertEQ(murl.hostname, "255.255.255.1");
	console.log('-------------------------84 done')
}

function testUrlTest085() {
	let murl = url.URL.parseURL("file:///www.example.com");
	murl.hostname = "localhost/";
 arktest.assertEQ(murl.urlInner.flags[0], false);
	console.log('-------------------------85 done')
}

function testUrlTest086() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?foo=1&bar=2");
 arktest.assertEQ(murl.search, "?foo=1&bar=2");
	console.log('-------------------------86 done')
}

function testUrlTest087() {
	let murl = url.URL.parseURL("https://example.com?d=value1&c=value2&b=大&4=key4");  // CC-OFF(G.OTH.05) test logic
 arktest.assertEQ(murl.search, "?d=value1&c=value2&b=%E5%A4%A7&4=key4");
	console.log('-------------------------87 done')
}

function testUrlTest088() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?你好=china#qwer=da");
 arktest.assertEQ(murl.search, "?%E4%BD%A0%E5%A5%BD=china");
	console.log('-------------------------88 done')
}

function testUrlTest089() {
	let params = new url.URLParams();
	params.append("ma", "jk");
 arktest.assertEQ(params.toString(), "ma=jk");
	console.log('-------------------------89 done')
}

function testUrlTest090() {
	let params = new url.URLParams();
	params.append("ma 大", "jk￥");
 arktest.assertEQ(params.toString(), "ma+%E5%A4%A7=jk%EF%BF%A5");
	console.log('-------------------------90 done')
}

function testUrlTest091() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", "jk");
 arktest.assertEQ(params.toString(), "foo%7E%21%40%23%24%25%5E%26*%28%29_%2B-%3D=jk");
	console.log('-------------------------91 done')
}

function testUrlTest092() {
	let params = new url.URLParams();
	params.append('', "jk");
 arktest.assertEQ(params.get('jk'), undefined);
	console.log('-------------------------92 done')
}

function testUrlTest093() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", '');
 arktest.assertEQ(params.get('foo~!@#$%^&*()_+-='), '');
	console.log('-------------------------93 done')
}

function testUrlTest094() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", "jk");
	params.delete("foo~!@#$%^&*()_+-=");
 arktest.assertEQ(params.toString(), "");
	console.log('-------------------------94 done')
}

function testUrlTest095() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", "jk");
	params.append("ma", "jk");
	params.delete("foo~!@#$%^&*()_+-=");
 arktest.assertEQ(params.toString(), "ma=jk");
	console.log('-------------------------95 done')
}

function testUrlTest096() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", "jk");
	params.append("ma", "jk");
	params.delete("foo~!@#$%^&*()_+-=");
	params.delete("ma");
 arktest.assertEQ(params.toString(), "");
	console.log('-------------------------96 done')
}

function testUrlTest097() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", "jk");
	params.append("ma", "jk");
	params.delete("foo~!@#$%^&*()_+-=");
	params.delete('');
 arktest.assertEQ(params.get('ma'), "jk");
	console.log('-------------------------97 done')
}

function testUrlTest098() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", "jk");
 arktest.assertEQ(params.get('foo~!@#$%^&*()_+-='), "jk");
	console.log('-------------------------98 done')
}

function testUrlTest099() {
	let params = new url.URLParams();
 arktest.assertEQ(params.get('jk'), undefined);
	console.log('-------------------------99 done')
}

function testUrlTest100() {
	let params = new url.URLParams();
 arktest.assertEQ(params.get("hello, \U0001F600 world!"), undefined);
	console.log('-------------------------100 done')
}

function testUrlTest101() {
	let params = new url.URLParams([["foo~!@#$%^&*()_+-=", "jk"]]);
 arktest.assertEQ(params.get("hello, \U0001F600 world!"), undefined);
	console.log('-------------------------101 done')
}


function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");

	suite.addTest("testUrlTest068", testUrlTest068);
	suite.addTest("testUrlTest069", testUrlTest069);
	suite.addTest("testUrlTest070", testUrlTest070);
	suite.addTest("testUrlTest071", testUrlTest071);
	suite.addTest("testUrlTest072", testUrlTest072);
	suite.addTest("testUrlTest073", testUrlTest073);
	suite.addTest("testUrlTest074", testUrlTest074);
	suite.addTest("testUrlTest075", testUrlTest075);
	suite.addTest("testUrlTest076", testUrlTest076);
	suite.addTest("testUrlTest077", testUrlTest077);
	suite.addTest("testUrlTest078", testUrlTest078);
	suite.addTest("testUrlTest079", testUrlTest079);
	suite.addTest("testUrlTest080", testUrlTest080);
	suite.addTest("testUrlTest081", testUrlTest081);
	suite.addTest("testUrlTest082", testUrlTest082);
	suite.addTest("testUrlTest083", testUrlTest083);
	suite.addTest("testUrlTest084", testUrlTest084);
	suite.addTest("testUrlTest085", testUrlTest085);
	suite.addTest("testUrlTest086", testUrlTest086);
	suite.addTest("testUrlTest087", testUrlTest087);
	suite.addTest("testUrlTest088", testUrlTest088);
	suite.addTest("testUrlTest089", testUrlTest089);
	suite.addTest("testUrlTest090", testUrlTest090);
	suite.addTest("testUrlTest091", testUrlTest091);
	suite.addTest("testUrlTest092", testUrlTest092);
	suite.addTest("testUrlTest093", testUrlTest093);
	suite.addTest("testUrlTest094", testUrlTest094);
	suite.addTest("testUrlTest095", testUrlTest095);
	suite.addTest("testUrlTest096", testUrlTest096);
	suite.addTest("testUrlTest097", testUrlTest097);
	suite.addTest("testUrlTest098", testUrlTest098);
	suite.addTest("testUrlTest099", testUrlTest099);
	suite.addTest("testUrlTest100", testUrlTest100);
	suite.addTest("testUrlTest101", testUrlTest101);

	//passed
	return suite.run();
}
