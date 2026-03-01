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
const { init, triggerXGC } = require('./mark_test_utils.js');
let gEtsVm;
let gWeakRef1;
let gWeakRef2;
let gWeakRef3;

/**
 * Create weak reference objects
 * weak1 -> js pobj -> sts obj
 * weak2 -> js obj <- sts pobj <- sts obj
 */
function createWeakRefObject1() {
    const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
    let obj1 = createStsObject(false, false);
    gWeakRef1 = new WeakRef(obj1);

    let obj2 = Promise.resolve();
    const proxyJsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObject');
    proxyJsObject(obj2, false, false);
    gWeakRef2 = new WeakRef(obj2);
}

/** 
 * Create weak reference objects
 *  weak1 -> js pobj -> sts obj
 *              ^
 *              |
 *  weak2 ->  js obj 
 */
function createWeakRefObject2() {
    const createStsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createStsObject');
    let obj1 = createStsObject(false, false);
    let obj2 = Promise.resolve();
    obj2.ref = obj1;
    gWeakRef1 = new WeakRef(obj1);
    gWeakRef2 = new WeakRef(obj2);
}

/** 
 * Create weak reference objects
 *  weak1 -> js obj
 *             ^
 *             |
 *  weak2 -> js obj <- sts pobj <- sts obj
 *             ^
 *             |
 *  weak3 -> js obj 
 */
function createWeakRefObject3() {
    let obj1 = Promise.resolve();
    let obj2 = Promise.resolve();
    let obj3 = Promise.resolve();

    const proxyJsObject = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'proxyJsObject');
    proxyJsObject(obj2, false, false);
    obj2.ref = obj1;
    obj3.ref = obj2;
    gWeakRef1 = new WeakRef(obj1);
    gWeakRef2 = new WeakRef(obj2);
    gWeakRef3 = new WeakRef(obj3);
}

// Deselect the weak reference object as the root object
function cancelWeakRootObject() {
    ArkTools.GC.clearWeakRefForTest();
}

// Trigger js full GC
function triggerJsFullGC() {
    ArkTools.GC.waitForFinishGC(ArkTools.GC.startGC('full'));
}

/**
 * Before XGC, cross-reference garbage objects cannot be recycled
 */
function jsGCTest1() {
    createWeakRefObject1();
    cancelWeakRootObject();
    triggerJsFullGC();
    
    if (!gWeakRef1.deref() || !gWeakRef2.deref()) {
        throw new Error('JS GC cannot collect cross-reference table objects!');
    }
    print('The jsGCTest1 method executes successful');
}

/**
 * Before XGC, cross-reference garbage objects cannot be recycled,
 * the normal garbage object should be recycled
 */
function jsGCTest2() {
    createWeakRefObject2();
    cancelWeakRootObject();
    triggerJsFullGC();
    
    if (!gWeakRef1.deref()) {
        throw new Error('JS GC cannot collect cross-reference table objects!');
    }
    if (gWeakRef2.deref()) {
        throw new Error('JS GC should collect garbage object!');
    }
    print('The jsGCTest2 method executes successful');
}

/**
 * Before XGC, cross-reference garbage objects cannot be recycled,
 * the normal objects that are referenced by cross-reference garbage objects cannot be recycled
 */
function jsGCTest3() {
    createWeakRefObject3();
    cancelWeakRootObject();
    triggerJsFullGC();
    
    if (!gWeakRef1.deref()) {
        throw new Error('JS GC cannot collect the objects that are referenced by cross-referenced objects!');
        
    }
    if (!gWeakRef2.deref()) {
        throw new Error('JS GC cannot collect cross-reference table objects!');
        
    }
    if (gWeakRef3.deref()) {
        throw new Error('JS GC should collect garbage object!');
    }
    print('The jsGCTest3 method executes successful');
}

/**
 * After XGC, garbage objects should be recycled
 */
function jsGCTest4() {
    createWeakRefObject1();
    cancelWeakRootObject();
    triggerXGC();
    triggerJsFullGC();

    if (gWeakRef1.deref() || gWeakRef2.deref()) {
        throw new Error('JS GC should collect cross-reference table objects that are dereferenced!');
    }
    print('The jsGCTest4 method executes successful');
}

/**
 * After XGC, garbage objects should be recycled
 */
function jsGCTest5() {
    createWeakRefObject2();
    cancelWeakRootObject();
    triggerXGC();
    triggerJsFullGC();
    
    if (gWeakRef1.deref()) {
        throw new Error('JS GC should collect cross-reference table objects that are dereferenced!');
    }
    if (gWeakRef2.deref()) {
        throw new Error('JS GC should collect garbage object!');
    }
    print('The jsGCTest5 method executes successful');
}

/**
 * After XGC, garbage objects should be recycled
 */
function jsGCTest6() {
    createWeakRefObject3();
    cancelWeakRootObject();
    triggerXGC();
    triggerJsFullGC();
    
    if (gWeakRef1.deref()) {
        throw new Error('JS GC should collect the objects that are referenced by cross-referenced objects!');
    }
    if (gWeakRef2.deref()) {
        throw new Error('JS GC should collect cross-reference table objects that are dereferenced!');
    }
    if (gWeakRef3.deref()) {
        throw new Error('JS GC should collect garbage object!');
    }
    print('The jsGCTest6 method executes successful');
}

/**
 * Before XGC, cross-reference garbage objects cannot be recycled
 */
function callStsGCTest1() {
    const createWeakRefObject1 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createWeakRefObject1');
    createWeakRefObject1(Promise.resolve());
    const stsGCTest1 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'stsGCTest1');
    stsGCTest1();
    print('The callStsGCTest1 method executes successful');
}

/**
 * Before XGC, cross-reference garbage objects cannot be recycled,
 * the normal garbage object should be recycled
 */
function callStsGCTest2() {
    const createWeakRefObject2 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createWeakRefObject2');
    createWeakRefObject2(Promise.resolve());
    const stsGCTest2 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'stsGCTest2');
    stsGCTest2();
    print('The callStsGCTest2 method executes successful');
}

/**
 * Before XGC, cross-reference garbage objects cannot be recycled,
 * the normal objects that are referenced by cross-reference garbage objects cannot be recycled
 */
function callStsGCTest3() {
    const createWeakRefObject3 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createWeakRefObject3');
    createWeakRefObject3();
    const stsGCTest3 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'stsGCTest3');
    stsGCTest3();
    print('The callStsGCTest3 method executes successful');
}

/**
 * After XGC, garbage objects should be recycled
 */
function callStsGCTest4() {
    const createWeakRefObject1 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createWeakRefObject1');
    createWeakRefObject1(Promise.resolve());
    const stsGCTest4 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'stsGCTest4');
    stsGCTest4();
    print('The callStsGCTest4 method executes successful');
}

/**
 * After XGC, garbage objects should be recycled
 */
function callStsGCTest5() {
    const createWeakRefObject2 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createWeakRefObject2');
    createWeakRefObject2(Promise.resolve());
    const stsGCTest5 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'stsGCTest5');
    stsGCTest5();
    print('The callStsGCTest5 method executes successful');
}

/**
 * After XGC, garbage objects should be recycled
 */
function callStsGCTest6() {
    const createWeakRefObject3 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'createWeakRefObject3');
    createWeakRefObject3();
    const stsGCTest6 = gEtsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'stsGCTest6');
    stsGCTest6();
    print('The callStsGCTest6 method executes successful');
}

gEtsVm = init('mark_test_gc_module', 'xgc_tests.abc');

jsGCTest1();
jsGCTest2();
jsGCTest3();
jsGCTest4();
jsGCTest5();
jsGCTest6();

callStsGCTest1();
callStsGCTest2();
callStsGCTest3();
callStsGCTest4();
callStsGCTest5();
callStsGCTest6();
