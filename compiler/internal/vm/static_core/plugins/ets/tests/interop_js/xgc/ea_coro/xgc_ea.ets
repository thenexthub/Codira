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

package xgc_ea

const SHARED_OBJECTS_COUNT: int = 10;

class SharedClass {
    public jsRef?: JSValue;
}
// NOTE(mgroshev): revert this change, when #29283
let sharedObjects: Array<(WeakRef<SharedClass>)>;
static {
    sharedObjects = new Array<(WeakRef<SharedClass>)>();
    for (let i: int = 0; i < SHARED_OBJECTS_COUNT; ++i) {
        sharedObjects.push(new WeakRef<SharedClass>(new SharedClass()));
    }
}


function xgc(): void {
    GC.waitForFinishGC(JSRuntime.xgc());
}

function runFullGC(): void {
    GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
}

// If cross reference is not existed then create it: JS object -> ETS object
function getSharedObject(index: int): SharedClass|undefined {
    return sharedObjects[index]!.deref();
}

// Create refs: ETS object -> JSValue (ETS object) -> JS object
function setRefToSharedObject(index: int, jsRef: JSValue): void {
    sharedObjects[index]!.deref()!.jsRef = jsRef;
}
