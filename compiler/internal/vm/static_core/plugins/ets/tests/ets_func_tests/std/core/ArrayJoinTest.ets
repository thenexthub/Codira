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

function nullTest() {
    let arr = [null, undefined, undefined, null];
    let res = ',,,';
    arktest.assertEQ(arr.join(), res);

    let arr2 = [null, [undefined, [undefined, [null]]]];
    res = '_,,';
    arktest.assertEQ(arr2.join('_'), res);
}

function booleanTest() {
    let arr1 = [true, false, false, true];
    let res = 'true,false,false,true';
    arktest.assertEQ(arr1.join(), res);

    let arr2 = [true, [false, [false, [true]]]];
    res = 'true_false,false,true';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<boolean> = [true, false]
    let arr3 = [false, x, true]
    res = 'false_true,false_true'
    arktest.assertEQ(arr3.join('_'), res);
}

function charTest() {
    let arr1 = [c'0', c'1', c'a', c'b'];
    let res = '0,1,a,b';
    arktest.assertEQ(arr1.join(), res);

    let arr2 = [c'0', [c'1', [c'a', [c'b']]]];
    res = '0_1,a,b';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<char> = [c'a', c'b']
    let arr3 = [c'c', x, c'd']
    res = 'c_a,b_d'
    arktest.assertEQ(arr3.join('_'), res);
}

function intTest() {
    let arr = [1, 2, Int.MIN_VALUE, Int.MAX_VALUE];
    let res = '1,2,-2147483648,2147483647';
    arktest.assertEQ(arr.join(), res);

    let arr2 = [1, [2, [Int.MIN_VALUE, [Int.MAX_VALUE]]]];
    res = '1_2,-2147483648,2147483647';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<int> = [1, 2, 3];
    let arr3 = [2, x, 1, 3];
    res = '2_1,2,3_1_3';
    arktest.assertEQ(arr3.join('_'), res);
}

function longTest() {
    let arr = [1, 2, Long.MIN_VALUE, Long.MAX_VALUE];
    let res = '1,2,-9223372036854775808,9223372036854775807';
    arktest.assertEQ(arr.join(), res);

    let arr2 = [1, [2, [Long.MIN_VALUE, [Long.MAX_VALUE]]]];
    res = '1_2,-9223372036854775808,9223372036854775807';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<long> = [Long.MIN_VALUE, Long.MAX_VALUE]
    let arr3 = [(4).toLong(), x, (5).toLong()]
    res = '4_-9223372036854775808,9223372036854775807_5'
    arktest.assertEQ(arr3.join('_'), res);
}

function floatTest() {
    let arr = [Float.NaN, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY, 1, -1, Float.MIN_VALUE, Float.MAX_VALUE];
    let res = 'NaN,Infinity,-Infinity,1,-1,1.401298464324817e-45,3.4028234663852886e+38';
    arktest.assertEQ(arr.join(), res);

    let arr2 = [Float.NaN, [Float.POSITIVE_INFINITY, [Float.NEGATIVE_INFINITY, [Float.MIN_VALUE, [Float.MAX_VALUE]]]]];
    res = 'NaN_Infinity,-Infinity,1e-45,3.4028235e+38';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<float> = [Float.MIN_VALUE, Float.MAX_VALUE]
    let arr3 = [(4).toFloat(), x, (5).toFloat()]
    res = '4_1e-45,3.4028235e+38_5'
    arktest.assertEQ(arr3.join('_'), res);
}

function doubleTest() {
    let arr = [NaN, Infinity, -Infinity, Double.MIN_VALUE, Float.MIN_VALUE, Double.MAX_VALUE, Float.MAX_VALUE];
    let res = 'NaN,Infinity,-Infinity,5e-324,1.401298464324817e-45,1.7976931348623157e+308,3.4028234663852886e+38';
    arktest.assertEQ(arr.join(), res);

    let arr2 = [NaN, [Infinity, [-Infinity, [Double.MIN_VALUE, Float.MIN_VALUE, [Double.MAX_VALUE, Float.MAX_VALUE]]]]];
    res = 'NaN_Infinity,-Infinity,5e-324,1e-45,1.7976931348623157e+308,3.4028235e+38';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<double> = [Double.MIN_VALUE, Double.MAX_VALUE]
    let arr3 = [(4).toDouble(), x, (5).toDouble()]
    res = '4_5e-324,1.7976931348623157e+308_5'
    arktest.assertEQ(arr3.join('_'), res);
}

function bigIntTest() {
    let arr = [-1n, 1n, new BigInt(Long.MIN_VALUE), new BigInt(Long.MAX_VALUE), BigInt(-3.4028235e+38)];
    let res = '-1,1,-9223372036854775808,9223372036854775807,-340282349999999991754788743781432688640';
    arktest.assertEQ(arr.join(), res);

    let arr2 = [-1n, [1n, [new BigInt(Long.MIN_VALUE), [new BigInt(Long.MAX_VALUE), [BigInt(-3.4028235e+38)]]]]];
    res = '-1_1,-9223372036854775808,9223372036854775807,-340282349999999991754788743781432688640';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<bigint> = [new BigInt(Long.MIN_VALUE), new BigInt(Long.MAX_VALUE)]
    let arr3 = [4n, x, 5n]
    res = '4_-9223372036854775808,9223372036854775807_5'
    arktest.assertEQ(arr3.join('_'), res);
}

function stringTest() {
    let arr = ['test1', 'test2', 'test3', 'test4'];
    let res = 'test1,test2,test3,test4';
    arktest.assertEQ(arr.join(), res);

    let arr2 = ['test1', ['test2', ['test3', ['test4']]]];
    res = 'test1_test2,test3,test4';
    arktest.assertEQ(arr2.join('_'), res);

    let x: FixedArray<string> = ['ab', 'cd']
    let arr3 = ['123', x, '456']
    res = '123_ab,cd_456'
    arktest.assertEQ(arr3.join('_'), res);
}

type SelfArray = (SelfArray | number)[];
type SelfArray2 = (SelfArray2 | (string | null | undefined))[];

function recursiveArrayTest1() {
    let arr: SelfArray = [];
    arr.push(arr);
    arr.push(1);
    arr.push(arr);
    arr.push(2);
    let res = ',1,,2';
    arktest.assertEQ(arr.join(), res);
}

function recursiveArrayTest2() {
    let arr: SelfArray2 = [];
    arr.push(arr);
    arr.push('test1');
    arr.push(undefined);
    arr.push('test2');
    arr.push(null);
    arr.push('test3');
    arr.push(arr);
    let res = ',test1,,test2,,test3,';
    arktest.assertEQ(arr.join(), res);
}

function recursiveArrayTest3() {
    let arr1: SelfArray = [];
    let arr2: SelfArray = [];
    let arr3: SelfArray = [];
    arr1.push(1);
    arr1.push(arr2);
    arr2.push(2);
    arr2.push(arr3);
    arr3.push(3);
    arr3.push(arr1)
    let res = '1,2,3,';
    arktest.assertEQ(arr1.join(), res);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array.join tests');
    suite.addTest('for null test', nullTest);
    suite.addTest('for boolean test', booleanTest);
    suite.addTest('for char test', charTest);
    suite.addTest('for int test', intTest);
    suite.addTest('for long test', longTest);
    suite.addTest('for float test', floatTest);
    suite.addTest('for double test', doubleTest);
    suite.addTest('for BigInt test', bigIntTest);
    suite.addTest('for String test', stringTest);
    suite.addTest('for recursive array test 1', recursiveArrayTest1);
    suite.addTest('for recursive array test 2', recursiveArrayTest2);
    suite.addTest('for recursive array test 3', recursiveArrayTest3);
    return suite.run();
}
