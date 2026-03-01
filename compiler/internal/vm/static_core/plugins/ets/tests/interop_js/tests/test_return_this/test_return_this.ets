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

let module = ESValue.load('js_code');
let TestClass = module.getProperty('TestClass');
let testFuncReturnStr = module.getProperty('testFuncReturnStr');
let getThis = module.getProperty('getThis');
let getThisAsFunction = module.getProperty('getThisAsFunction');
let getThisAsAsyncFunction = module.getProperty('getThisAsAsyncFunction');

const TEST_NUM_ARRAY: FixedArray <number> = [1, 2, 3, 4, 5]
const TEST_STRING: string = 'Test'
const TEST_NUMBER: number = 100
const TEST_NULL: null = null
const TEST_OBJ: Object = true
const TEST_TUPLE: [string, number] = ['Test', 1]
const TEST_MAP: Map<string,string> = new Map<string,string>()
TEST_MAP.set('TEST', 'test')

type TestTupleType = [string, number] ;

function returnThis<T>(arg: T): T {
    return getThis.invokeMethod('call', ESValue.wrap(arg)).unwrap() as T;
}

function testReturnThisInClass(): boolean {
    let testClass = TestClass.instantiate();
    let testVal = testClass.invokeMethod('getThis');
    return ESValue.areStrictlyEqual(testClass, testVal);
}

function testReturnThisAsNumber(): boolean {
    let tmp = getThis.invokeMethod('call', TEST_NUMBER);
    return tmp.toNumber() == TEST_NUMBER;
}

function testReturnThisAsString(): boolean {
    return getThis.invokeMethod('call',TEST_STRING).toString() == TEST_STRING;
}

function testReturnThisAsNull(): boolean {
    return getThis.invokeMethod('call',TEST_NULL).toNull() == TEST_NULL;
}

function testReturnThisAsMap(): boolean {
   return getThis.invokeMethod('call',TEST_MAP) == TEST_MAP;
}

function testReturnThisAsTuple(): boolean {
    return returnThis<TestTupleType>(TEST_TUPLE) == TEST_TUPLE;
}

function testReturnThisAsArray(): boolean {
    return returnThis<FixedArray<number>>(TEST_NUM_ARRAY) == TEST_NUM_ARRAY;
}

function testReturnThisAsObj(): boolean {
    return returnThis<Object>(TEST_OBJ) == TEST_OBJ;
}

function testReturnThisAsObjClass(): boolean {
    const testClass = TestClass.instantiate();
    const testVal = getThis.invokeMethod('call',testClass);
    return ESValue.areStrictlyEqual(testVal, testClass);
}

function testReturnThisAsFunc(): boolean {
    const testVal = getThisAsFunction.invokeMethod('call',testFuncReturnStr);
    return testVal.invoke().toString() == TEST_STRING;
}

// Note: Test is commented until #18468 is resolved
/*
function testReturnThisAsAsyncFunc(): boolean {
    console.log("Start TestReturnThisAsAsyncFunc");
    let p = getThisAsAsyncFunction.call(testFuncReturnStr());
    let testVal = "";
    console.log("P = " + p);
    p.then<string | null>((value: string): string | null => {
        testVal = value;
        return null;
    });
    p.catch<string | null>((err: Any): string | null => {
        return null;
    });
    return testVal == TEST_STRING;
}
*/
