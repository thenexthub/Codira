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
+------------------------------------------------------+
|             Main Thread           |    EA Worker     |
|-----------------------------------|------------------|
|       JS        |       ETS       |        JS        |
|-----------------|-----------------|------------------|
|                 |     WeakRef     |                  |
|                 |        |        |                  |
|                 |        v        |                  |
|    j11 (root) --+-----> so1  <----+------ j21 (root) |
|                 |                 |                  |
|                 |     WeakRef     |                  |
|                 |        |        |                  |
|                 |        v        |                  |
|    j12 (root) --+-----> so2  <----+------ j22 (dead) |
|                 |                 |                  |
|    WeakRef      |     WeakRef     |                  |
|       |         |        |        |                  |
|       v         |        v        |                  |
|      j13 -------+-----> so3  <----+------ j23 (root) |
|                 |                 |                  |
|    WeakRef      |     WeakRef     |                  |
|       |         |        |        |                  |
|       v         |        v        |                  |
|      j14 -------+-----> so4  <----+------ j24 (dead) |
+------------------------------------------------------+
Checks:
                  so1 should be alived
           so2 should be alived, j22 should be collected
           so3 should be alived, j13 should be collected
so4 should be collected, j14 should be collected, j24 should be collected
*/

const testRunner = require('./ea_test_runner.js');

function newWeakRefForEtsObject(getSharedObject, index) {
    return new WeakRef(getSharedObject(index));
}

function test(etsVm, testModule) {
    // Get ETS functions
    const getSharedObject = etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'getSharedObject');
    // Wait until JS runtime for EA worker created and EA Worker got ETS objects
    testModule.waitForEAWorkers();
    // Run ETS Full GC, and check that given objects are not collected, because XGC was not run
    testRunner.runEtsFullGC(etsVm);
    let j11 = getSharedObject(1);
    if (j11 === undefined) {
        throw new Error('ETS object for j11 is undefined');
    }
    let j12 = getSharedObject(2);
    if (j12 === undefined) {
        throw new Error('ETS object for j12 is undefined');
    }
    let wrJ13 = newWeakRefForEtsObject(getSharedObject, 3);
    if (wrJ13.deref() === undefined) {
        throw new Error('ETS object for j13 is undefined');
    }
    let wrJ14 = newWeakRefForEtsObject(getSharedObject, 4);
    if (wrJ14.deref() === undefined) {
        throw new Error('ETS object for j14 is undefined');
    }
    // Run XGC
    testRunner.runXGC(etsVm);
    testRunner.runEtsFullGC(etsVm);
    // Notify EA Worker about XGC Finish
    testModule.fireEventToEAWorkers();
    // Run JS Full GC: try to clear WeakRef
    testRunner.runJsFullGC();
    // Checks after GCs
    if (j11.jsRef !== undefined) {
        throw new Error('Ref from ETS object so1 is not undefined');
    }
    if (getSharedObject(1) === undefined) {
        throw new Error('ETS object so1 is collected, but reachable from Main and EA workers');
    }
    if (j12.jsRef !== undefined) {
        throw new Error('Ref from ETS object so2 is not undefined');
    }
    if (getSharedObject(2) === undefined) {
        throw new Error('ETS object so2 is collected, but reachable from Main worker');
    }
    if (wrJ13.deref() !== undefined) {
        throw new Error('j13 is not collected, but it is not reachable more via a strong ref');
    }
    if (getSharedObject(3) === undefined) {
        throw new Error('ETS object so3 is collected, but reachable from EA worker');
    }
    if (wrJ14.deref() !== undefined) {
        throw new Error('j14 is not collected, but it is not reachable more via a strong ref');
    }
    if (getSharedObject(4) !== undefined) {
        throw new Error('ETS object so4 is not collected, but it is not reachable from any worker');
    }
}

testRunner.runTest('ea_shared_objects_test_module', 'xgc_ea.abc', test, 'ea_shared_objects_worker');
