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
import { uri } from "@ohos.uri"


function testNormalize001() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/66./../././mm/.././path1?query#fagment');
  let res = gaogao.normalize();
  arktest.assertEQ(res.path, "/path/path1");
  arktest.assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/path1?query#fagment");
}

function testNormalize002() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment');
  let res = gaogao.normalize();
  arktest.assertEQ(res.path, "/../../path");
  arktest.assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment");
}

function testNormalize003() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment');
  let res = gaogao.normalize();
  arktest.assertEQ(res.path, "/../../../aa/bb/cc");
  arktest.assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../../aa/bb/cc?query#fagment");
}

function testNormalize004() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../path/.././../aa/bb/cc?query');
  let res = gaogao.normalize();
  arktest.assertEQ(res.path, "/../../aa/bb/cc");
  arktest.assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../aa/bb/cc?query");
}

function NormalizeTest001() {
  let muri = new uri.URI("http://user@[1:0:0:1:2:1:2:1]:99/path/66./../././mm/.././path1?query#fagment");
  let normalize = muri.normalize();
  arktest.assertEQ(normalize.toString(), "http://user@[1:0:0:1:2:1:2:1]:99/path/path1?query#fagment");
}

function NormalizeTest002() {
  let muri = new uri.URI("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment");
  let normalize = muri.normalize();
  arktest.assertEQ(normalize.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment");
}

function NormalizeTest003() {
  let muri = new uri.URI("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment");
  let normalize = muri.normalize();
  arktest.assertEQ(normalize.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../../aa/bb/cc?query#fagment");
}

function NormalizeTest004() {
  let muri = new uri.URI("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99?query#fagment");
  let normalize = muri.normalize();
  arktest.assertEQ(normalize.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99?query#fagment");
}

function main(): int {
  let suite = new arktest.ArkTestsuite("URI Normalize tests");
  suite.addTest("testNormalize001", testNormalize001);
  suite.addTest("testNormalize002", testNormalize002);
  suite.addTest("testNormalize003", testNormalize003);
  suite.addTest("testNormalize004", testNormalize004);
  
  suite.addTest("NormalizeTest001", NormalizeTest001);
  suite.addTest("NormalizeTest002", NormalizeTest002);
  suite.addTest("NormalizeTest003", NormalizeTest003);
  suite.addTest("NormalizeTest004", NormalizeTest004);
  //passed
  return suite.run();
}