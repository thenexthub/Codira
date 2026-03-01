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

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("Testing List insert and validate with getIndexOf, removeByIndex, and has",
        testInsertAndValidateWithGetIndexOf);
    suite.addTest("Testing List insert, add, and validate with getLastIndexOf, removeByIndex, and has",
        testInsertAddAndValidateWithGetLastIndexOf);
    suite.addTest("Testing List has method with various data types", testHasMethodWithVariousDataTypes);
    suite.addTest("Testing List get method with various data types", testGetMethodWithVariousDataTypes);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LIST_INSERT_0100
function testInsertAndValidateWithGetIndexOf() {
    let list = new List<double | string>();
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    list.insert(1, 0);
    list.insert(2, 1);
    list.insert(param, 0);
    let index = list.getIndexOf(param);
    arktest.assertEQ(index, 0);
    let removeResult = list.removeByIndex(index);
    arktest.assertFalse(list.has(param), "The list should not contain 'param' after it is removed");
}

// Test cases ported from ArkTS 1.0: LIST_INSERT_0200
function testInsertAddAndValidateWithGetLastIndexOf() {
    let list = new List<double | string>();
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    list.insert(1, 0);
    list.insert(2, 1);
    list.add(param);
    let index = list.getLastIndexOf(param);
    arktest.assertEQ(index, list.length - 1);
    let removeResult = list.removeByIndex(index);
    arktest.assertFalse(list.has(param), "The list should not contain 'param' after it is removed");
}

// Test cases ported from ArkTS 1.0: LIST_HAS_0100
function testHasMethodWithVariousDataTypes() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    list.add(temp);
    list.add(undefined);
    list.add(null);
    list.add(true);
    let arr = [0, 1, 2, undefined];
    list.add(arr);
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    list.add(param);
    arktest.assertTrue(list.has("zhang san"), "The list should contain the string 'zhang san'");
    arktest.assertTrue(list.has(1), "The list should contain the double 1");
    arktest.assertTrue(list.has(temp), "The list should contain the Temp object");
    arktest.assertTrue(list.has(undefined), "The list should contain 'undefined'");
    arktest.assertTrue(list.has(null), "The list should contain 'null'");
    arktest.assertTrue(list.has(true), "The list should contain 'true'");
    arktest.assertTrue(list.has(arr), "The list should contain the array '[0, 1, 2, undefined]'");
    arktest.assertTrue(list.has(param), "The list should contain the parameter 'param'");
}

// Test cases ported from ArkTS 1.0: LIST_GET_0100
function testGetMethodWithVariousDataTypes() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    list.add(temp);
    list.add(undefined);
    list.add(null);
    list.add(true);
    let arr = [0, 1, 2, undefined];
    list.add(arr);
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    list.add(param);
    list.forEach((value, index) => {
        arktest.assertEQ(list.get(index), value);
    })
}
