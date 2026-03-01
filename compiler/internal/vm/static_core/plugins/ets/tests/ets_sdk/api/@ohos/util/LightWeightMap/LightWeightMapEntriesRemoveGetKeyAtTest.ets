/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import LightWeightMap from '@ohos.util.LightWeightMap';
import { BusinessError } from '@ohos.base';

const OUT_OF_BOUNDS_ERROR_CODE_ID: int = 10200001;

function main(): int {
    const suite = new arktest.ArkTestsuite('LightWeightMap Remove GetKeyAt Entries API tests')

    suite.addTest('Testing LightWeightMap remove data by key', testRemoveExistingKey)
    suite.addTest('Testing LightWeightMap remove data by not existed key', testRemoveNonExistingKey)
    suite.addTest('Testing LightWeightMap remove data by large string key', testRemoveLargeStringKey)
    suite.addTest('Testing LightWeightMap remove data by large number key', testRemoveLargeNumberKey)
    suite.addTest('Testing LightWeightMap get key by index', testGetKeyAtValidIndex)
    suite.addTest('Testing LightWeightMap get key by outside length index', testGetKeyAtInvalidIndex)
    suite.addTest('Testing LightWeightMap use entries and remove function', testEntriesAndRemove)
    suite.addTest('Testing LightWeightMap use entries to iterate over each item', testEntriesIteration)
    suite.addTest('Testing LightWeightMap use entries to iterate over each item for contain duplicate value',
        testEntriesIterationWithDuplicateValues)
    suite.addTest('Testing LightWeightMap use entries to iterate over each item for an empty LightWeightMap',
        testEntriesIterationOnEmptyMap)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_REMOVE_0100
function testRemoveExistingKey() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set('squirrel', 123);
    lightWeightMap.set('sparrow', 356);
    arktest.assertEQ(lightWeightMap.remove('sparrow'), 356);
    arktest.assertFalse(lightWeightMap.hasKey('sparrow'), 'Expected hasKey(\'sparrow\') to be false after removal');
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_REMOVE_0300
function testRemoveNonExistingKey() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set('squirrel', 123);
    arktest.assertEQ(lightWeightMap.remove('sparrow'), undefined);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_REMOVE_0400
function testRemoveLargeStringKey() {
    let key = '';
    for (let i: number = 0; i < 10000; i++) {
        key += i;
    }
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set(key, 123);
    arktest.assertEQ(lightWeightMap.remove(key), 123);
    arktest.assertFalse(lightWeightMap.hasKey(key), 'Expected hasKey(largeStringKey) to be false after removal');
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_REMOVE_0500
function testRemoveLargeNumberKey() {
    let lightWeightMap = new LightWeightMap<number, string>();
    lightWeightMap.set(Number.MAX_VALUE, 'squirrel');
    arktest.assertEQ(lightWeightMap.remove(Number.MAX_VALUE), 'squirrel');
    arktest.assertFalse(lightWeightMap.hasKey(Number.MAX_VALUE),
        'Expected hasKey(Number.MAX_VALUE) to be false after removal');
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETKEYAT_0100
function testGetKeyAtValidIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set('squirrel', 123);
    lightWeightMap.set('sparrow', 356);
    let index = lightWeightMap.getIndexOfKey('squirrel');
    arktest.assertNE(index, -1);
    arktest.assertEQ(lightWeightMap.getKeyAt(index), 'squirrel');
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETKEYAT_0300
function testGetKeyAtInvalidIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set('squirrel', 123);
    lightWeightMap.set('sparrow', 356);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message ==
                'The value of "index" is out of range. It must be >= 0 && <= 1. Received value is: 10' &&
                e.code == OUT_OF_BOUNDS_ERROR_CODE_ID;
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightMap.getKeyAt(10) }, exceptionCheck);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ENTRIES_0100
function testEntriesAndRemove() {
    let lightWeightMap = new LightWeightMap<number, string>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, `${i}`);
    }
    let iter = lightWeightMap.entries();
    let temp = iter.next();
    while (!temp.done) {
        arktest.assertEQ(lightWeightMap.remove(temp.value![0]), temp.value![1]);
        temp = iter.next();
    }
    iter = lightWeightMap.entries();
    temp = iter.next();
    while (!temp.done) {
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ENTRIES_0200
function testEntriesIteration() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set('squirrel', 123);
    lightWeightMap.set('sparrow', 356);
    let iter = lightWeightMap.entries();
    let temp = iter.next();
    while (!temp.done) {
        if (temp.value![0] === 'squirrel') {
            arktest.assertEQ(temp.value![1], 123);
        } else if (temp.value![0] === 'sparrow') {
            arktest.assertEQ(temp.value![1], 356);
        } else {
            arktest.assertFalse(lightWeightMap.hasKey(temp.value![0]),
                'Expected map not to have unknown key temp.value[0]');
            arktest.assertFalse(lightWeightMap.hasValue(temp.value![1]),
                'Expected map not to have unknown value temp.value[1]');
        }
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ENTRIES_0300
function testEntriesIterationWithDuplicateValues() {
    let data: number = 100;
    let lightWeightMap = new LightWeightMap<number, number>();
    for (let i = 0; i < 10; i++) {
        lightWeightMap.set(i, data);
    }
    arktest.assertEQ(lightWeightMap.length, 10);
    let iter = lightWeightMap.entries();
    let temp = iter.next();
    while (!temp.done) {
        arktest.assertTrue(temp.value![0] < 10, 'Expected key temp.value[0] to be less than 10');
        arktest.assertEQ(temp.value![1], data);
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ENTRIES_0400
function testEntriesIterationOnEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    let iter = lightWeightMap.entries();
    let temp = iter.next();
    arktest.assertTrue(temp.done, 'Expected iterator to be done for an empty LightWeightMap\'s entries');
}
