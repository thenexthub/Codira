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

/*---
ark_options: ['--compiler-inlining=false']
---*/
// NOTE(mgroshev): remove ark_options after #29029
function jsonStringifyArrayBoolean() {
    let arr = Array.of<boolean>(false, true, false);
    let expect = '[false,true,false]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayByte() {
    let arr = Array.of<Byte>(Byte.MIN_VALUE, -1 as byte, 0 as byte, 1 as byte, Byte.MAX_VALUE);
    let expect = '[-128,-1,0,1,127]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayChar() {
    let arr = Array.of<char>(c'0', c'1', c'a', c'z', c'A', c'Z');
    let expect = '["0","1","a","z","A","Z"]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayInt() {
    let arr = Array.of<int>(Int.MIN_VALUE, -1, 0, 1, Int.MAX_VALUE);
    let expect = '[-2147483648,-1,0,1,2147483647]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayLong() {
    let arr = Array.of<long>(Long.MIN_VALUE, -1 as long, 0 as long, 1 as long, Long.MAX_VALUE);
    let expect = '[-9223372036854775808,-1,0,1,9223372036854775807]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayFloat() {
    let arr = Array.of<float>(Float.MIN_VALUE, -1 as float, 0 as float, 1 as float, Float.MAX_VALUE,
                              Float.MAX_SAFE_INTEGER, Float.NaN, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY);
    let expect = '[1e-45,-1,0,1,3.4028235e+38,16777215,null,null,null]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayDouble() {
    let arr = Array.of<double>(Double.MIN_VALUE, -1 as double, 0 as double, 1 as double, Double.MAX_VALUE,
                               Double.MAX_SAFE_INTEGER, Double.NaN, Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY);
    let expect = '[5e-324,-1,0,1,1.7976931348623157e+308,9007199254740991,null,null,null]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

class TestClass {
    param1: int = 0;
    param2: string = 'test';
}

function jsonStringifyArrayClass() {
    let arr = Array.of<TestClass>(new TestClass(), new TestClass(), new TestClass());
    let expect = '[{"param1":0,"param2":"test"},{"param1":0,"param2":"test"},{"param1":0,"param2":"test"}]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArraySpace() {
    let arr = Array.of<double>(1, 2, 3, 4);
    let space: string | int = 1;
    let expect = '[\n 1,\n 2,\n 3,\n 4\n]';
    let ret = JSON.stringify(arr, undefined, space);
    arktest.assertEQ(ret, expect);

    space = ' ';
    ret = JSON.stringify(arr, undefined, space);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayReplacer1() {
    let arr = Array.of<double>(1, 2, 3, 4);
    let replacer: FixedArray<string | double> = [2, 4];
    let expect = '[1,2,3,4]';
    let ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);

    replacer = ['1', '2'];
    ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayReplacer2() {
    let arr = Array.of<int>(1, 2, 3, 4);
    let replacer: Array<string | number> = [2, 4];
    let expect = '[1,2,3,4]';
    let ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);

    replacer = ['1', '2'];
    ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayReplacer3() {
    let arr = Array.of<TestClass>(new TestClass(), new TestClass());
    let replacer: Array<string | number> = ['param2', 0];
    let expect = '[{"param2":"test"},{"param2":"test"}]';
    let ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);
}

function replaceNumber(key: string, value: Any): Any {
    if (key == '1' || key == '4') {
        return key + '_' + value;
    } else {
        return value;
    }
}

function jsonStringifyArrayReplacer4() {
    let arr = Array.of<int>(1, 2, 3, 4);
    let expect = '[1,"1_2",3,4]';
    let ret = JSON.stringify(arr, replaceNumber);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyRegExptest1() {
    const str = "For more information, see Chapter 3.4.5.1";
    const re = new RegExp('see (chapter \\d+(\\.\\d)*)', 'i');
    const found = str.match(re);
    let str1 = JSON.stringify(found);
    let expect = '["see Chapter 3.4.5.1","Chapter 3.4.5.1",".1"]'
    arktest.assertEQ(str1, expect);
}

function jsonStringifyRegExptest2() {
    const str = "ABCDEFGHIJKLMNOPQRSTUVWHIZabcdefghijklmnopqrstuvwxyz";
    const re = new RegExp('[A-E]', 'gi');
    const found = str.match(re);
    let str1 = JSON.stringify(found);
    let expect = '["A","B","C","D","E","a","b","c","d","e"]'
    arktest.assertEQ(str1, expect);
}

function jsonStringifyRegExptest3() {
    const str = "The quick brown fox jumps over the lazy dog. It barked.";
    const re = new RegExp('(?<animal>fox|cat) jumps over');
    const found = str.match(re);
    let str1 = JSON.stringify(found);
    let expect = '["fox jumps over","fox"]'
    arktest.assertEQ(str1, expect);
}

function jsonStringifyRegExptest4() {
    const str = encodeURI('\r'); 
    const re = new RegExp(str, 'g');
    let search = re.exec(str);
    let expected = '["%0D"]'; 
    let ret = JSON.stringify(search);
    arktest.assertEQ(ret, expected);
}

function jsonStringifyRegExptest5() {
    const str = encodeURI('\b\t\n');
    const re = new RegExp(str, 'g');
    let match = re.exec(str);
    let ret = JSON.stringify(match);
    let expected = '["%08%09%0A"]';
    arktest.assertEQ(ret, expected);
}

class ObjWithBoolArray {
    arr: FixedArray<boolean> = [true]
}

function jsonStringifyObjWithBoolArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithBoolArray()), '{"arr":[true]}')
}

class ObjWithByteArray {
    arr: FixedArray<byte> = [1]
}

function jsonStringifyObjWithByteArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithByteArray()), '{"arr":[1]}')
}

class ObjWithCharArray {
    arr: FixedArray<char> = [c'a']
}

function jsonStringifyObjWithCharArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithCharArray()), '{"arr":["a"]}')
}

class ObjWithShortArray {
    arr: FixedArray<short> = [ 1 ]
}

function jsonStringifyObjWithShortArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithShortArray()), '{"arr":[1]}')
}

class ObjWithIntArray {
    arr: FixedArray<int> = [ 1 ]
}

function jsonStringifyObjWithIntArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithIntArray()), '{"arr":[1]}')
}

class ObjWithLongArray {
    arr: FixedArray<long> = [ 1 ]
}

function jsonStringifyObjWithLongArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithLongArray()), '{"arr":[1]}')
}

class ObjWithFloatArray {
    arr: FixedArray<float> = [ 1.0 ]
}

function jsonStringifyObjWithFloatArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithFloatArray()), '{"arr":[1]}')
}

class ObjWithDoubleArray {
    arr: FixedArray<double> = [ 1.0 ]
}

function jsonStringifyObjWithDoubleArray(): void {
    arktest.assertEQ(JSON.stringify(new ObjWithDoubleArray()), '{"arr":[1]}')
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Json stringify for Array tests');
    suite.addTest('for boolean', jsonStringifyArrayBoolean);
    suite.addTest('for byte', jsonStringifyArrayByte);
    suite.addTest('for char', jsonStringifyArrayChar);
    suite.addTest('for int', jsonStringifyArrayInt);
    suite.addTest('for long', jsonStringifyArrayLong);
    suite.addTest('for float', jsonStringifyArrayFloat);
    suite.addTest('for double', jsonStringifyArrayDouble);
    suite.addTest('for class', jsonStringifyArrayClass);
    suite.addTest('for space', jsonStringifyArraySpace);
    suite.addTest('for replacer 1', jsonStringifyArrayReplacer1);
    suite.addTest('for replacer 2', jsonStringifyArrayReplacer2);
    suite.addTest('for replacer 3', jsonStringifyArrayReplacer3);
    suite.addTest('for replacer 4', jsonStringifyArrayReplacer4);
    suite.addTest('for RegExp 1', jsonStringifyRegExptest1);
    suite.addTest('for RegExp 2', jsonStringifyRegExptest2);
    suite.addTest('for RegExp 3', jsonStringifyRegExptest3);
    suite.addTest('for RegExp 2', jsonStringifyRegExptest4);
    suite.addTest('for RegExp 3', jsonStringifyRegExptest5);
    suite.addTest('for object with FixedArray<boolean> field', jsonStringifyObjWithBoolArray)
    suite.addTest('for object with FixedArray<byte> field', jsonStringifyObjWithByteArray)
    suite.addTest('for object with FixedArray<char> field', jsonStringifyObjWithCharArray)
    suite.addTest('for object with FixedArray<short> field', jsonStringifyObjWithShortArray)
    suite.addTest('for object with FixedArray<int> field', jsonStringifyObjWithIntArray)
    suite.addTest('for object with FixedArray<long> field', jsonStringifyObjWithLongArray)
    suite.addTest('for object with FixedArray<float> field', jsonStringifyObjWithFloatArray)
    suite.addTest('for object with FixedArray<double> field', jsonStringifyObjWithDoubleArray)
    return suite.run();
}
