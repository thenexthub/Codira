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
import * as bigint_new from "bigint_new";
import * as bigint_test from "bigint_test";

loadLibrary("ani_bigint");

function main() {
    console.info("bigint_test run start...");
    const suite = new arktest.ArkTestsuite("bigint_test ut")
    suite.addTest("testBigint01", testBigint01);
    suite.addTest("testBigint02", testBigint02);
    suite.addTest("testBigint03", testBigint03);
    suite.addTest("testBigint04", testBigint04);
    suite.addTest("testBigint05", testBigint05);
    suite.addTest("testBigint06", testBigint06);

    suite.addTest("testProcessBigInt_1", testProcessBigInt_1);
    suite.addTest("testProcessBigInt_2", testProcessBigInt_2);
    suite.addTest("testProcessBigInt_3", testProcessBigInt_3);
    suite.addTest("testProcessBigInt_4", testProcessBigInt_4);
    suite.addTest("testProcessBigInt_5", testProcessBigInt_5);
    suite.addTest("testProcessBigInt_6", testProcessBigInt_6);

    suite.addTest("test_bigint_with_printBigInt", test_bigint_with_printBigInt);
    suite.addTest(
        "test_bigint_with_createBigInt", test_bigint_with_createBigInt);
    suite.addTest(
        "test_bigint_with_setAndgetBigInt", test_bigint_with_setAndgetBigInt);
    suite.addTest(
        "test_bigint_with_setupStructAndPrint",
        test_bigint_with_setupStructAndPrint);
    suite.addTest(
        "test_bigint_with_getBigIntOptional_1",
        test_bigint_with_getBigIntOptional_1);
    suite.addTest(
        "test_bigint_with_getBigIntOptional_2",
        test_bigint_with_getBigIntOptional_2);
    suite.addTest("test_bigint_with_mapBigInt", test_bigint_with_mapBigInt);
    suite.addTest("test_bigint_with_union", test_bigint_with_union);

    exit(suite.run());
    console.info("bigint_test run end...");
}

function testProcessBigInt_1() {
    let arg: bigint = (1n << 63n) - 1n;
    let res: bigint = bigint_new.processBigInt(arg);
    arktest.assertEQ(res, arg << 64n);
}

function testProcessBigInt_2() {
    let arg: bigint = 1n << 63n;
    let res: bigint = bigint_new.processBigInt(arg);
    arktest.assertEQ(res, arg << 64n);
}

function testProcessBigInt_3() {
    let arg: bigint = (1n << 64n) - 1n;
    let res: bigint = bigint_new.processBigInt(arg);
    arktest.assertEQ(res, arg << 64n);
}

function testProcessBigInt_4() {
    let arg: bigint = 1n << 64n;
    let res: bigint = bigint_new.processBigInt(arg);
    arktest.assertEQ(res, arg << 64n);
}

function testProcessBigInt_5() {
    let arg: bigint = -1n << 63n;
    let res: bigint = bigint_new.processBigInt(arg);
    arktest.assertEQ(res, arg << 64n);
}

function testProcessBigInt_6() {
    let arg: bigint = -1n << 64n;
    let res: bigint = bigint_new.processBigInt(arg);
    arktest.assertEQ(res, arg << 64n);
}

// test function Bigint01(a : number, b : BigInt) : boolean
function testBigint01() {
    let a: number = 666666;
    let b: bigint = 77777777777777777777777777n;
    let result: boolean = bigint_test.bigInt001(a, b);
    arktest.assertEQ(result, true);
}

// test function Bigint02(a: number) : number
function testBigint02() {
    let a: number = 77777777777;
    let result: number = bigint_test.bigInt002(a);
    arktest.assertEQ(result, a);
}

// test function Bigint03(a : BigInt) : BigInt
function testBigint03() {
    let a: bigint = 8888888888888888888888888888n;
    let result: bigint = bigint_test.bigInt003(a);
    arktest.assertEQ(result, a);
}

// test function Bigint04(a : BigInt) : void
function testBigint04() {
    let a: bigint = 8888888888888888888888888888n;
    bigint_test.bigInt004(a);
}

// test function Bigint05(a : number, b : BigInt) : BigInt
function testBigint05() {
    let a: number = 1;
    let b: bigint = 8888888888888888888888888888n;
    let result: bigint = bigint_test.bigInt005(a, b);
    arktest.assertEQ(result, b);
}

// test function Bigint06(a : number, b : BigInt) : number
function testBigint06() {
    let a: number = 1;
    let b: bigint = 8888888888888888888888888888n;
    let result: number = bigint_test.bigInt006(a, b);
    arktest.assertEQ(result, a);
}


// test function printBigInt(a: @bigint Array<u64>): void;
function test_bigint_with_printBigInt() {
    let num1: BigInt = 18446744073709551616n;
    bigint_new.printBigInt(num1);
    let num2: BigInt = -65535n;
    bigint_new.printBigInt(num2);
}

// function createBigInt(a: @bigint Array<u64>): @bigint Array<u64>;
function test_bigint_with_createBigInt() {
    let num1: BigInt = 18446744073709551616n;
    let res = bigint_new.createBigInt(num1);
    let expect: BigInt = res as BigInt;
    arktest.assertEQ(expect.toString(), num1.toString());
}

// BigInt set and get
let want: bigint_new.Want = bigint_new.getWant();
function test_bigint_with_setAndgetBigInt() {
    want.bigInt = 123n;
    let res = want.bigInt;
    arktest.assertEQ(res, 123n);
}

// function setupStructAndPrint(v: BigInt1): void;
function test_bigint_with_setupStructAndPrint() {
    let a: bigint_new.BigInt1 = {a: 123n};
    bigint_new.setupStructAndPrint(a);
}

// function getBigIntOptional(a: Optional<@bigint Array<u64>>): @bigint
// Array<u64>;
function test_bigint_with_getBigIntOptional_1() {
    let a: BigInt = 123n;
    let res = bigint_new.getBigIntOptional(a);
    console.log("test_bigint_with_getBigIntOptional_1 have Optional: ", res);
}

function test_bigint_with_getBigIntOptional_2() {
    let res = bigint_new.getBigIntOptional(undefined);
    console.log("test_bigint_with_getBigIntOptional_2 have no Optional: ", res);
}

// function mapBigInt(a: @record Map<String, @bigint Array<u64>>): @record
// Map<String, @bigint Array<u64>>;
function test_bigint_with_mapBigInt() {
    let data: Record<string, BigInt> = {"key1": 123n};
    let res = bigint_new.mapBigInt(data);
    console.log("test_bigint_with_mapBigInt result: ", res);
}

// union @bigint Array<u64> @null string
function test_bigint_with_union() {
    // union bigInt
    let bigIntData = 1020847100762815390427017310442723737601n;
    bigint_new.showMyUnion(bigIntData);
    let bigIntResult = bigint_new.makeMyUnion(1);
    console.log("bigIntResult: ", bigIntResult);
    arktest.assertEQ(bigIntData, bigIntResult);

    // union string
    let strData = "this is string data";
    bigint_new.showMyUnion(strData);
    let strResult = bigint_new.makeMyUnion(2);
    console.log("strResult: ", strResult);
    arktest.assertEQ(strData, strResult);

    // union null
    bigint_new.showMyUnion(null);
    let nullResult = bigint_new.makeMyUnion(0);
    arktest.assertEQ(nullResult, null);
}