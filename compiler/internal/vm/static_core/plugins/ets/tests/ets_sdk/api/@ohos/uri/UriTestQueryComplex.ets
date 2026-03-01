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
import {uri} from "@ohos.uri";

function testUriGetQueryNames001() {
  let route = new uri.URI('https://www.test.com?param1=value1&param2=value2');  // CC-OFF(G.OTH.05) test logic
  let paramNames = route.getQueryNames();
  arktest.assertEQ(paramNames.length, 2);
  arktest.assertEQ(paramNames.values().next().value, "param1");
}

function testUriGetQueryNames002() {
  let route = new uri.URI('https://www.test.com');  // CC-OFF(G.OTH.05) test logic
  let paramNames = route.getQueryNames();
  arktest.assertEQ(paramNames.length, 0);
}

function testUriGetQueryNames003() {
  let route = new uri.URI('https://www.test.com?param1=value1&&param3=value3');  // CC-OFF(G.OTH.05) test logic
  let paramNames = route.getQueryNames();
  let values = paramNames.values();
  arktest.assertEQ(paramNames.length, 3);
  arktest.assertEQ(values.next().value, "param1");
  arktest.assertEQ(values.next().value, "");
  arktest.assertEQ(values.next().value, "param3");
}

function testUriGetQueryNames004() {
  let route = new uri.URI('https://www.test.com?param1=value1&=value2&param3=value3');  // CC-OFF(G.OTH.05) test logic
  let paramNames = route.getQueryNames();
  arktest.assertEQ(paramNames.length, 3);
  let values = paramNames.values();
  arktest.assertEQ(paramNames.length, 3);
  arktest.assertEQ(values.next().value, "param1");
  arktest.assertEQ(values.next().value, "");
  arktest.assertEQ(values.next().value, "param3");
}

function testUriGetQueryNames005() {
  let route = new uri.URI('https://www.test.com?param1=value1&&&a&b=v&param3');  // CC-OFF(G.OTH.05) test logic
  let paramNames = route.getQueryNames();
  arktest.assertEQ(paramNames.length, 5);
  let values = paramNames.values();
  arktest.assertEQ(values.next().value, "param1");
  arktest.assertEQ(values.next().value, "");
  arktest.assertEQ(values.next().value, "a");
  arktest.assertEQ(values.next().value, "b");
  arktest.assertEQ(values.next().value, "param3");
}

function testUriGetQueryNames006() {
  let route = new uri.URI('https://www.test.com?param%201=value1');  // CC-OFF(G.OTH.05) test logic
  let paramNames = route.getQueryNames();
  arktest.assertEQ(paramNames.length, 1);
  arktest.assertEQ(paramNames.values().next().value, "param 1");
  let route1 = new uri.URI('https://www.test.com?param+1=value1');  // CC-OFF(G.OTH.05) test logic
  let paramNames1 = route1.getQueryNames();
  arktest.assertEQ(paramNames1.length, 1);
  arktest.assertEQ(paramNames1.values().next().value, "param+1");
}

function testUriGetQueryValues001() {
  let route = new uri.URI('https://www.test.com/search?query=name&sort=asc&filter=f');  // CC-OFF(G.OTH.05) test logic
  let values = route.getQueryValues("query");
  arktest.assertEQ(values.length, 1);
}

function testUriGetQueryValues002() {
  let route = new uri.URI('https://www.test.com/search?query=name');  // CC-OFF(G.OTH.05) test logic
  let values = route.getQueryValues("query");
  arktest.assertEQ(values[0], "name");
}

function testUriGetQueryValues003() {
  let emptyParameter1 = new uri.URI('https://www.test.com/search?query=');  // CC-OFF(G.OTH.05) test logic
  let result1 = emptyParameter1.getQueryValues("query");
  arktest.assertEQ(result1.length, 1);
  arktest.assertEQ(result1[0], "");
  let emptyParameter2 = new uri.URI('https://www.test.com/search?=value&');  // CC-OFF(G.OTH.05) test logic
  let result2 = emptyParameter2.getQueryValues("");
  arktest.assertEQ(result2[0], "value");
  arktest.assertEQ(result2[1], "");
  let emptyParameter3 = new uri.URI('https://www.test.com?');  // CC-OFF(G.OTH.05) test logic
  let result3 = emptyParameter3.getQueryValues("");
  arktest.assertEQ(result3.length, 0);
  let emptyParameter4 = new uri.URI('https://www.test.com?param');  // CC-OFF(G.OTH.05) test logic
  let result4 = emptyParameter4.getQueryValues("param");
  arktest.assertEQ(result4.length, 1);
  arktest.assertEQ(result4[0], "");
  let emptyParameter5 = new uri.URI('https://www.test.com?param1=value1&&param3=value3');  // CC-OFF(G.OTH.05) test logic
  let result5 = emptyParameter5.getQueryValues("");
  arktest.assertEQ(result5[0], "");
}

function testUriGetQueryValues004() {
  let route = new uri.URI('https://www.test.com/search');  // CC-OFF(G.OTH.05) test logic
  let values = route.getQueryValues("query");
  arktest.assertEQ(values.length, 0);
}

function testUriGetQueryValues005() {
  let route = new uri.URI('https://www.test.com/search?query=my%20query');  // CC-OFF(G.OTH.05) test logic
  let values = route.getQueryValues("query");
  arktest.assertEQ(values[0], "my query");
}

function testUriGetQueryValues006() {
  let route = new uri.URI('https://www.test.com/search?query=name&query=my');  // CC-OFF(G.OTH.05) test logic
  let values = route.getQueryValues("query");
  arktest.assertEQ(values[0], "name");
  arktest.assertEQ(values[1], "my");
}

function testUriGetQueryValues007() {
  let route = new uri.URI('https://www.test.com?query=hello+world');  // CC-OFF(G.OTH.05) test logic
  let values = route.getQueryValues("query");
  arktest.assertEQ(values[0], "hello+world");
}

function testUriGetQueryValues008() {
  let route1 = new uri.URI('https://www.test.com?param_g=value_1');  // CC-OFF(G.OTH.05) test logic
  let values1 = route1.getQueryValues("param_g");
  arktest.assertEQ(values1[0], "value_1");
  arktest.assertEQ(route1.getQueryValue('param_g'), "value_1");
}

function testUriGetBooleanQueryValue001() {
  let route = new uri.URI('https://www.test.com/search?active=true');  // CC-OFF(G.OTH.05) test logic
  let isActive = route.getBooleanQueryValue("active", false);
  arktest.assertTrue(isActive);
}

function testUriGetBooleanQueryValue002() {
  let route = new uri.URI('https://www.test.com/search');  // CC-OFF(G.OTH.05) test logic
  let isActive = route.getBooleanQueryValue("active", false);
  arktest.assertFalse(isActive);
}

function testUriGetBooleanQueryValue003() {
  let route = new uri.URI('https://www.test.com/search?active=name');  // CC-OFF(G.OTH.05) test logic
  let isActive = route.getBooleanQueryValue("active", false);
  arktest.assertTrue(isActive);
}

function testUriGetBooleanQueryValue004() {
  let route = new uri.URI('https://www.test.com/search');  // CC-OFF(G.OTH.05) test logic
  let isActive = route.getBooleanQueryValue("active", true);
  arktest.assertTrue(isActive);
}

function testUriGetBooleanQueryValue005() {
  let route = new uri.URI('https://www.test.com/search?active=true&active=false');  // CC-OFF(G.OTH.05) test logic
  let isActive = route.getBooleanQueryValue("active", false);
  arktest.assertTrue(isActive);
  let uriInstance1 = new uri.URI("https://www.test.com/search?active=aa&active=false");  // CC-OFF(G.OTH.05) test logic
  arktest.assertTrue(uriInstance1.getBooleanQueryValue("active", false));
  let uriInstance2 = new uri.URI("https://www.test.com/search?active=0");  // CC-OFF(G.OTH.05) test logic
  arktest.assertFalse(uriInstance2.getBooleanQueryValue("active", true));
}

function main(): int {
  let suite = new arktest.ArkTestsuite("URI XTS tests");

  suite.addTest("testUriGetQueryNames001", testUriGetQueryNames001);
  suite.addTest("testUriGetQueryNames002", testUriGetQueryNames002);
  suite.addTest("testUriGetQueryNames003", testUriGetQueryNames003);
  suite.addTest("testUriGetQueryNames004", testUriGetQueryNames004);
  suite.addTest("testUriGetQueryNames005", testUriGetQueryNames005);
  suite.addTest("testUriGetQueryNames006", testUriGetQueryNames006);

  suite.addTest("testUriGetQueryValues001", testUriGetQueryValues001);
  suite.addTest("testUriGetQueryValues002", testUriGetQueryValues002);
  suite.addTest("testUriGetQueryValues003", testUriGetQueryValues003);
  suite.addTest("testUriGetQueryValues004", testUriGetQueryValues004);
  suite.addTest("testUriGetQueryValues005", testUriGetQueryValues005);
  suite.addTest("testUriGetQueryValues006", testUriGetQueryValues006);
  suite.addTest("testUriGetQueryValues007", testUriGetQueryValues007);
  suite.addTest("testUriGetQueryValues008", testUriGetQueryValues008);

  suite.addTest("testUriGetBooleanQueryValue001", testUriGetBooleanQueryValue001);
  suite.addTest("testUriGetBooleanQueryValue002", testUriGetBooleanQueryValue002);
  suite.addTest("testUriGetBooleanQueryValue003", testUriGetBooleanQueryValue003);
  suite.addTest("testUriGetBooleanQueryValue004", testUriGetBooleanQueryValue004);
  suite.addTest("testUriGetBooleanQueryValue005", testUriGetBooleanQueryValue005);

  //passed
  return suite.run();
}
