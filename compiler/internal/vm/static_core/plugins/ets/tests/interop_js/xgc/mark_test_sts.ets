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
class TestObject {
    ref: Object | null = null;
    p: Promise<Object> | null = null;
    num: number = 0;
    constructor() {}
}

let g_obj: TestObject = new TestObject();
let g_inlineObj: TestObject = new TestObject();
let g_array = new Array<Object>();
let g_weakRef1: WeakRef<Object>;
let g_weakRef2: WeakRef<Object>;
let g_weakRef3: WeakRef<Object>;

function xgc(): void {
    try {
        GC.waitForFinishGC(JSRuntime.xgc());
    } catch(e) {
        arktest.assertTrue(false, "Unexpected error during XGC");
    }
}

// Cancel the active objects reference cross-reference objects
function clearActiveRef(): void {
    g_obj = new TestObject();
    g_inlineObj = new TestObject();
    g_array = new Array<Object>();
}

/**
 * Create sts object
 * @param isRootRefStsObject   A root object references sts object
 * @param isStsObjectRefRoot   Sts object references the a root object
 */
function createStsObject(isRootRefStsObject: boolean, isStsObjectRefRoot: boolean): TestObject {
    let obj = new TestObject();
    if (isRootRefStsObject) {
        g_array.push(obj);
    }
    if (isStsObjectRefRoot) {
        obj.ref = g_inlineObj;
    }
    return obj;
}

// Proxy the js object on sts side
function proxyJsObject(p: Promise<Object>, isRootRefStsObject: boolean, isStsObjectRefRoot: boolean): void {
    let obj = createStsObject(isRootRefStsObject, isStsObjectRefRoot);
    obj.p = p;
}

function proxyJsObjectWithReturnValue(p: Promise<Object>, num: number, isRootRefStsObject: boolean,
                                        isStsObjectRefRoot: boolean): TestObject {
    let head = createStsObject(isRootRefStsObject, isStsObjectRefRoot);
    let tail = head;
    for (let i = 0; i < num; i++) {
        let to = new TestObject();
        tail.ref = to;
        tail = to;
    }
    tail.p = p;
    return head;
}

function createRecursiveStsObject(num: number): TestObject {
    let obj = new TestObject();
    let temp = obj;
    for (let i = 0; i < num - 1; i++) {
        let to = new TestObject();
        temp.ref = to;
        temp = to;
    }
    return obj;
}

/**
 * Circular reference
 * @param p                     sts proxy object
 * @param isRootRefStsObject    The sts ative object refers to the object in the ring
 * @param isRootInRing          A root object is in the ring
 */
function cycle(p: Promise<Object>, isRootRefStsObject: boolean, isRootInRing: boolean): Object {
    let obj = new TestObject();
    if (isRootInRing) {
        obj.ref = g_inlineObj;
        g_inlineObj.p = p;
    } else {
        obj.p = p;
    }
    if (isRootRefStsObject) {
        g_obj.ref = obj;
    }
    return obj;
}

// Triger sts full GC
function triggerFullGC(): void {
    try {
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
    } catch(e) {
        arktest.assertTrue(false, "Unexpected error during full GC");
    }
}

/**
 * Create weak reference objects
 * weak1 -> sts pobj -> js obj
 * weak2 -> sts obj <- js pobj
 */
function createWeakRefObject1(p: Promise<Object>): Object {
    g_weakRef1 = new WeakRef<Object>(p);
    let obj = new TestObject();
    g_weakRef2 = new WeakRef<Object>(obj);
    return obj;
}

/** 
 * Create weak reference objects
 *  weak1 -> sts pobj -> js obj
 *              ^
 *              |
 *  weak2 -> sts obj 
 */
function createWeakRefObject2(p: Promise<Object>): void {
    let obj = new TestObject();
    obj.p = p;
    g_weakRef1 = new WeakRef<Object>(p);
    g_weakRef2 = new WeakRef<Object>(obj);
}

/** 
 * Create weak reference objects
 *  weak1 -> sts obj
 *              ^
 *              |
 *  weak2 -> sts obj <- js pobj <- js obj
 *              ^
 *              |
 *  weak3 -> sts obj 
 */
function createWeakRefObject3(): Object {
    let obj1 = new TestObject();
    let obj2 = new TestObject();
    let obj3 = new TestObject();

    obj2.ref = obj1;
    obj3.ref = obj2;
    g_weakRef1 = new WeakRef<Object>(obj1);
    g_weakRef2 = new WeakRef<Object>(obj2);
    g_weakRef3 = new WeakRef<Object>(obj3);
    return obj2;
}

function stsGCTest1(): void {
    triggerFullGC();
    if ((g_weakRef1 != null && g_weakRef1.deref() == null) || (g_weakRef2 != null && g_weakRef2.deref() == null)) {
        throw new Error("STS GC cannot collect cross-reference table objects!");
    }
}

function stsGCTest2(): void {
    triggerFullGC();
    if (g_weakRef1 != null && g_weakRef1.deref() == null) {
        throw new Error("STS GC cannot collect cross-reference table objects!");
    }
    if (g_weakRef2 != null && g_weakRef2.deref() != null) {
        throw new Error('STS GC should collect garbage object!');
    }
}

function stsGCTest3(): void {
    triggerFullGC();
    if (g_weakRef1 != null && g_weakRef1.deref() == null) {
        throw new Error('STS GC cannot collect the objects that are referenced by cross-referenced objects!');
    }
    if (g_weakRef2 != null && g_weakRef2.deref() == null) {
        throw new Error("STS GC cannot collect cross-reference table objects!");
    }
    if (g_weakRef3 != null && g_weakRef3.deref() != null) {
        throw new Error('STS GC should collect garbage object!');
    }
}

function stsGCTest4(): void {
    xgc();
    triggerFullGC();
    if ((g_weakRef1 != null && g_weakRef1.deref() != null) || (g_weakRef2 != null && g_weakRef2.deref() != null)) {
        throw new Error("STS GC should collect cross-reference table objects that are dereferenced!");
    }
}

function stsGCTest5(): void {
    xgc();
    triggerFullGC();
    if (g_weakRef1 != null && g_weakRef1.deref() != null) {
        throw new Error("STS GC should collect cross-reference table objects that are dereferenced!");
    }
    if (g_weakRef2 != null && g_weakRef2.deref() != null) {
        throw new Error('STS GC should collect garbage object!');
    }
}

function stsGCTest6(): void {
    xgc();
    triggerFullGC();
    if (g_weakRef1 != null && g_weakRef1.deref() != null) {
        throw new Error('STS GC should collect the objects that are referenced by cross-referenced objects!');
    }
    if (g_weakRef2 != null && g_weakRef2.deref() != null) {
        throw new Error("STS GC should collect cross-reference table objects that are dereferenced!");        
    }
    if (g_weakRef3 != null && g_weakRef3.deref() != null) {
        throw new Error('STS GC should collect garbage object!');
    }
}
