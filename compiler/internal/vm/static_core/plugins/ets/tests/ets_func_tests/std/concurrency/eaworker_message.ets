/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function testNamedEAWorker() {
    let eaw1 = new EAWorker();
    eaw1.start();
    let eaw2 = new EAWorker('custom name');
    eaw2.start();
    arktest.assertEQ(eaw1.getName(), 'EAWorker');
    arktest.assertEQ(eaw2.getName(), 'custom name');
    arktest.assertNE(eaw1.getWorkerId(), eaw2.getWorkerId());
    eaw1.join();
    eaw2.join();
}

function testTaskEAWorker() {
    let waiter = new Event();
    let checkString = '';
    let eaw = new EAWorker(() => {
        checkString = 'abc';
        waiter.fire();
    });
    eaw.start();
    waiter.wait();
    arktest.assertEQ(checkString, 'abc');
    eaw.join();
}

const sleep = (ms: int): Promise<int> => {
    return new Promise<int>(resolve => {
        setTimeout(() => {
            resolve(1)
        }, ms)
    });
}

function testIsAliveAndQuit() {
    let eaw1 = new EAWorker();
    let eaw2 = new EAWorker('custom name');
    let eaw3 = new EAWorker(() => {});
    arktest.assertFalse(eaw1.isAlive());
    arktest.assertFalse(eaw2.isAlive());
    arktest.assertFalse(eaw3.isAlive());
    eaw1.start();
    eaw2.start();
    eaw3.start();
    arktest.assertTrue(eaw1.isAlive());
    arktest.assertTrue(eaw2.isAlive());
    arktest.assertTrue(eaw3.isAlive());
    eaw1.quit();
    eaw2.quit();
    eaw3.join();
    let maxChecks = 10;
    let sleepTime = 10;
    for (let i = 0; i < maxChecks; i++) {
        if (!eaw1.isAlive() && !eaw2.isAlive() && !eaw3.isAlive()) {
            break;
        }
        waitForCompletion(() => sleep(sleepTime));
    }
    arktest.assertFalse(eaw1.isAlive());
    arktest.assertFalse(eaw2.isAlive());
    arktest.assertFalse(eaw3.isAlive());
}

function testMainQuit() {
    let eawMain = EAWorker.main();
    arktest.expectError(() => {
        eawMain.quit();
    }, new Error('Main worker cannot be stopped'));
    arktest.expectError(() => {
        eawMain.quit();
    }, new Error('Main worker cannot be stopped'));
    arktest.expectError(() => {
        eawMain.join();
    }, new Error('Main worker cannot be stopped'));
}

function testQuit() {
    let eaw2 = new EAWorker();
    eaw2.start();
    let checkPoints = 0;
    eaw2.run<void>(() => {
        checkPoints += 1;
    });
    let job2 = eaw2.run<void>(() => {
        checkPoints += 1;
    });
    eaw2.quit();
    arktest.assertEQ(checkPoints, 2);
}

function testCurrent() {
    let eaw = new EAWorker();
    eaw.start();
    let waiter = new Event();
    eaw.run<void>(() => {
        arktest.assertEQ(eaw.getWorkerId(), EAWorker.current()!.getWorkerId());
        arktest.assertEQ(eaw, EAWorker.current());
        waiter.fire();
    }).Await()
    waiter.wait();
    eaw.join();
}

function testUncaughtExceptionHandler() {
    let eaw1 = new EAWorker();
    let waiter = new Event();
    eaw1.start();
    let expectError = new Error('expectError');
    eaw1.setUncaughtExceptionHandler((e: Error) => {
        arktest.assertEQ(expectError, e);
        waiter.fire();
    });
    eaw1.run<void>(() => {
        throw expectError;
    });
    waiter.wait();
    eaw1.join();
}

function testPostTask() {
    let waiter1 = new Event();
    let waiter2 = new Event();
    let eawMain = EAWorker.main();
    let checkCount = 0;
    eawMain.postTask(() => {
        checkCount ++;
        waiter1.fire();
    });
    waiter1.wait();
    let eawNoMain = new EAWorker();
    eawNoMain.start();
    eawNoMain.postTask(() => {
        checkCount ++ ;
        waiter2.fire();
    })

    waiter2.wait();
    eawNoMain.join();
    arktest.assertEQ(checkCount, 2);
}

function testPriority() {
    let eaw1 = new EAWorker();
    let eaw2 = new EAWorker();
    eaw1.start();
    eaw2.start();
    eaw1.setPriority(WorkerPriority.PRIORITY_HIGH);
    eaw2.setPriority(WorkerPriority.PRIORITY_LOW);
    arktest.assertEQ(eaw1.getPriority(), WorkerPriority.PRIORITY_HIGH);
    arktest.assertEQ(eaw2.getPriority(), WorkerPriority.PRIORITY_LOW);
    eaw1.join();
    eaw2.join();
}

function testCommunication() {
    let eaw1 = new EAWorker();
    let eaw2 = new EAWorker();
    let waiter = new Event();
    eaw1.start();
    eaw2.start();

    let eaw1Tag = 1111;
    let eaw2Tag = 2222;
    let messageGot = false;

    let handler1 : concurrency.MessageHandler;
    let handler2 : concurrency.MessageHandler;

    handler1 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), eaw2Tag);
        if (msg.getObject() != undefined) {
            let obj = msg.getObject() as string;
            arktest.assertEQ(obj, "message from eaw2");
            handler2.sendMessage(new concurrency.Message(eaw1Tag, "message from eaw1", handler2));
        }
    }, eaw1);

    handler2 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), eaw1Tag);
        arktest.assertEQ(msg.getObject(), "message from eaw1");
        messageGot = true;
        waiter.fire();
    }, eaw2);

    handler1.sendMessage(new concurrency.Message(eaw2Tag, "message from eaw2", handler1));

    waiter.wait();
    arktest.assertEQ(messageGot, true);
    eaw1.join();
    eaw2.join();
}

function testMain() {
    let mainWorker = EAWorker.main();
    let workerId = CoroutineExtras.getWorkerId();
    mainWorker.run<void>(()=>{
        let id = CoroutineExtras.getWorkerId();
        arktest.assertEQ(id, workerId);
    }).Await();

    arktest.assertEQ(mainWorker.getName(), "main");
    arktest.assertEQ(mainWorker.getWorkerId(), workerId);
    arktest.assertEQ(mainWorker.getPriority(), WorkerPriority.PRIORITY_DEFAULT);
    arktest.assertEQ(mainWorker.isAlive(), true);

    arktest.expectError(() => {
        mainWorker.quit();
    }, new Error('Main worker cannot be stopped'));

    arktest.expectError(() => {
        mainWorker.quit();
    }, new Error('Main worker cannot be stopped'));

    arktest.expectError(() => {
        mainWorker.join();
    }, new Error('Main worker cannot be stopped'));

    arktest.expectError(() => {
        mainWorker.setPriority(WorkerPriority.PRIORITY_HIGH);
    }, new Error('Main worker cannot set priority'));

    arktest.expectError(() => {
        mainWorker.setUncaughtExceptionHandler((e: Error) => {});
    }, new Error('Main worker cannot have uncaught exception handler'));

    arktest.expectError(() => {
        mainWorker.start();
    }, new Error('Main worker cannot be started'));

    arktest.assertEQ(mainWorker.getUncaughtExceptionHandler(), undefined);
}

function testQuitBeforeStart() {
    let eaw = new EAWorker();
    arktest.expectError(() => {
        eaw.quit();
    }, new Error('Cannot stop worker before start'));
    eaw.start();
    eaw.quit();
    arktest.expectError(() => {
        eaw.start();
    }, new Error('Worker already started or quit'));
}

function testPostTaskWhenNotStarted() {
    let eaw = new EAWorker();
    arktest.expectError(() => {
        eaw.postTask(() => {});
    }, new Error('Can not post task when worker is not started'));
}

function testStartAfterQuit() {
    let eaw = new EAWorker();
    eaw.quit();
    arktest.expectError(() => {
        eaw.start();
    }, new Error('Can not start worker after quit'));
}

function testStartafterJoin() {
    let eaw = new EAWorker();
    arktest.expectError(() => {
         eaw.join();
    }, new Error('Cannot stop worker before start'));
    eaw.start();
    eaw.quit();
}

function testRunAfterQuit() {
    let eaw = new EAWorker();
    eaw.start();
    eaw.run<void>(() => {
        sleep(1);
    });
    eaw.quit();
    /**
     * Since task below could be added to task queue in two scenarios:
     * 1. during quitting process, specifically, after enqueueing closingTask
     * 2. after quitting finished, specifically, after executing closingTask
     * So it might throw exception either
     * 1. "Can not run task when worker is quitted" or
     * 2. "Can not run task when worker is not started"
     */
    arktest.expectError(() => {
        eaw.run<void>(() => {});
    });
}

function main() {
    const suite = new arktest.ArkTestsuite('EAWorker Message Test Suite');
    suite.addTest('testNamedEAWorker', testNamedEAWorker);
    suite.addTest('testTaskEAWorker', testTaskEAWorker);
    suite.addTest('testIsAliveAndQuit', testIsAliveAndQuit);
    suite.addTest('testMainQuit', testMainQuit);
    suite.addTest('testQuit', testQuit);
    suite.addTest('testCurrent', testCurrent);
    suite.addTest('testUncaughtExceptionHandler', testUncaughtExceptionHandler);
    suite.addTest('testPostTask', testPostTask);
    suite.addTest('testPriority', testPriority);
    suite.addTest('testCommunication', testCommunication);
    suite.addTest('testMainWorker', testMain);
    suite.addTest('testQuitBeforeStart', testQuitBeforeStart);
    suite.addTest('testStartafterJoin', testStartafterJoin);
    suite.addTest('testRunAfterQuit', testRunAfterQuit);
    return suite.run();
}
