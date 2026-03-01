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

class MyObject {
    value: number;

    constructor(value: number) {
        this.value = value;
    }

    getDouble(): number {
        return this.value * 2;
    }
}

(globalThis as any).wrapobjTS = new MyObject(55);

(globalThis as any).unwrapobjTS = new MyObject(50);
function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    const wrapobj = new MyObject(42);
    nativeWrapRef(wrapobj);
    nativeSaveRef(wrapobj)
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckWrappedPtr")());

    nativeWrapRef(wrapobjTS);
    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckWrappedPtrFromTS")());

    ASSERT_TRUE(etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsCheckUnWrappedPtrFromTS")());
}

(globalThis as any).nativeSaveRef("temp");

const obj = {};

(globalThis as any).nativeWrapRef(obj);
main();
