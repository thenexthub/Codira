/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class Event {
    constructor() {
        this.promise = new Promise<boolean>((resolve: (val: boolean) => void) => {
            this.resolveFn = resolve;
        })
    }

    public wait(): boolean {
        return waitForCompletion<boolean>(() => {
            return this.promise;
        });
    }

    public fire(val: boolean = true) {
        this.resolveFn!(val);
    }

    private promise: Promise<boolean>;
    private resolveFn: ((val: boolean) => void) | null = null;
}

function testTaskPool() {
    let event = new Event();
    taskpool.execute(() => {
        arktest.expectError(() => {
            EAWorker.current();
        }, new Error('Can not get current worker in taskpool'));
        event.fire();
    });
    event.wait();
    CoroutineExtras.stopTaskpool();
}

function testWrapTaskErrorHandling() {
    let errorCaught = false;
    let waiter = new Event();
    let eaw = new EAWorker(() => {
        throw new Error('This is an uncaught exception!');
    });
    eaw.setUncaughtExceptionHandler((e: Error) => {
        arktest.assertEQ(e.message, 'This is an uncaught exception!');
        errorCaught = true;
        waiter.fire();
    });
    eaw.start();
    waiter.wait();
    arktest.assertTrue(errorCaught);
    arktest.expectError(() => {
        eaw.join();
    }, 'Unable to launch coroutine: no suitable worker was found');
}

function main() {
    const suite = new arktest.ArkTestsuite('EAWorker Message Test Suite');
    suite.addTest('testWrapTaskErrorHandling', testWrapTaskErrorHandling);
    suite.addTest('testTaskPool', testTaskPool);
    return suite.run();
}
