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

const input = ["hello", "HeLlO", "", "123!@#", "æøå", "ß"]
const outputUpper = ["HELLO", "HELLO", "", "123!@#", "ÆØÅ", "SS"]
const outputLower = ["hello", "hello", "", "123!@#", "æøå", "ß"]

function testStringtoLocaleUpperCase() {
    for (let i = 0;i < input.length;i++) {
        arktest.assertEQ(input[i].toLocaleUpperCase(), outputUpper[i]);
        arktest.assertEQ(input[i].toLocaleUpperCase("en-US"), outputUpper[i]);
        arktest.assertEQ(input[i].toLocaleUpperCase(["en-US", "tr-TR"]), outputUpper[i]);
        arktest.assertEQ(input[i].toLocaleUpperCase(["zh-CN", "tr-TR"]), outputUpper[i]);
        arktest.assertEQ(input[i].toLocaleUpperCase(new Array<string>(0)), outputUpper[i]);
    }
}

function testStringtoLocaleLowerCase() {
    for (let i = 0;i < input.length;i++) {
        arktest.assertEQ(input[i].toLocaleLowerCase(), outputLower[i]);
        arktest.assertEQ(input[i].toLocaleLowerCase("en-US"), outputLower[i]);
        arktest.assertEQ(input[i].toLocaleLowerCase(["en-US", "tr-TR"]), outputLower[i]);
        arktest.assertEQ(input[i].toLocaleLowerCase(["zh-CN", "tr-TR"]), outputLower[i]);
        arktest.assertEQ(input[i].toLocaleLowerCase(new Array<string>(0)), outputLower[i]);
    }
}

function main(): int {
   const suite = new arktest.ArkTestsuite("String.toLocaleUpperAndLowerCase");
   suite.addTest("Test testStringtoLocaleUpperCase", testStringtoLocaleUpperCase);
   suite.addTest("Test testStringtoLocaleLowerCase", testStringtoLocaleLowerCase);
   return suite.run()
}