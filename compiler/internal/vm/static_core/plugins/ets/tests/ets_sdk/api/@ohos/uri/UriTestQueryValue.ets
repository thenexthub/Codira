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
import {uri} from "@ohos.uri"


function testUriGetQueryValue001() {
  let route = new uri.URI('https://www.test.com?param1=value1&param2=value2');  // CC-OFF(G.OTH.05) test logic
  let param1Value = route.getQueryValue("param1");
  let param2Value = route.getQueryValue("param2");
  arktest.assertEQ(param1Value, "value1");
  arktest.assertEQ(param2Value, "value2");
}

function testUriGetQueryValue002() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(route.getQueryValue("param1"), null);
  let include = new uri.URI('https://www.test.com?param1=value1');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(include.getQueryValue("param"), null);
}

function testUriGetQueryValue003() {
  let route = new uri.URI('https://www.test.com?param1=value1&param1=value2&param1=value3');  // CC-OFF(G.OTH.05) test logic
  let param1Value = route.getQueryValue("param1");
  arktest.assertEQ(param1Value, "value1");
  arktest.assertEQ(route.getQueryValues("param1")[0], "value1");
}

function testUriGetQueryValue004() {
  let route = new uri.URI('https://www.test.com?param1=hello%20world');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(route.getQueryValue("param1"), "hello world");
  let specialSymbol = new uri.URI('https://www.test.com?param1=hello+world');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(specialSymbol.getQueryValue("param1"), "hello world");
}

function testUriGetQueryValue005() {
  let route = new uri.URI('https://www.test.com?param%201=hello');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(route.getQueryValue("param 1"), "hello");
  arktest.assertEQ(route.getQueryValue("1"), null);
  arktest.assertEQ(route.getQueryValue(" 1"), null);
}

function testUriGetQueryValue006() {
  let route = new uri.URI('https://www.test.com?param1=&param2=');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(route.getQueryValue("param1"), "");
  arktest.assertEQ(route.getQueryValue("param2"), "");
  let noNameUri = new uri.URI('https://www.test.com?=value');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(noNameUri.getQueryValue(""), "value");
}

function testUriGetQueryValue007() {
  let emptyParameter1 = new uri.URI('https://www.test.com?param1=value1&&param3=value3');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(emptyParameter1.getQueryValue(""), "");
  let emptyParameter12 = new uri.URI('https://www.test.com?param1=value1&');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(emptyParameter12.getQueryValue(""), "");
  let router = new uri.URI('https://www.test.com?');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(router.getQueryValue(""), null);
}

function testUriAddQueryValue001() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let newRoute = route.addQueryValue("param1", "value1");
  arktest.assertEQ(newRoute.getQueryValue("param1"), "value1");
  let clearRoute = newRoute.clearQuery();
  arktest.assertEQ(clearRoute.toString(), "https://www.test.com");  // CC-OFF(G.OTH.05) test logic
}

function testUriAddQueryValue002() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let newRoute = route.addQueryValue("param1", "value1").addQueryValue("param2", "value2");
  arktest.assertEQ(newRoute.getQueryValue('param1'), "value1");
  arktest.assertEQ(newRoute.getQueryValue('param2'), "value2");
  arktest.assertEQ(newRoute.toString(), "https://www.test.com?param1=value1&param2=value2");  // CC-OFF(G.OTH.05) test logic
}

function testUriAddQueryValue003() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let newRoute = route.addQueryValue("param1", "hello world");
  arktest.assertEQ(newRoute.toString(), "https://www.test.com?param1=hello%20world");  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(newRoute.getQueryValue("param1"), "hello world");
  let paramNames = newRoute.getQueryNames();
  arktest.assertEQ(paramNames.length, 1);
  arktest.assertEQ(paramNames.values().next().value, "param1");
}

function testUriAddQueryValue004() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let newRoute = route.addQueryValue("param1", "value1").addQueryValue("param1", "value2");
  arktest.assertEQ(newRoute.toString(), "https://www.test.com?param1=value1&param1=value2");  // CC-OFF(G.OTH.05) test logic
}

function testUriAddQueryValue005() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let newRoute = route.addQueryValue("param1", "value1").addQueryValue("param2", "null")
  arktest.assertEQ(newRoute.toString(), "https://www.test.com?param1=value1&param2=null");  // CC-OFF(G.OTH.05) test logic
  let values = newRoute.getQueryValues("param1");
  arktest.assertEQ(values[0], "value1");
}

function testUriAddQueryValue006() {
  let value = "value";
  let testUri = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let addQuery = testUri.addQueryValue("param", value);
  arktest.assertEQ(addQuery.getQueryValue("param"), value);
  arktest.assertEQ(addQuery.getQueryValues("param")[0], value);
  arktest.assertEQ(addQuery.toString(), "https://www.test.com?param=value");  // CC-OFF(G.OTH.05) test logic
  value = "value1";
  let valueChange = testUri.addQueryValue("param", value);
  arktest.assertEQ(valueChange.toString(), "https://www.test.com?param=value1");  // CC-OFF(G.OTH.05) test logic
  let newChange = addQuery.addQueryValue("param", value);
  arktest.assertEQ(newChange.toString(), "https://www.test.com?param=value&param=value1");  // CC-OFF(G.OTH.05) test logic
}

function testUriAddQueryValue007() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let newRoute = route.addQueryValue("param 1", "value 1");
  arktest.assertEQ(newRoute.getQueryValue("param 1"), "value 1");
  let mui = new uri.URI('http://www.test.com?sa%3D=\ud83d\ude0a')  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(mui.getQueryValue('sa='), "\ud83d\ude0a");
}

function testUriAddQueryValue008() {
  let route = new uri.URI('https://www.com?param1=\ud83d\ude0a&param2=value2');  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(route.getQueryValue("abc"), null);
  let newRoute = new uri.URI('http://www.test.com?sa%3D=\ud83d\ude0a')  // CC-OFF(G.OTH.05) test logic
  arktest.assertEQ(newRoute.getQueryValue('aa'), null);
}

function main(): int {
  let suite = new arktest.ArkTestsuite("URI XTS tests");
  suite.addTest("testUriGetQueryValue001", testUriGetQueryValue001);
  suite.addTest("testUriGetQueryValue002", testUriGetQueryValue002);
  suite.addTest("testUriGetQueryValue003", testUriGetQueryValue003);
  suite.addTest("testUriGetQueryValue004", testUriGetQueryValue004);
  suite.addTest("testUriGetQueryValue005", testUriGetQueryValue005);
  suite.addTest("testUriGetQueryValue006", testUriGetQueryValue006);
  suite.addTest("testUriGetQueryValue007", testUriGetQueryValue007);

  suite.addTest("testUriAddQueryValue001", testUriAddQueryValue001);
  suite.addTest("testUriAddQueryValue002", testUriAddQueryValue002);
  suite.addTest("testUriAddQueryValue003", testUriAddQueryValue003);
  suite.addTest("testUriAddQueryValue004", testUriAddQueryValue004);
  suite.addTest("testUriAddQueryValue005", testUriAddQueryValue005);
  suite.addTest("testUriAddQueryValue006", testUriAddQueryValue006);
  suite.addTest("testUriAddQueryValue007", testUriAddQueryValue007);
  suite.addTest("testUriAddQueryValue008", testUriAddQueryValue008);
  //passed
  return suite.run();
}
