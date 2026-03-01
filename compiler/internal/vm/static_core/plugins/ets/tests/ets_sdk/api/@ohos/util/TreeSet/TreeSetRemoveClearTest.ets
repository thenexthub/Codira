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

import {TreeSet} from "@ohos.util.TreeSet";
import {TreeSetComparator} from "@ohos.util.TreeSet";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeSet Remove Clear API tests")

    suite.addTest("TreeSet remove value", testRemoveExistingValue)
    suite.addTest("TreeSet remove not set value", testRemoveNonExistingValue)
    suite.addTest("TreeSet add value between remove value", testAddValueBetweenRemoveValue)
    suite.addTest("TreeSet remove value from the null treeSet", testRemoveValueFromEmptyTreeSet)
    suite.addTest("TreeSet remove empty string value when treeSet is empty", testRemoveEmptyStringValueFromEmptyTreeSet)
    suite.addTest("TreeSet clear data", testClearData)
    suite.addTest("TreeSet clear the null treeSet", testClearEmptyTreeSet)
    suite.addTest("TreeSet add data after clear the TreeSet", testAddDataAfterClearTreeSet)

    return suite.run()
}

function testRemoveExistingValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result1 = treeSet.remove("aa");
    let result2 = treeSet.has("aa");
    arktest.assertEQ(result1, true, "The treeSet should remove the value");
    arktest.assertEQ(result2, false, "The treeSet should not has the value");
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testRemoveNonExistingValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result = treeSet.remove("bb");
    arktest.assertEQ(result, false, "The treeSet should return false for the value that is not set");
    arktest.assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function removeItem(treeSet: TreeSet<double>){
    const first = treeSet.getFirstValue()
    console.log(first)
    treeSet.remove(first!)
}

function testAddValueBetweenRemoveValue() {
    let treeSetCb: TreeSetComparator<double> = (first: double, second: double) =>{
        return second - first;
    };
    let treeSet = new TreeSet<double>(treeSetCb);
    const insertArr = [643, 811, 807, 378, 226, 195, 599, 641, 494, 964, 156, 419, 977, 20, 788, 596]
    for(let i: int = 0;i < insertArr.length;i++) {
        treeSet.add(insertArr[i]);
    }
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    treeSet.add(664);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    removeItem(treeSet);
    let resArr = [807, 811, 964, 977]
    let index = 0;
    treeSet.forEach((element) => {
        console.log(element);
        arktest.assertEQ(element, resArr[index], "The result should be match");
        index++;
    });
}

function testRemoveValueFromEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    let result = treeSet.remove("aa");
    arktest.assertEQ(result, false, "The treeSet should return undefined for a value in an empty set");
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testRemoveEmptyStringValueFromEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    let result = treeSet.remove("");
    arktest.assertEQ(result, false, "The treeSet should return undefined for a value in an empty set");
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testClearData() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.clear();
    let result1 = treeSet.has("aa");
    let result2 = treeSet.isEmpty();
    arktest.assertEQ(result1, false, "The treeSet should not has the value");
    arktest.assertEQ(result2, true, "The treeSet should be empty");
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testClearEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.clear();
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testAddDataAfterClearTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return firstValue.compareTo(secondValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.clear();
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
    treeSet.add("cc");
    treeSet.add("dd");
    treeSet.add("ee");
    arktest.assertEQ(treeSet.length, 3, "The treeSet length should be 3");
    arktest.assertEQ(treeSet.getFirstValue(), "ee", "The treeMap first value should be ee");
    arktest.assertEQ(treeSet.getLastValue(), "cc", "The treeMap last value should be cc");
}
