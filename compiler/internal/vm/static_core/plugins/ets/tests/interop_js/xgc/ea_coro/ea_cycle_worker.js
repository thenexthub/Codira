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

class SomeRef {
    constructor(obj) {
        this.obj = obj;
    }
}

function createCyclePartForEa(getSharedObject, setRefToSharedObject) {
    // Create ref: j22 -> so3
    let j22 = getSharedObject(3);
    // Create ref: j21 -> j22
    let j21 = new SomeRef(j22);
    // Create refs: so1 -> jv2 -> j21
    setRefToSharedObject(1, j21);
}

export function test() {
    // Load native modules
    let etsVm = requireNapiPreview('ets_interop_js_napi', true);
    let testModule = requireNapiPreview('ea_cycle_test_module', true);
    // Get ETS functions
    const getSharedObject = etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'getSharedObject');
    const setRefToSharedObject = etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'setRefToSharedObject');
    // Create cycle references part for EA Worker
    createCyclePartForEa(getSharedObject, setRefToSharedObject);
    // Notify main JS VM that EA cycle part is created
    testModule.signalMainWorker();
    // Wait until XGC Finish to avoid situation when cross reference will be removed on detach EA Worker before XGC
    testModule.waitEventFromMainWorker();
}
