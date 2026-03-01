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

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("Testing getLastIndexOf with duplicate data", testGetLastIndexOfWithDuplicates);
    suite.addTest("Testing getLastIndexOf with add, removeByIndex, and validation", testGetLastIndexOfWithAddRemove);
    suite.addTest("Testing getLastIndexOf with removeByIndex and re-add", testGetLastIndexOfWithReAdd);
    suite.addTest("Testing getLastIndexOf with remove and re-add", testGetLastIndexOfWithRemoveReAdd);
    suite.addTest("Testing getIndexOf with duplicate data", testGetIndexOfWithDuplicates);
    suite.addTest("Testing getIndexOf with add, removeByIndex, and validation", testGetIndexOfWithAddRemove);
    suite.addTest("Testing getIndexOf with remove and re-add", testGetIndexOfWithRemoveReAdd);
    suite.addTest("Testing getIndexOf with removeByIndex and re-add", testGetIndexOfWithReAdd);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LIST_GETLASTINDEXOF_0100
function testGetLastIndexOfWithDuplicates() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    let index1 = list.getLastIndexOf(2);
    list.add(2);
    let index2 = list.getLastIndexOf(2);
    list.add(2);
    let index3 = list.getLastIndexOf(2);
    arktest.assertNE(index1, index2);
    arktest.assertNE(index2, index3);
}

// Test cases ported from ArkTS 1.0: LIST_GETLASTINDEXOF_0200
function testGetLastIndexOfWithAddRemove() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(2);
    list.add(3);
    list.add(3);
    let index101 = list.removeByIndex(list.getLastIndexOf(2));
    let index102 = list.removeByIndex(list.getLastIndexOf(2));
    arktest.assertEQ(index101, index102);
    let index103 = list.removeByIndex(list.getLastIndexOf(3));
    let index104 = list.removeByIndex(list.getLastIndexOf(3));
    arktest.assertEQ(index103, index104);
}

// Test cases ported from ArkTS 1.0: LIST_GETLASTINDEXOF_0300
function testGetLastIndexOfWithReAdd() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(3);
    list.add(5);
    list.add(5);
    let lastIndex = list.getLastIndexOf(5);
    let removeParam = list.removeByIndex(lastIndex);
    list.add(removeParam!);
    arktest.assertEQ(list.getLastIndexOf(removeParam!), lastIndex);
}

// Test cases ported from ArkTS 1.0: LIST_GETLASTINDEXOF_0400
function testGetLastIndexOfWithRemoveReAdd() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(3);
    list.add(4);
    list.add(5);
    let param: double = 5;
    list.remove(param);
    list.add(param);
    arktest.assertEQ(list.getLastIndexOf(param), list.length - 1);
}

// Test cases ported from ArkTS 1.0: LIST_GETINDEXOF_0100
function testGetIndexOfWithDuplicates() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    let index1 = list.getIndexOf(2);
    list.add(2);
    let index2 = list.getIndexOf(2);
    list.add(2);
    let index3 = list.getIndexOf(2);
    arktest.assertEQ(index1, index2);
    arktest.assertEQ(index2, index3);
}

// Test cases ported from ArkTS 1.0: LIST_GETINDEXOF_0200
function testGetIndexOfWithAddRemove() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(2);
    list.add(3);
    list.add(3);
    let index101 = list.removeByIndex(list.getIndexOf(2));
    let index102 = list.removeByIndex(list.getIndexOf(2));
    arktest.assertEQ(index101, index102);
    let index103 = list.removeByIndex(list.getIndexOf(3));
    let index104 = list.removeByIndex(list.getIndexOf(3));
    arktest.assertEQ(index103, index104);
}

// Test cases ported from ArkTS 1.0: LIST_GETINDEXOF_0300
function testGetIndexOfWithRemoveReAdd() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(3);
    list.add(4);
    list.add(5);
    let param: double = 5;
    list.remove(param);
    list.add(param);
    arktest.assertEQ(list.getIndexOf(param), list.length - 1);
}

// Test cases ported from ArkTS 1.0: LIST_GETINDEXOF_0400
function testGetIndexOfWithReAdd() {
    let list = new List<double>();
    list.add(1);
    list.add(2);
    list.add(3);
    list.add(4);
    list.add(5);
    let removeParam = list.removeByIndex(list.length - 1);
    list.add(removeParam!);
    arktest.assertEQ(list.getIndexOf(removeParam!), list.length - 1);
}
