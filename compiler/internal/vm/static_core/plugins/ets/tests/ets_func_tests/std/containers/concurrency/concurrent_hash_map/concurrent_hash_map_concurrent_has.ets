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

function assertEQAll(array: FixedArray<int>, value: int): void {
	for (let i=0; i < array.length; ++ i)
		arktest.assertEQ(array[i], value)
}

function testConcurrentHas() {
	const JOB_COUNT = 7;
	const SET_COUNT = 25;

	let map = new CHMap();
	let hasJob: Job<void>[] = [];

	let makeHasF = (start: int, end: int, shouldBe: boolean) => () => {
		for (let i = start; i < end; ++ i) {
			while (map.has(i) != shouldBe) {
				Coroutine.Schedule();
			}
		}
	}

	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(hasJob, makeHasF(i * SET_COUNT, (i + 1) * SET_COUNT, false));
	awaitAll(hasJob);

	hasJob = [];
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(hasJob, makeHasF(i * SET_COUNT, (i + 1) * SET_COUNT, true));
	for (let i = 0; i < JOB_COUNT * SET_COUNT; ++ i)
		map.set(i, i);
	awaitAll(hasJob);

	hasJob = [];
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(hasJob, makeHasF(i * SET_COUNT, (i + 1) * SET_COUNT, false));
	for (let i = 0; i < JOB_COUNT * SET_COUNT; ++ i)
		map.delete(i);
	awaitAll(hasJob);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addTest("testConcurrentHas", testConcurrentHas);

    return CHMTestSuitecase.run();
}

