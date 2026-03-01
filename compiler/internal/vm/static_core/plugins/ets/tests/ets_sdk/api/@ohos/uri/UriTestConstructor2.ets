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
import { uri } from "@ohos.uri";
import { BusinessError } from "@ohos.base";

function testUriConstruction001() {
  let uriText = '#http://username:password@host:8080/directory/file?foo=1&bar=2';
  let errText = 'Syntax Error. Invalid Uri string: The #It can\'t be the first.';
  testConstructorThrow(uriText, errText);
}

function testUriConstruction003() {
  let that = new uri.URI('ht/tp://username:pas sword@host:8080/directory/file?foo=1&bar=2');
  arktest.assertEQ(that.toString(), "ht/tp://username:pas%20sword@host:8080/directory/file?foo=1&bar=2");
}

function testUriConstruction004() {
  let uriText = 'http://username:password@[::]:8080/directory/file?Query#gaogao faofao';
  arktest.expectNoThrow(() => { new uri.URI(uriText) });
}

function testUriConstruction005() {
  let uriText = 'http://username:password@host:8080/directory/file?foo^=1&bar=2#gaogaofaofao';
  let errText = 'Syntax Error. Invalid Uri string: The Query does not conform to the rule.'
  testConstructorThrow(uriText, errText);
}

function testUriConstruction006() {
  let uriText = '1http://username:password@host:8080/directory/file?foo=1&bar=2#gaogaofaofao';
  let errText = "Syntax Error. Invalid Uri string: The Scheme the first character must be a letter.";
  testConstructorThrow(uriText, errText);
}

function testUriConstruction007() {
  let uriText = 'ht@tp://username:password@host:8080/directory/file?foo=1&bar=2#gaogaofaofao';
  let errText = "Syntax Error. Invalid Uri string: The Scheme does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function testUriConstruction008() {
  let uriText = 'http://username:password@[::]:80r80/directory/file?foo=1&bar=2#gaogaofaofao';
  let errText = "Syntax Error. Invalid Uri string: The Port does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function testUriConstruction009() {
  let uriText = 'http://username:password@[::12:55:8080/directory/file?foo=1&bar=2#gaogaofaofao';
  let errText = "Syntax Error. Invalid Uri string: The IPv6 is missing a closing bracket.";
  testConstructorThrow(uriText, errText);
}

function testUriConstruction010() {
  let uriText = 'http://username:pa^ssword@[::12:55]:8080/directory/file?foo=1&bar=2#gaogaofaofao';
  let errText = "Syntax Error. Invalid Uri string: The UserInfo does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function testUriConstruction011() {
  let uriText = 'http://username:password@[::1你2:55]:8080/directory/file?foo=1&bar=2#gaogaofaofao';
  let errText = "Syntax Error. Invalid Uri string: The Ipv6 does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function testUriConstruction012() {
  let gaogao = new uri.URI('http://gg:gaogao@www.test.com:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@www.test.com:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@www.test.com:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "www.test.com");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction013() {
  let gaogao = new uri.URI('http://gg:gaogao@[1080::8:800:200C:417A]:99/path/66path1?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[1080::8:800:200C:417A]:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[1080::8:800:200C:417A]:99/path/66path1?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[1080::8:800:200C:417A]");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/66path1");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction014() {
  let gaogao = new uri.URI('http://gg:gaogao@[::]:88/path/path66?foooo#gaogao');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[::]:88");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[::]:88/path/path66?foooo");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[::]");
  arktest.assertEQ(gaogao.port, "88");
  arktest.assertEQ(gaogao.path, "/path/path66");
  arktest.assertEQ(gaogao.query, "foooo");
  arktest.assertEQ(gaogao.fragment, "gaogao");
}

function testUriConstruction015() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/66path1?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[1:0:0:1:2:1:2:1]:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/66path1?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[1:0:0:1:2:1:2:1]");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/66path1");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction016() {
  let gaogao = new uri.URI('http://gg:gaogao@[::FFFF:129.144.52.38]:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[::FFFF:129.144.52.38]:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[::FFFF:129.144.52.38]:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[::FFFF:129.144.52.38]");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction017() {
  let gaogao = new uri.URI('http://gg:gaogao@[::192.9.5.5]:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[::192.9.5.5]:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[::192.9.5.5]:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[::192.9.5.5]");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction018() {
  let gaogao = new uri.URI('http://gg:gaogao@[22::22:2:2%ss]:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[22::22:2:2%ss]:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[22::22:2:2%ss]:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[22::22:2:2%ss]");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction019() {
  let gaogao = new uri.URI('http://gg:gaogao@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[fe80:0000:0001:0000:0440:44ff:1233:5678]");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction020() {
  let gaogao = new uri.URI('http://gg:gaogao@[fe80::0001:0000]:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@[fe80::0001:0000]:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@[fe80::0001:0000]:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "[fe80::0001:0000]");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction021() {
  let gaogao = new uri.URI('http://gg:gaogao@199.98.55.44:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "gg:gaogao@199.98.55.44:99");
  arktest.assertEQ(gaogao.ssp, "//gg:gaogao@199.98.55.44:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, "gg:gaogao");
  arktest.assertEQ(gaogao.host, "199.98.55.44");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction022() {
  let gaogao = new uri.URI('http://16.9.5.4:99/path/path?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "16.9.5.4:99");
  arktest.assertEQ(gaogao.ssp, "//16.9.5.4:99/path/path?query");
  arktest.assertEQ(gaogao.userInfo, null);
  arktest.assertEQ(gaogao.host, "16.9.5.4");
  arktest.assertEQ(gaogao.port, "99");
  arktest.assertEQ(gaogao.path, "/path/path");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction023() {
  let gaogao = new uri.URI('http://49.99.54.12:50/path/path23?query#fagment');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "49.99.54.12:50");
  arktest.assertEQ(gaogao.ssp, "//49.99.54.12:50/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, null);
  arktest.assertEQ(gaogao.host, "49.99.54.12");
  arktest.assertEQ(gaogao.port, "50");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "fagment");
}

function testUriConstruction024() {
  let gaogao = new uri.URI('http://user@49.10pe8.54.12:80/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "user@49.10pe8.54.12:80");
  arktest.assertEQ(gaogao.ssp, "//user@49.10pe8.54.12:80/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, null);
  arktest.assertEQ(gaogao.host, null);
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function testUriConstruction025() {
  let gaogao = new uri.URI('http://user@www.test.com/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "user@www.test.com");
  arktest.assertEQ(gaogao.ssp, "//user@www.test.com/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, "user");
  arktest.assertEQ(gaogao.host, "www.test.com");
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function testUriConstruction026() {
  let gaogao = new uri.URI('http://user@www.hw.com:77/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "user@www.hw.com:77");
  arktest.assertEQ(gaogao.ssp, "//user@www.hw.com:77/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, "user");
  arktest.assertEQ(gaogao.host, "www.hw.com");
  arktest.assertEQ(gaogao.port, "77");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function testUriConstruction027() {
  let gaogao = new uri.URI('ht2tp://user@www.h12343w.com:77/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "ht2tp");
  arktest.assertEQ(gaogao.authority, "user@www.h12343w.com:77");
  arktest.assertEQ(gaogao.ssp, "//user@www.h12343w.com:77/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, "user");
  arktest.assertEQ(gaogao.host, "www.h12343w.com");
  arktest.assertEQ(gaogao.port, "77");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function testUriConstruction028() {
  let gaogao = new uri.URI('ht2tp://user@www.1hw.1com:77/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "ht2tp");
  arktest.assertEQ(gaogao.authority, "user@www.1hw.1com:77");
  arktest.assertEQ(gaogao.ssp, "//user@www.1hw.1com:77/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, null);
  arktest.assertEQ(gaogao.host, null);
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function testUriConstruction029() {
  let gaogao = new uri.URI('http://user@hosthost/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "user@hosthost");
  arktest.assertEQ(gaogao.ssp, "//user@hosthost/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, "user");
  arktest.assertEQ(gaogao.host, "hosthost");
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function testUriConstruction030() {
  let gaogao = new uri.URI('http://user@[::]/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "user@[::]");
  arktest.assertEQ(gaogao.ssp, "//user@[::]/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, "user");
  arktest.assertEQ(gaogao.host, "[::]");
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function testConstructorThrow(uriText: string, errText: string) {
  arktest.expectError((): void => { let that = new uri.URI(uriText); }, new BusinessError(0, new Error(errText)))
}

function testUriConstruction031() {
  let gaogao = new uri.URI('http://[::192:0:5]/path/path23?query#qwer');
  arktest.assertEQ(gaogao.scheme, "http");
  arktest.assertEQ(gaogao.authority, "[::192:0:5]");
  arktest.assertEQ(gaogao.ssp, "//[::192:0:5]/path/path23?query");
  arktest.assertEQ(gaogao.userInfo, null);
  arktest.assertEQ(gaogao.host, "[::192:0:5]");
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.path, "/path/path23");
  arktest.assertEQ(gaogao.query, "query");
  arktest.assertEQ(gaogao.fragment, "qwer");
}

function main(): int {
  let suite = new arktest.ArkTestsuite("URI XTS tests");
  suite.addTest("testUriConstruction001", testUriConstruction001);
  suite.addTest("testUriConstruction003", testUriConstruction003);
  suite.addTest("testUriConstruction004", testUriConstruction004);
  suite.addTest("testUriConstruction005", testUriConstruction005);
  suite.addTest("testUriConstruction006", testUriConstruction006);
  suite.addTest("testUriConstruction007", testUriConstruction007);
  suite.addTest("testUriConstruction008", testUriConstruction008);
  suite.addTest("testUriConstruction009", testUriConstruction009);
  suite.addTest("testUriConstruction010", testUriConstruction010);
  suite.addTest("testUriConstruction011", testUriConstruction011);
  suite.addTest("testUriConstruction012", testUriConstruction012);
  suite.addTest("testUriConstruction013", testUriConstruction013);
  suite.addTest("testUriConstruction014", testUriConstruction014);
  suite.addTest("testUriConstruction015", testUriConstruction015);
  suite.addTest("testUriConstruction016", testUriConstruction016);
  suite.addTest("testUriConstruction017", testUriConstruction017);
  suite.addTest("testUriConstruction018", testUriConstruction018);
  suite.addTest("testUriConstruction019", testUriConstruction019);
  suite.addTest("testUriConstruction020", testUriConstruction020);
  suite.addTest("testUriConstruction021", testUriConstruction021);
  suite.addTest("testUriConstruction022", testUriConstruction022);
  suite.addTest("testUriConstruction023", testUriConstruction023);
  suite.addTest("testUriConstruction024", testUriConstruction024);
  suite.addTest("testUriConstruction025", testUriConstruction025);
  suite.addTest("testUriConstruction026", testUriConstruction026);
  suite.addTest("testUriConstruction027", testUriConstruction027);
  suite.addTest("testUriConstruction028", testUriConstruction028);
  suite.addTest("testUriConstruction029", testUriConstruction029);
  suite.addTest("testUriConstruction030", testUriConstruction030);
  suite.addTest("testUriConstruction031", testUriConstruction031);

  //passed
  return suite.run();
}