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

import {List} from '@ohos.util.List';

class Temp {
    age: double = 0
    name: string = ""
}

class Temp1 {
    age: double = 0
    name: string = ""
    constructor(age: double, name: string) {
        this.age = age;
        this.name = name;
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("Testing List set list and update data", testSetAndUpdateData);
    suite.addTest("Testing List set getIndexOf->set->removeByIndex", testSetWithGetIndexOfAndRemoveByIndex);
    suite.addTest("Testing List set getIndexOf->set->has", testSetWithGetIndexOfAndHas);
    suite.addTest("Testing List set set->convertToArray->instanceof", testSetAndConvertToArray);
    suite.addTest("Testing List set set->getFirst", testSetAndGetFirst);
    suite.addTest("Testing List set set->getLast", testSetAndGetLast);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LIST_SET_0100
function testSetAndUpdateData() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp = {
        age: 10, name: "zhang"
    };
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.add([0, 1, 2, 4]);
    list.forEach((value, index: int) => {
        arktest.assertNE(list.set(index, "li si"), undefined);
    })
}

// Test cases ported from ArkTS 1.0: LIST_SET_0200
function testSetWithGetIndexOfAndRemoveByIndex() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp = {
        age: 10, name: "zhang"
    };
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.add([0, 1, 2, 4]);
    let index = list.getIndexOf(param);
    let setParam = list.set(index, "li si");
    let removeParam = list.removeByIndex(index);
    arktest.assertEQ(setParam, removeParam);
}

// Test cases ported from ArkTS 1.0: LIST_SET_0300
function testSetWithGetIndexOfAndHas() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp = {
        age: 10, name: "zhang"
    };
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.add([0, 1, 2, 4]);
    let index = list.getIndexOf(param);
    let setParam = list.set(index, "li si");
    arktest.assertTrue(list.has(setParam), "The list should contain the element set by the set method");
}

// Test cases ported from ArkTS 1.0: LIST_SET_0400
function testSetAndConvertToArray() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.add([0, 1, 2, 4]);
    list.forEach((value, index) => {
        list.set(index, new Temp1(index, "zhang " + index));
    })
    let array = list.convertToArray();
    array.forEach((value) => {
        arktest.assertTrue(value instanceof Temp1, "Each element in the converted array should be an instance of Temp1");
    })
}

// Test cases ported from ArkTS 1.0: LIST_SET_0500
function testSetAndGetFirst() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.add([0, 1, 2, 4]);
    let setParam = list.set(0, "li si");
    let firstParam = list.getFirst();
    arktest.assertEQ(setParam, firstParam);
}

// Test cases ported from ArkTS 1.0: LIST_SET_0600
function testSetAndGetLast() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.add([0, 1, 2, 4]);
    let setParam = list.set(list.length - 1, "li si");
    let lastParam = list.getLast();
    arktest.assertEQ(setParam, lastParam);
}
