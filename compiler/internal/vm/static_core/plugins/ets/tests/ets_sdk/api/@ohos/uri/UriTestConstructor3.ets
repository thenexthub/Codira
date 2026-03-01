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

function ConstructorTest033() {
  let uriText = "/username/path/path?query#fagment";
  arktest.expectNoThrow(() => { new uri.URI(uriText) });
}

function ConstructorTest034() {
  let uriText = "http:/userna^me:password@www.test.com:99/path/path?query#fagment";
  arktest.expectNoThrow(() => { new uri.URI(uriText) });
}

function ConstructorTest035() {
  let uri = new uri.URI("http://?query#fagment");
  arktest.assertEQ(uri.scheme, "http");
}

function ConstructorTest036() {
  let uriText = "http:/username:password@www.test.com:99/pa^th/path?query#fagment";
  arktest.expectNoThrow(() => { new uri.URI(uriText) });
}

function ConstructorTest037() {
  let uriText = "http:/username:password@www.test.com:9^9/path/path?query#fagment";
  arktest.expectNoThrow(() => { new uri.URI(uriText) });
}

function ConstructorTest038() {
  let uriText = "http:/username:password@[1:0:0:1:2:1:2:1]:9^9/path/path?query#fagment";
  arktest.expectNoThrow(() => { new uri.URI(uriText) });


}

function ConstructorTest039() {
  let uriText = "http:/username:password@[1:0:0:1:2:1:2:1/path/path?query#fagment";
  arktest.expectNoThrow(() => { new uri.URI(uriText) });
}

function ConstructorTest040() {
  let uriText = "http://gg:gaogao@[1:0:0:sfvs1:2:1:2:1]:99/path1?query#fagment";
  let errText = "BusinessError: Syntax Error. Invalid Uri string: The Ipv6 does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest041() {
  let uriText = "http、:/username:password@www.test.com:99/path/path?query#fagment";
  let errText = "BusinessError: Syntax Error. Invalid Uri string: The Scheme does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest042() {
  let uriText = "http://[ffff::1]:、99/path/path?query#fagment";
  let errText = "BusinessError: Syntax Error. Invalid Uri string: The Port does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest043() {
  let uriText = "http://[ffff::1:99/path/path?query#fagment";
  let errText = "BusinessError: Syntax Error. Invalid Uri string: The IPv6 is missing a closing bracket.";
  testConstructorThrow(uriText, errText);
}

function ConstructorTest044() {
  let uriText = "http://ffff::1]:99/path/path?query#fagment";
  let errText = "BusinessError: Syntax Error. Invalid Uri string: The Host does not conform to the rule.";
  testConstructorThrow(uriText, errText);
}

function testConstructorThrow(uriText: string, errText: string) {
  let exceptionCheck = (e: Error): boolean | string => {
    if (e instanceof BusinessError) {
      return e.name == "BusinessError" && e.toString() == errText
    }
    return false
  }
  arktest.expectThrow((): void => { let that = new uri.URI(uriText); }, exceptionCheck)
}


function main(): int {
  let suite = new arktest.ArkTestsuite("URI UT tests");
  suite.addTest("ConstructorTest033", ConstructorTest033);
  suite.addTest("ConstructorTest034", ConstructorTest034);
  suite.addTest("ConstructorTest035", ConstructorTest035);
  suite.addTest("ConstructorTest036", ConstructorTest036);
  suite.addTest("ConstructorTest037", ConstructorTest037);
  suite.addTest("ConstructorTest038", ConstructorTest038);
  suite.addTest("ConstructorTest039", ConstructorTest039);
  suite.addTest("ConstructorTest040", ConstructorTest040);
  suite.addTest("ConstructorTest041", ConstructorTest041);
  suite.addTest("ConstructorTest042", ConstructorTest042);
  suite.addTest("ConstructorTest043", ConstructorTest043);
  suite.addTest("ConstructorTest044", ConstructorTest044);

  //passed
  return suite.run();
}