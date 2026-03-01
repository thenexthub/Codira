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

function getAndForgetEtsObject(getSharedObject, index) {
    let obj = getSharedObject(index);
    if (obj === undefined) {
        throw new Error('Ets object is undefined');
    }
}

export function test() {
    // Load native modules
    let etsVm = requireNapiPreview('ets_interop_js_napi', true);
    let testModule = requireNapiPreview('ea_shared_objects_test_module', true);
    // Get ETS functions
    const getSharedObject = etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'getSharedObject');
    // Get ETS objects
    let j21 = getSharedObject(1);
    if (j21 === undefined) {
        throw new Error('Ets object so1 for j21 is undefined');
    }
    getAndForgetEtsObject(getSharedObject, 2);
    let j23 = getSharedObject(3);
    if (j23 === undefined) {
        throw new Error('Ets object so3 for j23 is undefined');
    }
    getAndForgetEtsObject(getSharedObject, 4);
    // Notify main JS VM that ETS objects are gotten
    testModule.signalMainWorker();
    // Wait until XGC Finish
    testModule.waitEventFromMainWorker();
    if (j21.jsRef !== undefined) {
        throw new Error('Ref from ETS object so1 is not undefined');
    }
    if (j23.jsRef !== undefined) {
        throw new Error('Ref from ETS object so3 is not undefined');
    }
}
