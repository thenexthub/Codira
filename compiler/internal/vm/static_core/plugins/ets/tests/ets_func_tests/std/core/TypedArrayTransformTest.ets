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

function testJoin() {
    let arr = [-2, Infinity, NaN, -Infinity, 0.6, 9007199254740992] as number[];
    let sample1 = new Float64Array(arr);
    let separator = ",";
    let result = sample1.join(separator);
    arktest.assertEQ(result, '-2,Infinity,NaN,-Infinity,0.6,9007199254740992');
    
    separator = ",,";
    result = sample1.join(separator);
    arktest.assertEQ(result, '-2,,Infinity,,NaN,,-Infinity,,0.6,,9007199254740992');

    separator = "";
    result = sample1.join(separator);
    arktest.assertEQ(result, '-2InfinityNaN-Infinity0.69007199254740992');

    separator = " a b c ";
    result = sample1.join(separator);
    arktest.assertEQ(result, '-2 a b c Infinity a b c NaN a b c -Infinity a b c 0.6 a b c 9007199254740992');

    let sample2 = new Float32Array(arr);
    separator = ",";
    result = sample2.join(separator);
    arktest.assertEQ(result, '-2,Infinity,NaN,-Infinity,0.6000000238418579,9007199254740992');
    separator = ",,";
    result = sample2.join(separator);
    arktest.assertEQ(result, '-2,,Infinity,,NaN,,-Infinity,,0.6000000238418579,,9007199254740992');

    separator = "";
    result = sample2.join(separator);
    arktest.assertEQ(result, '-2InfinityNaN-Infinity0.60000002384185799007199254740992');

    separator = " a b c ";
    result = sample2.join(separator);
    arktest.assertEQ(result, '-2 a b c Infinity a b c NaN a b c -Infinity a b c 0.6000000238418579 a b c 9007199254740992');

    let sample3 = new Int32Array(arr);
    separator = ",";
    result = sample3.join(separator);
    arktest.assertEQ(result, "-2,0,0,0,0,0");

    separator = ",,";
    result = sample3.join(separator);
    arktest.assertEQ(result, "-2,,0,,0,,0,,0,,0");

    separator = "";
    result = sample3.join(separator);
    arktest.assertEQ(result, "-200000");

    separator = " a b c ";
    result = sample3.join(separator);
    arktest.assertEQ(result, "-2 a b c 0 a b c 0 a b c 0 a b c 0 a b c 0");

    let sample4 = new Int16Array(arr);
    separator = ",";
    result = sample4.join(separator);
    arktest.assertEQ(result, "-2,0,0,0,0,0");

    separator = ",,";
    result = sample4.join(separator);
    arktest.assertEQ(result, "-2,,0,,0,,0,,0,,0");

    separator = "";
    result = sample4.join(separator);
    arktest.assertEQ(result, "-200000");

    separator = " a b c ";
    result = sample4.join(separator);
    arktest.assertEQ(result, "-2 a b c 0 a b c 0 a b c 0 a b c 0 a b c 0");

    let sample5 = new Int8Array(arr);
    separator = ",";
    result = sample5.join(separator);
    arktest.assertEQ(result, "-2,0,0,0,0,0");

    separator = ",,";
    result = sample5.join(separator);
    arktest.assertEQ(result, "-2,,0,,0,,0,,0,,0");

    separator = "";
    result = sample5.join(separator);
    arktest.assertEQ(result, "-200000");

    separator = " a b c ";
    result = sample5.join(separator);
    arktest.assertEQ(result, "-2 a b c 0 a b c 0 a b c 0 a b c 0 a b c 0");

    let sample6 = new Uint32Array(arr);
    separator = ",";
    result = sample6.join(separator);
    arktest.assertEQ(result, "4294967294,0,0,0,0,0");

    separator = ",,";
    result = sample6.join(separator);
    arktest.assertEQ(result, "4294967294,,0,,0,,0,,0,,0");

    separator = "";
    result = sample6.join(separator);
    arktest.assertEQ(result, "429496729400000");

    separator = " a b c ";
    result = sample6.join(separator);
    arktest.assertEQ(result, "4294967294 a b c 0 a b c 0 a b c 0 a b c 0 a b c 0");

    let sample7 = new Uint16Array(arr);
    separator = ",";
    result = sample7.join(separator);
    arktest.assertEQ(result, "65534,0,0,0,0,0");

    separator = ",,";
    result = sample7.join(separator);
    arktest.assertEQ(result, "65534,,0,,0,,0,,0,,0");

    separator = "";
    result = sample7.join(separator);
    arktest.assertEQ(result, "6553400000");

    separator = " a b c ";
    result = sample7.join(separator);
    arktest.assertEQ(result, "65534 a b c 0 a b c 0 a b c 0 a b c 0 a b c 0");

    let sample8 = new Uint8Array(arr);
    separator = ",";
    result = sample8.join(separator);
    arktest.assertEQ(result, "254,0,0,0,0,0");

    separator = ",,";
    result = sample8.join(separator);
    arktest.assertEQ(result, "254,,0,,0,,0,,0,,0");

    separator = "";
    result = sample8.join(separator);
    arktest.assertEQ(result, "25400000");

    separator = " a b c ";
    result = sample8.join(separator);
    arktest.assertEQ(result, "254 a b c 0 a b c 0 a b c 0 a b c 0 a b c 0");

    let sample9 = new Uint8ClampedArray(arr);
    separator = ",";
    result = sample9.join(separator);
    arktest.assertEQ(result, "0,255,0,0,1,255");

    separator = ",,";
    result = sample9.join(separator);
    arktest.assertEQ(result, "0,,255,,0,,0,,1,,255");

    separator = "";
    result = sample9.join(separator);
    arktest.assertEQ(result, "0255001255");

    separator = " a b c ";
    result = sample9.join(separator);
    arktest.assertEQ(result, "0 a b c 255 a b c 0 a b c 0 a b c 1 a b c 255");
}

function testSet() {
    let sample1 = new Float64Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src1 = new Float32Array(sample1.buffer, 0, 2);
    src1[0] = 42;
    sample1.set(src1, 1);
    arktest.assertEQ(sample1.toString(), '1.0000002464512363,42,1.875,4,5,6,7,8');
    let sample2 = new Float32Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src2 = new Float64Array(sample2.buffer, 0, 2);
    src2[0] = 42;
    sample2.set(src2, 1);
    arktest.assertEQ(sample2.toString(), '0,42,512.0001220703125,4,5,6,7,8');
    let sample3 = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src3 = new Float32Array(sample3.buffer, 0, 2);
    src3[0] = 42;
    sample3.set(src3, 1);
    arktest.assertEQ(sample3.toString(), '1109917696,42,0,4,5,6,7,8');
    let sample4 = new Int16Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src4 = new Float32Array(sample4.buffer, 0, 2);
    src4[0] = 42;
    sample4.set(src4, 1);
    arktest.assertEQ(sample4.toString(), '0,42,0,4,5,6,7,8');
    let sample5 = new Int8Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src5 = new Float32Array(sample5.buffer, 0, 2);
    src5[0] = 42;
    sample5.set(src5, 1);
    arktest.assertEQ(sample5.toString(), '0,42,0,66,5,6,7,8');
    let sample6 = new Uint32Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src6 = new Float32Array(sample6.buffer, 0, 2);
    src6[0] = 42;
    sample6.set(src6, 1);
    arktest.assertEQ(sample6.toString(), '1109917696,42,0,4,5,6,7,8');
    let sample7 = new Uint16Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src7 = new Float32Array(sample7.buffer, 0, 2);
    src7[0] = 42;
    sample7.set(src7, 1);
    arktest.assertEQ(sample7.toString(), '0,42,0,4,5,6,7,8');
    let sample8 = new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]);
    let src8 = new Float32Array(sample8.buffer, 0, 2);
    src8[0] = 42;
    sample8.set(src8, 1);
    arktest.assertEQ(sample8.toString(), '0,42,0,66,5,6,7,8');
    let sample9 = new Uint8ClampedArray([1, 2, 3, 4, 5, 6, 7, 8]);
    let src9 = new Float32Array(sample9.buffer, 0, 2);
    src9[0] = 42;
    sample9.set(src9, 1);
    arktest.assertEQ(sample9.toString(), '0,42,0,66,5,6,7,8');
}

function testSort() {
    let sample = new Float32Array([2, NaN, NaN, 0, 1]).sort();
    arktest.assertEQ(sample[0], 0);
    arktest.assertEQ(sample[1], 1);
    arktest.assertEQ(sample[2], 2);
    arktest.assertEQ(Double.isNaN(sample[3]), true);
    arktest.assertEQ(Double.isNaN(sample[4]), true);

    sample = new Float32Array([3, NaN, NaN, Infinity, 0, -Infinity, 2]).sort();
    arktest.assertEQ(sample[0], -Infinity);
    arktest.assertEQ(sample[1], 0);
    arktest.assertEQ(sample[2], 2);
    arktest.assertEQ(sample[3], 3);
    arktest.assertEQ(sample[4], Infinity);
    arktest.assertEQ(Double.isNaN(sample[5]), true);
    arktest.assertEQ(Double.isNaN(sample[6]), true);

    let sample1 = new Float64Array([2, NaN, NaN, 0, 1]).sort();
    arktest.assertEQ(sample1[0], 0);
    arktest.assertEQ(sample1[1], 1);
    arktest.assertEQ(sample1[2], 2);
    arktest.assertEQ(Double.isNaN(sample1[3]), true);
    arktest.assertEQ(Double.isNaN(sample1[4]), true);

    sample1 = new Float64Array([3, NaN, NaN, Infinity, 0, -Infinity, 2]).sort();
    arktest.assertEQ(sample1[0], -Infinity);
    arktest.assertEQ(sample1[1], 0);
    arktest.assertEQ(sample1[2], 2);
    arktest.assertEQ(sample1[3], 3);
    arktest.assertEQ(sample1[4], Infinity);
    arktest.assertEQ(Double.isNaN(sample1[5]), true);
    arktest.assertEQ(Double.isNaN(sample1[6]), true);
}

function testSort2() {
    let int8Array1 = new Int8Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(int8Array1.toString(), '1,2,3,4');

    let int8Array2 = new Int8Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(int8Array2.toString(), '1,2,3,4');

    let uint8Array1 = new Uint8Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(uint8Array1.toString(), '1,2,3,4');

    let uint8Array2 = new Uint8Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(uint8Array2.toString(), '1,2,3,4');

    let uint8ClampedArray1 = new Uint8ClampedArray([4, 3, 2, 1]).sort();
    arktest.assertEQ(uint8ClampedArray1.toString(), '1,2,3,4');

    let uint8ClampedArray2 = new Uint8ClampedArray([3, 4, 1, 2]).sort();
    arktest.assertEQ(uint8ClampedArray2.toString(), '1,2,3,4');

    let int16Array1 = new Int16Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(int16Array1.toString(), '1,2,3,4');

    let int16Array2 = new Int16Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(int16Array2.toString(), '1,2,3,4');

    let uint16Array1 = new Uint16Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(uint16Array1.toString(), '1,2,3,4');

    let uint16Array2 = new Uint16Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(uint16Array2.toString(), '1,2,3,4');

    let int32Array1 = new Int32Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(int32Array1.toString(), '1,2,3,4');

    let int32Array2 = new Int32Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(int32Array2.toString(), '1,2,3,4');

    let uint32Array1 = new Uint32Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(uint32Array1.toString(), '1,2,3,4');

    let uint32Array2 = new Uint32Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(uint32Array2.toString(), '1,2,3,4');

    let float32Array1 = new Float32Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(float32Array1.toString(), '1,2,3,4');

    let float32Array2 = new Float32Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(float32Array2.toString(), '1,2,3,4');

    let float64Array1 = new Float64Array([4, 3, 2, 1]).sort();
    arktest.assertEQ(float64Array1.toString(), '1,2,3,4');

    let float64Array2 = new Float64Array([3, 4, 1, 2]).sort();
    arktest.assertEQ(float64Array2.toString(), '1,2,3,4');
}

function testTypedArrayInternalTransform() {
    let array = new Int32Array(new Uint32Array([-1]));
    arktest.assertEQ(String(array[0]), '-1');

    let bigintValue = BigInt(12345678901234567890n);
    let numberValue = Number(bigintValue);
    let int32Array = Int32Array.of(numberValue);
    arktest.assertEQ(String(int32Array[0]), '-350287872');
}

function testTypedArrayInternalTransform2() {
    let arrayOld = new Array<number>();
    arrayOld.push(2147483648);
    let array = new Int8Array(new Int32Array(arrayOld));
    arktest.assertEQ(String(array[0]), '0');

    arrayOld = new Array<number>();
    arrayOld.push(2147483648);
    array = new Int8Array(new Uint32Array(arrayOld));
    arktest.assertEQ(String(array[0]), '0');

    arrayOld = new Array<number>();
    arrayOld.push(4294967294);
    array = new Int8Array(new Uint32Array(arrayOld));
    arktest.assertEQ(String(array[0]), '-2');
}

function testTypedArrayBound() {
    let array = Int32Array.of(-2147483649);
    arktest.assertEQ(String(array[0]), '2147483647');

    array = Int32Array.of(2147483648);
    arktest.assertEQ(String(array[0]), '-2147483648');
}

function testTypedArrayBound2() {
    let arrayOld = new Array<number>();
    arrayOld.push(2147483648);
    let array = new Int8Array(new Int32Array(arrayOld));
    arktest.assertEQ(String(array[0]), '0');
}

function testUint8ClampedArray() {
    let array: Uint8ClampedArray = new Uint8ClampedArray([25, -81, 360]);
    const mapped = array.map(Math.sqrt);
    arktest.assertEQ(mapped.toString(), '5,0,16');

    let array2: Uint8ClampedArray = new Uint8ClampedArray([0.5, 0.6, 1.5, 1.6]);
    arktest.assertEQ(array2.toString(), '0,1,2,2');
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite("TypedArray Trasnform Datas Suite");
    myTestsuite.addTest("testJoin", testJoin);
    myTestsuite.addTest("testSet", testSet);
    myTestsuite.addTest("testSort", testSort);
    myTestsuite.addTest("testSort2", testSort2);
    myTestsuite.addTest("testTypedArrayInternalTransform", testTypedArrayInternalTransform);
    myTestsuite.addTest("testTypedArrayInternalTransform2", testTypedArrayInternalTransform2);
    myTestsuite.addTest("testTypedArrayBound", testTypedArrayBound);
    myTestsuite.addTest("testTypedArrayBound2", testTypedArrayBound2);
    myTestsuite.addTest("testUint8ClampedArray", testUint8ClampedArray);
    myTestsuite.run();
}
