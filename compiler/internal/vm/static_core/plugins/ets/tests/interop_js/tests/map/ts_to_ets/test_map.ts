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

const etsVm = globalThis.gtest.etsVm;

const TestMapSetFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapSet');
const TestMapGetFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapGet');
const TestMapHasFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapHas');
const TestMapDeleteFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapDelete');
const TestMapSizeFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapSize');
const TestMapKeysFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapKeys');
const TestMapValuesFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapValues');
const TestMapEntriesFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapEntries');
const TestMapForEachFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapForEach');
const TestMapClearFunc = etsVm.getFunction('Lmap/test/ETSGLOBAL;', 'TestMapClear');

let myMap: Map<string, number> = new Map<string, number>();
myMap.set('apple', 1);

function testMapSet(): boolean {
    return TestMapSetFunc(myMap) && myMap.get('banana') === 2 && myMap.get('cherry') === 3;
}

function testMapGet(): boolean {
    return TestMapGetFunc(myMap);
}

function testMapHas(): boolean {
    return TestMapHasFunc(myMap);
}

function testMapDelete(): boolean {
    return TestMapDeleteFunc(myMap) && !myMap.has('banana');
}

function testMapSize(): boolean {
    return TestMapSizeFunc(myMap) && myMap.size === 3;
}

function testMapKeys(): boolean {
    return TestMapKeysFunc(myMap);
}

function testMapValues(): boolean {
    return TestMapValuesFunc(myMap);
}

function testMapEntries(): boolean {
    return TestMapEntriesFunc(myMap);
}

function testMapForEach(): boolean {
  return TestMapForEachFunc(myMap);
}

function testMapClear(): boolean {
    return TestMapClearFunc(myMap) && myMap.size === 0;
}

ASSERT_TRUE(testMapSet());
ASSERT_TRUE(testMapGet());
ASSERT_TRUE(testMapHas());
ASSERT_TRUE(testMapDelete());
ASSERT_TRUE(testMapSize());
ASSERT_TRUE(testMapKeys());
ASSERT_TRUE(testMapValues());
ASSERT_TRUE(testMapEntries());
ASSERT_TRUE(testMapForEach());
ASSERT_TRUE(testMapClear());