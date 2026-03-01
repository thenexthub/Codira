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


function testTooManyAdd(constr: BQConstructorI) {
	let queue = constr(10);
	for (let i = 0; i < 20; ++ i)
		arktest.assertTrue(queue.add(i) == (i < 10));
	for (let i = 0; i < 10; ++ i)
		arktest.assertEQ(queue.pop(), i);
	arktest.assertTrue(queue.isEmpty());
}

function testTooManyPoll(constr: BQConstructorI) {
	let queue = constr(10);
	for (let i = 0; i < 10; ++ i)
		arktest.assertTrue(queue.add(i));
	for (let i = 0; i < 10; ++ i)
		arktest.assertEQ(queue.poll(), i);
	for (let i = 0; i < 10; ++ i)
		arktest.assertEQ(queue.poll(), undefined);
}

function testGetFirstNoConcurent(constr: BQConstructorI) {
	let queue = constr(50);
	queue.push(10);
	arktest.assertEQ(queue.getFirst(), 10);
	arktest.assertEQ(queue.getFirst(), 10);
	queue.push(20);
	arktest.assertEQ(queue.getFirst(), 10);
	queue.pop();
	arktest.assertEQ(queue.getFirst(), 20);
	queue.pop();
	arktest.assertEQ(queue.getFirst(), undefined);
}

function testGetEndNoConcurent(constr: BQConstructorI) {
	let queue = constr(50);
	let queueBQ = (queue as BlockingQueue);
	queue.push(10);
	arktest.assertEQ(queueBQ.getEnd(), 10);
	arktest.assertEQ(queueBQ.getEnd(), 10);
	queue.push(20);
	arktest.assertEQ(queueBQ.getEnd(), 20);
	queue.pop();
	arktest.assertEQ(queueBQ.getEnd(), 20);
	queue.pop();
	arktest.assertEQ(queueBQ.getEnd(), undefined);
}

function testIsEmptyNoConcurrent(constr: BQConstructorI) {
	let queue = constr(10);
	arktest.assertTrue(queue.isEmpty());
	queue.push(10);
	arktest.assertFalse(queue.isEmpty());
	queue.push(10);
	arktest.assertFalse(queue.isEmpty());
	queue.pop();
	arktest.assertFalse(queue.isEmpty());
	queue.pop();
	arktest.assertTrue(queue.isEmpty());
	queue.push(10);
	arktest.assertFalse(queue.isEmpty());
	queue.pop();
	arktest.assertTrue(queue.isEmpty());
}

function testSizeNoConcurrent(constr: BQConstructorI) {
	let queue = constr(5);
	arktest.assertEQ(queue.size, 0);
	queue.push(1);
	arktest.assertEQ(queue.size, 1);
	queue.push(1);
	queue.push(1);
	arktest.assertEQ(queue.size, 3);
	queue.pop();
	arktest.assertEQ(queue.size, 2);
	queue.push(1);
	queue.push(1);
	arktest.assertEQ(queue.size, 4);
	queue.pop();
	queue.pop();
	queue.pop();
	queue.pop();
	arktest.assertEQ(queue.size, 0);
	queue.poll();
	queue.poll();
	arktest.assertEQ(queue.size, 0);
}

function testSizeNoConcurrent2(constr: BQConstructorI) {
	let queue = constr(100);
	for (let i = 0; i < 100; ++ i) {
		queue.push(i);
		arktest.assertEQ(queue.size, i + 1);
	}
	for (let i = 100; i > 0; -- i) {
		arktest.assertEQ(queue.size, i);
		queue.pop();
	}
	arktest.assertEQ(queue.size, 0);
}

function testCapacityNoConcurrent(constr: BQConstructorI) {
	for (let i = 1; i < 10; ++ i) {
		let queue = constr(i * 10);
		arktest.assertEQ(queue.capacity, i * 10);
	}
}

function testRemainingCapacityNoC(constr: BQConstructorI) {
	let queue = constr(10);
	for (let i = 0; i < 10; ++ i) {
		arktest.assertEQ(queue.remainingCapacity(), 10 - i);
		queue.push(i);
	}
	for (let i = 0; i < 10; ++ i) {
		arktest.assertEQ(queue.remainingCapacity(), i);
		queue.pop();
	}
	arktest.assertEQ(queue.remainingCapacity(), 10);
}

function main(): int {
	let tests: Array<[String, TestFunction]> = [
		["testTooManyAdd", testTooManyAdd],
		["testTooManyPoll", testTooManyPoll],
		["testGetFirstNoConcurent", testGetFirstNoConcurent],
		["testGetEndNoConcurent", testGetEndNoConcurent],
		["testIsEmptyNoConcurrent", testIsEmptyNoConcurrent],
		["testSizeNoConcurrent", testSizeNoConcurrent],
		["testSizeNoConcurrent2", testSizeNoConcurrent2],
		["testCapacityNoConcurrent", testCapacityNoConcurrent],
		["testRemainingCapacityNoC", testRemainingCapacityNoC]
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

