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
import {BusinessError} from "@ohos.base";
import * as StringIo from "string_io";
import * as StringOp from "string_op";

loadLibrary("ani_string");

function concatString() {
    let str = StringOp.concatString("abc", "d");
    arktest.assertEQ(str, "abcd")
}

function makeString() {
    let str = StringOp.makeString("abc", 3);
    arktest.assertEQ(str, "abcabcabc")
}

function split() {
    let strPair = StringOp.split("abcd", 2);
    arktest.assertEQ(strPair._0, "ab")
    arktest.assertEQ(strPair._1, "cd")
}

function split2() {
    let strArray = StringOp.split2("abcd", 2);
    arktest.assertEQ(strArray[0], "ab")
    arktest.assertEQ(strArray[1], "cd")
}

function to_i32() {
    let num = StringOp.to_i32("123");
    arktest.assertEQ(num, 123)
}

function from_i32() {
    let str = StringOp.from_i32(123);
    arktest.assertEQ(str, "123")
}

function to_f32() {
    let num = StringOp.to_f32("123.123");
    arktest.assertEQ((num - 123.123) > -1e-6 || (num - 123.123) < 1e-6, true)
}

function from_f32() {
    let str = StringOp.from_f32(123.123f);
    arktest.assertEQ(str, "123.123")
}

function input() {
    let str = StringIo.input();
    arktest.assertEQ(str, "input")
}

function println() {
    StringIo.println("println string");
}

function print() {
    StringIo.print("print string");
    StringIo.println(" ---");
}

function testPlayString() {
    let playStr = StringOp.makePlayStringIface();
    arktest.assertEQ(playStr.name, "PlayString")

    let res = playStr.pickString(["abc", "edf", "xyz"], 1, 2)
    arktest.assertEQ(res, "edfxyz")

    res = playStr.pickString(["abc", "edf", "xyz"], 0, 1)
    arktest.assertEQ(res, "abcedf")
}

function concatString2() {
    let s = "a";
    let sArr: string[] = ["b", "c", "d"];
    let numbersU8: byte[] = [1, 2, 3, 4];
    let buffer: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        buffer.set(i, numbersU8[i]);
    }
    arktest.assertEQ(
        StringOp.concatString2(s, 1, sArr, true, buffer), "abcd1234");
    arktest.assertEQ(StringOp.concatString2(s, 1, sArr, false, buffer), "a");
}


function main() {
    console.log("run main ... StringTest ...")

    const suite = new arktest.ArkTestsuite("StringTest")
    suite.addTest("concatString", concatString)
    suite.addTest("concatString2", concatString2)
    suite.addTest("makeString", makeString)
    suite.addTest("split", split)
    suite.addTest("split2", split2)
    suite.addTest("to_i32", to_i32)
    suite.addTest("from_i32", from_i32)
    suite.addTest("to_f32", to_f32)
    suite.addTest("from_f32", from_f32)
    suite.addTest("input", input)
    suite.addTest("println", println)
    suite.addTest("print", print)
    suite.addTest("testPlayString", testPlayString)
    exit(suite.run())
}
