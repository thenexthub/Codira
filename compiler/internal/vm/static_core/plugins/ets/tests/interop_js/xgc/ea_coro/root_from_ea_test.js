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
+-----------------------------------------------------+
|             Main Thread           |    EA Worker    |
|-----------------------------------|-----------------|
|       JS        |       ETS       |       JS        |
|-----------------|-----------------|-----------------|
|                 |     WeakRef     |                 |
|                 |        |        |                 |
|                 |        v        |                 |
|     WeakRef     |       so1  <----+----- jo2 (root) |
|        |        |        |        |                 |
|        v        |        v        |                 |
|       jo1  <----+-----  so2       |                 |
|                 |                 |                 |
+-----------------------------------------------------+
                jo1 alives from jo2 object
*/

const testRunner = require('./ea_test_runner.js');

class SomeRef {
    constructor(str) {
        this.str = str;
    }
}

function createWeakRefToJs(etsVm) {
    let jo1 = new SomeRef('string');
    const setRefToSharedObject = etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'setRefToSharedObject');
    setRefToSharedObject(0, jo1);
    return new WeakRef(jo1);
}

function test(etsVm, testModule) {
    // Wait until JS runtime for EA worker created and EA Worker get ETS object
    testModule.waitForEAWorkers();
    testRunner.runEtsFullGC(etsVm);
    // Before: so1 -> undefined
    // Create refs: so1 (SharedClass class) -> so2 (JSValue class) -> jo1 (SomeRef class)
    let wrReachableFromEA = createWeakRefToJs(etsVm);
    // Run XGC
    testRunner.runXGC(etsVm);
    testRunner.runEtsFullGC(etsVm);
    // Notify EA Worker about XGC Finish
    testModule.fireEventToEAWorkers();
    // Run JS Full GC: try to clear WeakRef
    testRunner.runJsFullGC();
    if (wrReachableFromEA.deref() === undefined) {
        throw new Error('Ref from shared object field is collected');
    }
}

testRunner.runTest('root_from_ea_test_module', 'xgc_ea.abc', test, 'root_from_ea_worker');
