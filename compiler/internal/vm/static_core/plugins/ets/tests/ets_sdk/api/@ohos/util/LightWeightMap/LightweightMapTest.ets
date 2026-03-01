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
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

//Test cases ported from ArkTS 1.0:container_lightweightmap.js
function main() {
    const suite = new arktest.ArkTestsuite("LightWeightMap API tests");

    suite.addTest("Test LightWeightMap general functionalities", testGeneralFunctionalities);
    suite.addTest("Test LightWeightMap exception handling for empty map", testEmptyMapExceptionHandling);
    suite.addTest("Test LightWeightMap index access with floored numbers", testIndexAccessWithFlooredNumbers);
    suite.addTest("Test LightWeightMap entries iterator and for...of iteration", testEntriesIteratorAndForOf);

    return suite.run();
}

function testGeneralFunctionalities() {
    let map = new LightWeightMap<string, string>();
    arktest.assertTrue(map.isEmpty(), "Expected isEmpty() to be true for a new LightWeightMap");
    map.set("a", "aa");
    map.set("b", "bb");

    arktest.assertEQ(map.length, 2);
    arktest.assertEQ(map.get("a"), "aa");
    arktest.assertEQ(map.get("b"), "bb");
    arktest.assertTrue(map.hasKey("a"), "Expected hasKey('a') to be true");
    arktest.assertTrue(map.hasKey("b"), "Expected hasKey('b') to be true");
    arktest.assertTrue(map.hasValue("aa"), "Expected hasValue('aa') to be true");
    arktest.assertTrue(map.hasValue("bb"), "Expected hasValue('bb') to be true");
    arktest.assertFalse(map.hasKey("c"), "Expected hasKey('c') to be false before adding it");
    arktest.assertFalse(map.hasValue("cc"), "Expected hasValue('cc') to be false before adding it");

    map.set("c", "cc");
    arktest.assertEQ(map.getIndexOfKey("a"), 0);
    arktest.assertEQ(map.getIndexOfValue("bb"), 1);
    arktest.assertEQ(map.getKeyAt(1), "b");
    arktest.assertEQ(map.getKeyAt(Math.floor(1.3).toInt()), "b");
    let iteratorKey1 = map.keys();
    arktest.assertEQ(iteratorKey1.next().value, "a");
    arktest.assertEQ(iteratorKey1.next().value, "b");
    arktest.assertEQ(iteratorKey1.next().value, "c");
    arktest.assertEQ(iteratorKey1.next().value, undefined);
    let iteratorValues1 = map.values();
    arktest.assertEQ(iteratorValues1.next().value, "aa");
    arktest.assertEQ(iteratorValues1.next().value, "bb");
    arktest.assertEQ(iteratorValues1.next().value, "cc");
    arktest.assertEQ(iteratorValues1.next().value, undefined);

    map.forEach((value, key, newMap: LightWeightMap<string, string>) => {
        arktest.assertEQ(newMap.get(key), value);
    });

    let dmap = new LightWeightMap<string, string>();
    dmap.set("a", "aa");
    dmap.set("b", "bb");
    dmap.set("c", "cc");
    dmap.set("d", "dd");
    dmap.set("e", "ee");
    dmap.setAll(map);
    arktest.assertEQ(dmap.length, 5);
    arktest.assertTrue(dmap.hasAll(map), "Expected dmap.hasAll(map) to be true after setAll");
    arktest.assertEQ(dmap.remove("a"), "aa");
    arktest.assertEQ(dmap.length, 4);
    arktest.assertTrue(dmap.removeAt(dmap.getIndexOfKey("b")), "Expected removeAt(indexOf('b')) to return true");
    arktest.assertEQ(dmap.length, 3);
    arktest.assertTrue(dmap.setValueAt(dmap.getIndexOfKey("d"), "ee"),
        "Expected setValueAt(indexOf('d'), 'ee') to be true");
    arktest.assertEQ(dmap.getValueAt(dmap.getIndexOfKey("d")), "ee");
    arktest.assertEQ(dmap.getValueAt(Math.floor(0).toInt()), "cc");
    arktest.assertEQ(dmap.getValueAt(Math.floor(1).toInt()), "ee");
    arktest.assertTrue(dmap.setValueAt(Math.floor(1.3).toInt(), "ff"), "Expected setValueAt(Math.floor(1.3).toInt(), 'ff') to be true");
    arktest.assertEQ(dmap.getValueAt(Math.floor(1.3).toInt()), "ff");
    arktest.assertEQ(dmap.toString(), "c:cc,d:ff,e:ee");
    arktest.assertTrue(dmap.removeAt(Math.floor(1.3).toInt()), "Expected removeAt(Math.floor(1.3).toInt()) to return true");
    arktest.assertEQ(dmap.length, 2);
    dmap.increaseCapacityTo(20)
    dmap.increaseCapacityTo(Math.floor(1.3).toInt());
    dmap.clear();
    arktest.assertEQ(dmap.length, 0);
}

function testEmptyMapExceptionHandling() {
    let empty_fm = new LightWeightMap<string, string>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`
        }
        return false;
    }
    arktest.expectThrow(() => {empty_fm.getKeyAt(0)}, exceptionCheck);
    let exceptionCheck1 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`
        }
        return false;
    }
    arktest.expectThrow(() => {empty_fm.getValueAt(0)}, exceptionCheck1);
    let exceptionCheck2 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`
        }
        return false;
    }
    arktest.expectThrow(() => {empty_fm.setValueAt(0, 'cc')}, exceptionCheck2);
}

function testIndexAccessWithFlooredNumbers() {
    let myLm = new LightWeightMap<string, string>();
    myLm.set("a", "aa");
    myLm.set("b", "bb");
    arktest.assertEQ(myLm.getKeyAt(Math.floor(1.5).toInt()), "b");
    arktest.assertEQ(myLm.getValueAt(Math.floor(1.5).toInt()), "bb");
    arktest.assertTrue(myLm.setValueAt(Math.floor(1.5).toInt(), "cc"),
        "Expected setValueAt(Math.floor(1.5).toInt(), 'cc') to return true");
    arktest.assertTrue(myLm.removeAt(Math.floor(1.5).toInt()), "Expected removeAt(Math.floor(1.5).toInt()) to return true");
}

function testEntriesIteratorAndForOf() {
    let map = new LightWeightMap<string, string>();

    map.set("a", "aa");
    map.set("b", "bb");
    map.set("c", "cc");

    let iteratorEntries1 = map.entries();
    iteratorEntries1.next().value;
    iteratorEntries1.next().value;
    arktest.assertNE(iteratorEntries1.next().value, undefined);
    arktest.assertEQ(iteratorEntries1.next().value, undefined);

    let arr1 = ["aa", "bb", "cc"];
    let j = 0;
    for (const item of map) {
        arktest.assertEQ(item[1], arr1[j]);
        j++;
    }
}
