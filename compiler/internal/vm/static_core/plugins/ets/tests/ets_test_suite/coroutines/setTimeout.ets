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

let zeroTimeout = 0;
let shortTimeout = 10;
let mediumTimeout = 100;
let longTimeout = 1000;

function setTimeoutTest() {
    let flag = false;
    let start = Date.now();
    setTimeout(() => {
        arktest.assertTrue(flag);
        arktest.assertTrue(Date.now() - start >= shortTimeout);
    }, shortTimeout);
    flag = true;
}

function firstTimerIdTest() {
    let id = setTimeout(() => {}, 0);
    arktest.assertTrue(id == 1);
}

function clearTimeoutTest() {
    let id = setTimeout(() => { arktest.assertTrue(false); }, mediumTimeout);
    clearTimeout(id);
}

function clearTimeoutTestWithOutId() {
    clearTimeout();
}

function clearTimeoutTestWithNullId() {
    clearTimeout(null);
}

function clearTimeoutTestWithUndefinedId() {
    clearTimeout(undefined);
}

function clearIntervalTestWithOutId() {
    clearInterval();
}

function clearIntervalTestWithNullId() {
    clearInterval(null);
}

function clearIntervalTestWithUndefinedId() {
    clearInterval(undefined);
}

function clearIntervalTestFromCallback() {
    let flag = true;
    let id: int = -1;
    id = setInterval(() => {
        arktest.assertTrue(flag);
        flag = false;
        arktest.assertTrue(id != -1);
        clearInterval(id);
    }, zeroTimeout);
}

function clearIntervalFromAnotherTimer() {
    let flag = false;
    let id = setInterval(() => { flag = true; }, shortTimeout);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id);
    }, mediumTimeout);
}

function setIntervalWithDifferentMsgAndDelay() {
    let flag = true;

    let id0 = setInterval("setIntervalMsg0 in setIntervalWithDifferentMsgAndDelay", shortTimeout);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id0);
    }, shortTimeout);

    let id1 = setInterval("setIntervalMsg1 in setIntervalWithDifferentMsgAndDelay", null);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id1);
    }, shortTimeout);

    let id2 = setInterval("setIntervalMsg2 in setIntervalWithDifferentMsgAndDelay", undefined);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id2);
    }, shortTimeout);

    let id3 = setInterval("setIntervalMsg3 in setIntervalWithDifferentMsgAndDelay");
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id3);
    }, shortTimeout);

    let id4 = setInterval("", null);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id4);
    }, shortTimeout);

    let id5 = setInterval("", undefined);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id5);
    }, shortTimeout);

    let id6 = setInterval("");
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id6);
    }, shortTimeout);
}

function setIntervalWithFuncAndDifferentDelay() {
    let flag = false;

    let id0 = setInterval(() => { flag = true; }, shortTimeout);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id0);
    }, shortTimeout);

    flag = false;
    let id1 = setInterval(() => { flag = true; }, null);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id1);
    }, shortTimeout);

    flag = false;
    let id2 = setInterval(() => { flag = true; }, undefined);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id2);
    }, shortTimeout);
}

function setTimeoutWithDifferentMsgAndDelay() {
    let id0 = setInterval("setIntervalMsg0 in setTimeoutWithDifferentMsgAndDelay", shortTimeout);
    setTimeout("setTimeoutMsg0 in setTimeoutWithDifferentMsgAndDelay", shortTimeout);
    clearInterval(id0);

    let id1 = setInterval("setIntervalMsg1 in setTimeoutWithDifferentMsgAndDelay", null);
    setTimeout("setTimeoutMsg1 in setTimeoutWithDifferentMsgAndDelay", null);
    clearInterval(id1);

    let id2 = setInterval("setIntervalMsg2 in setTimeoutWithDifferentMsgAndDelay", undefined);
    setTimeout("setTimeoutMsg2 in setTimeoutWithDifferentMsgAndDelay", undefined);
    clearInterval(id2);

    let id3 = setInterval("setIntervalMsg3 in setTimeoutWithDifferentMsgAndDelay");
    setTimeout("setTimeoutMsg3 in setTimeoutWithDifferentMsgAndDelay", shortTimeout);
    clearInterval(id3);

    let id4 = setInterval("", null);
    setTimeout("", null);
    clearInterval(id4);

    let id5 = setInterval("", undefined);
    setTimeout("", undefined);
    clearInterval(id5);

    let id6 = setInterval("");
    setTimeout("", shortTimeout);
    clearInterval(id6);
}

function setTimeoutWithFuncAndDifferentDelay() {
    let flag = true;

    let id0 = setInterval("setIntervalMsg0 in setTimeoutWithFuncAndDifferentDelay", shortTimeout);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id0);
    }, shortTimeout);

    let id1 = setInterval("setIntervalMsg1 in setTimeoutWithFuncAndDifferentDelay", null);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id1);
    }, null);

    let id2 = setInterval("setIntervalMsg2 in setTimeoutWithFuncAndDifferentDelay", undefined);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id2);
    }, undefined);

    let id3 = setInterval("setIntervalMsg3 in setTimeoutWithFuncAndDifferentDelay");
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id3);
    }, shortTimeout);

    let id4 = setInterval("", null);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id4);
    }, null);

    let id5 = setInterval("", undefined);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id5);
    }, undefined);

    let id6 = setInterval("");
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id6);
    }, shortTimeout);
}

function clearTimeoutFromAnotherWorkerTest() {
    let id = setTimeout(() => { arktest.assertTrue(false); }, longTimeout);
    launch<void, (i: int) => void>(clearTimeout, id);
}

function compareTwoStringArrays(array1: string[] , array2: string[]): boolean {
    if (array1.length != array2.length) {
        arktest.assertEQ(array1.length , array2.length);
        return false;
    }

    for (let i = 0; i < array1.length; i++) {
        if (array1[i] != array2[i]) {
            arktest.assertEQ(array1[i], array2[i]);
            return false;
        }
    }
    return true;
}

function setTimeoutAndIntervalTest() {
    const execSeq: string[] = [];
    const expectedExecSeq: string[] = ['Timeout-100ms',
                                       'Interval-50ms-1','Timeout-20ms',
                                       'Interval-50ms-2','Timeout-20ms',
                                       'Interval-50ms-3','Timeout-20ms'];
    let intervalCnt: int = 0;
    let intervalId: int = 0;

    setTimeout(()=> {
        execSeq.push('Timeout-100ms');
        intervalId = setInterval(()=> {
            execSeq.push(`Interval-50ms-${++intervalCnt}`);
            setTimeout(()=> {
                execSeq.push('Timeout-20ms');
                if (intervalCnt >= 3) {
                    clearInterval(intervalId);
                    arktest.assertTrue(compareTwoStringArrays(execSeq , expectedExecSeq));
                }
            } , 20);
        } , 50);
    }, 100);
}

function sameWorkerIdTest() {
    let parentWorkerId = CoroutineExtras.getWorkerId();
    setTimeout(() => { arktest.assertTrue(CoroutineExtras.getWorkerId() == parentWorkerId) }, zeroTimeout);
}

function testSetTimeoutLaunch() {
    launch<void, () => void>(setTimeoutTest);
}

function testClearTimeoutLaunch() {
    launch<void, () => void>(clearTimeoutTest);
}

function testClearIntervalFromCbLaunch() {
    launch<void, () => void>(clearIntervalTestFromCallback);
}

function testClearIntervalFromTimerLaunch() {
    launch<void, () => void>(clearIntervalFromAnotherTimer);
}

function testCearTimeoutFromWorkerLaunch() {
    launch<void, () => void>(clearTimeoutFromAnotherWorkerTest);
}

function testSameWorkerIdTestLaunch() {
    launch<void, () => void>(sameWorkerIdTest);
}

function main(): int {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    
    let testSuite = new arktest.ArkTestsuite('coroutines.setTimeout');

    /**
     * Test on main coroutine worker
     * Note: firstTimerIdTest should be executed at the first, or it will fail
     */
    testSuite.addTest('firstTimerIdTest', firstTimerIdTest);
    testSuite.addTest('setTimeoutTest', setTimeoutTest);
    testSuite.addTest('clearTimeoutTestWithOutId', clearTimeoutTestWithOutId);
    testSuite.addTest('clearTimeoutTestWithNullId', clearTimeoutTestWithNullId);
    testSuite.addTest('clearTimeoutTestWithUndefinedId', clearTimeoutTestWithUndefinedId);
    testSuite.addTest('clearTimeoutTest', clearTimeoutTest);
    testSuite.addTest('clearIntervalTestWithOutId', clearIntervalTestWithOutId);
    testSuite.addTest('clearIntervalTestWithNullId', clearIntervalTestWithNullId);
    testSuite.addTest('clearIntervalTestWithUndefinedId', clearIntervalTestWithUndefinedId);
    testSuite.addTest('clearIntervalTestFromCallback', clearIntervalTestFromCallback);
    testSuite.addTest('clearIntervalFromAnotherTimer', clearIntervalFromAnotherTimer);
    testSuite.addTest('setIntervalWithDifferentMsgAndDelay', setIntervalWithDifferentMsgAndDelay);
    testSuite.addTest('setIntervalWithFuncAndDifferentDelay', setIntervalWithFuncAndDifferentDelay);
    testSuite.addTest('setTimeoutWithDifferentMsgAndDelay', setTimeoutWithDifferentMsgAndDelay);
    testSuite.addTest('setTimeoutWithFuncAndDifferentDelay', setTimeoutWithFuncAndDifferentDelay);
    testSuite.addTest('clearTimeoutFromAnotherWorkerTest', clearTimeoutFromAnotherWorkerTest);
    testSuite.addTest('setTimeoutAndIntervalTest', setTimeoutAndIntervalTest);
    testSuite.addTest('sameWorkerIdTest', sameWorkerIdTest);
    
    /**
     * Test on non-main coroutine worker using launch
     */
    testSuite.addTest('testSetTimeoutLaunch', testSetTimeoutLaunch);
    testSuite.addTest('testClearTimeoutLaunch', testClearTimeoutLaunch);
    testSuite.addTest('testClearIntervalFromCbLaunch', testClearIntervalFromCbLaunch);
    testSuite.addTest('testClearIntervalFromTimerLaunch', testClearIntervalFromTimerLaunch);
    testSuite.addTest('testCearTimeoutFromWorkerLaunch', testCearTimeoutFromWorkerLaunch);
    testSuite.addTest('testSameWorkerIdTestLaunch', testSameWorkerIdTestLaunch);

    /**
     * Note: none of these tests will actually be registered as failing by ArkTest in case if it fails. 
     * Instead, the whole program will fail with the uncaught error. This is to be fixed in future.
     * This is because all the setTimeouts are executed as separate coroutines.
     */
    let res = testSuite.run();
    
    return res;
}
