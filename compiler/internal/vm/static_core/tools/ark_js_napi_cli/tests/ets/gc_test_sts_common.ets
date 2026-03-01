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

// This is a test common file with ETS entities that are used
// in interop

package PandaGC

let storage:Object;
let gwr: WeakRef<Object>;
static {
    storage = new Object();
    gwr = new WeakRef<Object>(new Object());
}


let gpwr: WeakRef<Promise<Object>> | undefined;

function GetSTSObject(): Object {
    return new Object();
}

function GetSTSArrayOfObjects(): Object[] {
    let arr = new Object[8];
    return arr;
}

function GetSTSObjectAndKeepIt(): Object {
    storage = new Object();
    return storage;
}

function GetSTSObjectWithWeakRef(): Object {
    let a = new Object();
    gwr = new WeakRef<Object>(a);
    return a;
}

function GetSTSStringArrayAsObject(): Object {
    let a = new String[1];
    a[0] = "abc";
    for(let i = 0; i < 15; i++) {
        a[0] += a[0];
    }
    return a as Object;
}

function isSTSObjectCollected(): boolean {
    return gwr.deref() === undefined;
}

function isSTSPromiseCollected(): boolean {
    return gpwr!.deref() === undefined;
}

function SetJSObject(obj: JSValue): void {
    let a = obj;
}

function SetJSObjectAndKeepIt(obj: Object): void {
    storage = obj;
}

function SetJSObjectWithWeakRef(obj: Object): void {
    gwr = new WeakRef<Object>(obj);
}

function SetJSPromiseWithWeakRef(prms: Promise<Object>): void {
    gpwr = new WeakRef<Promise<Object>>(prms);
}

function AddPandaArray(arr: Array<Object>): void {
    let a = arr;
    RunInteropGC();
    RunPandaGC();
}

function RunPandaGC(): void {
    try {
        console.log("--- Start ETS GC Full ---");
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
        console.log("--- Finish ETS GC Full ---");
    } catch(e) {
        throw new Error("Unexpected exception during ETS full GC");
    }
}

function RunInteropGC(): void {
    try {
        console.log("--- Start XGC ---");
        GC.waitForFinishGC(JSRuntime.xgc());
        console.log("--- Finish XGC ---");
    } catch(e) {
        throw new Error("Unexpected exception XGC");
    }
}

function GetPandaFreeHeapSize(): long {
    return GC.getFreeHeapSize();
}

function GetPandaUsedHeapSize(): long {
    return GC.getUsedHeapSize();
}

class PandaBaseClass {
    public obj: Object;
    public str: string;
    constructor () {
        this.obj = new Object();
        this.str = "PandaBaseClass";
        for(let i = 0; i < 10; i++) {
            this.str += this.str;
        }
    }
}
