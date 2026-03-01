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

import TreeMap from "@ohos.util.TreeMap";
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeMap GetFirstKey GetLastKey API tests")

    suite.addTest("TreeMap getFirstKey getLastKey first and last key",
        testGetFirstKeyAndLastKeySingleElement)
    suite.addTest("TreeMap getFirstKey getLastKey first and last key from the null treeMap",
        testGetFirstKeyAndLastKeyEmptyTreeMap)
    suite.addTest("TreeMap getFirstKey getLastKey first and last key with a set of data",
        testGetFirstKeyAndLastKeyMultipleElements)
    suite.addTest("TreeMap getFirstKey getLastKey first and last key by descending",
        testGetFirstKeyAndLastKeyDescendingOrder)
    suite.addTest("TreeMap getFirstKey getLastKey first and last object key",
        testGetFirstKeyAndLastKeyObjectKeys)
    suite.addTest("TreeMap getFirstKey getLastKey first and last object key with a set of data",
        testGetFirstKeyAndLastKeyMultipleObjectKeys)

    return suite.run()
}

class MyObject extends Object {
    size: double;
    constructor(size: double) {
        super();
        this.size = size;
    }
}

function testGetFirstKeyAndLastKeySingleElement() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue);
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    treeMap.set("cc", "789");
    let first = treeMap.getFirstKey();
    let last = treeMap.getLastKey();
    arktest.assertEQ(first, "aa", "The treeMap should return the correct first key");
    arktest.assertEQ(last, "cc", "The treeMap should return the correct last key");
    arktest.assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testGetFirstKeyAndLastKeyEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue);
        });
    try {
        let first = treeMap.getFirstKey();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    try {
        let last = treeMap.getLastKey();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testGetFirstKeyAndLastKeyMultipleElements() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue);
        });
    treeMap.set("aa", "123");
    let first = treeMap.getFirstKey();
    let last = treeMap.getLastKey();
    arktest.assertEQ(first, "aa", "The treeMap should return the correct first key");
    arktest.assertEQ(last, "aa", "The treeMap should return the correct last key");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testGetFirstKeyAndLastKeyDescendingOrder() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    treeMap.set("cc", "789");
    let first = treeMap.getFirstKey();
    let last = treeMap.getLastKey();
    arktest.assertEQ(first, "cc", "The treeMap should return the correct first key");
    arktest.assertEQ(last, "aa", "The treeMap should return the correct last key");
    arktest.assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testGetFirstKeyAndLastKeyObjectKeys() {
    let treeMapo: TreeMap<MyObject, MyObject> =
        new TreeMap<MyObject, MyObject>((firstValue: MyObject, secondValue: MyObject): double => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    let obj_sec: MyObject = new MyObject(2);
    let obj_trd: MyObject = new MyObject(3);
    let obj_v: MyObject = new MyObject(4);
    let obj_sec_v: MyObject = new MyObject(5);
    let obj_trd_v: MyObject = new MyObject(6);
    treeMapo.set(obj, obj_v);
    treeMapo.set(obj_sec, obj_sec_v);
    treeMapo.set(obj_trd, obj_trd_v);
    let first = treeMapo.getFirstKey();
    let last = treeMapo.getLastKey();
    arktest.assertEQ(first, obj, "The treeMap should return the correct first key");
    arktest.assertEQ(last, obj_trd, "The treeMap should return the correct last key");
    arktest.assertEQ(treeMapo.length, 3, "The treeMap length should be 3");
}

function testGetFirstKeyAndLastKeyMultipleObjectKeys() {
    let treeMapo: TreeMap<MyObject, MyObject> =
    new TreeMap<MyObject, MyObject>((firstValue: MyObject, secondValue: MyObject): double => {
        return secondValue.size - firstValue.size;
    });
    let obj: MyObject = new MyObject(1);
    let obj2: MyObject = new MyObject(2);
    treeMapo.set(obj, obj2);
    let first = treeMapo.getFirstKey();
    let last = treeMapo.getLastKey();
    arktest.assertEQ(first, obj, "The treeMap should return the correct first key");
    arktest.assertEQ(last, obj, "The treeMap should return the correct last key");
    arktest.assertEQ(treeMapo.length, 1, "The treeMap length should be 1");
}
