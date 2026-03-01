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

function addLaunch<T> (arr: Array<Job<T>>, func: ()=>T): void {
	arr.push(launch <T, ()=>T>(func));
}

function awaitAll <T> (arr: Array<Job<T>>): Array<T> {
	let res: Array<T> = new Array<T> (arr.length);
	for (let i = 0; i < arr.length; ++ i)
		res[i] = arr[i].Await();
	return res;
}

function sumArrInto (to: FixedArray<int>, from: FixedArray<int>): void {
	arktest.assertEQ(to.length, from.length);
	for (let i = 0; i < to.length; ++ i) {
		to[i] += from[i];
	}
}

function assertEQAll(array: FixedArray<int>, value: int): void {
	for (let i=0; i < array.length; ++ i)
		arktest.assertEQ(array[i], value)
}

function testConcurrentItersSR() {
	const JOB_COUNT = 7;
	const SET_COUNT = 70;

	let map = new CHMap();

	let setF = () => {
		for (let i = 0; i < SET_COUNT; ++ i)
			map.set(i, i);
	}
	let rmF = () => {
		for (let i = 0; i < SET_COUNT; ++ i)
			map.remove(i);
	}
	let iterF = () => {
		for (let i = 0; i < SET_COUNT; ++ i)
			map.forEach((v) => arktest.assertTrue(0 <= v && v < SET_COUNT));
	}

	let jobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(jobs, setF);
		addLaunch(jobs, rmF);
		addLaunch(jobs, iterF);
	}
	awaitAll(jobs);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addTest("testConcurrentItersSR", testConcurrentItersSR);

    return CHMTestSuitecase.run();
}

