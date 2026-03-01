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

const object_count = 20;
const promise_count = 30;

let token = new Object();
let registry: FinalizationRegistry<String> | null = null;

let obj_array: Object[] = new Object[object_count];


function doFullGC() {
	try {
		GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
	} catch (e) {
	 arktest.assertTrue(false, "Unexpected error during GC");
	}
}

async function doFullGCAsync() {
	try {
		GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
	} catch (e) {
	 arktest.assertTrue(false, "Unexpected error during GC");
	}
}


function addToRegistry(finreg: FinalizationRegistry<String> | null, count: Int, token: Object): void {

	for (let i = 0; i < count; i++) {
		finreg!.register(new Object(), "test", token);
	}
}

function removeFromRegistry(finreg: FinalizationRegistry<String> | null, token: Object): void {
	finreg!.unregister(token);
}

// ==============================  GetSize tests ==================  

let promise_array: Array<Job<void>> = new Array<Job<void>>;

function getSizeTests(finreg: FinalizationRegistry<String> | null) {

	for (let i = 0; i < promise_count; i++) {
		promise_array.push(launch<void, (a: FinalizationRegistry<String> | null, b: Int, c: Object) => void>(addToRegistry, finreg, 3, token));
	}

	for (let i = 0; i < promise_array.length; i++) {
		promise_array[i]!.Await();
	}

 arktest.assertEQ(3 * promise_count, finreg!.getSize(), "Size of the finreg is not true");
}

// ========================= Unregister tests =========================================== 

let unreg_array: Array<Job<void>> = new Array<Job<void>>;

function unregisterTests(finreg: FinalizationRegistry<String> | null) {

	finreg!.unregister(token);
 arktest.assertEQ(0, finreg!.getSize(), "Start size of unregister is invalid");

	let token1 = new Object();
	let token2 = new Object();
	let token3 = new Object();
	let token4 = new Object();

	addToRegistry(finreg, object_count, token1);
	addToRegistry(finreg, object_count, token2);
	addToRegistry(finreg, object_count, token3);
	addToRegistry(finreg, object_count, token4);

 arktest.assertEQ(object_count * 4, finreg!.getSize(), " Size after register of Unregister test is invalid");

	const unregister = (finreg: FinalizationRegistry<String>, s: Object) => {
		return finreg.unregister(s);
	};

	launch<void, (finreg: FinalizationRegistry<String>, token: Object) => void>(unregister, finreg!, token1).Await();
	launch<void, (finreg: FinalizationRegistry<String>, token: Object) => void>(unregister, finreg!, token2).Await();
	launch<void, (finreg: FinalizationRegistry<String>, token: Object) => void>(unregister, finreg!, token3).Await();
	launch<void, (finreg: FinalizationRegistry<String>, token: Object) => void>(unregister, finreg!, token4).Await();

 arktest.assertEQ(0, finreg!.getSize(), "Size of unregister is invalid");

	for (let i = 0; i < obj_array.length; i++) {
		addToRegistry(finreg!, object_count, obj_array[i]);
	}

 arktest.assertEQ(obj_array.length * object_count, finreg!.getSize(), "Register function is invalid");

	for (let i = 0; i < obj_array.length; i++) {
		unreg_array.push(launch<void, (a: FinalizationRegistry<String> | null, b: Object) => void>(removeFromRegistry, finreg!, obj_array[i]));
	}

	for (let i = 0; i < unreg_array.length; ++i) {
		unreg_array[i].Await();
	}
 arktest.assertEQ(0, finreg!.getSize(), "unregister  test failed");
}

// ===============================  CtorTests ==============================

let regs_promise: Array<Promise<FinalizationRegistry<String>>> = new Array<Promise<FinalizationRegistry<String>>>;;
let cntr = 0;


function createCoro(idx: int): FinalizationRegistry<String> {
	let finreg = new FinalizationRegistry<String>((string: String): void => { cntr++ });
	let o = new Object();
	finreg.register(o, "<<< :: >>> " + idx, o);
	return finreg;
}


async function createCoroA(idx: int, noToken: boolean): Promise<FinalizationRegistry<String>> {
    let finreg = new FinalizationRegistry<String>((s: String): void => { cntr++; });
	let o = new Object();
    if (noToken) {
        finreg.register(o, "<<< :Async1: >>> " + idx);
    } else {
        finreg.register(o, "<<< :Async2: >>> " + idx, o);
    }
	return finreg;
}


async function ctorTests() {

    for (let i = 0; i < promise_count; i++) {
        regs_promise.push(createCoroA(i, true));
	    regs_promise.push(createCoroA(i, false));
	}

    for (let i = 0; i < regs_promise.length; i++) {
		await regs_promise[i];
	}

	await doFullGCAsync();
	doFullGC();
	Coroutine.Schedule();

}

function main(): void {
    registry = new FinalizationRegistry<String>((string: String): void => { });
	getSizeTests(registry!);
	unregisterTests(registry!);

	waitForCompletion(ctorTests);
 arktest.assertEQ(cntr, promise_count * 2, "Counter of ctors is not valid");

}
