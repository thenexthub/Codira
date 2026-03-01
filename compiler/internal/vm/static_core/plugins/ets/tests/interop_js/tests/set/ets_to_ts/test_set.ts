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

const mySet = etsVm.getClass('Lset/test/ETSGLOBAL;').mySet;
const HasSetElement = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'HasSetElement');
const GetSetSize = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'GetSetSize');

function testSetAdd(): boolean {
    let res: boolean = true;
    mySet.add(4);
    mySet.add(5);
    res = mySet.has(1) && mySet.has(2) && mySet.has(3) && mySet.has(4) && mySet.has(5) &&
          HasSetElement(4) && HasSetElement(5);
    return res;
}

function testSetHas(): boolean {
    let res: boolean = true;
    res = mySet.has(1) && mySet.has(2) && mySet.has(3) &&
          mySet.has(4) && mySet.has(5) && !mySet.has(999);
    return res;
}

function testSetDelete(): boolean {
    let res: boolean = true;
    mySet.delete(4);
    res = !mySet.has(4) && !HasSetElement(4);
    return res;
}

function testSetSize(): boolean {
    let res: boolean = true;
    mySet.add(4);
    res = mySet.size === 5 && GetSetSize() === 5;
    return res;
}

function testSetValues(): boolean {
    let res: boolean = true;
    let values: number[] = [];
    for (let value of mySet.values()) {
        values.push(value);
    }
    res = values[0] === 1 && values[1] === 2 &&
          values[2] === 3 && values[3] === 5 &&
          values[4] === 4 && values.length === 5;
    return res;
}

function testSetKeys(): boolean {
    let res: boolean = true;
    let keys: number[] = [];
    mySet.add(6);
    for (let key of mySet.keys()) {
        keys.push(key);
    }
    res = keys.includes(1) && keys.includes(2) && keys.includes(3) &&
          keys.includes(5) && keys.includes(4) && keys.includes(6) &&
          keys.length === 6;
    return res;
}

function testSetEntries(): boolean {
    let res: boolean = true;
    mySet.entries();
    return res;
}

function testSetForEach(): boolean {
    let res: boolean = true;
    let items: number[] = [];
    mySet.forEach((value) => {
        items.push(value);
    });
    res = items.includes(1) && items.includes(2) && items.includes(3) &&
          items.includes(4) && items.includes(5) && items.includes(6) &&
          items.length === 6;
    return res;
}

function testSetClear(): boolean {
    let res: boolean = true;
    mySet.clear();
    res = mySet.size === 0 && GetSetSize() === 0;
    return res;
}


ASSERT_TRUE(testSetAdd());
ASSERT_TRUE(testSetHas());
ASSERT_TRUE(testSetDelete());
ASSERT_TRUE(testSetSize());
ASSERT_TRUE(testSetValues());
ASSERT_TRUE(testSetKeys());
ASSERT_TRUE(testSetEntries());
ASSERT_TRUE(testSetForEach());
ASSERT_TRUE(testSetClear());