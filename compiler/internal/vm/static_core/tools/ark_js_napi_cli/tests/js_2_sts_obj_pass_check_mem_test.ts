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

// This test check that transferring JS objects from JS -> ETS
// are collected and no memory leak occurs

import { interop } from './gc_test_common';

let a = new Object();

function main(): void {
    for (let j = 0; j < 500000; j++) {
        a = new Object();
        interop.SetJSObject(a);
        if (j % 10000 === 0) {
            interop.RunPandaGC();
        }
    }
    interop.RunPandaGC();
    let usedMem1 = interop.GetPandaUsedHeapSize();

    for (let j = 0; j < 500000; j++) {
        a = new Object();
        interop.SetJSObject(a);
        if (j % 10000 === 0) {
            interop.RunPandaGC();
        }
    }
    interop.RunPandaGC();
    let usedMem2 = interop.GetPandaUsedHeapSize();
    if ((usedMem2 / usedMem1) as double > 1.25) {
        throw Error('Some memory leak occurs. Used mem between test cycles is ' + (usedMem2 / usedMem1) as double + 'times larger');
    }
}
main();
