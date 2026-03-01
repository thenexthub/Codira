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

function ConstructorTest001() {
  let muri = new uri.URI("http://username:password@www.test.com:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@www.test.com:99");
  arktest.assertEQ(muri.ssp, "//username:password@www.test.com:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "www.test.com");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest002() {
  let muri = new uri.URI("http://username:password@[1080::8:800:200C:417A]:99/path/66path1?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[1080::8:800:200C:417A]:99");
  arktest.assertEQ(muri.ssp, "//username:password@[1080::8:800:200C:417A]:99/path/66path1?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[1080::8:800:200C:417A]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/66path1");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest003() {
  let muri = new uri.URI("http://username:password@[::]:88/path/path66?foooo#gaogao");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[::]:88");
  arktest.assertEQ(muri.ssp, "//username:password@[::]:88/path/path66?foooo");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[::]");
  arktest.assertEQ(muri.port, '88');
  arktest.assertEQ(muri.path, "/path/path66");
  arktest.assertEQ(muri.query, "foooo");
  arktest.assertEQ(muri.fragment, "gaogao");
}

function ConstructorTest004() {
  let muri = new uri.URI("http://username:password@[1:0:0:1:2:1:2:1]:99/path/66path1?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[1:0:0:1:2:1:2:1]:99");
  arktest.assertEQ(muri.ssp, "//username:password@[1:0:0:1:2:1:2:1]:99/path/66path1?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[1:0:0:1:2:1:2:1]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/66path1");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest005() {
  let muri = new uri.URI("http://username:password@[::FFFF:129.144.52.38]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[::FFFF:129.144.52.38]:99");
  arktest.assertEQ(muri.ssp, "//username:password@[::FFFF:129.144.52.38]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[::FFFF:129.144.52.38]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest006() {
  let muri = new uri.URI("http://username:password@[::192.9.5.5]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[::192.9.5.5]:99");
  arktest.assertEQ(muri.ssp, "//username:password@[::192.9.5.5]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[::192.9.5.5]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest007() {
  let muri = new uri.URI("http://username:password@[22::22:2:2%ss]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[22::22:2:2%ss]:99");
  arktest.assertEQ(muri.ssp, "//username:password@[22::22:2:2%ss]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[22::22:2:2%ss]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest008() {
  let muri = new uri.URI("http://username:password@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99");
  arktest.assertEQ(muri.ssp, "//username:password@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[fe80:0000:0001:0000:0440:44ff:1233:5678]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest009() {
  let muri = new uri.URI("http://username:password@[fe80::0001:0000]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@[fe80::0001:0000]:99");
  arktest.assertEQ(muri.ssp, "//username:password@[fe80::0001:0000]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "[fe80::0001:0000]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest010() {
  let muri = new uri.URI("http://username:password@www.test.com:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@www.test.com:99");
  arktest.assertEQ(muri.ssp, "//username:password@www.test.com:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "www.test.com");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest011() {
  let muri = new uri.URI("http://username:password@199.98.55.44:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "username:password@199.98.55.44:99");
  arktest.assertEQ(muri.ssp, "//username:password@199.98.55.44:99/path/path?query");
  arktest.assertEQ(muri.userInfo, "username:password");
  arktest.assertEQ(muri.host, "199.98.55.44");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest012() {
  let muri = new uri.URI("http://16.9.5.4:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "16.9.5.4:99");
  arktest.assertEQ(muri.ssp, "//16.9.5.4:99/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, "16.9.5.4");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest013() {
  let muri = new uri.URI("http://[::168:169:333]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "[::168:169:333]:99");
  arktest.assertEQ(muri.ssp, "//[::168:169:333]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, "[::168:169:333]");
  arktest.assertEQ(muri.port, '99');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest014() {
  let muri = new uri.URI("http://user@49.10hh8.54.12:80/path/path?query#qwer");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "user@49.10hh8.54.12:80");
  arktest.assertEQ(muri.ssp, "//user@49.10hh8.54.12:80/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, null);
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "qwer");
}

function ConstructorTest015() {
  let muri = new uri.URI("http://user@www.test.com/path/path?query#qwer");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "user@www.test.com");
  arktest.assertEQ(muri.ssp, "//user@www.test.com/path/path?query");
  arktest.assertEQ(muri.userInfo, "user");
  arktest.assertEQ(muri.host, "www.test.com");
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "qwer");
}

function ConstructorTest016() {
  let muri = new uri.URI("ftp://user@www.1hw.1com:77/path/path?query#qwer");
  arktest.assertEQ(muri.scheme, "ftp");
  arktest.assertEQ(muri.authority, "user@www.1hw.1com:77");
  arktest.assertEQ(muri.ssp, "//user@www.1hw.1com:77/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, null);
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "qwer");
}

function ConstructorTest017() {
  let muri = new uri.URI("http://user@hosthost/path/path?query#qwer");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "user@hosthost");
  arktest.assertEQ(muri.ssp, "//user@hosthost/path/path?query");
  arktest.assertEQ(muri.userInfo, "user");
  arktest.assertEQ(muri.host, "hosthost");
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "qwer");
}

function ConstructorTest018() {
  let muri = new uri.URI("http://user@[::]/path/path?query#qwer");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "user@[::]");
  arktest.assertEQ(muri.ssp, "//user@[::]/path/path?query");
  arktest.assertEQ(muri.userInfo, "user");
  arktest.assertEQ(muri.host, "[::]");
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "qwer");
}

function ConstructorTest019() {
  let muri = new uri.URI("http://[::192:0:5]/path/path?query#qwer");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "[::192:0:5]");
  arktest.assertEQ(muri.ssp, "//[::192:0:5]/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, "[::192:0:5]");
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "qwer");
}

function ConstructorTest020() {
  let muri = new uri.URI("http://[::192:0:5]/path/path?query#qwer");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, "[::192:0:5]");
  arktest.assertEQ(muri.ssp, "//[::192:0:5]/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, "[::192:0:5]");
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "qwer");
}

function ConstructorTest021() {
  let muri = new uri.URI("http:/&username:password@[1080::8:800:200C:417A]:99/path/66path1?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, null);
  arktest.assertEQ(muri.ssp, "/&username:password@[1080::8:800:200C:417A]:99/path/66path1?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, null);
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/&username:password@[1080::8:800:200C:417A]:99/path/66path1");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest022() {
  let muri = new uri.URI("http:/[username:password@[::FFFF:129.144.52.38]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, null);
  arktest.assertEQ(muri.ssp, "/[username:password@[::FFFF:129.144.52.38]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, null);
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, "/[username:password@[::FFFF:129.144.52.38]:99/path/path");
  arktest.assertEQ(muri.query, "query");
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest023() {
  let muri = new uri.URI("http:username:password@[1080::8:800:200C:417A]:99/path/66path1?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, null);
  arktest.assertEQ(muri.ssp, "username:password@[1080::8:800:200C:417A]:99/path/66path1?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, null);
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, null);
  arktest.assertEQ(muri.query, null);
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest024() {
  let muri = new uri.URI("http:^$username:password@[::192.9.5.5]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, null);
  arktest.assertEQ(muri.ssp, "^$username:password@[::192.9.5.5]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, null);
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, null);
  arktest.assertEQ(muri.query, null);
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest025() {
  let muri = new uri.URI("http:[?]username:password@[fe80::0001:0000]:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "http");
  arktest.assertEQ(muri.authority, null);
  arktest.assertEQ(muri.ssp, "[?]username:password@[fe80::0001:0000]:99/path/path?query");
  arktest.assertEQ(muri.userInfo, null);
  arktest.assertEQ(muri.host, null);
  arktest.assertEQ(muri.port, '-1');
  arktest.assertEQ(muri.path, null);
  arktest.assertEQ(muri.query, null);
  arktest.assertEQ(muri.fragment, "fagment");
}

function ConstructorTest026() {
  let uriText = "";
  let errText = "Syntax Error. Invalid Uri string: The Uri is empty.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest027() {
  let uriText = "#asd;";
  let errText = "Syntax Error. Invalid Uri string: The #It can't be the first.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest028() {
  let uriText = "?sa^d:s#asd;";
  let errText = "Syntax Error. Invalid Uri string: The Query does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest029() {
  let uriText = "?sad:s#a^sd;";
  let errText = "Syntax Error. Invalid Uri string: The Fragment does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest030() {
  let uriText = "4http:/username:password@www.test.com:99/path/path?query#fagment";
  let errText = "Syntax Error. Invalid Uri string: The Scheme the first character must be a letter.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest031() {
  let muri = new uri.URI("ht*tp:/username:password@www.test.com:99/path/path?query#fagment");
  arktest.assertEQ(muri.scheme, "ht*tp");
}

function ConstructorTest032() {
  let uriText = "/usern]ame/path/path?query#fagment";
  let errText = "Syntax Error. Invalid Uri string: The SpecialPath does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}


function testConstructorThrow(uriText: string, errText: string) {
  arktest.expectError((): void => { let that = new uri.URI(uriText); }, new BusinessError(0, new Error(errText)))
}

function main(): int {
  let suite = new arktest.ArkTestsuite("URI UT tests");
  suite.addTest("ConstructorTest001", ConstructorTest001);
  suite.addTest("ConstructorTest002", ConstructorTest002);
  suite.addTest("ConstructorTest003", ConstructorTest003);
  suite.addTest("ConstructorTest004", ConstructorTest004);
  suite.addTest("ConstructorTest005", ConstructorTest005);
  suite.addTest("ConstructorTest006", ConstructorTest006);
  suite.addTest("ConstructorTest007", ConstructorTest007);
  suite.addTest("ConstructorTest008", ConstructorTest008);
  suite.addTest("ConstructorTest009", ConstructorTest009);
  suite.addTest("ConstructorTest010", ConstructorTest010);
  suite.addTest("ConstructorTest011", ConstructorTest011);
  suite.addTest("ConstructorTest012", ConstructorTest012);
  suite.addTest("ConstructorTest013", ConstructorTest013);
  suite.addTest("ConstructorTest014", ConstructorTest014);
  suite.addTest("ConstructorTest015", ConstructorTest015);
  suite.addTest("ConstructorTest016", ConstructorTest016);
  suite.addTest("ConstructorTest017", ConstructorTest017);
  suite.addTest("ConstructorTest018", ConstructorTest018);
  suite.addTest("ConstructorTest019", ConstructorTest019);
  suite.addTest("ConstructorTest020", ConstructorTest020);
  suite.addTest("ConstructorTest021", ConstructorTest021);
  suite.addTest("ConstructorTest022", ConstructorTest022);
  suite.addTest("ConstructorTest023", ConstructorTest023);
  suite.addTest("ConstructorTest024", ConstructorTest024);
  suite.addTest("ConstructorTest025", ConstructorTest025);
  suite.addTest("ConstructorTest026", ConstructorTest026);
  suite.addTest("ConstructorTest027", ConstructorTest027);
  suite.addTest("ConstructorTest028", ConstructorTest028);
  suite.addTest("ConstructorTest029", ConstructorTest029);
  suite.addTest("ConstructorTest030", ConstructorTest030);
  suite.addTest("ConstructorTest031", ConstructorTest031);
  suite.addTest("ConstructorTest032", ConstructorTest032);
  //passed
  return suite.run();
}