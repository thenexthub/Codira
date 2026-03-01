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

class Test {
    constructor(numCheckpoints: int) {
        this.numCheckpoints = numCheckpoints;
        this.sequence = new Array<number>();
    }

    check(): boolean {
        if (this.result == Test.RESULT_FAILED || this.result == Test.RESULT_UNSET) {
            return false;
        }
        if (this.sequence.length != this.numCheckpoints) {
            console.log("Test failed. Expected " + this.numCheckpoints + " checkpoints, but got " + this.sequence.length);
            this.result = Test.RESULT_FAILED;
            return false;
        }
        for (let i = 0; i < this.sequence.length; ++i) {
            if (this.sequence[i] != i) {
                console.log("Test failed. Expected " + i + "-th checkpoint to be " + i + ", but got " + this.sequence[i]);
                this.result = Test.RESULT_FAILED;
                return false;
            }
        }
        return true;
    }

    fail(message: string): void {
        this.result = Test.RESULT_FAILED;
        console.log(message);
    }

    setPassed(): void {
        arktest.assertEQ(this.result, Test.RESULT_UNSET)
        this.result = Test.RESULT_PASSED;
    }

    checkpoint(value: int) {
        this.sequence.push(value);
    }

    private static RESULT_UNSET: int = 0;
    private static RESULT_PASSED: int = 1;
    private static RESULT_FAILED: int = 2;

    result: int = Test.RESULT_UNSET;
    failMessage: string = "";
    private sequence: Array<number>;
    private numCheckpoints: int;
}

function check(): boolean {
    return globalTest!.check();
}

let globalTest: Test | null = null;

function testSetTimeout(isExternalTimer: boolean): void {
    globalTest = new Test(3);
    let sequence = new Array<number>();
    let delay = 100;
    globalTest!.checkpoint(0);
    let start = Date.now();
    setTimeout((): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < delay) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after " + delay + "ms at least.");
        }
        globalTest!.checkpoint(2);
        globalTest!.setPassed();
    }, delay);
    globalTest!.checkpoint(1);
}

function testClearTimeout(isExternalTimer: boolean): void {
    globalTest = new Test(2);
    let sequence = new Array<number>();
    globalTest!.checkpoint(0);
    let timerId = setTimeout((): void => {
        globalTest!.fail("The callback should not be called.");
    }, 0);
    clearTimeout(timerId);
    globalTest!.checkpoint(1);
    globalTest!.setPassed();
}

function testSetInterval(isExternalTimer: boolean): void {
    globalTest = new Test(6);
    let sequence = new Array<number>();
    let delay = 100;
    let checkpoint = 2;
    globalTest!.checkpoint(0);
    let start = Date.now();
    let timerId: int;
    timerId = setInterval((): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < delay) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after " + delay + "ms at least.");
        }
        if (checkpoint == 5) {
            clearInterval(timerId);
            globalTest!.setPassed();
        }
        globalTest!.checkpoint(checkpoint);
        ++checkpoint;
    }, delay);
    globalTest!.checkpoint(1);
}

function testSetTimeoutExecuteOrder(isExternalTimer: boolean) : void {
    globalTest = new Test(6);
    globalTest!.checkpoint(0);
    setTimeout((): void => {
        globalTest!.checkpoint(2);
    }, 1);
    let now = Date.now();
    while(Date.now() - now < 2) {}
    setTimeout((): void => {
        globalTest!.checkpoint(3);
    }, 0);
    setTimeout((): void => {
        globalTest!.checkpoint(4);
    }, 0);
    setTimeout((): void => {
        globalTest!.checkpoint(5);
        globalTest!.setPassed();
    }, 0);
    globalTest!.checkpoint(1);
}

function testSetTimeoutInvokeCallbackWithSpecifiedParameter(isExternalTimer: boolean): void {
    globalTest = new Test(3);
    let delay = 100;
    globalTest!.checkpoint(0);
    let start = Date.now();
    let para: string = "Parameter"
    setTimeout((p : string): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < 0) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after 0 ms at least.");
        }
        if (p !== para) {
            globalTest!.fail("The parameter passed to the callback is " + p + ". Expected to be " + para + ".");
        }
        globalTest!.checkpoint(2);
        globalTest!.setPassed();
    }, delay, para);
    globalTest!.checkpoint(1);
}

function testSetTimeoutInvokeCallbackWith4SpecifiedParameters(isExternalTimer: boolean): void {
    globalTest = new Test(3);
    let delay = 100;
    globalTest!.checkpoint(0);
    let start = Date.now();
    let para1: string = "Parameter1";
    let para2: int = 2;
    let para3: number = 3.0;
    let para4: string = "Parameter4";

    setTimeout((p1 :string, p2: int, p3: number, p4: string): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < delay) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after " + delay + "ms at least.");
        }
        if (p1 !== para1) {
            globalTest!.fail("The parameter passed to the callback is " + p1 + ". Expected to be " + para1 + ".");
        }
        if (p2 !== para2) {
            globalTest!.fail("The parameter passed to the callback is " + p2 + ". Expected to be " + para2 + ".");
        }
        if (p3 !== para3) {
            globalTest!.fail("The parameter passed to the callback is " + p3 + ". Expected to be " + para3 + ".");
        }
        if (p4 !== para4) {
            globalTest!.fail("The parameter passed to the callback is " + p4 + ". Expected to be " + para4 + ".");
        }
        globalTest!.checkpoint(2);
        globalTest!.setPassed();
    }, delay, para1, para2, para3, para4);
    globalTest!.checkpoint(1);
}

function testSetIntervalInvokeCallbacWithSpecifiedParameter(isExternalTimer: boolean): void {
    globalTest = new Test(6);
    let sequence = new Array<number>();
    let delay = 100;
    let checkpoint = 2;
    globalTest!.checkpoint(0);
    let para : string = "para"
    let start = Date.now();
    let timerId: int;
    timerId = setInterval((p: string): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < delay) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after " + delay + "ms at least.");
        }
        if (p !== para) {
            globalTest!.fail("The parameter passed to the callback is " + p + ". Expected to be " + para + ".");
        }
        if (checkpoint == 5) {
            clearInterval(timerId);
            globalTest!.setPassed();
        }
        globalTest!.checkpoint(checkpoint);
        ++checkpoint;
    }, delay, para);
    globalTest!.checkpoint(1);
}

function testSetIntervalInvokeCallbacWith4SpecifiedParameters(isExternalTimer: boolean): void {
    globalTest = new Test(6);
    let sequence = new Array<number>();
    let delay = 100;
    let checkpoint = 2;
    globalTest!.checkpoint(0);
    let para1: string = "Parameter1";
    let para2: int = 2;
    let para3: number = 3.0;
    let para4: string = "Parameter4";
    let start = Date.now();
    let timerId: int;
    timerId = setInterval((p1 :string, p2: int, p3: number, p4: string): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < delay) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after " + delay + "ms at least.");
        }
        if (p1 !== para1) {
            globalTest!.fail("The parameter passed to the callback is " + p1 + ". Expected to be " + para1 + ".");
        }
        if (p2 !== para2) {
            globalTest!.fail("The parameter passed to the callback is " + p2 + ". Expected to be " + para2 + ".");
        }
        if (p3 !== para3) {
            globalTest!.fail("The parameter passed to the callback is " + p3 + ". Expected to be " + para3 + ".");
        }
        if (p4 !== para4) {
            globalTest!.fail("The parameter passed to the callback is " + p4 + ". Expected to be " + para4 + ".");
        }
        if (checkpoint == 5) {
            clearInterval(timerId);
            globalTest!.setPassed();
        }
        globalTest!.checkpoint(checkpoint);
        ++checkpoint;
    }, delay, para1, para2, para3, para4);
    globalTest!.checkpoint(1);
}

function testSetTimeoutWithNegitiveDelay(isExternalTimer: boolean): void {
    globalTest = new Test(3);
    let sequence = new Array<number>();
    let delay = -10;
    globalTest!.checkpoint(0);
    let start = Date.now();
    setTimeout((): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < 0) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after 0 ms at least.");
        }
        globalTest!.checkpoint(2);
        globalTest!.setPassed();
    }, delay);
    globalTest!.checkpoint(1);
}

function testSetIntervalWithNegitiveDelay(isExternalTimer: boolean): void {
    globalTest = new Test(6);
    let sequence = new Array<number>();
    let delay = -10;
    let checkpoint = 2;
    globalTest!.checkpoint(0);
    let start = Date.now();
    let timerId: int;
    timerId = setInterval((): void => {
        // To get time Date.now and libuv uses clock_gettime under the hood, but with different parameters. Date.now() uses CLOCK_REALTIME, but libuv uses CLOCK_MONOTONIC.
        // When we calculate time interval there may be an error. For example time interval measured by Date.now may be 5.9ms round to 5
        // and time interval measured by libuv is 6.1 round to 6.
        // To avoid such error just add +1 to time interval.
        let spentTime = Date.now() - start + 1;
        if (spentTime < 0) {
            globalTest!.fail("The callback is called after " + spentTime + "ms. Expected to be called after 0 ms at least.");
        }
        if (checkpoint == 5) {
            clearInterval(timerId);
            globalTest!.setPassed();
        }
        globalTest!.checkpoint(checkpoint);
        ++checkpoint;
    }, delay);
    globalTest!.checkpoint(1);
}

function testSetTimeoutWithString(isExternalTimer: boolean): void {
    globalTest = new Test(2);
    let delay = 100;
    globalTest!.checkpoint(0);
    setTimeout("test", delay);
    globalTest!.checkpoint(1);
    globalTest!.setPassed();
}

function testSetIntervalWithString(isExternalTimer: boolean): void {
    globalTest = new Test(2);
    let delay = 100;
    globalTest!.checkpoint(0);
    let checkpoint = 2;
    let id1 = setInterval("test", delay);
    let id2: int;
    id2 = setInterval(() => {
        if (checkpoint == 5) {
            clearInterval(id1);
            clearInterval(id2);
            globalTest!.setPassed();
        } 
        checkpoint ++ ;
    }, delay)
    globalTest!.checkpoint(1);
}

function testClearTimerCrossWorker(isExternalTimer: boolean)
{
    globalTest = new Test(0);

    if (isExternalTimer) {
        globalTest!.setPassed();
        return;
    }

    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let delay = 100;

    /// Clear worker timer from main worker
    {
        let isCleared = new AtomicFlag(false);
        let workerTimerCb = () => {
            setTimeout(() => { while (!isCleared.get()) {} }, 0);
            return setTimeout(() => { arktest.assertTrue(false); }, delay);
        };
        let id = launch<int, () => int>(workerTimerCb).Await();
        clearTimeout(id);
        isCleared.set(true);
    }

    /// Clear main timer from worker
    {
        let isCleared = new AtomicFlag(false);
        let errorThrown = false;
        let id = setTimeout(() => { arktest.assertTrue(isCleared.get()); }, delay);
        try {
            launch<void, () => void>((): void => {
                isCleared.set(true);
                clearTimeout(id);
            }).Await();
        } catch (e) {
            arktest.assertEQ(e.toString(), "Error: Failed to clear timer. Unable to clear interop timer from non-interop worker");
            errorThrown = true;
        }
        arktest.assertEQ(isExternalTimer, errorThrown);
        arktest.assertTrue(isCleared.get());
    }
    globalTest!.setPassed();
}
