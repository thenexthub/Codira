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

function testEworkersReachedLimit() {
    let errorWasCatched = false;
    let eaWorkers = new Array<EAWorker>();
    try {
        while (true) {
            let eaWorker = new EAWorker();
            eaWorker.start();
            eaWorkers.push(eaWorker);
        }
    } catch (e) {
        arktest.assertEQ(e.toString(), 'CoroutinesLimitExceedError: The limit of Exclusive Workers has been reached');
        errorWasCatched = true;
    }
    eaWorkers.forEach((eaWorker: EAWorker) => {
        eaWorker.join();
    });
    arktest.assertTrue(errorWasCatched);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworkers_reached_limit');
    testSuite.addTest('testEworkersReachedLimit', testEworkersReachedLimit);
    let res = testSuite.run();
    return res;
}
