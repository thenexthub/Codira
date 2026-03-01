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

let mainId = CoroutineExtras.getWorkerId();

function testEworkerId() {
    let w = new EAWorker();
    w.start();
    let p1 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    });
    let p2 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    });
    w.join();
    let runId1 = p1.Await();
    let runId2 = p2.Await();
    arktest.assertEQ(runId1, runId2);
}

function testGetEAWorkerId() {
    let ew1 = new EAWorker('testWorker');
    let ew2 = new EAWorker('testWorker');
    let ew3 = new EAWorker('testWorker');
    arktest.assertEQ(ew1.getWorkerId(), undefined);
    ew1.start();
    ew2.start();
    ew3.start();
    let worker1Id1 = ew1.getWorkerId();
    let worker2Id1 = ew2.getWorkerId();
    let worker3Id1 = ew3.getWorkerId();
    let worker1Id2 = ew1.run<int>(() => {
        return CoroutineExtras.getWorkerId();
    }).Await();
    arktest.assertEQ(worker1Id1, worker1Id2);
    ew1.quit();
    arktest.assertEQ(worker1Id1, ew1.getWorkerId());
    ew2.quit();
    arktest.assertEQ(worker2Id1, ew2.getWorkerId());
    ew3.join();
    arktest.assertEQ(worker3Id1, ew3.getWorkerId());
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworker_id');
    testSuite.addTest('testEworkerId', testEworkerId);
    testSuite.addTest('testGetEAWorkerId', testGetEAWorkerId);
    let res = testSuite.run();
    return res;
}
