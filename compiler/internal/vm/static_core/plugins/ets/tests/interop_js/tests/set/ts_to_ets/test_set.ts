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

const TestSetAddFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetAdd');
const TestSetHasFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetHas');
const TestSetDeleteFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetDelete');
const TestSetSizeFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetSize');
const TestSetKeysFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetKeys');
const TestSetValuesFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetValues');
const TestSetEntriesFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetEntries');
const TestSetForEachFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetForEach');
const TestSetClearFunc = etsVm.getFunction('Lset/test/ETSGLOBAL;', 'TestSetClear');

let mySet: Set<number> = new Set<number>([1, 2, 3]);

function testSetAdd(): boolean {
    return TestSetAddFunc(mySet) && mySet.has(4) && mySet.has(5);
}

function testSetHas(): boolean {
    return TestSetHasFunc(mySet);
}

function testSetDelete(): boolean {
    return TestSetDeleteFunc(mySet) && !mySet.has(4);
}

function testSetSize(): boolean {
  return TestSetSizeFunc(mySet) && mySet.size === 5;
}

function testSetKeys(): boolean {
    return TestSetKeysFunc(mySet);
}

function testSetValues(): boolean {
    return TestSetValuesFunc(mySet);
}

function testSetEntries(): boolean {
    return TestSetEntriesFunc(mySet);
}

function testSetForEach(): boolean {
  return TestSetForEachFunc(mySet);
}

function testSetClear(): boolean {
    return TestSetClearFunc(mySet) && mySet.size === 0;
}

ASSERT_TRUE(testSetAdd());
ASSERT_TRUE(testSetHas());
ASSERT_TRUE(testSetDelete());
ASSERT_TRUE(testSetSize());
ASSERT_TRUE(testSetKeys());
ASSERT_TRUE(testSetValues());
ASSERT_TRUE(testSetEntries());
ASSERT_TRUE(testSetForEach());
ASSERT_TRUE(testSetClear());