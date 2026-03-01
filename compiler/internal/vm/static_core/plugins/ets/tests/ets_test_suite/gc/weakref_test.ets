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

let globalWr: WeakRef<Object>|undefined = undefined;

function check() {
}

class WeakRefTestSuite extends arktest.ArkTestsuite {
    constructor() {
        super('WeakRefTestSuite');
    }

    before() {
        globalWr = new WeakRef<Object>(new Object());
    }

    after() {
    }
}

function weakRefTest() {
    // Allocate some objects to make sure runtime pops the current region
    // from TLAB
    for (let i = 0; i < 200; ++i) {
        new Object();
    }
    GC.startGC(GC.Cause.FULL, true);
    arktest.assertEQ(globalWr!.deref(), undefined);
}

function main() {
    let testsuite = new WeakRefTestSuite();
    testsuite.addTest('weakRefTest', weakRefTest);
    return testsuite.run();
}

