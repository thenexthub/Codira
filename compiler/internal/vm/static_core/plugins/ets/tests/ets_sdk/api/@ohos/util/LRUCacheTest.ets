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

import {util} from "@ohos.util"

const success = 0
const fail = 1

function main(): int {
    const suite = new arktest.ArkTestsuite("LRUCache API tests")

    suite.addTest("LRUCache Constructor No Parameter", testLRUCacheConstructorNoParameter);
    suite.addTest("LRUCache Constructor Int", testLRUCacheConstructorInt);
    suite.addTest("LRUCache update&getCapacity Int", testLRUCacheUpdateGetCapacityInt);
    suite.addTest("LRUCache Get Int", testLRUCacheGetInt);
    suite.addTest("LRUCache Get Str&Undefined", testLRUCacheGetStrAndUndefined);
    suite.addTest("LRUCache Get CapacityChanged", testLRUCacheGetCapacityChanged);
    suite.addTest("LRUCache Put Once", testLRUCachePutOnce);
    suite.addTest("LRUCache Put Twice", testLRUCachePutTwice);
    suite.addTest("LRUCache Put&Get", testLRUCachePutAndGet);
    suite.addTest("LRUCache Put CapacityChenged", testLRUCachePutCapacityChenged);
    suite.addTest("LRUCache RemoveOnce", testLRUCacheRemoveOnce);
    suite.addTest("LRUCache RemoveTwice", testLRUCacheRemoveTwice);
    suite.addTest("LRUCache RemoveUndefined", testLRUCacheRemoveUndefined);
    suite.addTest("LRUCache Remove&Get", testLRUCacheRemoveAndGet);
    suite.addTest("LRUCache Clear One", testLRUCacheClearOne);
    suite.addTest("LRUCache Clear Multiple", testLRUCacheClearMulti);
    suite.addTest("LRUCache Contains True", testLRUCacheContainsTrue);
    suite.addTest("LRUCache Contains False", testLRUCacheContainsFalse);
    suite.addTest("LRUCache Contains Mixed", testLRUCacheContainsMixed);
    suite.addTest("LRUCache ToString Put", testLRUCacheToStringPut);
    suite.addTest("LRUCache ToString Hit Once", testLRUCacheToStringHitOnce);
    suite.addTest("LRUCache ToString HitAll", testLRUCacheToStringHitAll);
    suite.addTest("LRUCache ToString Hit&Miss", testLRUCacheToStringHitAndMiss);
    suite.addTest("LRUCache ToString Mixed", testLRUCacheToStringMixed);

    return suite.run()
}

function testLRUCacheUpdateGetCapacityInt() {
    let that = new util.LRUCache<number, number>();
    that.updateCapacity(10);
    let res1: number = that.getCapacity();
    arktest.assertEQ(res1, 10);
    that.updateCapacity(2147483646);
    let res2: number = that.getCapacity();
    arktest.assertEQ(res2, 2147483646);
}

function testLRUCacheGetInt() {
    let that = new util.LRUCache<number, number>();
    that.put(1,2);
    let res = that.get(1);
    arktest.assertEQ(res, 2);
}

function testLRUCacheGetStrAndUndefined() {
    let that = new util.LRUCache<number|string, number|string>(100);
    that.put(1,2)
    that.put(2,10)
    that.put('abcd',15)
    let res1 = that.get(20)
    let res2 = that.get('abcd')
    let res3 = that.get(2)
    arktest.assertEQ(res1, undefined);
    arktest.assertEQ(res2, 15);
    arktest.assertEQ(res3, 10);
}

function testLRUCacheGetCapacityChanged() {
    let that = new util.LRUCache<number|string, number|string>(100);
    that.put('1111','bcodedshc')
    that.put(20,'cdjcaxb')
    that.updateCapacity(2)
    that.put('abcd',15)
    let res1 = that.get('1111')
    let res2 = that.get('abcd')
    arktest.assertEQ(res1, undefined);
    arktest.assertEQ(res2, 15);
}

function testLRUCachePutOnce() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111', 'bcodedshc');
    let res1 = that.get('1111');
    arktest.assertEQ(res1, 'bcodedshc');
}

function testLRUCachePutTwice() {
    let that = new util.LRUCache<string|number, string|number>();
    let temp = that.put('1111', 'bcodedshc');
    let temp2 = that.put('1111', 22);
    arktest.assertEQ(temp, undefined);
    arktest.assertEQ(temp2, 'bcodedshc');
}

function testLRUCachePutAndGet() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111','bcodedshc');
    that.put(1,12);
    that.put(2,5);
    let temp = that.put(1,10);
    let temp2 = that.put(2,22);
    let temp3 = that.put(2,30);
    let res1 = that.get(1);
    let res2 = that.get(2);
    arktest.assertEQ(temp, 12);
    arktest.assertEQ(temp2, 5);
    arktest.assertEQ(temp3, 22);
    arktest.assertEQ(res1, 10);
    arktest.assertEQ(res2, 30);
}

function testLRUCachePutCapacityChenged() {
    let that = new util.LRUCache<string|number|number, string|number>();
    that.put('1111','bcodedshc');
    that.put(1,12);
    that.put(2,5);
    that.updateCapacity(2);
    that.put(2,'adasfdad');
    that.put(3,'a');
    let res1 = that.get(1);
    arktest.assertEQ(res1, undefined);
}

function testLRUCacheClearOne() {
    let that = new util.LRUCache<number, number>();
    that.put(1,2);
    let result1 = that.length;
    that.clear();
    let result2 = that.length;
    arktest.assertEQ(result1, 1);
    arktest.assertEQ(result2, 0);
}

function testLRUCacheClearMulti() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(2,10)
    that.put(5,33);
    that.put('abcd','string');
    that.updateCapacity(3);
    let result1 = that.length;
    that.clear();
    let result2 = that.length;
    arktest.assertEQ(result1, 3);
    arktest.assertEQ(result2, 0);
}

function testLRUCacheContainsTrue() {
    let that = new util.LRUCache<number, number>();
    that.put(1,2);
    let result1 = that.contains(1);
    arktest.assertEQ(result1, true);
}

function testLRUCacheContainsFalse() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('abc','abc');
    let result1 = that.contains(1);
    arktest.assertEQ(result1, false);
}

function testLRUCacheContainsMixed() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put('string','string')
    let result1 = that.contains(1)
    let result2 = that.contains('string')
    let result3 = that.contains(0)
    arktest.assertEQ(result1, true);
    arktest.assertEQ(result2, true);
    arktest.assertEQ(result3, false);
}

function testLRUCacheRemoveOnce() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    let res1 = that.remove(1);
    arktest.assertEQ(res1, 2);
}

function testLRUCacheRemoveTwice() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put('abcd',20);
    let res1 = that.remove(1);
    let res2 = that.remove('abcd');
    arktest.assertEQ(res1, 2);
    arktest.assertEQ(res2, 20);
}

function testLRUCacheRemoveUndefined() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    let res1 = that.remove(1);
    let res2 = that.remove(5);
    arktest.assertEQ(res1, 2);
    arktest.assertEQ(res2, undefined);
}

function testLRUCacheRemoveAndGet() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put(3,10);
    that.put(5,15);
    let res1 = that.remove(3);
    let res2 = that.get(3);
    let res3 = that.remove(3);
    arktest.assertEQ(res1, 10);
    arktest.assertEQ(res2, undefined);
    arktest.assertEQ(res3, undefined);
}

function testLRUCacheToStringPut() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111','bcodedshc')
    that.put(1,2)
    let result = that.toString();
    arktest.assertEQ(result, 'LRUCache[ maxSize = 64, hits = 0, misses = 0, hitRate = 0% ]');
}

function testLRUCacheToStringHitAll() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111','bcodedshc')
    that.put(1,2)
    that.get(1);
    that.get('1111');
    let result = that.toString();
    arktest.assertEQ(result, 'LRUCache[ maxSize = 64, hits = 2, misses = 0, hitRate = 100% ]');
}

function testLRUCacheToStringHitOnce() {
    let that = new util.LRUCache<string|number, string|number>(100);
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20)
    that.get(3)
    let result = that.toString();
    arktest.assertEQ(result, 'LRUCache[ maxSize = 100, hits = 1, misses = 0, hitRate = 100% ]');
}

function testLRUCacheToStringHitAndMiss() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20)
    that.get(3)
    that.get(2);
    let result = that.toString();
    arktest.assertEQ(result, 'LRUCache[ maxSize = 64, hits = 1, misses = 1, hitRate = 50% ]');
}

function testLRUCacheToStringMixed() {
    let that = new util.LRUCache<string|number, string|number>(123456);
    that.put(1,2);
    that.put(3,10);
    that.put(5,15);
    that.put('abc',20);
    that.get(3);
    that.get(2);
    that.get(1);
    that.contains(5);
    let result = that.toString();
    arktest.assertEQ(result, 'LRUCache[ maxSize = 123456, hits = 3, misses = 1, hitRate = 75% ]');
}

function testLRUCacheConstructorNoParameter() {
    let that = new util.LRUCache<number, number>();
    let res: number = that.getCapacity();
    arktest.assertEQ(res, 64);
}

function testLRUCacheConstructorInt() {
    let that = new util.LRUCache<number, number>(2147483646);
    let res: number = that.getCapacity();
    arktest.assertEQ(res, 2147483646);
}
