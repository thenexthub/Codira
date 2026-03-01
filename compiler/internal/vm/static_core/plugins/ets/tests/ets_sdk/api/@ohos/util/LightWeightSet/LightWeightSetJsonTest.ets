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

import {LightWeightSet} from "@ohos.util.LightWeightSet";

function main() {
    const suite = new arktest.ArkTestsuite("LightWeightSet API tests");

    suite.addTest("Test JSON after forEach remove elements.", testLightWeightSetJsonForEachRemove);
    suite.addTest("Test JSON after forEach traversal output.", testLightWeightSetJsonForEachTraversal);
    suite.addTest("Test JSON after forEach traversal on empty set.", testLightWeightSetJsonForEachEmptySet);
    suite.addTest("Test JSON for entries iterator with multiple elements.", testLightWeightSetJsonEntriesMultiElement);
    suite.addTest("Test JSON for entries iterator with duplicate elements.",
        testLightWeightSetJsonEntriesDuplicateElement);
    suite.addTest("Test JSON for entries iterator with empty set.", testLightWeightSetJsonEntriesEmptySet);
    suite.addTest("Test JSON for iterator with duplicate elements.", testLightWeightSetJsonIteratorDuplicateElement);
    suite.addTest("Test JSON for iterator traversal with large number set.",
        testLightWeightSetJsonIteratorLargeNumberSet);

    return suite.run();
}

function testLightWeightSetJsonForEachRemove() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("sparrow");
    lightWeightSet.add("gull");
    lightWeightSet.forEach((item: string, index: string) => {
        lightWeightSet.remove(item);
    });
    arktest.assertEQ(JSON.stringify(lightWeightSet), '{"0":"sparrow"}');
}

function testLightWeightSetJsonForEachTraversal() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("sparrow");
    lightWeightSet.add("gull");
    lightWeightSet.forEach((value: string, key: string) => {});
    arktest.assertEQ(JSON.stringify(lightWeightSet), '{"0":"gull","1":"sparrow"}');
}

function testLightWeightSetJsonForEachEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.forEach((value: string, key: string) => {});
    arktest.assertEQ(JSON.stringify(lightWeightSet), '{}');
}

function testLightWeightSetJsonEntriesMultiElement() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let iter = lightWeightSet.entries();
    let expectedValue = ['["sparrow","sparrow"]', '["squirrel","squirrel"]'];
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(JSON.stringify(iter.next().value), expectedValue[index]);
        index++;
    }
}

function testLightWeightSetJsonEntriesDuplicateElement() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("squirrel");
    let iter = lightWeightSet.entries();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(JSON.stringify(iter.next().value), '["squirrel","squirrel"]');
        index++;
    }
}

function testLightWeightSetJsonEntriesEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let iter = lightWeightSet.entries();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(JSON.stringify(iter.next().value), 'iter.next');
        index++;
    }
}

function testLightWeightSetJsonIteratorDuplicateElement() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("squirrel");
    let iter = lightWeightSet.$_iterator().next();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(JSON.stringify(iter.value), '"squirrel"');
        index++;
    }
}

function testLightWeightSetJsonIteratorLargeNumberSet() {
    let lightWeightSet = new LightWeightSet<number>();
    let expectedValue = new Array<string>();
    for (let index = 0; index < 1024; index++) {
        expectedValue.push(index.toString());
        lightWeightSet.add(index++);
    }
    let iter = lightWeightSet.values();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(JSON.stringify(iter.next().value), expectedValue[index]);
        index++;
    }
}
