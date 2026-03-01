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

let gArray = new Array();
let gEtsVm;

// clear the cross-reference objects that are referenced by the active objects
function clearActiveRef() {
    gArray = new Array();
    const clearActiveRef = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'clearActiveRef');
    clearActiveRef();
}

function clearRefStorage() {
    clearActiveRef();
    triggerXGC();
    checkXRefsNumber(0, 0);
}

/**
 * Create proxy objects on the sts side
 * js obj <- sts pobj <- sts obj
 * js obj <- sts pobj <- sts obj <- root
 */
function sweepTest01(num) {
    const proxyJsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObject');
    for (let i = 0; i < num; i++) {
        proxyJsObject(Promise.resolve(), false, false);
        proxyJsObject(Promise.resolve(), true, false);
    }
}

/**
 * Create proxy objects on the js side
 * js obj -> js pobj  -> sts obj
 * root   -> js obj   -> js pobj -> sts obj
 */
function sweepTest02(num) {
    const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
    for (let i = 0; i < num; i++) {
        createStsObject(false, false);
        gArray.push(createStsObject(false, false));
    }
}

/**
 * Create proxy objects on the sts side and js side
 * js obj <- sts pobj <- sts obj
 * js obj <- sts pobj <- sts obj <- root
 * js obj -> js pobj  -> sts obj
 * root   -> js obj   -> js pobj -> sts obj
 */
function sweepTest03(num) {
    const proxyJsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObject');
    const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
    for (let i = 0; i < num; i++) {
        proxyJsObject(Promise.resolve(), false, false);
        proxyJsObject(Promise.resolve(), true, false);
        createStsObject(false, false);
        gArray.push(createStsObject(false, false));
    }
}

/**
 * Create proxy objects on the js side
 * root3 -> sts obj <- js pobj <- js obj <- root2
 *             |
 *             v
 *            ...
 *             |
 *             v
 *         sts obj -> sts pobj -> js obj <- root1
 */
function sweepRecursiveTest01(num, isRootRef1, isRootRef2, isRootRef3) {
    let obj1 = Promise.resolve();
    let obj2 = Promise.resolve();
    const proxyJsObjectWithReturnValue = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObjectWithReturnValue');
    obj2.ref = proxyJsObjectWithReturnValue(obj1, num, isRootRef3, false);
    if (isRootRef1) {
        gArray.push(obj1);
    }
    if (isRootRef2) {
        gArray.push(obj2);
    }
}

function createRecursiveJsObject(num) {
    let obj = Promise.resolve();
    let temp = obj;
    for (let i = 0; i < num - 1; i++) {
        temp.ref = Promise.resolve();
        temp = temp.ref;
    }
    return {head: obj, tail: temp};
}

/**
 * Create proxy objects on the sts side
 * sts obj -> sts pobj -> js obj -> ... -> js obj -> js pobj -> sts obj
 * root3 -> js obj <- sts pobj <- sts obj <- root2
 *             |
 *             v
 *            ...
 *             |
 *             v
 *          js obj -> js pobj -> sts obj <- root1
 */
function sweepRecursiveTest02(num, isRootRef1, isRootRef2, isRootRef3) {
    let ret = createRecursiveJsObject(num);
    const proxyJsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObject');
    proxyJsObject(ret.head, isRootRef2, false);
    const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
    ret.tail.ref = createStsObject(isRootRef1, false);
    if (isRootRef3) {
        gArray.push(ret.head);
    }
}

gEtsVm = init('mark_test_mass_ref_module', 'xgc_tests.abc');

let num = 200;
sweepTest01(num);
validationXGCResult(num * 2, 0, num, 0);
clearRefStorage();

sweepTest02(num);
validationXGCResult(0, num * 2, 0, num);
clearRefStorage();

sweepTest03(num);
validationXGCResult(num * 2, num * 2, num, num);
clearRefStorage();

sweepRecursiveTest01(num, false, false, false);
validationXGCResult(1, 1, 0, 0);

sweepRecursiveTest01(num, true, false, false);
validationXGCResult(1, 1, 0, 0);
clearRefStorage();

sweepRecursiveTest01(num, false, true, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

sweepRecursiveTest01(num, false, false, true);
validationXGCResult(1, 1, 1, 0);
clearRefStorage();

sweepRecursiveTest02(num, false, false, false);
validationXGCResult(1, 1, 0, 0);

sweepRecursiveTest02(num, true, false, false);
validationXGCResult(1, 1, 0, 0);

sweepRecursiveTest02(num, false, true, false);
validationXGCResult(1, 1, 1, 1);
clearRefStorage();

sweepRecursiveTest02(num, false, false, true);
validationXGCResult(1, 1, 0, 1);
clearRefStorage();

print('mark_test_mass_ref executes sucessful');
