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

function testEworkerPost2Main() {
    let job1 = EAWorker.postToMain<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertEQ(runId, mainId);
        return runId;
    });

    let job2 = EAWorker.postToMain<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertEQ(runId, mainId);
        return runId;
    });

    let job3 = launch<int, () => int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    });

    let runId1 = job1.Await();
    let runId2 = job2.Await();
    let runId3 = job3.Await();

    arktest.assertEQ(runId1, runId2);
    arktest.assertNE(runId1, runId3);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworker_post_to_main');
    testSuite.addTest('testEworkerPost2Main', testEworkerPost2Main);
    let res = testSuite.run();
    return res;
}

