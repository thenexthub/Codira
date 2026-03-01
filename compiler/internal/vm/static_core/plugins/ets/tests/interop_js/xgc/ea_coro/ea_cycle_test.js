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

/*
so - shared object
jv - js value
+------------------------------------------------------------+
|                 Main Thread             |    EA Worker     |
|-----------------------------------------|------------------|
|       JS        |          ETS          |        JS        |
|-----------------|-----------------------|------------------|
|    j10 (root/   |   WeakRef             |                  |
|     |   dead)   |      |                |                  |
|     v           |      v                |                  |
|    j11 ---------+---> so1 ----> jv1 ----+------> j21       |
|     ^           |                       |         |        |
|     |           |                       |         v        |
|    j12 <--------+---- jv2 <---- so3 <---+------- j22       |
|                 |                ^      |                  |
|                 |                |      |                  |
|                 |             WeakRef   |                  |
+------------------------------------------------------------+
*/

const testRunner = require('./ea_test_runner.js');

class SomeRef {
    constructor(obj) {
        this.obj = obj;
    }
}

function createCyclePartForMain(getSharedObject, setRefToSharedObject) {
    // Create Ref: j11 -> so1
    let j11 = getSharedObject(1);
    // Create ref: j12 -> j11
    let j12 = new SomeRef(j11);
    // Create refs: so3 -> jv2 -> j12
    setRefToSharedObject(3, j12);
    // Create Ref: j10 -> j11
    return new SomeRef(j11);
}

function createCycleAndCheckLiveCycle(etsVm, getSharedObject, setRefToSharedObject) {
    let j10 = createCyclePartForMain(getSharedObject, setRefToSharedObject);
    // Run XGC
    testRunner.runXGC(etsVm);
    testRunner.runEtsFullGC(etsVm);
    // Checks for live cycle
    if (j10.obj === undefined) {
        throw new Error('j11 is nullish');
    }
    if (getSharedObject(1) === undefined) {
        throw new Error('so1 is collected, but should be live in live cycle');
    }
    if (getSharedObject(3) === undefined) {
        throw new Error('so1 is collected, but should be live in live cycle');
    }
}

function test(etsVm, testModule) {
    // Get ETS functions
    const getSharedObject = etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'getSharedObject');
    const setRefToSharedObject = etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'setRefToSharedObject');
    // Wait until JS runtime for EA worker created and EA Worker created part of cycle
    testModule.waitForEAWorkers();
    // Create live cycle
    createCycleAndCheckLiveCycle(etsVm, getSharedObject, setRefToSharedObject);
    // Run XGC. j10 is dead, so cycle should be removed
    testRunner.runXGC(etsVm);
    testRunner.runEtsFullGC(etsVm);
    // Notify EA Worker about XGC Finish
    testModule.fireEventToEAWorkers();
    // Checks after GCs
    if (getSharedObject(1) !== undefined) {
        throw new Error('so1 is not collected, but cycle is dead');
    }
    if (getSharedObject(3) !== undefined) {
        throw new Error('so3 is not collected, but cycle is dead');
    }
}

testRunner.runTest('ea_cycle_test_module', 'xgc_ea.abc', test, 'ea_cycle_worker');
