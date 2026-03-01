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

function main(): int {
   const suite = new arktest.ArkTestsuite("String API test for SDK");
   suite.addTest("String.padEnd() fills in the correct characters when str is empty", testStringPadEnd);
   suite.addTest("String.padStart() fills in the correct characters when str is empty", testStringPadEnd);
   return suite.run()
}

class TestData {
   static readonly str = 'abc';
   static readonly padend_length = 5;
   static readonly padstart_length = 4;
   static readonly expected = 'abc'

}

function testStringPadEnd() {
   let str = TestData.str;
   let a = str.padEnd(TestData.padend_length, '')
   arktest.assertEQ(TestData.expected, a, `Expected: "${TestData.expected}", Actual: "${a}"`);
}

function testStringPadStart() {
   let str = TestData.str;
   let a = str.padStart(TestData.padstart_length, '')
   arktest.assertEQ(TestData.expected, a, `Expected: "${TestData.expected}", Actual: "${a}"`);
}
