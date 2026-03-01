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

    suite.addTest("Testing List Symbol.iterator with for-of loop", testListSymbolIteratorForOf);
    suite.addTest("Testing List Symbol.iterator with next method", testListSymbolIteratorNext);
    suite.addTest("Testing List forEach with value and index", testListForEachValueAndIndex);
    suite.addTest("Testing List forEach with value-only iteration", testListForEachValueOnly);
    suite.addTest("Testing List forEach with value and index validation", testListForEachValueIndexValidation);
    suite.addTest("Testing List forEach with value, index, and list validation", testListForEachFullValidation);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LIST_SYMBOL_ITERATOR_0100
function testListSymbolIteratorForOf() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(3);
    list.add(4);
    let iter = list.$_iterator();
    for (let tmp of iter) {
        arktest.assertTrue(list.has(tmp), `The list should contain the element '${tmp}'`);
    }
}

// Test cases ported from ArkTS 1.0: LIST_SYMBOL_ITERATOR_0200
function testListSymbolIteratorNext() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(3);
    list.add(4);
    let iter = list.$_iterator();
    let tmp = iter.next();
    while (!tmp.done) {
        if (tmp.value) {
            arktest.assertTrue(list.has(tmp.value!), `The list should contain the element '${tmp.value}'`);
        }
        tmp = iter.next();
    }
}

// Test cases ported from ArkTS 1.0: LIST_FOREACH_0100
function testListForEachValueAndIndex() {
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
    list.forEach((value, index) => {
        arktest.assertEQ(list.get(index), value);
    });
}

// Test cases ported from ArkTS 1.0: LIST_FOREACH_0200
function testListForEachValueOnly() {
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
    let index = 0;
    list.forEach((value) => {
        arktest.assertEQ(list.get(index), value);
        index++;
    });
    arktest.assertEQ(list.length, index);
}

// Test cases ported from ArkTS 1.0: LIST_FOREACH_0300
function testListForEachValueIndexValidation() {
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
    let index = 0;
    list.forEach((value) => {
        arktest.assertEQ(list.get(index), value);
        index++;
    });
}

// Test cases ported from ArkTS 1.0: LIST_FOREACH_0400
function testListForEachFullValidation() {
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
    list.forEach((value, index, list: List<double | string | boolean | undefined | null | Object>) => {
        arktest.assertEQ(list.get(index), value);
    });
}
