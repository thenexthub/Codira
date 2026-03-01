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

function testConcurrentReplace() {
	const JOB_COUNT = 7;
	const SET_COUNT = 20;

	let map = new CHMap();
	for (let i = 0; i < SET_COUNT * JOB_COUNT; ++ i)
		map.set(i, -1);
	let makeReplaceF = (start: int, end: int) => () => {
		for (let i = start; i < end; i ++ )
			arktest.assertTrue(map.replace(i, i));
	}

	let replaceJobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		let start = i * SET_COUNT;
		addLaunch(replaceJobs, makeReplaceF(start, start + SET_COUNT));
	}
	awaitAll(replaceJobs);
	for (let i = 0; i < JOB_COUNT * SET_COUNT; ++ i)
		arktest.assertEQ(map.get(i), i);
}

function testConcurrentReplace2() {
	const JOB_COUNT = 7;
	const SET_COUNT = 20;

	let map = new CHMap();
	for (let i = 0; i < SET_COUNT; ++ i)
		map.set(i, -1);
	let makeReplaceF = (seed: int) => () => {
		for (let i = 0; i < SET_COUNT; i ++ )
			arktest.assertTrue(map.replace(i, i + seed * SET_COUNT));
	}

	let replaceJobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(replaceJobs, makeReplaceF(i));
	}
	awaitAll(replaceJobs);
	for (let i = 0; i < SET_COUNT; ++ i)
		arktest.assertEQ(map.get(i) as int % SET_COUNT, i);
}

function testConcurrentReplaceAndGet() {
	const JOB_COUNT = 7;
	const SET_COUNT = 20;

	let map = new CHMap();
	for (let i = 0; i < SET_COUNT; ++ i)
		map.set(i, -1);
	let getF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			let val = map.get(i);
			arktest.assertNE(val, undefined);
			val = val as int;
			while (val == -1) {
				val = map.get(i);
				Coroutine.Schedule();
			}
			arktest.assertNE(val, undefined);
			arktest.assertEQ(val as int % SET_COUNT, i);
		}
	}
	let makeReplaceF = (seed: int) => () => {
		for (let i = 0; i < SET_COUNT; i ++ )
			arktest.assertTrue(map.replace(i, seed * SET_COUNT + i));
	}

	let getJobs: Job<void>[] = [];
	let replaceJobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(getJobs, getF);
		addLaunch(replaceJobs, makeReplaceF(i));
	}
	awaitAll(getJobs);
	awaitAll(replaceJobs);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addTest("testConcurrentReplace", testConcurrentReplace);
	CHMTestSuitecase.addTest("testConcurrentReplace2", testConcurrentReplace2);
	CHMTestSuitecase.addTest("testConcurrentReplaceAndGet", testConcurrentReplaceAndGet);

    return CHMTestSuitecase.run();
}

