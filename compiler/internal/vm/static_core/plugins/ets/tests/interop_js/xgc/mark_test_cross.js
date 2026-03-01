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
let gRedundantNum = 0;
let gEtsVm;

class Result {
    constructor() {
        this.headJsObj = null;
        this.tailJsObj = null;
        this.beforeJsNum = 0;
        this.beforeStsNum = 0;
        this.afterJsNum = 0;
        this.afterStsNum = 0;
    }

    addResult = (tr) => {
        if (this.tailJsObj == null) {
            throw new Error('The tail object cannot be empty!');
        }
        if (tr.headJsObj == null) {
            return this;
        }
        this.tailJsObj.ref = tr.headJsObj;
        this.beforeJsNum += tr.beforeJsNum;
        this.beforeStsNum += tr.beforeStsNum;
        this.afterJsNum += tr.afterJsNum;
        this.afterStsNum += tr.afterStsNum;
        return this;
    };

    clearObjectRef = () => {
        this.headJsObj = null;
        this.tailJsObj = null;
        return this;
    };
}

function clearActiveRef() {
    gObj = Promise.resolve();
    const clearActiveRef = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'clearActiveRef');
    clearActiveRef();
}

function clearRefStorage() {
    clearActiveRef();
    triggerXGC();
    checkXRefsNumber(0, 0);
}

function createRecursiveJsObject(num) {
    let head = Promise.resolve();
    let tail = head;
    for (let i = 0; i < num; i++) {
        tail.ref = Promise.resolve();
        tail = tail.ref;
    }
    let res = new Result();
    res.headJsObj = head;
    res.tailJsObj = tail;
    return res;
}

/**
 * Create a cross-reference intermediate node
 * js obj -> js pobj -> sts obj -> sts pobj -> js obj
 */
function createCrossRefNode(obj, isRootRef2) {
    let res = createRecursiveJsObject(gRedundantNum);
    const proxyJsObjectWithReturnValue = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObjectWithReturnValue');
    obj.ref = proxyJsObjectWithReturnValue(res.headJsObj, gRedundantNum, isRootRef2, false);
    return res.tailJsObj;
}

/**
 * Create a cross-referenced linked list
 * js obj -> js pobj -> sts obj -> sts pobj -> js obj -> js pobj -> sts obj -> sts pobj -> js obj
 *    ^                    ^
 *    |                    |
 *  root1                root2
 * @param crossNum   Number of cross-references
 * @param isRootRef1 The first js object is referenced by the root object
 * @param isRootRef2 The first sts object is referenced by the root object
 */
function createCrossRef(crossNum, isRootRef1, isRootRef2) {
    let result = new Result();
    if (crossNum < 1) {
        return result;
    }
    let head = Promise.resolve();
    if (isRootRef1) {
        gObj.ref = head;
    }
    let temp = head;
    for (let i = 0; i < crossNum; i++) {
        temp = createCrossRefNode(temp, isRootRef2);
        result.beforeJsNum++;
        result.beforeStsNum++;
        if (isRootRef1) {
            result.afterJsNum++;
            result.afterStsNum++;
            isRootRef2 = false;
        } else if (isRootRef2) {
            result.afterJsNum++;
            isRootRef2 = false;
            isRootRef1 = true;
        }
    }
    result.headJsObj = head;
    result.tailJsObj = temp;
    return result;
}

/**
 * Cross reference test
 * js obj -> js pobj -> sts obj -> sts pobj -> js obj -> js pobj -> sts obj -> sts pobj -> js obj
 *    ^                    ^
 *    |                    |
 *  root1                root2
 */
function createCrossRefTest1(num, isRootRef1, isRootRef2) {
    let ret = createCrossRef(num, isRootRef1, isRootRef2);
    ret.clearObjectRef();
    return ret;
}

/**
 * Test that half of the reference objects are active
 * js obj -> js pobj -> sts obj -> sts pobj -> js obj -> js pobj -> sts obj -> sts pobj -> js obj
 *                                               ^                      ^
 *                                               |                      |
 *                                             root1                  root2
 */
function createCrossRefTest2(num, isRootRef1, isRootRef2) {
    let count = num / 2;
    let res1 = createCrossRef(count, false, false);
    count = num - count;
    let res2 = createCrossRef(count, isRootRef1, isRootRef2);

    res1.addResult(res2);
    res1.clearObjectRef();

    return res1;
}

// Initialize the runtime
gEtsVm = init('mark_test_cross_module', 'xgc_tests.abc');

let res = createCrossRefTest1(10, false, false);
validationXGCResult(res.beforeJsNum, res.beforeStsNum, res.afterJsNum, res.afterStsNum);

res = createCrossRefTest1(10, true, false);
validationXGCResult(res.beforeJsNum, res.beforeStsNum, res.afterJsNum, res.afterStsNum);
clearRefStorage();

res = createCrossRefTest1(10, false, true);
validationXGCResult(res.beforeJsNum, res.beforeStsNum, res.afterJsNum, res.afterStsNum);
clearRefStorage();

res = createCrossRefTest2(10, true, false);
validationXGCResult(res.beforeJsNum, res.beforeStsNum, res.afterJsNum, res.afterStsNum);
clearRefStorage();

res = createCrossRefTest2(10, false, true);
validationXGCResult(res.beforeJsNum, res.beforeStsNum, res.afterJsNum, res.afterStsNum);
clearRefStorage();

print('mark_test_cross executes successful');
