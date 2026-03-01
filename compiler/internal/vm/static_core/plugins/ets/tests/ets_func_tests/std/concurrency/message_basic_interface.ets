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


enum MessageType {
    EMPTY = 0,
    STRING = 1,
    NUMBER = 2,
    BOOLEAN = 3,
    ARRAY = 4,
}

function messageBasicInterfaceTest() {
    let eaw = new EAWorker();
    let handler = new concurrency.MessageHandler((msg:concurrency.Message)=>{
        let a = 0;
    }, eaw);


    let emptyMessage = new concurrency.Message(MessageType.EMPTY, handler);
    let msg1 = new concurrency.Message(MessageType.STRING, "msg1", handler);
    let msg2 = new concurrency.Message(MessageType.NUMBER, 100, handler);
    let msg3 = new concurrency.Message(MessageType.BOOLEAN, true, handler);
    let array = [1, 2, 3, 4, 5];
    let msg4 = new concurrency.Message(MessageType.ARRAY, array, handler);
    
    let callback = ()=>{
        let a = 0;
    }

    let msg5 = new concurrency.Message(callback, handler);

    arktest.assertEQ(emptyMessage.getWhat(), MessageType.EMPTY);
    arktest.assertEQ(msg1.getWhat(), MessageType.STRING);
    arktest.assertEQ(msg2.getWhat(), MessageType.NUMBER);
    arktest.assertEQ(msg3.getWhat(), MessageType.BOOLEAN);
    arktest.assertEQ(msg4.getWhat(), MessageType.ARRAY);

    arktest.assertEQ(emptyMessage.getTarget(), handler);
    arktest.assertEQ(msg1.getTarget(), handler);
    arktest.assertEQ(msg2.getTarget(), handler);
    arktest.assertEQ(msg3.getTarget(), handler);
    arktest.assertEQ(msg4.getTarget(), handler);

    arktest.assertEQ(emptyMessage.getObject(), undefined);
    arktest.assertEQ(msg1.getObject(), "msg1");
    arktest.assertEQ(msg2.getObject(), 100);
    arktest.assertEQ(msg3.getObject(), true);
    arktest.assertEQ(msg4.getObject(), array);

    arktest.assertEQ(msg5.getCallback(), callback);
    eaw.join();
}

function main() {
    let testSuite = new arktest.ArkTestsuite('MessageBasicInterface');
    testSuite.addTest('testMessageBasicInterface', messageBasicInterfaceTest);
    testSuite.run();
}
