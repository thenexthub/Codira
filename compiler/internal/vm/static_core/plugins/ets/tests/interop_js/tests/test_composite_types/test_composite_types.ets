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

let module = ESValue.load('js_code')
let isTTupleTS = module.getProperty('isTTupleTS');
let returnTuple = module.getProperty('returnTuple');
let returnStrArray = module.getProperty('returnStrArray');
let returnNumArray = module.getProperty('returnNumArray');
let returnRecord = module.getProperty('returnRecord');
let returnInterface = module.getProperty('returnInterface');


const TEST_STRING: string = 'This is a test string';
const TEST_INT: int = 100;
const TEST_BOOLEAN: boolean = true;
const EXP_STR_ARRAY: FixedArray<string> = ["One", "Two", "Three"];
const EXP_NUM_ARRAY: FixedArray<int> = [1, 2, 3];

type ETSTupleType = [ string, int, boolean ];

function TestReturnTuple(): boolean {
    const actTuple = returnTuple.invoke();

    return (isTTupleTS.invoke(actTuple).toBoolean() &&
        actTuple.getProperty(0).toString() == TEST_STRING &&
        actTuple.getProperty(1).toNumber().toInt() == TEST_INT &&
        actTuple.getProperty(2).toBoolean() == TEST_BOOLEAN
    );
}

function TestReturnStrArray(): boolean {
    const actArray = returnStrArray.invoke();
    return (
        actArray.getProperty(0).toString() == EXP_STR_ARRAY[0] &&
        actArray.getProperty(1).toString() == EXP_STR_ARRAY[1] &&
        actArray.getProperty(2).toString() == EXP_STR_ARRAY[2]
    );
}

function TestReturnNumArray(): boolean {
    const actArray = returnNumArray.invoke();
    return (
        actArray.getProperty(0).toNumber().toInt() == EXP_NUM_ARRAY[0] &&
        actArray.getProperty(1).toNumber().toInt() == EXP_NUM_ARRAY[1] &&
        actArray.getProperty(2).toNumber().toInt() == EXP_NUM_ARRAY[2]
    );
}

function TestReturnRecord(): boolean {
    const actRecord = returnRecord.invoke();
    return (
        actRecord.getProperty('one').toNumber().toInt() == 1 &&
        actRecord.getProperty('two').toNumber().toInt() == 2 &&
        actRecord.getProperty('three').toNumber().toInt() == 3
    );
}

interface Shape {
    width: int
}

interface TwoDimensioned extends Shape {
    length: int
}

interface ThreeDimensioned extends Shape {
    length: int
    height: int
}

function TestReturnInterface(): boolean {
    const testInterface = returnInterface;
    const testTwoVal = testInterface.invoke(false);
    const testThreeVal = testInterface.invoke(true);

    return (testTwoVal.getProperty('width').toNumber().toInt() == 100 &&
        testTwoVal.getProperty('length').toNumber().toInt() == 40 &&
        testThreeVal.getProperty('width').toNumber().toInt() == 100 &&
        testThreeVal.getProperty('length').toNumber().toInt() == 50 &&
        testThreeVal.getProperty('height').toNumber().toInt() == 25);
}
