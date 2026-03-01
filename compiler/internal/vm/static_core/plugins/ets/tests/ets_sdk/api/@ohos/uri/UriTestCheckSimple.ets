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

function testCheckIsAbsolute001() {
  let gaogao = new uri.URI('f/tp://username:password@www.test.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  arktest.assertFalse(res);
}

function testCheckIsAbsolute002() {
  let gaogao = new uri.URI('ftp://username:password@www.test.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  arktest.assertTrue(res);
}

function testCheckIsAbsolute003() {
  let gaogao = new uri.URI('htt/p://username:password@www.test.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  arktest.assertFalse(res);
}

function testCheckIsAbsolute004() {
  let gaogao = new uri.URI('https://username:password@www.test.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  arktest.assertTrue(res);
}

function testCheckIsAbsolute005() {
  let gaogao = new uri.URI('http1://username:password@www.test.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  arktest.assertTrue(res);
}

function testCheckRelative001() {
  let route = new uri.URI('http://www.test.com/images/pic.jpg');  // CC-OFF(G.OTH.05) test logic
  let res = route.checkRelative();
  arktest.assertFalse(res);
}

function testCheckRelative002() {
  let route = new uri.URI('/images/pic.jpg');
  let res = route.checkRelative();
  arktest.assertTrue(res);
}

function testCheckRelative003() {
  let route = new uri.URI('https://www.test.com/images/logo.jpg');  // CC-OFF(G.OTH.05) test logic
  let res = route.checkRelative();
  arktest.assertFalse(res);
}

function testCheckRelative004() {
  let route = new uri.URI('aaa');
  let res = route.checkRelative();
  arktest.assertTrue(res);
}

function testCheckRelative005() {
  let mailtoRoute = new uri.URI('mailto:user@test.com');
  arktest.assertFalse(mailtoRoute.checkRelative());
}

function testCheckOpaque001() {
  let route = new uri.URI('http://www.test.com/images/pic.jpg');  // CC-OFF(G.OTH.05) test logic
  let res = route.checkOpaque();
  arktest.assertFalse(res);
}

function testCheckOpaque002() {
  let route = new uri.URI('images/pic.jpg');
  let res = route.checkOpaque();
  arktest.assertFalse(res);
}

function testCheckOpaque003() {
  let route = new uri.URI('mailto:user@test.com');
  arktest.assertTrue(route.checkOpaque());
}

function testCheckOpaque004() {
  let route = new uri.URI('tel:1234567890');
  let res = route.checkOpaque();
  arktest.assertTrue(res);
}

function testCheckHierarchical001() {
  let route = new uri.URI('http://www.test.com/images/pic.jpg');  // CC-OFF(G.OTH.05) test logic
  let res = route.checkHierarchical();
  arktest.assertTrue(res);
}

function testCheckHierarchical002() {
  let route = new uri.URI('mailto:test@test.com');
  let res = route.checkHierarchical();
  arktest.assertFalse(res);
}

function testCheckHierarchical003() {
  let route = new uri.URI('images/pic.jpg');
  let res = route.checkHierarchical();
  arktest.assertTrue(res);
}

function testCheckHierarchical004() {
  let route = new uri.URI('path');
  arktest.assertTrue(route.checkHierarchical());
  let testUri = new uri.URI('path:');
  arktest.assertFalse(testUri.checkHierarchical());
}

function main(): int {
  let suite = new arktest.ArkTestsuite("URI XTS tests");

  suite.addTest("testCheckIsAbsolute001", testCheckIsAbsolute001);
  suite.addTest("testCheckIsAbsolute002", testCheckIsAbsolute002);
  suite.addTest("testCheckIsAbsolute003", testCheckIsAbsolute003);
  suite.addTest("testCheckIsAbsolute004", testCheckIsAbsolute004);
  suite.addTest("testCheckIsAbsolute005", testCheckIsAbsolute005);

  suite.addTest("testCheckRelative001", testCheckRelative001);
  suite.addTest("testCheckRelative002", testCheckRelative002);
  suite.addTest("testCheckRelative003", testCheckRelative003);
  suite.addTest("testCheckRelative004", testCheckRelative004);
  suite.addTest("testCheckRelative005", testCheckRelative005);

  suite.addTest("testCheckOpaque001", testCheckOpaque001);
  suite.addTest("testCheckOpaque002", testCheckOpaque002);
  suite.addTest("testCheckOpaque003", testCheckOpaque003);
  suite.addTest("testCheckOpaque004", testCheckOpaque004);

  suite.addTest("testCheckHierarchical001", testCheckHierarchical001);
  suite.addTest("testCheckHierarchical002", testCheckHierarchical002);
  suite.addTest("testCheckHierarchical003", testCheckHierarchical003);
  suite.addTest("testCheckHierarchical004", testCheckHierarchical004);


  //passed
  return suite.run();
}
