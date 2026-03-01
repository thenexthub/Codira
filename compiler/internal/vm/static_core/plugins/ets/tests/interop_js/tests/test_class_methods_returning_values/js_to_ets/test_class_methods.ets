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

let returnIntegerAsAny = module.getProperty('returnIntegerAsAny');
let returnStringAsAny = module.getProperty('returnStringAsAny');
let returnBigIntegerAsAny = module.getProperty('returnBigIntegerAsAny');
let returnBooleanAsAny = module.getProperty('returnBooleanAsAny');
let returnUndefinedAsAny = module.getProperty('returnUndefinedAsAny');
let returnNullAsAny = module.getProperty('returnNullAsAny');
let returnMapAsAny = module.getProperty('returnMapAsAny');
let returnSetAsAny = module.getProperty('returnSetAsAny');

let returnStrLiteral = module.getProperty('returnStrLiteral');
let returnIntLiteral = module.getProperty('returnIntLiteral');
let returnBoolLiteral = module.getProperty('returnBoolLiteral');
let returnsBigNLiteral = module.getProperty('returnsBigNLiteral');

let returnMap = module.getProperty('returnMap');
let returnSet = module.getProperty('returnSet');
let returnTuple = module.getProperty('returnTuple');
let isTTupleTS = module.getProperty('isTTupleTS');

let returnStringSubsetByRef = module.getProperty('returnStringSubsetByRef');
let returnMapSubsetByRef = module.getProperty('returnMapSubsetByRef');
let returnStringSubsetByValue = module.getProperty('returnStringSubsetByValue');
let returnIntSubsetByValue = module.getProperty('returnIntSubsetByValue');
let returnLongIntSubsetByValue = module.getProperty('returnLongIntSubsetByValue');
let returnBoolSubsetByValue = module.getProperty('returnBoolSubsetByValue');
let returnUndefinedSubsetByValue = module.getProperty('returnUndefinedSubsetByValue');
let returnNullSubsetByValue = module.getProperty('returnNullSubsetByValue');
let returnUnion = module.getProperty('returnUnion');
let returnClass = module.getProperty('returnClass');
let returnInterface = module.getProperty('returnInterface');

const SAMPLE_STRING = 'This is a test string'
const TEST_LONG_INT = 9007199254740991
const TEST_INT = 100
const TEST_BIG_INT = 10000000n
const TEST_BOOLEAN = true

const TEST_LITERAL_STRING = 'Test'
const TEST_LITERAL_INTEGER = 1
const TEST_LITERAL_BOOLEAN  = false
const TEST_LITERAL_BIGN = 100n

type ETSTupleType = [ string, int, boolean ]

function TestReturnIntegerAsAny(): boolean {
    const TestFunc: Any = returnIntegerAsAny;
    const testVal = TestFunc.invoke().toNumber();
    return testVal === TEST_INT;
}

function TestReturnStringAsAny(): boolean {
    const TestFunc: Any = returnStringAsAny;
    const testVal = TestFunc.invoke().toString();
    return testVal === SAMPLE_STRING;
}

function TestReturnBigIntegerAsAny(): boolean {
    const TestFunc: Any = returnBigIntegerAsAny;
    const testVal = TestFunc.invoke().toBigInt();
    return testVal === TEST_BIG_INT
}

function TestReturnBooleanAsAny(): boolean {
    const TestFunc: Any = returnBooleanAsAny;
    const testVal = TestFunc.invoke().toBoolean();
    return testVal === TEST_BOOLEAN;
}

// Note: Using Any is a workaround until #17745 is not resolved
function TestReturnNullAsAny(): boolean {
    const TestFunc: Any = returnNullAsAny;
    const testVal = TestFunc.invoke().unwrap() as Any;
    return testVal === null;
}

function TestReturnUndefinedAsAny(): boolean {
    const TestFunc: Any = returnUndefinedAsAny
    const testVal = TestFunc.invoke().toUndefined();
    return testVal === undefined
}

function TestReturnMapAsAny(): boolean {
    const TestFunc: Any = returnMapAsAny;
    const map = TestFunc.invoke().unwrap() as Map<Any, Any>;
    return map.has(SAMPLE_STRING.toUpperCase()) as boolean;
}

function TestReturnSetAsAny(): boolean {
    const TestFunc: Any = returnSetAsAny;
    const set = TestFunc.invoke().unwrap() as Set<Any>;
    return (
        set.size as int > 0
        && !(set.has(SAMPLE_STRING) as boolean)
        && set.has(SAMPLE_STRING.toLowerCase()) as boolean
    );
}

function TestReturnStringAsLiteral(): boolean {
    const TestFunc: Any = returnStrLiteral;
    const testVal = TestFunc.invoke().toString();
    return testVal === TEST_LITERAL_STRING;
}

function TestReturnBigNAsLiteral(): boolean {
    const TestFunc: Any = returnsBigNLiteral;
    const testVal = TestFunc.invoke().toBigInt();
    return testVal === TEST_LITERAL_BIGN;
}

function TestReturnIntAsLiteral(): boolean {
    const TestFunc: Any = returnIntLiteral;
    const testVal = TestFunc.invoke().toNumber();
    return testVal === TEST_LITERAL_INTEGER;
}

function TestReturnBoolAsLiteral(): boolean {
    const TestFunc: Any = returnBoolLiteral;
    const testVal = TestFunc.invoke().toBoolean();
    return testVal === TEST_LITERAL_BOOLEAN;
}

function TestReturnMap(): boolean {
    const map = returnMap.invoke().unwrap() as Map<Any, Any>;
    return map.has(SAMPLE_STRING.toUpperCase()) as boolean;
}

function TestReturnSet(): boolean {
    const set = returnSet.invoke().unwrap() as Set<Any>;
    return (
        set.size as int > 0
        && !(set.has(SAMPLE_STRING) as boolean)
        && set.has(SAMPLE_STRING.toLowerCase()) as boolean
    );
}

function TestReturnTuple(): boolean {
    const expectedTuple: Any = returnTuple.invoke().unwrap() as ETSTupleType;
    return (isTTupleTS.invoke(expectedTuple).toBoolean());
}

function TestReturnStringSubsetByRef(): boolean {
    const TestFunc: Any = returnStringSubsetByRef;
    const testVal = TestFunc.invoke();
    return (
        testVal.toString() === SAMPLE_STRING
        && testVal.typeOf() === 'string'
    );
}

function TestReturnMapSubsetByRef(): boolean {
    const TestFunc: Any = returnMapSubsetByRef
    const initialMap = TestFunc.invoke().unwrap() as Map<Any, Any>;
    const changedMap = TestFunc.invoke().unwrap() as Map<Any, Any>;

    const initialInitialMapSize: number = initialMap.size;
    const initialChangedMapSize: number = changedMap.size;
    changedMap.set('2', 'Test');
    const finalInitialMapSize: number = initialMap.size;
    const finalChangedMapSize: number = changedMap.size;

    return (
        initialInitialMapSize === 1
        && initialChangedMapSize === 1
        && initialMap.has('2') as boolean
        && finalInitialMapSize === 2
        && finalChangedMapSize === 2
    )
}

function TestReturnStringSubsetByValue(): boolean {
    const testVal  = returnStringSubsetByValue.invoke();
    return (
        testVal.toString() === SAMPLE_STRING
        && testVal.typeOf() === 'string'
    );
}

function TestReturnIntSubsetByValue(): boolean {
    const TestFunc = returnIntSubsetByValue;
    const testVal = TestFunc.invoke();
    return (
        testVal.toNumber() === TEST_INT
        && testVal.typeOf() === 'number'
    );
}

function TestReturnLongIntSubsetByValue(): boolean {
    const TestFunc = returnLongIntSubsetByValue;
    const testVal = TestFunc.invoke().unwrap() as double;
    return (
        testVal === TEST_LONG_INT
        && typeof testVal === 'number'
    );
}

function TestReturnBoolSubsetByValue(): boolean {
    const TestFunc = returnBoolSubsetByValue;
    const testVal = TestFunc.invoke();
    return (
        testVal.toBoolean() === TEST_BOOLEAN
        && testVal.typeOf() === 'boolean'
    );
}

// Note: Using Any is a workaround until #17745 is not resolved
function TestReturnNullSubsetByValue(): boolean {
    const TestFunc = returnNullSubsetByValue;
    const testVal = TestFunc.invoke().unwrap() as Any;
    return (
        testVal === null
        && typeof testVal === 'object'
    );
}

function TestReturnUndefinedSubsetByValue(): boolean {
    const TestFunc = returnUndefinedSubsetByValue;
    const testVal = TestFunc.invoke().toUndefined();
    return testVal === undefined;
}

function TestReturnUnion(): boolean {
    const TestFunc = returnUnion;
    const testValStr = TestFunc.invoke(true).toString();
    const testValNum = TestFunc.invoke(false).toNumber();
    return (
        typeof testValStr === 'string' && testValStr as string === '1000'
        && typeof testValNum === 'number' && testValNum as number === 1000);

}

function TestReturnClass(): boolean {
    const testFunc: Any = returnClass;
    const testClassVal = testFunc.invoke();
    return (
        testClassVal.getProperty('testString').toString() === 'Test'
        && testClassVal.getProperty('testNumber').toNumber() === 1
        && testClassVal.invokeMethod('testFunction').toNumber() === 1
        && TestClass.invokeMethod('isTestClass', testClassVal).toBoolean() === true
    );
}

function TestReturnInterface(): boolean {
    const TestFunc: Any = returnInterface;
    const testVal = TestFunc.invoke();
    return (
        testVal.getProperty('testString').toString() === 'Test'
        && testVal.getProperty('testNumber').toNumber() === 100
    );
}
