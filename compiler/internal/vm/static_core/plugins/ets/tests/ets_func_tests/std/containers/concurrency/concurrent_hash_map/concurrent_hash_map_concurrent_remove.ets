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

function testConcurrentRemove() {
	const SET_COUNT = 100;
	const JOB_COUNT = 7;

	let map = new CHMap();
	let makeRemoveF = (start: int, end: int) => () => {
		for (let i = start; i < end; ++ i)
			arktest.assertEQ(map.remove(i), i);
	}
	let removeJobs: Job<void>[] = [];
	for (let i = 0; i < SET_COUNT * JOB_COUNT; ++ i)
		map.set(i, i);
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(removeJobs, makeRemoveF(i * SET_COUNT, (i + 1) * SET_COUNT));
	awaitAll(removeJobs);
	for (let i = 0; i < SET_COUNT * JOB_COUNT; ++ i)
		arktest.assertFalse(map.has(i));
}

function testConcurrentRemove2() {
	const SET_COUNT = 100;
	const JOB_COUNT = 10;

	let map = new CHMap();
	let removeF: ()=>FixedArray<int> = () => {
		let arr: FixedArray<int> = new FixedArray<int>(SET_COUNT);
		for (let i = 0; i < SET_COUNT; ++ i) {
			let val = map.remove(i);
			if (val !== undefined ) {
				arktest.assertEQ(val, i);
				arr[i] = 1;
			}
		}
		return arr;
	}

	let removeJobs: Job<FixedArray<int>>[] = [];
	for (let i = 0; i < SET_COUNT; ++ i)
		map.set(i, i);
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(removeJobs, removeF);

	let arrs = awaitAll(removeJobs);
	let accum = arrs[0];
	for (let i = 1; i < JOB_COUNT; ++ i )
		sumArrInto(accum, arrs[i]);
	assertEQAll(accum, 1);
}

function testConcurrentRemoveAndSet() {
	const JOB_COUNT = 7;
	const SET_COUNT = 100;

	let map = new CHMap();
	let setF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.set(i, i);
		}
	}
	let removeF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			let val = map.remove(i);
			if (val !== undefined)
				arktest.assertEQ(val, i);
		}
	}
	let setJobs: Job<void>[] = [];
	let removeJobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(setJobs, setF);
		addLaunch(removeJobs, removeF);
	}
	awaitAll(setJobs);
	awaitAll(removeJobs);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addTest("testConcurrentRemove", testConcurrentRemove);
	CHMTestSuitecase.addTest("testConcurrentRemove2", testConcurrentRemove2);
	CHMTestSuitecase.addTest("testConcurrentRemoveAndSet", testConcurrentRemoveAndSet);

    return CHMTestSuitecase.run();
}

