/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {LightWeightSet} from "@ohos.util.LightWeightSet";

//Test cases ported from ArkTS 1.0:container_lightweightset.js
function main() {
    const suite = new arktest.ArkTestsuite("LightWeightSet API tests");

    suite.addTest("Test LightWeightSet comprehensive functions.", testLightWeightSetComprehensiveFunctionality);
    suite.addTest("Test LightWeightSet toArray() snapshot behavior and array length after additions to the set.",
        testToArraySnapshotBehaviorAndArrayLengthAfterAdditions);
    suite.addTest("Test LightWeightSet getValueAt() and removeAt() with floored non-integer indices.",
        testGetValueAtRemoveAtWithFlooredNonIntegerIndices);
    suite.addTest("Test LightWeightSet entries() iterator and for...of iteration behavior.",
        testEntriesIteratorAndForOfIterationBehavior);
    suite.addTest("Test LightWeightSet entries() iterator, verifying key equals value for each entry.",
        testEntriesVerifyingKeyEqualsValueForEachEntry);
    suite.addTest('Test LightWeightSet increaseCapacityTo() Error.',
        testLightweightincreaseCapacityToError);

    return suite.run();
}

function testLightWeightSetComprehensiveFunctionality() {
    let lightWeightSet = new LightWeightSet<double>();
    arktest.assertTrue(lightWeightSet.isEmpty(), "Expected new LightWeightSet to be empty");
    lightWeightSet.add(1);
    lightWeightSet.add(2);
    lightWeightSet.add(3);
    arktest.assertTrue(lightWeightSet.has(1), "Expected set to contain 1 after adding it");
    arktest.assertTrue(lightWeightSet.has(2), "Expected set to contain 2 after adding it");
    arktest.assertTrue(lightWeightSet.has(3), "Expected set to contain 3 after adding it");
    arktest.assertFalse(lightWeightSet.has(4), "Expected set not to contain 4 as it was not added");

    let iteratorValues1 = lightWeightSet.values();
    arktest.assertEQ(iteratorValues1.next().value, 1);
    arktest.assertEQ(iteratorValues1.next().value, 2);
    arktest.assertEQ(iteratorValues1.next().value, 3);
    arktest.assertEQ(iteratorValues1.next().value, undefined);

    let itemArr = [1, 2, 3]
    let item: double = 0;
    let i: int = 0;
    for (item of lightWeightSet) {
        arktest.assertEQ(item, itemArr[i]);
        i++;
    }

    lightWeightSet.forEach((value, key, lightWeightSet) => {
        arktest.assertTrue(lightWeightSet.has(key), "Expected set to have the key during forEach iteration");
        arktest.assertTrue(lightWeightSet.has(value), "Expected set to have the value during forEach iteration");
    });

    let dlightWeightSet = new LightWeightSet<double>();
    dlightWeightSet.add(4);
    dlightWeightSet.add(5);
    dlightWeightSet.add(6);
    dlightWeightSet.add(7);
    dlightWeightSet.add(8);
    dlightWeightSet.add(9);
    arktest.assertTrue(dlightWeightSet.addAll(lightWeightSet),
        "Expected addAll to return true when elements are added");
    arktest.assertTrue(dlightWeightSet.hasAll(lightWeightSet),
        "Expected hasAll to be true after adding all elements from another set");
    arktest.assertEQ(dlightWeightSet.remove(1), 1);
    arktest.assertEQ(dlightWeightSet.length, 8);
    arktest.assertTrue(dlightWeightSet.removeAt(3), "Expected removeAt(3) to return true for a valid index");
    arktest.assertEQ(dlightWeightSet.length, 7);
    arktest.assertEQ(dlightWeightSet.getValueAt(3), 6);
    arktest.assertEQ(dlightWeightSet.getIndexOf(2), 0);
    arktest.assertEQ(dlightWeightSet.toString(), "2,3,4,6,7,8,9");
    let arr = dlightWeightSet.toArray()
    dlightWeightSet.increaseCapacityTo(20)
    dlightWeightSet.clear();
    arktest.assertEQ(dlightWeightSet.length, 0);
}

function testToArraySnapshotBehaviorAndArrayLengthAfterAdditions() {
    let LOOP_COUNT = 5;
    let myTest = new LightWeightSet<double>();

    for (let i: int = 0; i < LOOP_COUNT; i++) {
        myTest.add(i);
    }

    let a = myTest.toArray();
    arktest.assertEQ(a.length, LOOP_COUNT);
    myTest.add(10);
    arktest.assertEQ(a.length, LOOP_COUNT);
    let b = myTest.toArray();
    arktest.assertEQ(b.length, LOOP_COUNT + 1);
}

function testGetValueAtRemoveAtWithFlooredNonIntegerIndices() {
    let myLs = new LightWeightSet<string>();
    myLs.add("a");
    myLs.add("b");
    arktest.assertEQ(myLs.getValueAt(Math.floor(1.5).toInt()), "b");
    arktest.assertTrue(myLs.removeAt(Math.floor(1.5).toInt()), "Expected removeAt(1) to return true");
}

function testEntriesIteratorAndForOfIterationBehavior() {
    let lightWeightSet = new LightWeightSet<double>();

    lightWeightSet.add(1);
    lightWeightSet.add(2);
    lightWeightSet.add(3);

    let iteratorEntries1 = lightWeightSet.entries();
    iteratorEntries1.next().value;
    iteratorEntries1.next().value;
    arktest.assertNE(iteratorEntries1.next().value, undefined);
    arktest.assertEQ(iteratorEntries1.next().value, undefined);

    let arr1 = [1, 2, 3];
    let j = 0;
    for (const item of lightWeightSet) {
        arktest.assertEQ(item, arr1[j]);
        j++;
    }
}

function testEntriesVerifyingKeyEqualsValueForEachEntry() {
    let lightWeightSeten = new LightWeightSet<double>();
    lightWeightSeten.add(1);
    lightWeightSeten.add(2);
    lightWeightSeten.add(3);
    lightWeightSeten.add(4);
    lightWeightSeten.add(5);
    let iter = lightWeightSeten.entries();
    let temp = iter.next();
    while(!temp.done) {
        arktest.assertEQ(temp.value![0], temp.value![1]);
        temp = iter.next();
    }
}
function testLightweightincreaseCapacityToError() {
    let lightWeightSet = new LightWeightSet<number>();
    lightWeightSet.add(1);
    lightWeightSet.add(2);
    lightWeightSet.add(3);
    lightWeightSet.add(4);
    lightWeightSet.add(5);
    arktest.expectThrow(
        () => lightWeightSet.increaseCapacityTo(3),
        (err: Error) => {
            arktest.assertEQ(err.name, 'BusinessError')
            arktest.assertEQ(err.code, 10200001)
            arktest.assertEQ(err.message, `The value of "minimumCapacity" is out of range. It must be > 8. Received value is: 3`)
            return (err instanceof Error)
        }
    );
}