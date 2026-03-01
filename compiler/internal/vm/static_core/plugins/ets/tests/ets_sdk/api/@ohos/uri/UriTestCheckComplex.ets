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

function IsAbsoluteTest001() {
  let muri = new uri.URI("f/tp://username:password@www.test.com:88/path?query#fagment");
  arktest.assertFalse(muri.checkIsAbsolute());
}

function IsAbsoluteTest002() {
  let muri = new uri.URI("ftp://username:password@www.test.com:88/path?query#fagment");
  arktest.assertTrue(muri.checkIsAbsolute());
}

function IsAbsoluteTest003() {
  let muri = new uri.URI("htt/p://username:password@www.test.com:88/path?query#fagment");
  arktest.assertFalse(muri.checkIsAbsolute());
}

function IsRelativeTest001() {
  let muri = new uri.URI("https://www.example.com/aaa");  // CC-OFF(G.OTH.05) test logic
  arktest.assertFalse(muri.checkRelative());
}

function IsRelativeTest002() {
  let muri = new uri.URI("/bbb");
  arktest.assertTrue(muri.checkRelative());
}

function IsOpaqueTest001() {
  let muri = new uri.URI("aaa:user@example.com");
  arktest.assertTrue(muri.checkOpaque());
}

function IsOpaqueTest002() {
  let muri = new uri.URI("content://com.example/bbb");
  arktest.assertFalse(muri.checkOpaque());
}

function IsHierarchicalTest001() {
  let muri = new uri.URI("https://www.example.com/path/to/resource");  // CC-OFF(G.OTH.05) test logic
  arktest.assertTrue(muri.checkHierarchical());
}

function IsHierarchicalTest002() {
  let muri = new uri.URI("/path/to/resource");
  arktest.assertTrue(muri.checkHierarchical());
}

function IsHierarchicalTest003() {
  let muri = new uri.URI("tel:123456789");
  arktest.assertFalse(muri.checkHierarchical());
}

function AddQueryValueTest001() {
  let muri = new uri.URI("https://username:password@host:8080/file?aaa=1#myfragment");
  let temp = muri.addQueryValue("bbb", "2");
  arktest.assertEQ(temp.toString(), "https://username:password@host:8080/file?aaa=1&bbb=2#myfragment");
}

function AddQueryValueTest002() {
  let muri = new uri.URI("mao:user@example.com");
  let temp = muri.addQueryValue("bb", "cc");
  arktest.assertEQ(temp.toString(), "mao:?bb=cc");
}

function ClearQueryTest001() {
  let muri = new uri.URI("https://username:password@host:8080/file?aaa=1#myfragment");
  let temp = muri.clearQuery();
  arktest.assertEQ(temp.toString(), "https://username:password@host:8080/file#myfragment");
}


function main(): int {
  let suite = new arktest.ArkTestsuite("URI UT tests");

  suite.addTest("IsAbsoluteTest001", IsAbsoluteTest001);
  suite.addTest("IsAbsoluteTest002", IsAbsoluteTest002);
  suite.addTest("IsAbsoluteTest003", IsAbsoluteTest003);

  suite.addTest("IsRelativeTest001", IsRelativeTest001);
  suite.addTest("IsRelativeTest002", IsRelativeTest002);

  suite.addTest("IsOpaqueTest001", IsOpaqueTest001);
  suite.addTest("IsOpaqueTest002", IsOpaqueTest002);

  suite.addTest("IsHierarchicalTest001", IsHierarchicalTest001);
  suite.addTest("IsHierarchicalTest002", IsHierarchicalTest002);
  suite.addTest("IsHierarchicalTest003", IsHierarchicalTest003);

  suite.addTest("AddQueryValueTest001", AddQueryValueTest001);
  suite.addTest("AddQueryValueTest002", AddQueryValueTest002);

  suite.addTest("ClearQueryTest001", ClearQueryTest001);
  //passed
  return suite.run();
}
