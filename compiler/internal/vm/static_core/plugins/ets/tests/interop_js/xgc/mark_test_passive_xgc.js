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
const { init, triggerXGC, checkXRefsNumber } = require('./mark_test_utils.js');

let gThreasholdSize = 2048;
let gArray = new Array();
let gEtsVm;

// clear the cross-reference objects that are referenced by the active objects
function clearActiveRef() {
    gArray = new Array();
    const clearActiveRef = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'clearActiveRef');
    clearActiveRef();
}

/**
 * Test xgc passive trigger
 * js obj <- sts pobj <- sts obj
 * js obj <- sts pobj <- sts obj <- root
 * js obj -> js pobj  -> sts obj
 * root   -> js obj   -> js pobj -> sts obj
 */
function passiveXGCTest() {
    let jsNum = 0;
    let stsNum = 0;
    let jsNumAfter = 0;
    let stsNumAfter = 0;
    checkXRefsNumber(jsNum, stsNum);
    const proxyJsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObject');
    const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
    for (let i = 0; i < gThreasholdSize; i++) {
        let j = i % 4;
        switch (j) {
            case 0:
                proxyJsObject(Promise.resolve(), false, false);
                jsNum++;
                break;
            case 1:
                proxyJsObject(Promise.resolve(), true, false);
                jsNum++;
                jsNumAfter++;
                break;
            case 2:
                createStsObject(false, false);
                stsNum++;
                break;
            default:
                gArray.push(createStsObject(false, false));
                stsNum++;
                stsNumAfter++;
        }
    }
    checkXRefsNumber(jsNum, stsNum);
    return {jsNumAfter: jsNumAfter, stsNumAfter: stsNumAfter};
}

gEtsVm = init('mark_test_passive_xgc_module', 'xgc_tests.abc');

let res = passiveXGCTest();
// When the threshold is exceeded, the XGC is triggered
const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
createStsObject(false, false);

checkXRefsNumber(res.jsNumAfter, res.stsNumAfter + 1);
clearActiveRef();
triggerXGC();
checkXRefsNumber(0, 0);
print('The passiveXGCTest method executes successful');
