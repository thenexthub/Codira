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

type CHMap = containers.ConcurrentHashMap<int, int>;

function testSetAndGetNC() {
	let map = new CHMap();
	arktest.assertEQ(map.get(0), undefined);
	map.set(0, 1);
	arktest.assertEQ(map.get(0), 1);
	map.set(0, 2);
	arktest.assertEQ(map.get(0), 2);
	arktest.assertEQ(map.get(1), undefined);
	arktest.assertEQ(map.get(-129380), undefined);
	map.set(-129380, -100);
	arktest.assertEQ(map.get(-129380), -100);
}

function testSetAndGetNC2() {
	let map = new CHMap();
	for (let i = 0; i < 10; i += 2)
		map.set(i, i * 3);
	for (let i = 0; i < 10; ++ i ) {
		let val: int|undefined = [i * 3, undefined][i&1];
		arktest.assertEQ(map.get(i), (val) );
	}
}

function testReplaceNC() {
	let map = new CHMap();
	arktest.assertFalse(map.replace(1, 1));
	arktest.assertEQ(map.get(1), undefined);
	map.set(1, 1);
	arktest.assertEQ(map.get(1), 1);
	arktest.assertTrue(map.replace(1, 2));
	arktest.assertEQ(map.get(1), 2);
	arktest.assertTrue(map.replace(1, 3));
	arktest.assertEQ(map.get(1), 3);
}

function testReplaceNC2() {
	let map = new CHMap();
	map.set(1, -1);
	for (let i = 0; i < 10; ++ i) {
		arktest.assertTrue(map.replace(1, i));
		arktest.assertEQ(map.get(1), i);
	}
}

function testHasAndDeleteNC() {
	let map = new CHMap();
	arktest.assertFalse(map.has(0));
	arktest.assertFalse(map.delete(0));
	map.set(0, 0);
	arktest.assertFalse(map.has(1));
	arktest.assertFalse(map.delete(1));
	arktest.assertTrue(map.has(0));
	arktest.assertTrue(map.delete(0));
	arktest.assertFalse(map.has(0));
	arktest.assertFalse(map.delete(0));
	map.set(0, 0);
	arktest.assertTrue(map.has(0));
	arktest.assertTrue(map.delete(0));
}

function testHasAndDeleteNC2() {
	let map = new CHMap();
	for (let i = 0; i < 10; ++ i) {
		arktest.assertFalse(map.has(i));
		arktest.assertFalse(map.delete(i));
	}
	for (let i = 0; i < 10; ++ i)
		map.set(i, i);
	for (let i = 0; i < 10; ++ i) {
		arktest.assertTrue(map.has(i));
		arktest.assertTrue(map.delete(i));
	}
	for (let i = 0; i < 10; ++ i) {
		arktest.assertFalse(map.has(i));
		arktest.assertFalse(map.delete(i));
	}
}

function testHasAndRemoveNC() {
	let map = new CHMap();
	arktest.assertEQ(map.remove(0), undefined);
	arktest.assertEQ(map.remove(0), undefined);
	map.set(0, 0);
	arktest.assertEQ(map.remove(0), 0);
	arktest.assertEQ(map.remove(0), undefined);
	arktest.assertFalse(map.has(0));

	map.set(0, 0);
	map.set(1, 1);
	arktest.assertEQ(map.remove(1), 1);
	arktest.assertEQ(map.remove(1), undefined);
	arktest.assertFalse(map.has(1));
}

function testHasAndRemoveNC2() {
	let map = new CHMap();
	for (let i = 0; i < 10; ++ i)
		arktest.assertEQ(map.remove(i), undefined);
	for (let i = 0; i < 10; ++ i)
		map.set(i, i);
	for (let i = 0; i < 10; ++ i)
		arktest.assertEQ(map.remove(i), i);
	for (let i = 0; i < 10; ++ i)
		arktest.assertFalse(map.has(i));
}

function testClearNC() {
	let map = new CHMap();
	map.clear();
	map.clear();
	map.set(1, 1);
	arktest.assertEQ(map.get(1), 1);
	map.set(2, 2);
	map.set(3, 3);
	map.clear();
	arktest.assertFalse(map.has(1));
	arktest.assertFalse(map.has(2));
	arktest.assertFalse(map.has(3));
	arktest.assertTrue(map.isEmpty());
}

function testClearNC2() {
	let map = new CHMap();
	arktest.assertTrue(map.isEmpty());
}

function testSizeNC() {
	let map = new CHMap();
	arktest.assertEQ(map.size, 0);
	map.set(1, 1);
	arktest.assertEQ(map.size, 1);
	map.set(1, 1);
	arktest.assertEQ(map.size, 1);
	map.set(2, 2);
	arktest.assertEQ(map.size, 2);
	map.remove(1);
	map.delete(2);
	map.remove(3);
	map.delete(4);
	arktest.assertEQ(map.size, 0);
	map.set(1, 1);
	arktest.assertEQ(map.size, 1);
}

function testSizeNC2() {
	let map = new CHMap();
	for (let i = 0; i < 10; ++ i) {
		arktest.assertEQ(i, Double.toInt(map.size));
		map.set(i, i);
	}
	for (let i = 0; i < 10; ++ i) {
		arktest.assertEQ(10 - i, map.size);
		map.remove(i);
	}
	arktest.assertEQ(0, Double.toInt(map.size));
}

function testIteratorsNC () {
	let map = new CHMap();
	let arr1 = Array.from(map.keys());
	let arr2 = Array.from(map.values());
	let arr3 = Array.from(map.entries());
	arr1.forEach(()=>arktest.assertFalse(true));
	arr2.forEach(()=>arktest.assertFalse(true));
	arr3.forEach(()=>arktest.assertFalse(true));
	map.forEach(()=>arktest.assertFalse(true));
}

function testIteratorsNC1 () {
	let map = new CHMap();
	for (let i = 0; i < 100; ++ i)
		map.set(i, i);
	for (let i = 0; i < 100; ++ i)
		map.remove(i);
	let arr1 = Array.from(map.keys());
	let arr2 = Array.from(map.values());
	let arr3 = Array.from(map.entries());
	arr1.forEach(()=>arktest.assertFalse(true));
	arr2.forEach(()=>arktest.assertFalse(true));
	arr3.forEach(()=>arktest.assertFalse(true));
	map.forEach(()=>arktest.assertFalse(true));
}

function testIteratorsNC2 () {
	let map = new CHMap();
	map.set(1, 2);
	map.set(3, 4);
	let arr1 = Array.from(map.keys());
	let arr2 = Array.from(map.values());
	let arr3 = Array.from(map.entries());
	arktest.assertEQ(arr1.toString(), [1, 3].toString());
	arktest.assertEQ(arr2.toString(), [2, 4].toString());
	arktest.assertEQ(arr3.toString(), [[1, 2], [3, 4]].toString());
	let val = 1;
	map.forEach((v, k, mp) => {
		arktest.assertEQ(k, val);
		arktest.assertEQ(v, val + 1);
		arktest.assertEQ(mp.toString(), map.toString());
		val += 2;
	});
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addTest("testSetAndGetNC", testSetAndGetNC);
	CHMTestSuitecase.addTest("testSetAndGetNC2", testSetAndGetNC2);
	CHMTestSuitecase.addTest("testReplaceNC", testReplaceNC);
	CHMTestSuitecase.addTest("testReplaceNC2", testReplaceNC2);
	CHMTestSuitecase.addTest("testHasAndDeleteNC", testHasAndDeleteNC);
	CHMTestSuitecase.addTest("testHasAndDeleteNC2", testHasAndDeleteNC2);
	CHMTestSuitecase.addTest("testHasAndRemoveNC", testHasAndRemoveNC);
	CHMTestSuitecase.addTest("testHasAndRemoveNC2", testHasAndRemoveNC2);
	CHMTestSuitecase.addTest("testClearNC", testClearNC);
	CHMTestSuitecase.addTest("testClearNC2", testClearNC2);
	CHMTestSuitecase.addTest("testSizeNC", testSizeNC);
	CHMTestSuitecase.addTest("testSizeNC2", testSizeNC2);
	CHMTestSuitecase.addTest("testIteratorsNC", testIteratorsNC);
	CHMTestSuitecase.addTest("testIteratorsNC1", testIteratorsNC1);
	CHMTestSuitecase.addTest("testIteratorsNC2", testIteratorsNC2);
    return CHMTestSuitecase.run();
}

