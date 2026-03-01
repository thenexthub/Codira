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

import LightWeightMap from "@ohos.util.LightWeightMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightMap ForEach ToString Iterator API tests")

    suite.addTest("Testing LightWeightMap use forEach and remove function", testForEachAndRemove)
    suite.addTest("Testing LightWeightMap use forEach to iterate over each item", testForEachIteration)
    suite.addTest("Testing LightWeightMap use forEach to iterate over each item for a duplicate value",
        testForEachWithDuplicateValues)
    suite.addTest("Testing LightWeightMap use forEach to iterate over each item for an empty LightWeightMap",
        testForEachOnEmptyMap)
    suite.addTest("Testing LightWeightMap use forEach to check ArgThis", testForEachCallbackParameters)
    suite.addTest("Testing LightWeightMap use forEach to check ArgThis", testForEachCallbackParamsNoSideEffect)
    suite.addTest("Testing LightWeightMap use $_iterator and remove function", testSymbolIteratorAndRemove)
    suite.addTest("Testing LightWeightMap use $_iterator to iterate over each item",
        testSymbolIteratorIteration)
    suite.addTest("Testing LightWeightMap use $_iterator to iterate over each item for contain duplicate value",
        testSymbolIteratorWithDuplicateValues)
    suite.addTest("Testing LightWeightMap use $_iterator to iterate over each item for an empty LightWeightMap",
        testSymbolIteratorOnEmptyMap)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_FOREACH_0100
function testForEachAndRemove() {
    let lightWeightMap = new LightWeightMap<number, number>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, 100 + i);
    }
    lightWeightMap.forEach((value: number, key: number) => {
        arktest.assertEQ(lightWeightMap.remove(key), value);
    });
    lightWeightMap.forEach((value: number, key: number) => {
        arktest.assertEQ(lightWeightMap.get(key), value);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_FOREACH_0200
function testForEachIteration() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    lightWeightMap.forEach((value: number, key: string) => {
        if (key === "sparrow") {
            arktest.assertEQ(value, 123);
        } else if (key === "gull") {
            arktest.assertEQ(value, 357);
        } else {
            arktest.assertFalse(lightWeightMap.hasKey(key),
                "Expected map not to have key in the second forEach (should be empty)");
            arktest.assertFalse(lightWeightMap.hasValue(value),
                "Expected map not to have value in the second forEach (should be empty)");
        }
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_FOREACH_0300
function testForEachWithDuplicateValues() {
    let data: number = 100;
    let lightWeightMap = new LightWeightMap<number, number>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, data);
    }
    arktest.assertEQ(lightWeightMap.length, 10);
    lightWeightMap.forEach((value: number) => {
        arktest.assertEQ(value, data);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_FOREACH_0400
function testForEachOnEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    let tmp = 0;
    lightWeightMap.forEach(() => {
        tmp++
    });
    arktest.assertEQ(tmp, 0);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_FOREACH_0500
function testForEachCallbackParameters() {
    let data: number = 100;
    let lightWeightMap = new LightWeightMap<number, number>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, data);
    }
    arktest.assertEQ(lightWeightMap.length, 10);
    lightWeightMap.forEach((value: number, index, lightWeightMap: LightWeightMap<number, number>) => {
        arktest.assertEQ(value, data);
        arktest.assertEQ(lightWeightMap.length, 10);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_FOREACH_0600
function testForEachCallbackParamsNoSideEffect() {
    let data: number = 100;
    let lightWeightMap = new LightWeightMap<number, number>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, data);
    }
    arktest.assertEQ(lightWeightMap.length, 10);
    lightWeightMap.forEach((value: number, index, lightWeightMap: LightWeightMap<number, number>) => {
        arktest.assertEQ(value, data);
        arktest.assertEQ(lightWeightMap.length, 10);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SYMBOLITERATOR_0100
function testSymbolIteratorAndRemove() {
    let lightWeightMap = new LightWeightMap<number, string>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, `${i}`);
    }
    let iter = lightWeightMap.$_iterator();
    let temp = iter.next();
    while (!temp.done) {
        arktest.assertEQ(lightWeightMap.remove(temp.value![0]), temp.value![1]);
        temp = iter.next();
    }
    iter = lightWeightMap.$_iterator();
    temp = iter.next();
    while (!temp.done) {
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SYMBOLITERATOR_0200
function testSymbolIteratorIteration() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let iter = lightWeightMap.$_iterator();
    let temp = iter.next();
    while (!temp.done) {
        if (temp.value![0] === "squirrel") {
            arktest.assertEQ(temp.value![1], 123);
        } else if (temp.value![0] === "sparrow") {
            arktest.assertEQ(temp.value![1], 356);
        } else {
            arktest.assertFalse(lightWeightMap.hasKey(temp.value![0]),
                "Expected map not to have unknown key temp.value[0]");
            arktest.assertFalse(lightWeightMap.hasValue(temp.value![1]),
                "Expected map not to have unknown value temp.value[1]");
        }
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SYMBOLITERATOR_0300
function testSymbolIteratorWithDuplicateValues() {
    const VALUE = 'VALUE';
    let lightWeightMap = new LightWeightMap<number, string>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, VALUE);
    }
    let iter = lightWeightMap.$_iterator();
    let temp = iter.next();
    while (!temp.done) {
        arktest.assertTrue(temp.value![0] < 10, "Expected temp.value[0] to be less than 10 when using Symbol.iterator");
        arktest.assertEQ(temp.value![1], VALUE);
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SYMBOLITERATOR_0400
function testSymbolIteratorOnEmptyMap() {

    let lightWeightMap = new LightWeightMap<string, number>();
    let iter = lightWeightMap.$_iterator();
    let temp = iter.next();
    arktest.assertTrue(temp.done,
        "Expected iterator to be done for an empty LightWeightMap when using Symbol.iterator");
}
