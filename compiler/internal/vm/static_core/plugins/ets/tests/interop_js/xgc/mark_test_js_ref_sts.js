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
const { init, triggerXGC, checkXRefsNumber, validationXGCResult } = require('./mark_test_utils.js');

let gObj = Promise.resolve();
let gInlineObj = Promise.resolve();
let gEtsVm;

// clear the cross-reference objects that are referenced by the active objects
function clearActiveRef() {
    gObj = Promise.resolve();
    gInlineObj = Promise.resolve();
    const clearActiveRef = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'clearActiveRef');
    clearActiveRef();
}

function clearRefStorage() {
    clearActiveRef();
    triggerXGC();
    checkXRefsNumber(0, 0);
}

/**
 * Create js object
 * @param isRootRef1  A root object references return object
 * @param isRootRef2  Return object references the a root object
 */
function createJsObject(isRootRef1, isRootRef2) {
    let obj = Promise.resolve();
    if (isRootRef1) {
        gObj.ref = obj;
    }
    if (isRootRef2) {
        obj.ref = gInlineObj;
    }
    return obj;
}

/**
 * Proxy sts object test
 * The reference table is not reclaimed only if isRootRef1 is true
 *          root1 -> js obj  -> js pobj -> sts obj <- root3
 *                    |                       |
 *                    v                       v
 *                  root2                   root4
 */
function proxyStsObjectTest(isRootRef1, isRootRef2, isRootRef3, isRootRef4) {
    let obj = createJsObject(isRootRef1, isRootRef2);
    const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
    obj.p = createStsObject(isRootRef3, isRootRef4);
}

gEtsVm = init('mark_test_js_ref_sts_module', 'xgc_tests.abc');

proxyStsObjectTest(false, false, false, false);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(false, false, false, true);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(false, false, true, false);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(false, false, true, true);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(false, true, false, false);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(false, true, false, true);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(false, true, true, false);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(false, true, true, true);
validationXGCResult(0, 1, 0, 0);

proxyStsObjectTest(true, false, false, false);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

proxyStsObjectTest(true, false, false, true);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

proxyStsObjectTest(true, false, true, false);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

proxyStsObjectTest(true, false, true, true);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

proxyStsObjectTest(true, true, false, false);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

proxyStsObjectTest(true, true, false, true);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

proxyStsObjectTest(true, true, true, false);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

proxyStsObjectTest(true, true, true, true);
validationXGCResult(0, 1, 0, 1);
clearRefStorage();

print('mark_test_js_ref_sts executes sucessful');
