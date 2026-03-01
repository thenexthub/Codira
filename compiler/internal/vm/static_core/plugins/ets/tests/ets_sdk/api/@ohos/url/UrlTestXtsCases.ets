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
import { BusinessError } from "@ohos.base";

function testGetAllValuesForKey0005() {
	let k = '耄耋鍟毗粢ASGDDest1312%#^$%';
	let searchParamsObject = new url.URLParams(k + "=1&a=2&b=3&" + k + "a!$#%#=4&c=5");
	let valuesForKeyD = searchParamsObject.getAll(k);
	let expectedValuesForKeyD = ["1"];
	arktest.assertEQ(String(valuesForKeyD.length), String(expectedValuesForKeyD.length));
	for (let i = 0; i < valuesForKeyD.length; i++) {
		arktest.assertEQ(String(valuesForKeyD[i]), String(expectedValuesForKeyD[i]));
	}
}

interface KeyTestCase {
	key: string;
	expectedResult: boolean;
}
function testGetValueForKey0005() {
	let searchParamsObject = new url.URLParams("耄耋鍟毗粢ASGDDest1312%#^$%=1&a=2&b=3&" +
		"耄耋鍟毗粢ASGDDest1312%#^$%a!$#%#=4&c=5");
	let valueForKeyD = searchParamsObject.get("耄耋鍟毗粢ASGDDest1312%#^$%");
	let expectedValueForKeyD = "1";
	arktest.assertEQ(String(valueForKeyD), String(expectedValueForKeyD));
}

function testHasKey0005() {
	const testKeyPresence = (url: string, keys: KeyTestCase[]) => {
		let searchParamsObject = new url.URLParams(url);
		for (let i = 0; i < keys.length; i++) {
			let hasKey = searchParamsObject.has(keys[i].key);
			arktest.assertEQ(String(hasKey), String(keys[i].expectedResult));
		}
	};
	const testCases1: KeyTestCase[] = [
		{
			key: "耄耋鍟毗粢ASGDDest1312%#^$%", expectedResult: true
		}
	];
	testKeyPresence("耄耋鍟毗粢ASGDDest1312%#^$%=1&a=2&b=3&耄耋鍟毗粢ASGDDest1312%#^$%a!$#%#=4&c=5", testCases1);
}

function testSpecial0001() {
	let params1 = new url.URLParams('fod=!!#%(())**++-..//::<<==>>??[');
	let params2 = new url.URLParams('fod=!!22%22%23%23%24%24%25%25%26%26(())**%2B');
	let params3 = new url.URLParams('fod=21%2122%2223%23%24%24%25%25%26%26%27%27%28');
	let res1 = params1.get('fod');
	let res2 = params2.get('fod');
	let res3 = params3.get('fod');
	arktest.assertEQ(res1, "!!#%(())**  -..//::<<==>>??[");
	arktest.assertEQ(res2, "!!22\"##$$%%&&(())**+");
	arktest.assertEQ(res3, "21!22\"23#$$%%&&''(");
}

function testSpecial0014() {
	let str1 = 'nan=po&fod=%21%21%22%22%23%23%24%24%25%25&=&%27%27%28%28%29' +
		'%29**++%2C%2C--..%2F%2F%3A%3A%3B%3B%3C%3C=%3D%3E%3E%3F%3F%40%40%5B' +
		'%5B%5C%5C%5D%5D%5E%5E__%60%60%7B%7B%7C%7C%7D%7D%7E%7E&bard=2';
	let str2 = 'nan=po&fod=%21%21%22%22%23%23%24%24%25%25%26%26%27%27%28%28' +
		'%29%29**%2B%2B%2C%2C--..%2F%2F%3A%3A%3B%3B%3C%3C%3D%3D%3E%3E%3F%3F' +
		'%40%40%5B%5B%5C%5C%5D%5D%5E%5E__%60%60%7B%7B%7C%7C%7D%7D%7E%7E&bard=2';
	let str3 = 'nan=po&fod=%21%21%22%22%23%23%24%24%25%25%26%26%27%27%28%28' +
		'**%2B%2B%2C%2C--..%2F%2F%3A%3A%3B%3B%3C%3C%3D%3D%3E%3E%3F%3F%40%40' +
		'%5B%5B%5C%5C%5D%5D%5E%5E__%60%60%7B%7B%7C%7C%7D%7D%7E%7E&bard=2';
	let Params1 = new url.URLParams('nan=po&fod=!!""##' +
		'$$%%&&\'\'(())**++,,--..//::;;<<==>>??@@[[\\\\]]^^__``{{||}}~~&bard=2');
	let Params2 = new url.URLParams('nan=po&fod=!!%22%22%23%23%24%24%25%25%26' +
		'%26\'\'(())**%2B%2B%2C%2C--..%2F%2F%3A%3A%3B%3B%3C%3C%3D%3D%3E%3E%3F' +
		'%3F%40%40%5B%5B%5C%5C%5D%5D%5E%5E__%60%60%7B%7B%7C%7C%7D%7D~~&bard=2');
	let Params3 = new url.URLParams('nan=po&fod=%21%21%22%22%23%23%24%24%25%2' +
		'5%26%26%27%27%28%28%2A%2A%2B%2B%2C%2C%2D%2D%2E%2E%2F%2F%3A%3A%3B%3B%' +
		'3C%3C%3D%3D%3E%3E%3F%3F%40%40%5B%5B%5C%5C%5D%5D%5E%5E%5F%5F%60%60%7B' +
		'%7B%7C%7C%7D%7D%7E%7E&bard=2');
	arktest.assertEQ(Params1.toString(), str1);
	arktest.assertEQ(Params2.toString(), str2);
	arktest.assertEQ(Params3.toString(), str3);
}

function testUrlIPv6001() {
	let params = url.URL.parseURL('http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html')
	const result = params.href
	arktest.assertEQ(result, 'http://[fedc:ba98:7654:3210:fedc:ba98:7654:3210]/index.html')
}

function testUrlIPv6002() {
	let params = url.URL.parseURL('http://[1080:0:0:0:8:800:200C:417A]/index.html')
	const result = params.href
	arktest.assertEQ(result, 'http://[1080::8:800:200c:417a]/index.html')
}

function testUrlIPv6003() {
	let params = url.URL.parseURL('http://[::FFFF:129.144.52.38]:80/index.html')
	const result = params.href
	arktest.assertEQ(result, 'http://[::ffff:8190:3426]/index.html')
}

function testUrlIPv4001() {
	let params = url.URL.parseURL('http://0377.0xff.255.1:80/index.html')
	const result = params.href
	arktest.assertEQ(result, 'http://255.255.255.1/index.html')
}

function testUrlIPv4002() {
	let params = url.URL.parseURL('http://0377.0xff.255.g/index.html')
	const result = params.href
	arktest.assertEQ(result, 'http://0377.0xff.255.g/index.html')
}

function testUrlIPv4003() {
	let params = url.URL.parseURL('http://190.254.245.9:80/index.html')
	const result = params.href
	arktest.assertEQ(result, 'http://190.254.245.9/index.html')
}


function testUrlparseURL0031() {
	let strUrl = "bytedance://large_image?url=https%3A%2F%2Fp3-sign.toutiaoimg.com%2Flarge%2Ftos-cn-i-6w9my0ksv" +
		"p%2Fc2c8b671c3ea4f968ae3a89a21b5f952%3F_iz%3D67594%26from%3Darticle.detail%26x-expires%3D1703755589%26x" +
		"-signature%3DFUbhPcvUfgI%252BJLdkCiaj1S1pBSQ%253D&index=0";
	let res = "https://p3-sign.toutiaoimg.com/large/tos-cn-i-6w9my0ksvp/c2c8b671c3ea4f968ae3a89a21b5f952?_iz=" +
		"67594&from=article.detail&x-expires=1703755589&x-signature=FUbhPcvUfgI%2BJLdkCiaj1S1pBSQ%3D"
	let urlObj = url.URL.parseURL(strUrl)
	const result = urlObj.params.get("url")
	arktest.assertEQ(result, res)
}

function testUrlparseURL0045() {
	let uu = url.URL.parseURL('http://www.baidu.com/sajd中文测试/aa?xx=%26优优');
	let uu2 = url.URL.parseURL(uu.toString());
	let res = 'http://www.baidu.com/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa?xx=%26%E4%BC%98%E4%BC%98';
	arktest.assertEQ(uu2.toString(), res);
	arktest.assertEQ(uu2.pathname, '/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa');
	arktest.assertEQ(uu2.params.get('xx'), '&优优');
}

function testUrlparseURL0046() {
	let uu = url.URL.parseURL('http://www.baidu.com/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa?xx=%26优优');
	let uu2 = url.URL.parseURL(uu.toString());
	let res = 'http://www.baidu.com/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa?xx=%26%E4%BC%98%E4%BC%98';
	arktest.assertEQ(uu2.toString(), res);
}

function testUrlparseURL0048() {
	let uu = url.URL.parseURL('http://www.baidu.com/sajd中文测试/aa?xx=%26优优');
	let uu2 = url.URL.parseURL(uu.toString());
	let res = 'http://www.baidu.com/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa?xx=%26%E4%BC%98%E4%BC%98';
	arktest.assertEQ(uu2.toString(), res);
	arktest.assertEQ(uu2.pathname, '/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa');
	arktest.assertEQ(uu2.params.get('xx'), '&优优');
}


function testUrlparseURL0053() {
	let uu = url.URL.parseURL('http://www.baidu.com/sajd中文测试/aa?xx=%26优优');
	let uu2 = url.URL.parseURL(uu.toString());
	let res = 'http://www.baidu.com/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa?xx=%26%E4%BC%98%E4%BC%98';
	arktest.assertEQ(uu2.toString(), res);
	arktest.assertEQ(uu2.pathname, '/sajd%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95/aa');
	arktest.assertEQ(uu2.params.get('xx'), '&优优');
}


function testUrlPathname006() {
	let obj = url.URL.parseURL('http://username:password@host:8080/directory/file?foo=1&bar=2');
	let str = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ\\[]"
		+ "^_`abcdefghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸"
		+ "¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
	obj.pathname = str;
	const pathnameEncode = "/%20!%22#$%&'()*+,-./0123456789:;%3C=%3E?@ABCDEFGHIJKLMNOPQRSTUVWXYZ/[]%5E_%60abcde"
		+ "fghijklmnopqrstuvwxyz%7B|%7D~%E2%82%AC%E2%80%9A%C6%92%E2%80%9E%E2%80%A6%E2%80%A0%E2%80%A1%CB%86%E2%80"
		+ "%B0%C5%A0%E2%80%B9%C5%92%C5%BD%E2%80%98%E2%80%99%E2%80%9C%E2%80%9D%E2%80%A2%E2%80%93%E2%80%94%CB%9C%E2"
		+ "%84%A2%C5%A1%E2%80%BA%C5%93%C5%BE%C5%B8%C2%A1%C2%A2%C2%A3%C2%A4%C2%A5%C2%A6%C2%A7%C2%A8%C2%A9%C2%AA%C2"
		+ "%AB%C2%AC%C2%AD%C2%AE%C2%AF%C2%B0%C2%B1%C2%B2%C2%B3%C2%B4%C2%B5%C2%B6%C2%B7%C2%B8%C2%B9%C2%BA%C2%BB%C2"
		+ "%BC%C2%BD%C2%BE%C2%BF%C3%80%C3%81%C3%82%C3%83%C3%84%C3%85%C3%86%C3%87%C3%88%C3%89%C3%8A%C3%8B%C3%8C%C3"
		+ "%8D%C3%8E%C3%8F%C3%90%C3%91%C3%92%C3%93%C3%94%C3%95%C3%96%C3%97%C3%98%C3%99%C3%9A%C3%9B%C3%9C%C3%9D%C3"
		+ "%9E%C3%9F%C3%A0%C3%A1%C3%A2%C3%A3%C3%A4%C3%A5%C3%A6%C3%A7%C3%A8%C3%A9%C3%AA%C3%AB%C3%AC%C3%AD%C3%AE%C3"
		+ "%AF%C3%B0%C3%B1%C3%B2%C3%B3%C3%B4%C3%B5%C3%B6%C3%B7%C3%B8%C3%B9%C3%BA%C3%BB%C3%BC%C3%BD%C3%BE%C3%BF";
	arktest.assertEQ(obj.pathname, pathnameEncode);
	obj.pathname = pathnameEncode;
	arktest.assertEQ(obj.pathname, pathnameEncode);
}


function testUrlHref006() {
	let obj = url.URL.parseURL('http://username:password@host:8080/directory/file?foo=1&bar=2');
	let str = " !\"#$%&'()*+,-./09:;<=>?@A\\[]^_`a{|}~€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁ"
	let hrefEncode = "http://%20!%22%23$%&'()*+,-.%2F09%3A%3B%3C%3D%3E%3F%40A%5C%5B%5D%5E_%60a%7B%7C%7D~%E2"
		+ "%82%AC%E2%80%9A%C6%92%E2%80%9E%E2%80%A6%E2%80%A0%E2%80%A1%CB%86%E2%80%B0%C5%A0%E2%80%B9%C5%92%C5%BD"
		+ "%E2%80%98%E2%80%99%E2%80%9C%E2%80%9D%E2%80%A2%E2%80%93%E2%80%94%CB%9C%E2%84%A2%C5%A1%E2%80%BA%C5%93"
		+ "%C5%BE%C5%B8%C2%A1%C2%A2%C2%A3%C2%A4%C2%A5%C2%A6%C2%A7%C2%A8%C2%A9%C2%AA%C2%AB%C2%AC%C2%AD%C2%AE%C2"
		+ "%AF%C2%B0%C2%B1%C2%B2%C2%B3%C2%B4%C2%B5%C2%B6%C2%B7%C2%B8%C2%B9%C2%BA%C2%BB%C2%BC%C2%BD%C2%BE%C2%BF"
		+ "%C3%80%C3%81:%20!%22%23$%&'()*+,-.%2F09%3A%3B%3C%3D%3E%3F%40A%5C%5B%5D%5E_%60a%7B%7C%7D~%E2%82%AC%E2"
		+ "%80%9A%C6%92%E2%80%9E%E2%80%A6%E2%80%A0%E2%80%A1%CB%86%E2%80%B0%C5%A0%E2%80%B9%C5%92%C5%BD%E2%80%98"
		+ "%E2%80%99%E2%80%9C%E2%80%9D%E2%80%A2%E2%80%93%E2%80%94%CB%9C%E2%84%A2%C5%A1%E2%80%BA%C5%93%C5%BE%C5"
		+ "%B8%C2%A1%C2%A2%C2%A3%C2%A4%C2%A5%C2%A6%C2%A7%C2%A8%C2%A9%C2%AA%C2%AB%C2%AC%C2%AD%C2%AE%C2%AF%C2%B0"
		+ "%C2%B1%C2%B2%C2%B3%C2%B4%C2%B5%C2%B6%C2%B7%C2%B8%C2%B9%C2%BA%C2%BB%C2%BC%C2%BD%C2%BE%C2%BF%C3%80%C3"
		+ "%81@host:8080/%20!%22#$%&'()*+,-./09:;%3C=%3E?@A/[]%5E_%60a%7B|%7D~%E2%82%AC%E2%80%9A%C6%92%E2%80"
		+ "%9E%E2%80%A6%E2%80%A0%E2%80%A1%CB%86%E2%80%B0%C5%A0%E2%80%B9%C5%92%C5%BD%E2%80%98%E2%80%99%E2%80%9C"
		+ "%E2%80%9D%E2%80%A2%E2%80%93%E2%80%94%CB%9C%E2%84%A2%C5%A1%E2%80%BA%C5%93%C5%BE%C5%B8%C2%A1%C2%A2%C2"
		+ "%A3%C2%A4%C2%A5%C2%A6%C2%A7%C2%A8%C2%A9%C2%AA%C2%AB%C2%AC%C2%AD%C2%AE%C2%AF%C2%B0%C2%B1%C2%B2%C2%B3"
		+ "%C2%B4%C2%B5%C2%B6%C2%B7%C2%B8%C2%B9%C2%BA%C2%BB%C2%BC%C2%BD%C2%BE%C2%BF%C3%80%C3%81?%20!%22%23$%&'"
		+ "()*+,-./09:;%3C=%3E?@A%5C[]^_`a{|}~%E2%82%AC%E2%80%9A%C6%92%E2%80%9E%E2%80%A6%E2%80%A0%E2%80%A1%CB%86"
		+ "%E2%80%B0%C5%A0%E2%80%B9%C5%92%C5%BD%E2%80%98%E2%80%99%E2%80%9C%E2%80%9D%E2%80%A2%E2%80%93%E2%80%94%CB"
		+ "%9C%E2%84%A2%C5%A1%E2%80%BA%C5%93%C5%BE%C5%B8%C2%A1%C2%A2%C2%A3%C2%A4%C2%A5%C2%A6%C2%A7%C2%A8%C2%A9%C2"
		+ "%AA%C2%AB%C2%AC%C2%AD%C2%AE%C2%AF%C2%B0%C2%B1%C2%B2%C2%B3%C2%B4%C2%B5%C2%B6%C2%B7%C2%B8%C2%B9%C2%BA%C2"
		+ "%BB%C2%BC%C2%BD%C2%BE%C2%BF%C3%80%C3%81#%20!%22#$%&'()*+,-./09:;%3C=%3E?@A%5C[]%5E_`a{|}~%E2%82%AC%E2"
		+ "%80%9A%C6%92%E2%80%9E%E2%80%A6%E2%80%A0%E2%80%A1%CB%86%E2%80%B0%C5%A0%E2%80%B9%C5%92%C5%BD%E2%80%98%E2"
		+ "%80%99%E2%80%9C%E2%80%9D%E2%80%A2%E2%80%93%E2%80%94%CB%9C%E2%84%A2%C5%A1%E2%80%BA%C5%93%C5%BE%C5%B8%C2"
		+ "%A1%C2%A2%C2%A3%C2%A4%C2%A5%C2%A6%C2%A7%C2%A8%C2%A9%C2%AA%C2%AB%C2%AC%C2%AD%C2%AE%C2%AF%C2%B0%C2%B1%C2"
		+ "%B2%C2%B3%C2%B4%C2%B5%C2%B6%C2%B7%C2%B8%C2%B9%C2%BA%C2%BB%C2%BC%C2%BD%C2%BE%C2%BF%C3%80%C3%81";
	obj.username = str;
	obj.password = str;
	obj.pathname = str;
	obj.hash = str;
	obj.search = str;
	arktest.assertEQ(obj.href, hrefEncode);
}


function testNewURLParams002() {
	let params = new url.URLParams('aa=%E4%B8%AD%E5%9B%BD%BD');
	arktest.assertEQ(String(params), 'aa=%E4%B8%AD%E5%9B%BD%EF%BF%BD');
	arktest.assertEQ(params.get('aa'), '中国�');
}

function testNewURLParams003() {
	let params = new url.URLParams('aa=bc%F0%9F%98%82%F0Ad');
	arktest.assertEQ(params.toString(), 'aa=bc%F0%9F%98%82%EF%BF%BDAd');
	arktest.assertEQ(params.get('aa'), 'bc😂�Ad');
}


function testSpecial0005() {

	let params = new url.URLParams('bard=2');
	params.append('fod', "!\"#$%&&&'()*+++,-./:;<=>?@[\\]^_`{|}~");
	params.append('fod', 'Another special value: %^&*()_+|');
	params.append('fod', 'More special characters: ~`!@#$%^&*()-_=+');
	let arr = Array.from<string>(params.values())
	let str = arr.join(',');

	arktest.assertEQ(str, "2,!\"#$%&&&'()*+++,-./:;<=>?@[\\]^_`" +
		"{|}~,Another special value: %^&*()_+|,More special characters: " +
		"~`!@#$%^&*()-_=+");
}

function testUrlparseURL0032() {
	let strUrl = "bytedance://large_image?url=https%3A%2F%2Fp3-sign.toutiaoimg.com%2Flarge%2Ftos-cn-i-6w9my0ksv" +
		"p%2Fc22%3F_iz%3D67594%26from%3Dal%26x-expires%3D19%26x-sie%3DFI%252BJ&index=0&url=https%3A%2F%2Fssse?iz=67" +
		"594&from=article.detail%26x-expires%3D17";
	let res = '["https://p3-sign.toutiaoimg.com/large/tos-cn-i-6w9my0ksvp/c22?_iz=675' +
		'94&from=al&x-expires=19&x-sie=FI%2BJ","https://ssse?iz=67594"]';
	let urlObj = url.URL.parseURL(strUrl)
	const result = JSON.stringify(urlObj.params.getAll("url"))
	arktest.assertEQ(result, res);
}

function testUrLParseURLEmoji0002() {
	const errFunc = (): void => { url.URL.parseURL('/\ud83d\ude0a/path2', ''); };
	const errCheck = (e: Error) => {
		if (e instanceof BusinessError) {
			return e.name == "BusinessError" && e.message == "Parameter error. The type of  must be string";
		}
		return false;
	};
	arktest.expectThrow(errFunc, errCheck);
}

function testUrLParseURLEmoji0004() {
	const errFunc = (): void => { url.URL.parseURL('/\ud83d\udc64/path2', ''); };
	const errCheck = (e: Error) => {
		if (e instanceof BusinessError) {
			return e.name == "BusinessError" && e.message == "Parameter error. The type of  must be string";
		}
		return false;
	};
	arktest.expectThrow(errFunc, errCheck);
}

function testUrLParseURLEmoji0023() {
	let url1 = url.URL.parseURL('/path2?emoji=\ud83e\udd3c\u200d\u2642\ufe0f', 'https://www.example.com/test/test1');
	let url2 = url.URL.parseURL('/\ud83e\udd3c\u200d\u2642\ufe0f/path2', 'https://www.example.com/test/test1');

	arktest.assertEQ(url1.toString(), 'https://www.example.com/path2?emoji=%F0%9F%A4%BC%E2%80%8D%E2%99%82%EF%B8%8F');
	arktest.assertEQ(url2.toString(), 'https://www.example.com/%F0%9F%A4%BC%E2%80%8D%E2%99%82%EF%B8%8F/path2');
}

function testUrlDeleteEscape0003() {
	let urlObject = url.URL.parseURL('https://developer.exampleUrl/?\n=1&bar=2');
	let sl = urlObject.search.slice(1);
	let paramsObject = new url.URLParams(sl);

	paramsObject.delete('');
	arktest.assertEQ(paramsObject.toString(), "bar=2");
}

function testUrlDeleteEscape0004() {
	let urlObject = url.URL.parseURL('https://developer.exampleUrl/?\r=1&bar=2');
	let paramsObject = new url.URLParams(urlObject.search.slice(1));
	paramsObject.delete('');
	arktest.assertEQ(paramsObject.toString(), "bar=2");
}

function testUrlDeleteEscape0005() {
	let urlObject = url.URL.parseURL('https://developer.exampleUrl/?\t=1&bar=2');
	let paramsObject = new url.URLParams(urlObject.search.slice(1));
	paramsObject.delete('');
	arktest.assertEQ(paramsObject.toString(), "bar=2");
}

function testUrlGetAll9Escape0003() {
	let urlObject = url.URL.parseURL('https://developer.exampleUrl/?\n=1&bard=2');
	let params = new url.URLParams(urlObject.search.slice(1));
	params.append('', '3');
	arktest.assertEQ(params.getAll('').toString(), "1,3");
}

function testUrlGetAll9Escape0004() {
	let urlObject = url.URL.parseURL('https://developer.exampleUrl/?\r=1&bard=2');
	let params = new url.URLParams(urlObject.search.slice(1));
	params.append('', '3');
	arktest.assertEQ(params.getAll('').toString(), "1,3");
}

function testUrlGetAll9Escape0005() {
	let urlObject = url.URL.parseURL('https://developer.exampleUrl/?\t=1&bard=2');
	let params = new url.URLParams(urlObject.search.slice(1));
	params.append('', '3');
	arktest.assertEQ(params.getAll('').toString(), "1,3");
}

function testUrlParams9ForEach9Escape0003() {
	const myURLObject = url.URL.parseURL('https://developer.exampleUrl/?text=abc\n&anotherParam=xyz');
	let callbackInvoked = false;
	myURLObject.params.forEach((value, name, searchParams) => {
		if (name === "text") {
			arktest.assertEQ(encodeURI(value), encodeURI("abc"));
		}
		callbackInvoked = true;
	});
	arktest.assertEQ(callbackInvoked, true);
}

function testUrlParams9ForEach9Escape0004() {
	const myURLObject = url.URL.parseURL('https://developer.exampleUrl/?text=abc\r&anotherParam=xyz');
	let callbackInvoked = false;
	myURLObject.params.forEach((value, name, searchParams) => {
		if (name === "text") {
			arktest.assertEQ(encodeURI(value), encodeURI("abc"));
		}
		callbackInvoked = true;
	});
	arktest.assertEQ(callbackInvoked, true);
}

function testUrlParams9ForEach9Escape0005() {
	const myURLObject = url.URL.parseURL('https://developer.exampleUrl/?text=abc\t&anotherParam=xyz');
	let callbackInvoked = false;
	myURLObject.params.forEach((value, name, searchParams) => {
		if (name === "text") {
			arktest.assertEQ(value, "abc");
		}
		callbackInvoked = true;
	});
	arktest.assertEQ(callbackInvoked, true);
}

function testUrlParseUrl9Escape0001() {
	let url2 = url.URL.parseURL('/\b/path2', 'https://www.example.com/test/test1');
	arktest.assertEQ(url2.toString(), 'https://www.example.com/%08/path2');

	let url1 = url.URL.parseURL('\b/path2', 'https://www.example.com/test/test1');
	arktest.assertEQ(url1.toString(), 'https://www.example.com/test//path2');
}

function testUrlParseUrl9Escape0003() {
	let url2 = url.URL.parseURL('/\f/path2', 'https://www.example.com/test/test1');
	arktest.assertEQ(url2.toString(), 'https://www.example.com/%0C/path2');

	let url1 = url.URL.parseURL('\f/path2', 'https://www.example.com/test/test1');
	arktest.assertEQ(url1.toString(), 'https://www.example.com/test//path2');
}

function testUrlParseUrl9Escape0005() {
	let url2 = url.URL.parseURL('/\n/path2', 'https://www.example.com/test/test1');
	arktest.assertEQ(url2.toString(), 'https://www.example.com/%0A/path2');

	let url1 = url.URL.parseURL('\n/path2', 'https://www.example.com/test/test1');
	arktest.assertEQ(url1.toString(), 'https://www.example.com/test//path2');
}

function testUrlToStringEscape0003() {
	const urlObject = url.URL.parseURL('https://username:password@host:8080/directory/\n');
	let result = urlObject.toString();
	arktest.assertEQ(result, 'https://username:password@host:8080/directory/');
}

function main(): int {
	let suite = new arktest.ArkTestsuite("URL UT tests");
	suite.addTest("testGetAllValuesForKey0005", testGetAllValuesForKey0005);
	suite.addTest("testGetValueForKey0005", testGetValueForKey0005);
	suite.addTest("testHasKey0005", testHasKey0005);
	suite.addTest("testUrlIPv4001", testUrlIPv4001);
	suite.addTest("testUrlIPv4002", testUrlIPv4002);
	suite.addTest("testUrlIPv4003", testUrlIPv4003);
	suite.addTest("testUrlIPv6001", testUrlIPv6001);
	suite.addTest("testUrlIPv6002", testUrlIPv6002);
	suite.addTest("testUrlIPv6003", testUrlIPv6003);
	suite.addTest("testSpecial0001", testSpecial0001);
	suite.addTest("testSpecial0005", testSpecial0005);
	suite.addTest("testSpecial0014", testSpecial0014);
	suite.addTest("testUrlparseURL0031", testUrlparseURL0031);
	suite.addTest("testUrlparseURL0032", testUrlparseURL0032);
	suite.addTest("testUrlparseURL0045", testUrlparseURL0045);
	suite.addTest("testUrlparseURL0046", testUrlparseURL0046);
	suite.addTest("testUrlparseURL0048", testUrlparseURL0048);
	suite.addTest("testUrlparseURL0053", testUrlparseURL0053);
	suite.addTest("testUrlPathname006", testUrlPathname006);
	suite.addTest("testUrlHref006", testUrlHref006);
	suite.addTest("testNewURLParams002", testNewURLParams002);
	suite.addTest("testNewURLParams003", testNewURLParams003);
	suite.addTest("testUrLParseURLEmoji0002", testUrLParseURLEmoji0002);
	suite.addTest("testUrLParseURLEmoji0004", testUrLParseURLEmoji0004);
	suite.addTest("testUrLParseURLEmoji0023", testUrLParseURLEmoji0023);
	suite.addTest("testUrlDeleteEscape0003", testUrlDeleteEscape0003);
	suite.addTest("testUrlDeleteEscape0004", testUrlDeleteEscape0004);
	suite.addTest("testUrlDeleteEscape0005", testUrlDeleteEscape0005);
	suite.addTest("testUrlGetAll9Escape0003", testUrlGetAll9Escape0003);
	suite.addTest("testUrlGetAll9Escape0004", testUrlGetAll9Escape0004);
	suite.addTest("testUrlGetAll9Escape0005", testUrlGetAll9Escape0005);
	suite.addTest("testUrlParams9ForEach9Escape0003", testUrlParams9ForEach9Escape0003);
	suite.addTest("testUrlParams9ForEach9Escape0004", testUrlParams9ForEach9Escape0004);
	suite.addTest("testUrlParams9ForEach9Escape0005", testUrlParams9ForEach9Escape0005);
	suite.addTest("testUrlParseUrl9Escape0001", testUrlParseUrl9Escape0001);
	suite.addTest("testUrlParseUrl9Escape0003", testUrlParseUrl9Escape0003);
	suite.addTest("testUrlParseUrl9Escape0005", testUrlParseUrl9Escape0005);
	suite.addTest("testUrlToStringEscape0003", testUrlToStringEscape0003);

	return suite.run();
}
