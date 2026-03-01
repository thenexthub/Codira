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

let resolve: ((value: String) => void) | null = null;
     
let p = new Promise<String>((_resolve: (value: String) => void): void => {
        resolve = _resolve;
});

function runGC(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let fullGcId = GC.startGC(GC.Cause.YOUNG);
        GC.waitForFinishGC(fullGcId);
    } catch (e) {
        throw new Error("Unexpected error during GC");
    }
}


async function deadlockTest(): Promise<void> {
    let finreg = new FinalizationRegistry<int>((tag: int) => {
    if (tag == 1) {
        await p;
    } else if (tag == 2) {
        resolve!("Panda");
    }
    });
    // Regarding the implementation, after GC the callback will be executed 
    // first for register with number 1 and then for number 2
    finreg.register(new Object(), 2);
    finreg.register(new Object(), 1);
}

function main(): void {
    deadlockTest();
    runGC();
    runGC();
}
