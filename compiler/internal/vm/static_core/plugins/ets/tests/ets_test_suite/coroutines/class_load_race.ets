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

import { launch } from "std/concurrency"

type L = StdDebug.Logger

class Racer {
    static x: int = Racer.f();

    static f(): int {
        Racer.heavyComputation();
        return 1;
    }

    static heavyComputation(): int {
        let i = 0;
        for (let j = 0; j < 10000; j++) {
            i += j * j;
        }
        return i;
    }
}

function reader(): Int {
    if (Racer.x != 1) {
        L.log('Data race in class initializer detected!');
        exit(1);
    }
    return 0;
}

function testRaceInTheCCtor(): void {
    for (let i = 0; i < 20; i++) {
        launch<Int, () => Int>(reader);
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.class_load_race');
    testSuite.addTest('testRaceInTheCCtor', testRaceInTheCCtor);
    let res = testSuite.run();
    return res;
}
