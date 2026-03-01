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

function testConcurrentDelete() {
	const JOB_COUNT = 7;
	const SET_COUNT = 100;

	let map = new CHMap();

	let deleteJobs: Job<void>[] = [];
	let makeDeleteF = (start: int, end: int, shouldBe: boolean) => () => {
		for (let i = start; i < end; ++ i)
			arktest.assertEQ(map.delete(i), shouldBe);
	}
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(deleteJobs, makeDeleteF(i * SET_COUNT, (i + 1) * SET_COUNT, false));
	awaitAll(deleteJobs);

	deleteJobs = [];
	for (let i = 0; i < SET_COUNT * JOB_COUNT; ++ i)
		map.set(i, i);
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(deleteJobs, makeDeleteF(i * SET_COUNT, (i + 1) * SET_COUNT, true));
	awaitAll(deleteJobs);
	for (let i = 0; i < JOB_COUNT * SET_COUNT; ++ i)
		arktest.assertFalse(map.has(i));
}

function testConcurrentDelete2() {
	const JOB_COUNT = 10;
	const SET_COUNT = 100;

	let map = new CHMap();

	let deleteJobs: Job<FixedArray<int>>[] = [];
	let deleteF: ()=>FixedArray<int> = () => {
		let arr: FixedArray<int> = new FixedArray<int>(SET_COUNT);
		for (let i = 0; i < SET_COUNT; ++ i)
			if(map.delete(i))
				arr[i] = 1;
		return arr;
	}

	for (let i = 0; i < SET_COUNT; ++ i)
		map.set(i, i);
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(deleteJobs, deleteF);
	let arrs = awaitAll(deleteJobs);
	let accum = arrs[0];
	for (let i = 1; i < JOB_COUNT; ++ i )
		sumArrInto(accum, arrs[i]);
	assertEQAll(accum, 1);
}

function testConcurrentDeleteSetAndHas() {
	const JOB_COUNT = 5;
	const SET_COUNT = 70;

	let map = new CHMap();
	let setF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.set(i, i);
		}
	}
	let deleteF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.delete(i);
		}
	}
	let hasF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.has(i);
		}
	}
	let setJobs: Job<void>[] = [];
	let deleteJobs: Job<void>[] = [];
	let hasJobs: Job<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(setJobs, setF);
		addLaunch(deleteJobs, deleteF);
		addLaunch(hasJobs, hasF);
	}
	awaitAll(setJobs);
	awaitAll(deleteJobs);
	awaitAll(hasJobs);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addTest("testConcurrentDelete", testConcurrentDelete);
	CHMTestSuitecase.addTest("testConcurrentDelete2", testConcurrentDelete2);
	CHMTestSuitecase.addTest("testConcurrentDeleteSetAndHas", testConcurrentDeleteSetAndHas);

    return CHMTestSuitecase.run();
}

