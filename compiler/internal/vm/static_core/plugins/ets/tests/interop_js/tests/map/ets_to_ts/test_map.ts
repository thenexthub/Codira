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

const etsVm = globalThis.gtest.etsVm;

const myMap = etsVm.getClass('Lmap/test/ETSGLOBAL;').myMap;
const GetMapElement = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'GetMapElement');
const HasMapElement = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'HasMapElement');
const GetMapSize = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'GetMapSize');
const WeekDays = etsVm.getClass('Lmap/test/ETSGLOBAL;').WeekDays;

function testMapSet(): boolean {
    let res: boolean = true;
    myMap.set('banana', 2);
    myMap.set('cherry', 3);
    res = myMap.get('apple') === 1 && myMap.get('banana') === 2 && myMap.get('cherry') === 3 &&
        GetMapElement('banana') === 2 && GetMapElement('cherry') === 3;
    return res;
}

function testMapGet(): boolean {
    let res: boolean = true;
    res = myMap.get('apple') === 1 && myMap.get('banana') === 2 &&
        myMap.get('cherry') === 3 && myMap.get('unknown') === undefined;
    return res;
}

function testMapHas(): boolean {
    let res: boolean = true;
    res = myMap.has('apple') && myMap.has('banana') &&
        myMap.has('cherry') && !myMap.has('unknown');
    return res;
}

function testMapDelete(): boolean {
    let res: boolean = true;
    myMap.delete('banana')
    res = !myMap.has('banana') && !HasMapElement('banana');
    return res;
}

function testMapSize(): boolean {
    let res: boolean = true;
    myMap.set('banana', 2);
    res = myMap.size === 3 && GetMapSize() === 3;
    return res;
}

function testMapKeys(): boolean {
    let res: boolean = true;
    let keys: string[] = [];
    myMap.set('banana', 2);
    for (let key of myMap.keys()) {
        keys.push(key);
    }
    res = keys[0] === 'apple' && keys[1] === 'cherry' &&
        keys[2] === 'banana' && keys.length === 3;
    return res;
}

function testMapValues(): boolean {
    let res: boolean = true;
    let values: number[] = [];
    for (let value of myMap.values()) {
        values.push(value);
    }
    res = values[0] === 1 && values[1] === 3 &&
        values[2] === 2 && values.length === 3;
    return res;
}

function testMapEntries(): boolean {
    // Will the static Tuple class
    let WeekDaysStatic: Map<string, number> = new Map<string, number>();
    WeekDaysStatic.set('Monday', 1);
    WeekDaysStatic.set('Tuesday', 2);
    WeekDaysStatic.set('Wednesday', 3);
    WeekDaysStatic.set('Thursday', 4);
    WeekDaysStatic.set('Friday', 5);
    WeekDaysStatic.set('Saturday', 6);
    WeekDaysStatic.set('Sunday', 7);
    for (let day of WeekDays.entries()) {
        ASSERT_TRUE(WeekDaysStatic.has(day[0]) && WeekDaysStatic.get(day[0]) === day[1]);
    }
    return true;
}

function testMapForEach(): boolean {
    let res: boolean = true;
    let keys: string[] = [];
    let values: number[] = [];
    myMap.forEach((val, key) => {
        keys.push(key);
        values.push(val);
    });
    res = keys[0] === 'apple' && keys[1] === 'cherry' && keys[2] === 'banana' && keys.length === 3 &&
        values[0] === 1 && values[1] === 3 && values[2] === 2 && values.length === 3;
    return res;
}

function testMapClear(): boolean {
    let res: boolean = true;
    myMap.clear();
    res = myMap.size === 0 && GetMapSize() === 0;
    return res;
}


ASSERT_TRUE(testMapSet());
ASSERT_TRUE(testMapGet());
ASSERT_TRUE(testMapHas());
ASSERT_TRUE(testMapDelete());
ASSERT_TRUE(testMapSize());
ASSERT_TRUE(testMapKeys());
ASSERT_TRUE(testMapValues());
ASSERT_TRUE(testMapEntries());
ASSERT_TRUE(testMapForEach());
ASSERT_TRUE(testMapClear());