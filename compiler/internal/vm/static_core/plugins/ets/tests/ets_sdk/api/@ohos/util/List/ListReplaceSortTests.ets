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
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

class Temp {
    age: double = 0
    name: string = ""
}

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("Testing List replaceAllElement insert new string", testReplaceAllElementsWithNewString);
    suite.addTest("Testing List getIndexOf add->clear->add->insert->sort(a-b)", testSortAscendingOrder);
    suite.addTest("Testing List getIndexOf add->clear->add->insert->sort(b-a)", testSortDescendingOrder);
    suite.addTest("Testing List getIndexOf add->clear->add->insert->sort(a-b)->getSubList", testSortAndGetSubList);
    suite.addTest("Testing List getIndexOf add->clear->add->insert->sort(a-b)->getSubList->add",
        testSortGetSubListAndAdd);
    suite.addTest("Testing List JSON stringify and sort(a-b)", testListJsonStringifySortAsc);
    suite.addTest("Testing List JSON stringify and sort(b-a)", testListJsonStringifySortDes);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LIST_REPLACEALLELEMENT_0100
function testReplaceAllElementsWithNewString() {
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
    let array = [0, 1, 2, undefined];
    list.add(array);
    list.add("");
    list.add("`");
    list.add("%");
    list.add(`"`);
    list.replaceAllElements((value, index, tempList) => {
        let length = tempList ? tempList.length - 1 : 0
        if (index === length) {
            let tmp = value as string;
            value = tmp + `add last index`;
        }
        return value;
    })
    let index = list.getLast() as string;
    arktest.assertEQ(index, `"add last index`);
}

// Test cases ported from ArkTS 1.0: LIST_SORT_0100
function testSortAscendingOrder() {
    let list = new List<double>();
    list.add(1);
    arktest.assertFalse(list.isEmpty(), "The list should not be empty after adding elements");
    list.clear();
    arktest.assertTrue(list.isEmpty(), "The list should be empty after clearing all elements");
    list.add(4);
    list.add(1);
    list.insert(2, 0);
    list.insert(10, 0);
    list.insert(8, 0);
    list.sort((first, second) => first - second);
    list.add(3);
    arktest.assertEQ(list.get(0), 1);
    arktest.assertEQ(list.get(1), 2);
    arktest.assertEQ(list.get(2), 4);
    arktest.assertEQ(list.get(3), 8);
    arktest.assertEQ(list.get(4), 10);
    arktest.assertEQ(list.get(5), 3);
}

// Test cases ported from ArkTS 1.0: LIST_SORT_0200
function testSortDescendingOrder() {
    let list = new List<double>();
    list.add(1);
    arktest.assertFalse(list.isEmpty(), "The list should not be empty after adding elements");
    list.clear();
    arktest.assertTrue(list.isEmpty(), "The list should be empty after clearing all elements");
    list.add(4);
    list.add(1);
    list.insert(2, 0);
    list.insert(10, 0);
    list.insert(8, 0);
    list.sort((first, second) => second - first);
    list.add(3);
    arktest.assertEQ(list.get(0), 10);
    arktest.assertEQ(list.get(1), 8);
    arktest.assertEQ(list.get(2), 4);
    arktest.assertEQ(list.get(3), 2);
    arktest.assertEQ(list.get(4), 1);
    arktest.assertEQ(list.get(5), 3);
}

// Test cases ported from ArkTS 1.0: LIST_SORT_0300
function testSortAndGetSubList() {
    let list = new List<double>();
    list.add(1);
    arktest.assertFalse(list.isEmpty(), "The list should not be empty after adding elements");
    list.clear();
    arktest.assertTrue(list.isEmpty(), "The list should be empty after clearing all elements");
    list.add(4);
    list.add(1);
    list.insert(2, 0);
    list.insert(10, 0);
    list.insert(8, 0);
    list.sort((first, second) => first - second);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"fromIndex\" is out of range. It must be >= 0 && <= 0. Received value is: 5" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {list.getSubList(5, 1)}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: LIST_SORT_0400
function testSortGetSubListAndAdd() {
    let list = new List<double>();
    list.add(1);
    arktest.assertFalse(list.isEmpty(), "The list should not be empty after adding elements");
    list.clear();
    arktest.assertTrue(list.isEmpty(), "The list should be empty after clearing all elements");
    list.add(4);
    list.add(1);
    list.insert(2, 0);
    list.insert(10, 0);
    list.insert(8, 0);
    list.sort((first, second) => first - second);
    list.getSubList(1, 5);
    list.add(3);
    arktest.assertEQ(list.get(0), 1);
    arktest.assertEQ(list.get(1), 2);
    arktest.assertEQ(list.get(2), 4);
    arktest.assertEQ(list.get(3), 8);
    arktest.assertEQ(list.get(4), 10);
    arktest.assertEQ(list.get(5), 3);
}

function testListJsonStringifySortAsc() {
    let list = new List<number>();
    list.add(1);
    arktest.assertFalse(list.isEmpty(), "The list should not be empty after adding elements");
    list.clear();
    arktest.assertTrue(list.isEmpty(), "The list should be empty after clearing all elements");
    list.add(4);
    list.add(1);
    list.insert(2, 0);
    list.insert(10, 0);
    list.insert(8, 0);
    list.sort((first, second) => first - second);
    list.add(3);
    let result = `{"0":1,"1":2,"2":4,"3":8,"4":10,"5":3}`;
    arktest.assertEQ(JSON.stringify(list), result);
}

function testListJsonStringifySortDes() {
    let list = new List<number>();
    list.add(1);
    arktest.assertFalse(list.isEmpty(), "The list should not be empty after adding elements");
    list.clear();
    arktest.assertTrue(list.isEmpty(), "The list should be empty after clearing all elements");
    list.add(4);
    list.add(1);
    list.insert(2, 0);
    list.insert(10, 0);
    list.insert(8, 0);
    list.sort((first, second) => second - first);
    list.add(3);
    let result = `{"0":10,"1":8,"2":4,"3":2,"4":1,"5":3}`;
    arktest.assertEQ(JSON.stringify(list), result);
}
