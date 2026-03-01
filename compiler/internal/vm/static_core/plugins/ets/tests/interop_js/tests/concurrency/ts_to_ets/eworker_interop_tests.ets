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

let eaw = new EAWorker(true);

function testProcessInput() {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let processInputDy = module.getProperty('processInput');
        let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
        let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
        let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
        let res = (res1.toNumber() === 10) && (res2.toString() === 'HELLO') && (!res3.toBoolean());
        return res;
    };

    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

function testCreateArray() {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let createArrayDy = module.getProperty('createArray');
        let res1 = createArrayDy.invoke(ESValue.wrapNumber(5), ESValue.wrapNumber(1));
        let res2 = createArrayDy.invoke(ESValue.wrapString('a'), ESValue.wrapNumber(1), ESValue.wrapNumber(4));
        let res3 = createArrayDy.invoke(ESValue.wrapBoolean(true), ESValue.wrapNumber(2));
        return (res1.getProperty(0).toNumber() === 5) &&
                    (res2.getProperty(0).toString() === 'a') &&
                    (res3.getProperty(0).toBoolean() === true);
    };
    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

function testGetUser() {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let getUserDy = module.getProperty('getUser');
        // { id: 123, name: 'John Doe' }
        let res1 = getUserDy.invoke(ESValue.wrapNumber(123));
        // { id: 1, name: 'admin' }
        let res2 = getUserDy.invoke(ESValue.wrapString('admin'));
        return res1.getProperty('id').toNumber() === 123 &&
                    res2.getProperty('role').toString() === 'admin';
    };
    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

class NewTestClass {
  prop: number = 1;
}

function testUnion() {
  let result = false;
  try {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let dynamicObj = module.getProperty('dynamicObj');
        let TestClass = module.getProperty("TestClass");
        dynamicObj.setProperty('unionProp', ESValue.wrapNumber(123));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').toNumber() === 123);
        dynamicObj.setProperty('unionProp', ESValue.wrapString('123'));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').toString() === '123');
        dynamicObj.setProperty('unionProp', ESValue.wrapBoolean(true));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').toBoolean() === true);
        let testInstance = TestClass.instantiate();
        dynamicObj.setProperty('unionProp', testInstance);
        arktest.assertTrue(dynamicObj.getProperty('unionProp').getProperty('name').toString() === 'aaa');
        
        dynamicObj.setProperty('unionProp', ESValue.wrap(new NewTestClass()));
        arktest.assertTrue(dynamicObj.getProperty('unionProp').getProperty('prop').toNumber() === 1);
        result = true;
    }
    eaw.run<void>(callback).Await();
  } catch (error) {
    console.error(error.toString());
    arktest.assertFalse(result);
  }
  arktest.assertTrue(result);
}

function testBaseClass() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers')
        let obj = module.getProperty('Base').instantiate().invokeMethod('getCurrent')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    }
    eaw.run<void>(callback).Await();
}

function testChildClass() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers')
        let obj = module.getProperty('Child').instantiate().invokeMethod('getSuper')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
        arktest.assertTrue(obj.getProperty('childVal').toNumber() === 2)
    }
    eaw.run<void>(callback).Await();
}

function testBaseFunc() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers')
        let obj = module.getProperty('baseFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testChildFunc() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('childFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testFooBaseObjInvoke() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testFooBaseObjInvokeMethod() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooBaseObj))
    }
    eaw.run<void>(callback).Await();
}

function testFooChildObjInvoke() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testFooChildObjInvokeMethod() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooChildObj))
    }
    eaw.run<void>(callback).Await();
}

function testArrowFuncInvoke() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let result = module.getProperty('arrowFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    eaw.run<void>(callback).Await();
}

function testOptionalParams() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        arktest.assertTrue(module.getProperty('foo1').invoke(ESValue.wrap(1), ESValue.wrap('str')).toBoolean());
        arktest.assertTrue(module.getProperty('foo2').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(true)).toBoolean());
        arktest.assertTrue(module.getProperty('foo3').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(true), ESValue.wrap(2)).toBoolean());

        arktest.assertTrue(module.getProperty('fun1').invoke(ESValue.wrap(1), ESValue.wrap('str')).toBoolean());
        arktest.assertTrue(module.getProperty('fun2').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(false)).toBoolean());
        arktest.assertTrue(module.getProperty('fun3').invoke(ESValue.wrap(1), ESValue.wrap('str'), ESValue.wrap(false), ESValue.wrap(2)).toBoolean());
    }
    eaw.run<void>(callback).Await();
}


const ErrMsg: string = 'This is a Error';
const ErrorMessage: string = 'Panda: throw new error in ets!';
const CustomErrorMessage: string = 'Panda: Something went wrong!';
const CustomErrorCode: number = 1001;

function testError() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let err = module.getProperty('err');
        let res: boolean;
        res = (err.getProperty('message').toString() == ErrMsg);
        return res;
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

function testThrowError() {
    let callback = () => {
    let module = ESValue.load('concurrency_helpers');
    let res: boolean;
    try {
        module.getProperty('ErrorFunc').invoke(ESValue.wrapString(ErrorMessage));
        res = false;
        } catch (error) {
            res = (error instanceof Error) && (error.message == ErrorMessage);
        }
        return res;
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

function testCustomThrowError() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let res: boolean;
        try {
            module.getProperty('CustomErrorFunc').invoke(ESValue.wrapString(CustomErrorMessage), ESValue.wrapNumber(CustomErrorCode));
            res = false;
        } catch (e) {
            res = (e instanceof Error) && (e.message == CustomErrorMessage) &&
                (e.code == CustomErrorCode);
        }
        return res;
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertTrue(res);
}

function testEAWorkerNestedExecute() {
    let callback = ():boolean => {
        let innerWorker = new EAWorker(true);
        innerWorker.start();
        let module = ESValue.load('concurrency_helpers');
        let processInputDy = module.getProperty('processInput');
        let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
        let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
        let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
        let res = (res1.toNumber() === 10) && (res2.toString() === 'HELLO') && (!res3.toBoolean());

        let callbackInner = ():boolean => {
            let module = ESValue.load('concurrency_helpers');
            let processInputDy = module.getProperty('processInput');
            let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
            let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
            let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
            return res1.toNumber() === 10 && res2.toString() === 'HELLO' && !res3.toBoolean();
        }
        let resInner = innerWorker.run<boolean>(callbackInner).Await() as boolean;
        innerWorker.join();
        arktest.assertTrue(resInner);
        return res && resInner;
    };
    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertEQ(res, true);
}

function testEAWorkerInteropLoop() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let processInputDy = module.getProperty('processInput');
        for (let i = 0; i < 100; i++) {
            let res1 = processInputDy.invoke(ESValue.wrapNumber(5)); //  2 * 5 = 10
            let res2 = processInputDy.invoke(ESValue.wrapString('hello')); // 'HELLO'
            let res3 = processInputDy.invoke(ESValue.wrapBoolean(true)); // false
            arktest.assertEQ(res1.toNumber(), 10);
            arktest.assertEQ(res2.toString(), 'HELLO');
            arktest.assertEQ(res3.toBoolean(), false);
        }
        return true;
    }
    let res = eaw.run<boolean>(callback).Await() as boolean;
    arktest.assertEQ(res, true);
}

function testFunc(serializeTestObj: ESValue) {
    let value1 = serializeTestObj.getProperty('value1').toString();
    let value2 = serializeTestObj.getProperty('value2').toNumber();
    let valueArray = serializeTestObj.getProperty('valueArray').getProperty(0).toString();
    arktest.assertEQ(value1, 'SerializeTest');
    arktest.assertEQ(value2, 1);
    arktest.assertEQ(valueArray, '1');
    return true;
}

function testEAWorkerSerialize() {
    let module = ESValue.load('concurrency_helpers');
    let serializeTestObj = module.getProperty('serializeTestObj');
    let res = eaw.run<boolean>(testFunc, serializeTestObj).Await() as boolean;
    arktest.assertEQ(res, true);
}

function testEAWorkerSerializeClassInstantiate() {
    let module = ESValue.load('concurrency_helpers');
    let SerializeTest = module.getProperty('SerializeTest');
    let serializeTestObj = SerializeTest.instantiate('SerializeTest', 1, ['1', '2', '3']);
    let res = eaw.run<boolean>(testFunc, serializeTestObj).Await() as boolean;
    arktest.assertEQ(res, true);
}

function testEAWorkerSerializeUndefined() {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeUndefined');
    let callback = (value: ESValue) => {
        return value.isUndefined();
    }
    let res = eaw.run<boolean>(callback, serializeObj).Await() as boolean;
    arktest.assertEQ(res, true);
}

function testEAWorkerSerializeNull() {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeNull');
    let callback = (value: ESValue) => {
        return value.isNull();
    }
    let res = eaw.run<boolean>(callback, serializeObj).Await() as boolean;
    arktest.assertEQ(res, true);
}

function testEAWorkerMessageCancellation() {
    let handler = new concurrency.MessageHandler((msg: concurrency.Message)=>{
        if (msg.getObject() != undefined) {
            console.log("msg.getObject() " + msg.getObject());
        }
        if (msg.getCallback() != undefined) {
            console.log("callback");
            let callback = msg.getCallback() as () => void;
            callback();
        }
    }, eaw);

    let msg1 = new concurrency.Message(1, handler);
    let msg2 = new concurrency.Message(2, "msg2", handler);
    let cb = ()=>{
        arktest.assertEQ(handler.removeMessages(1), true);
        arktest.assertEQ(handler.removeMessages(2, "msg2"), true);
        arktest.assertEQ(handler.hasMessages(1), false);
        arktest.assertEQ(handler.hasMessages(2, "msg2"), false);
    }
    let msg3 = new concurrency.Message(cb, handler);

    arktest.assertEQ(handler.sendMessage(msg3), true);
    arktest.assertEQ(handler.sendMessage(msg2), true);
    arktest.assertEQ(handler.sendMessage(msg1), true);
}
class Event {
    public Wait() {
        while (this.flag.get() != true) {
            Coroutine.Schedule();
        }
    }

    public Fire() {
        this.flag.set(true);
    }

    private flag = new AtomicFlag(false);
};

type Callback = () => void;
enum MessageType {
    EMPTY,
    STRING,
    NUMBER,
    BOOLEAN,
    ARRAY,
}
function removeMessageHandlerTest() {
    let waiter = new Event();
    let handler: concurrency.MessageHandler;
    let restExecuted = false;
    handler = new concurrency.MessageHandler((msg: concurrency.Message)=>{
        if (msg.getWhat() == 1) {
            restExecuted = true;
        }
        if (msg.getCallback() != undefined) {
            let callback = msg.getCallback() as Callback;
            callback();
        }
    }, eaw);

    let msg = new concurrency.Message(1, handler);
    let callback = () => {
        handler.removeMessages(1);
        waiter.Fire();
    }
    let msg2 = new concurrency.Message(callback, handler);

    handler.sendMessage(msg2);
    handler.sendMessage(msg);

    waiter.Wait();
    arktest.assertEQ(restExecuted, false);
}

function testEAWorkerCommmunicateWithMessage()
{
    let eaw2 = new EAWorker();
    let waiter = new Event();
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
    }, eaw);

    handler2 = new concurrency.MessageHandler((msg: concurrency.Message) => {
        arktest.assertEQ(msg.getWhat(), eaw1Tag);
        arktest.assertEQ(msg.getObject(), "message from eaw1");
        messageGot = true;
        waiter.Fire();
    }, eaw2);

    handler1.sendMessage(new concurrency.Message(eaw2Tag, "message from eaw2", handler1));

    waiter.Wait();
    arktest.assertEQ(messageGot, true);
    eaw2.join();
}

function eworkerInteropTest() {
    eaw.start();
    let eworkerInteropSuite = new arktest.ArkTestsuite('eworker.InteropTest');
    eworkerInteropSuite.addTest('testProcessInput', testProcessInput);
    eworkerInteropSuite.addTest('testCreateArray', testCreateArray);
    eworkerInteropSuite.addTest('testGetUser', testGetUser);
    eworkerInteropSuite.addTest('testUnion', testUnion);
    eworkerInteropSuite.addTest('testBaseClass', testBaseClass);
    eworkerInteropSuite.addTest('testChildClass', testChildClass);
    eworkerInteropSuite.addTest('testBaseFunc', testBaseFunc);
    eworkerInteropSuite.addTest('testChildFunc', testChildFunc);
    eworkerInteropSuite.addTest('testFooBaseObjInvoke', testFooBaseObjInvoke);
    eworkerInteropSuite.addTest('testFooBaseObjInvokeMethod', testFooBaseObjInvokeMethod);
    eworkerInteropSuite.addTest('testFooChildObjInvoke', testFooChildObjInvoke);
    eworkerInteropSuite.addTest('testFooChildObjInvokeMethod', testFooChildObjInvokeMethod);
    eworkerInteropSuite.addTest('testArrowFuncInvoke', testArrowFuncInvoke);
    eworkerInteropSuite.addTest('testOptionalParams', testOptionalParams);
    eworkerInteropSuite.addTest('testError', testError);
    eworkerInteropSuite.addTest('testThrowError', testThrowError);
    eworkerInteropSuite.addTest('testCustomThrowError', testCustomThrowError);
    eworkerInteropSuite.addTest('testEAWorkerNestedExecute', testEAWorkerNestedExecute);
    eworkerInteropSuite.addTest('testEAWorkerInteropLoop', testEAWorkerInteropLoop);
    eworkerInteropSuite.addTest('testEAWorkerSerialize', testEAWorkerSerialize);
    eworkerInteropSuite.addTest('testEAWorkerSerializeClassInstantiate', testEAWorkerSerializeClassInstantiate);
    eworkerInteropSuite.addTest('testEAWorkerSerializeUndefined', testEAWorkerSerializeUndefined);
    eworkerInteropSuite.addTest('testEAWorkerSerializeNull', testEAWorkerSerializeNull);
    eworkerInteropSuite.addTest('testEAWorkerMessageCancellation', testEAWorkerMessageCancellation);
    eworkerInteropSuite.addTest('testRemoveMessageHandlerTest', removeMessageHandlerTest);
    eworkerInteropSuite.addTest('testEAWorkerCommmunicateWithMessage', testEAWorkerCommmunicateWithMessage);
    const res = eworkerInteropSuite.run();
    eaw.join();
    return res == 0;
}
