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

function testConcurrentGetAndSet() {
	const JOB_COUNT = 7;
	const SET_COUNT = 100;

	let map = new CHMap();
	let getF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			let val: int|undefined = undefined;
			while (val == undefined) {
				val = map.get(i);
				Coroutine.Schedule();
			}
			arktest.assertEQ(val as int % SET_COUNT, i);
		}
	}
	let makeSetF = (seed: int) => () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.set(i, seed * SET_COUNT + i);
		}
	}
	let setJobs: Job<void>[] = [];
	let getJobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(getJobs, getF);
		addLaunch(setJobs, makeSetF(i));
	}
	awaitAll(setJobs);
	awaitAll(getJobs);
}

function testConcurrentGetAndSet2() {
	const JOB_COUNT = 10;
	const SET_COUNT = 200;

	let map = new CHMap();
	let getF = () => {
		let prev: int = -1;
		while (prev != SET_COUNT - 1) {
			Coroutine.Schedule();
			let val: int|undefined = map.get(0);
			if (val == undefined || val == prev) continue;
			arktest.assertLT(prev, val as int);
			prev = val as int;
		}
	}
	let setF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.set(0, i);
		}
	}
	let getJobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(getJobs, getF);
	}
	launch<void, ()=>void>(setF).Await();
	awaitAll(getJobs);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addTest("testConcurrentGetAndSet", testConcurrentGetAndSet);
	CHMTestSuitecase.addTest("testConcurrentGetAndSet2", testConcurrentGetAndSet2);

    return CHMTestSuitecase.run();
}

