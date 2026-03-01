/**
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

import {AtomicFlag} from "std/debug/concurrency"
type Callback = () => void;

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
};

enum MessageType {
    EMPTY,
    STRING,
    NUMBER,
    BOOLEAN,
    ARRAY,
}

function sleep(ms: int) {
    return new Promise<void>((res, rej) => {
        setTimeout(() => {
            res(undefined);
        }, ms);
    });
}

function testMessageHandlerCreation() {
    let eaw = new EAWorker();
    eaw.start();
    
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        // do nothing
    }, eaw);
    
    arktest.assertEQ(handler.getWorker(), eaw);
    eaw.join();
}

function testMessageHandlerWithoutWorker() {
    arktest.expectError(() => {
        new concurrency.MessageHandler((msg: concurrency.Message) => {
            // do nothing
        });
    }, new Error('No worker found for MessageHandler'));
}


function testSendEmptyMessage() {
    let eaw = new EAWorker();
    eaw.start();
    let event = new Event();
    let messageReceived = false;
    
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), MessageType.EMPTY);
        messageReceived = true;
        event.fire();
    }, eaw);
    
    handler.sendEmptyMessage(MessageType.EMPTY);
    event.wait();
    arktest.assertEQ(messageReceived, true);
    eaw.join();
}

function testSendStringMessage() {
    let eaw = new EAWorker();
    eaw.start();
    let event = new Event();
    let messageReceived = false;
    
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), MessageType.STRING);
        arktest.assertEQ(msg.getObject(), "test");
        messageReceived = true;
        event.fire();
    }, eaw);
    
    handler.sendMessage(new concurrency.Message(MessageType.STRING, "test", handler));
    event.wait();
    arktest.assertEQ(messageReceived, true);
    eaw.join();
}

function testSendNumberMessage() {
    let eaw = new EAWorker();
    eaw.start();
    let event = new Event();
    let messageReceived = false;
    
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), MessageType.NUMBER);
        arktest.assertEQ(msg.getObject(), 100);
        messageReceived = true;
        event.fire();
    }, eaw);
    
    handler.sendMessage(new concurrency.Message(MessageType.NUMBER, 100, handler));
    event.wait();
    arktest.assertEQ(messageReceived, true);
    eaw.join();
}

function testSendBooleanMessage() {
    let eaw = new EAWorker();
    eaw.start();
    let event = new Event();
    let messageReceived = false;
    
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), MessageType.BOOLEAN);
        arktest.assertEQ(msg.getObject(), true);
        messageReceived = true;
        event.fire();
    }, eaw);
    
    handler.sendMessage(new concurrency.Message(MessageType.BOOLEAN, true, handler));
    event.wait();
    arktest.assertEQ(messageReceived, true);
    eaw.join();
}


function testHasMessages() {
    let eaw = new EAWorker();
    eaw.start();
    let waiter1 = new Event();
    let waiter2 = new Event();
    let handler: concurrency.MessageHandler;
    
    handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        if (msg.getCallback() != undefined) {
            let cb = msg.getCallback() as Callback;
            cb();
        }
    }, eaw);

    let cb = () => {
        waiter1.wait();
        arktest.assertEQ(handler.hasMessages(MessageType.EMPTY), true);
        arktest.assertEQ(handler.hasMessages(MessageType.STRING, "test"), true);
        arktest.assertEQ(handler.hasMessages(MessageType.NUMBER, 100), true);
        arktest.assertEQ(handler.hasMessages(MessageType.BOOLEAN, true), true);
        
        arktest.assertEQ(handler.hasMessages(MessageType.STRING, "wrong"), false);
        arktest.assertEQ(handler.hasMessages(MessageType.NUMBER, 999), false);
        arktest.assertEQ(handler.hasMessages(MessageType.ARRAY), false);
        waiter2.fire();
    }
    
    handler.sendMessage(new concurrency.Message(cb, handler));
    handler.sendEmptyMessage(MessageType.EMPTY);
    handler.sendMessage(new concurrency.Message(MessageType.STRING, "test", handler));
    handler.sendMessage(new concurrency.Message(MessageType.NUMBER, 100, handler));
    handler.sendMessage(new concurrency.Message(MessageType.BOOLEAN, true, handler));
    waiter1.fire();
    waiter2.wait();
    eaw.join();
}

function testMessageCounting() {
    let eaw = new EAWorker();
    eaw.start();
    let event = new Event();
    let count = 0;
    let countEmpty = 0;
    let countString = 0;
    let countNumber = 0;
    let countBoolean = 0;
    
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        count++;
        if (msg.getWhat() == MessageType.EMPTY) {
            countEmpty++;
        } else if (msg.getWhat() == MessageType.STRING) {
            countString++;
        } else if (msg.getWhat() == MessageType.NUMBER) {
            countNumber++;
        } else if (msg.getWhat() == MessageType.BOOLEAN) {
            countBoolean++;
        }
        
        if (count >= 4) {
            arktest.assertEQ(count, 4);
            arktest.assertEQ(countEmpty, 1);
            arktest.assertEQ(countString, 1);
            arktest.assertEQ(countNumber, 1);
            arktest.assertEQ(countBoolean, 1);
            event.fire();
        }
    }, eaw);
    
    handler.sendEmptyMessage(MessageType.EMPTY);
    handler.sendMessage(new concurrency.Message(MessageType.STRING, "test", handler));
    handler.sendMessage(new concurrency.Message(MessageType.NUMBER, 100, handler));
    handler.sendMessage(new concurrency.Message(MessageType.BOOLEAN, true, handler));
    
    event.wait();
    eaw.join();
}


function testMessageCallback() {
    let eaw = new EAWorker();
    eaw.start();
    let event = new Event();
    let callbackExecuted = false;
    
    let callback = () => {
        callbackExecuted = true;
        event.fire();
    };
    
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        if (msg.getCallback() != undefined) {
            let cb = msg.getCallback() as Callback;
            cb();
        }
    }, eaw);
    
    let msg = new concurrency.Message(callback, handler);
    handler.sendMessage(msg);
    
    event.wait();
    arktest.assertEQ(callbackExecuted, true);
    eaw.join();
}


function testRemoveMultipleMessages() {
    let eaw1 = new EAWorker();
    let eaw2 = new EAWorker();
    eaw1.start();
    eaw2.start();
    let waiter1 = new Event();
    let waiter2 = new Event();
    let executeCount = 0;
    let expectedExecuteCount = 3;
    let handler1: concurrency.MessageHandler;
    let handler2: concurrency.MessageHandler;

    handler1 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        ++executeCount;
        if (msg.getCallback() != undefined) {
            let cb = msg.getCallback() as Callback;
            cb();
        }
    }, eaw1);

    handler2 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        if (msg.getCallback() != undefined) {
            let cb = msg.getCallback() as Callback;
            cb();
        }
    }, eaw2);
    
    let callback = () => {
        waiter1.wait();

        let isMsg1 = handler1.removeMessages(MessageType.STRING, 'test');
        let isMsg2 = handler1.removeMessages(MessageType.NUMBER, 100);
        let isMsg3 = handler1.removeMessages(MessageType.BOOLEAN, true);
    
        if (isMsg1){
            arktest.assertEQ(handler1.hasMessages(MessageType.STRING, 'test'), false);
            --expectedExecuteCount;
        } else {
            arktest.assertEQ(handler1.hasMessages(MessageType.STRING, 'test'), true);
        }
        if (isMsg2){
           arktest.assertEQ(handler1.hasMessages(MessageType.NUMBER, 100), false);
           --expectedExecuteCount;
        } else {
            arktest.assertEQ(handler1.hasMessages(MessageType.NUMBER, 100), true);
        }
        if (isMsg3){
           arktest.assertEQ(handler1.hasMessages(MessageType.BOOLEAN, true), false);
           --expectedExecuteCount;
        } else {
            arktest.assertEQ(handler1.hasMessages(MessageType.BOOLEAN, true), true);
        }
        waiter2.fire();
    };

    handler2.sendMessage(new concurrency.Message(callback, handler2));
    handler1.sendMessage(new concurrency.Message(MessageType.STRING, 'test', handler1));
    handler1.sendMessage(new concurrency.Message(MessageType.NUMBER, 100, handler1));
    handler1.sendMessage(new concurrency.Message(MessageType.BOOLEAN, true, handler1));
    
    waiter1.fire();
    waiter2.wait();

    eaw1.quit();
    eaw2.quit();
    arktest.assertEQ(executeCount, expectedExecuteCount);
}

function testHasMessage2() {
    let eaw = new EAWorker();
    eaw.start();
    let handler: concurrency.MessageHandler;
    let waiter = new Event();

    handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        if (msg.getCallback() != undefined) {
            let cb = msg.getCallback() as Callback;
            cb();
        }
    }, eaw);
    let message = new concurrency.Message(MessageType.STRING, "test", handler);
    let message2 = new concurrency.Message(MessageType.STRING, "test2", handler);

    let callback = () => {
        handler.sendMessage(message);
        handler.sendMessage(message2);
        arktest.assertEQ(handler.hasMessages(MessageType.STRING), true);
        handler.removeMessages(MessageType.STRING, "test");
        arktest.assertEQ(handler.hasMessages(MessageType.STRING), true);
        handler.sendMessage(new concurrency.Message(MessageType.STRING, "test3", handler));
        arktest.assertEQ(handler.hasMessages(MessageType.STRING, "test3"), true);
        handler.removeMessages(MessageType.STRING);
        arktest.assertEQ(handler.hasMessages(MessageType.STRING, "test3"), false);
        arktest.assertEQ(handler.hasMessages(MessageType.STRING, "test2"), false);
        arktest.assertEQ(handler.hasMessages(MessageType.STRING, "test"), false);
        arktest.assertEQ(handler.hasMessages(MessageType.STRING), false);
        waiter.fire();
    }

    handler.sendMessage(new concurrency.Message(callback, handler));
    waiter.wait();
    eaw.join();
}

function testMessageHandlerMismatch() {
    let eaw = new EAWorker();
    eaw.start();
    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        // do nothing
    }, eaw);
    let handler2 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        // do nothing
    }, eaw);

    let msg = new concurrency.Message(MessageType.STRING, "test", handler);
    arktest.assertEQ(handler2.sendMessage(msg), false);
    eaw.join();
}

function testDoubleRemoveMessage() {
    let worker = new EAWorker();
    worker.start();

    let awake_waiter: (val: int)=>void = ()=>{};
    let waiter = new Promise<int>((res, rej)=>{awake_waiter = res;})

    let handler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        await waiter;
    }, worker);

    let message = new concurrency.Message(1, handler);
    handler.sendMessage(message);
    if (handler.removeMessages(1)){
        arktest.assertEQ(handler.removeMessages(1), false);
    }
    awake_waiter(0);
    worker.quit();
}

function main() {
    let testSuite = new arktest.ArkTestsuite('messageHandlerBasicInterface');
    
    testSuite.addTest("testMessageHandlerCreation", testMessageHandlerCreation);
    testSuite.addTest("testSendEmptyMessage", testSendEmptyMessage);
    testSuite.addTest("testSendStringMessage", testSendStringMessage);
    testSuite.addTest("testSendNumberMessage", testSendNumberMessage);
    testSuite.addTest("testSendBooleanMessage", testSendBooleanMessage);
    testSuite.addTest("testHasMessages", testHasMessages);
    testSuite.addTest("testMessageCounting", testMessageCounting);
    testSuite.addTest("testMessageCallback", testMessageCallback);
    testSuite.addTest("testRemoveMultipleMessage", testRemoveMultipleMessages);
    testSuite.addTest("testMessageHandlerMismatch", testMessageHandlerMismatch);
    testSuite.addTest("testDoubleRemoveMessage", testDoubleRemoveMessage);
    testSuite.addTest("testHasMessage2", testHasMessage2);

    testSuite.run();
}
