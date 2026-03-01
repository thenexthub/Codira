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

type BlockingQueueI = containers.BlockingQueue<int>;
type BQConstructorI = (capacity: int)=>BlockingQueueI;
type TestFunction = (constr: BQConstructorI) => void;
type BlockingQueue = containers.LinkedBlockingQueue<int> | containers.ArrayBlockingQueue<int>;

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

function subArrInto(to: FixedArray<int>, from: FixedArray<int>): void {
	arktest.assertEQ(to.length, from.length);
	for (let i = 0; i < to.length; ++ i) {
		to[i] -= from[i];
	}
}

function assertEQAll(array: FixedArray<int>, value: int): void {
	for (let i=0; i < array.length; ++ i)
		arktest.assertEQ(array[i], value)
}

function testGetFirstConcurent(constr: BQConstructorI) {
	let queue = constr(50);
	const PUSH_COUNT = 100;
	const JOB_COUNT = 4;

	let pushAndPopF: ()=>void = ()=>{
		for (let i = 0; i < PUSH_COUNT; ++i ) {
			queue.push(i);
			queue.pop();
		}
	};

	let getF: ()=>void = ()=>{
		for (let i = 0; i < PUSH_COUNT; ++i ) {
			let val = queue.getFirst();
			arktest.assertTrue(i >= 0 && i < PUSH_COUNT);
		}
	}

	let setJobs = new Array<Job<void>>();
	let getJobs = new Array<Job<void>>();
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(setJobs, pushAndPopF);
		addLaunch(getJobs, getF);
	}
	awaitAll(setJobs);
	awaitAll(getJobs);
}

function testGetEndConcurent(constr: BQConstructorI) {
	let queue = constr(50);
	let queueBQ = queue as BlockingQueue;
	const PUSH_COUNT = 100;
	const JOB_COUNT = 4;

	let pushAndPopF: ()=>void = ()=>{
		for (let i = 0; i < PUSH_COUNT; ++i ) {
			queue.push(i);
			queue.pop();
		}
	};

	let getF: ()=>void = ()=>{
		for (let i = 0; i < PUSH_COUNT; ++i ) {
			let val = queueBQ.getEnd();
			arktest.assertTrue(i >= 0 && i < PUSH_COUNT);
		}
	}

	let setJobs = new Array<Job<void>>();
	let getJobs = new Array<Job<void>>();
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(setJobs, pushAndPopF);
		addLaunch(getJobs, getF);
	}
	awaitAll(setJobs);
	awaitAll(getJobs);
}

function main(): int {
	let tests: Array<[String, TestFunction]> = [
		["testGetFirstConcurent", testGetFirstConcurent],
		["testGetEndConcurent", testGetEndConcurent],
	];

    let LBQTestSuitecase = new arktest.ArkTestsuite("LinkedBlockingQueueTest")
    let ABQTestSuitecase = new arktest.ArkTestsuite("ArrayBlockingQueueTest")

	let LBQconstr: BQConstructorI = (capacity: int)=>new containers.LinkedBlockingQueue<int>(capacity);
	let ABQconstr: BQConstructorI = (capacity: int)=>new containers.ArrayBlockingQueue<int>(capacity);

	let suites: Array<[arktest.ArkTestsuite, BQConstructorI]> = [
		[LBQTestSuitecase, LBQconstr],
		[ABQTestSuitecase, ABQconstr],
	]

	for (let j = 0; j < suites.length; ++ j) {
		let arktestsuite = suites[j][0];
		let constr = suites[j][1];
		for (let i = 0; i < tests.length; ++ i) {
			let name = tests[i][0];
			let testF = tests[i][1];
			arktestsuite.addTest(name, ()=>testF(constr));
		}
	}

	let codeLBQ = LBQTestSuitecase.run();
	let codeABQ = ABQTestSuitecase.run();
	if (codeLBQ == 0)
		return codeABQ;
	return codeLBQ;
}

