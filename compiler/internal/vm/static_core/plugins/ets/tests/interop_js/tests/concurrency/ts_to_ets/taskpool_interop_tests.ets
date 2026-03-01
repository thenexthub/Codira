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
    let res = (waitForCompletion(() => taskpool.execute(callback))) as boolean;
    arktest.assertEQ(res, true);
}

function testCreateArray() {
    let callback = ():boolean => {
        let module = ESValue.load('concurrency_helpers');
        let createArrayDy = module.getProperty('createArray');
        // [5]
        let res1 = createArrayDy.invoke(ESValue.wrapNumber(5), ESValue.wrapNumber(1));
        // ['a', 1, 'a', 1]
        let res2 = createArrayDy.invoke(ESValue.wrapString('a'), ESValue.wrapNumber(1), ESValue.wrapNumber(4));
        // [true, true]
        let res3 = createArrayDy.invoke(ESValue.wrapBoolean(true), ESValue.wrapNumber(2));
        return (res1.getProperty(0).toNumber() === 5) &&
                    (res2.getProperty(0).toString() === 'a') &&
                    (res3.getProperty(0).toBoolean() === true);
    };
    let res = (waitForCompletion(() => taskpool.execute(callback))) as boolean;
    arktest.assertEQ(res, true);
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
    let res = (waitForCompletion(() => taskpool.execute(callback))) as boolean;
    arktest.assertEQ(res, true);
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
        let TestClass = module.getProperty('TestClass');
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
    waitForCompletion(() => taskpool.execute(callback));
  } catch (error) {
    console.error(error.toString());
    arktest.assertFalse(result);
  }
  arktest.assertEQ(result, true);
}

function testBaseClass() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('Base').instantiate().invokeMethod('getCurrent')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testChildClass() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('Child').instantiate().invokeMethod('getSuper')
        arktest.assertTrue(obj.typeOf() === 'object')
        arktest.assertTrue(obj.getProperty('baseVal').toNumber() === 1)
        arktest.assertTrue(obj.getProperty('childVal').toNumber() === 2)
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testBaseFunc() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('baseFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testChildFunc() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let obj = module.getProperty('childFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(obj, ESValue.Undefined))
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testFooBaseObjInvoke() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testFooBaseObjInvokeMethod() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooBaseObj = module.getProperty('fooBaseObj')
        let result = fooBaseObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooBaseObj))
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testFooChildObjInvoke() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.getProperty('foo').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testFooChildObjInvokeMethod() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let fooChildObj = module.getProperty('fooChildObj')
        let result = fooChildObj.invokeMethod('fooFunc')
        arktest.assertTrue(ESValue.areStrictlyEqual(result, fooChildObj))
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testArrowFuncInvoke() {
    let callback = () => {
        let module = ESValue.load('concurrency_helpers');
        let result = module.getProperty('arrowFunc').invoke()
        arktest.assertTrue(ESValue.areStrictlyEqual(result, ESValue.Undefined))
    }
    waitForCompletion(() => taskpool.execute(callback));
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
    waitForCompletion(() => taskpool.execute(callback));
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
    let res = (waitForCompletion(() => taskpool.execute(callback))) as boolean;
    arktest.assertEQ(res, true);
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
    let res = (waitForCompletion(() => taskpool.execute(callback))) as boolean;
    arktest.assertEQ(res, true);
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
    let res = (waitForCompletion(() => taskpool.execute(callback))) as boolean;
    arktest.assertEQ(res, true);
}

function testTaskpoolNestedExecute() {
    let callback = ():boolean => {
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
        let resInner = (waitForCompletion(() => taskpool.execute(callbackInner))) as boolean;
        arktest.assertTrue(resInner);
        return res && resInner;
    };
    let res = (waitForCompletion(() => taskpool.execute(callback))) as boolean;
    arktest.assertEQ(res, true);
}

function testTaskpoolInteropLoop() {
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
    }
    waitForCompletion(() => taskpool.execute(callback));
}

function testSerializeFunc(serializeTestObj: ESValue) {
    let value1 = serializeTestObj.getProperty('value1').toString();
    let value2 = serializeTestObj.getProperty('value2').toNumber();
    let valueArray = serializeTestObj.getProperty('valueArray').getProperty(0).toString();
    arktest.assertEQ(value1, 'SerializeTest');
    arktest.assertEQ(value2, 1);
    arktest.assertEQ(valueArray, '1');
    return true;
}

function testTaskpoolSerialize() {
    let module = ESValue.load('concurrency_helpers');
    let serializeTestObj = module.getProperty('serializeTestObj');
    let res = (waitForCompletion(() => taskpool.execute(testSerializeFunc, serializeTestObj))) as boolean;
    arktest.assertEQ(res, true);
}

function testTaskpoolSerializeClassInstantiate() {
    let module = ESValue.load('concurrency_helpers');
    let SerializeTest = module.getProperty('SerializeTest');
    let serializeTestObj = SerializeTest.instantiate('SerializeTest', 1, ['1', '2', '3']);
    let res = (waitForCompletion(() => taskpool.execute(testSerializeFunc, serializeTestObj))) as boolean;
    arktest.assertEQ(res, true);
}

function testTaskpoolSerializeUndefined() {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeUndefined');
    let callback = (value: ESValue) => {
        arktest.assertTrue(value.isUndefined());
    }
    waitForCompletion(() => taskpool.execute(callback, serializeObj));
}

function testTaskpoolSerializeNull() {
    let module = ESValue.load('concurrency_helpers');
    let serializeObj = module.getProperty('serializeNull');
    let callback = (value: ESValue) => {
        arktest.assertTrue(value.isNull());
    }
    waitForCompletion(() => taskpool.execute(callback, serializeObj));
}

function taskpoolInteropTest() {
    let taskpoolInteropSuite = new arktest.ArkTestsuite('taskpool.InteropTest');
    taskpoolInteropSuite.addTest('testProcessInput', testProcessInput);
    taskpoolInteropSuite.addTest('testCreateArray', testCreateArray);
    taskpoolInteropSuite.addTest('testGetUser', testGetUser);
    // Flaky fail: #30965
    // taskpoolInteropSuite.addTest('testUnion', testUnion);
    taskpoolInteropSuite.addTest('testBaseClass', testBaseClass);
    taskpoolInteropSuite.addTest('testChildClass', testChildClass);
    taskpoolInteropSuite.addTest('testBaseFunc', testBaseFunc);
    taskpoolInteropSuite.addTest('testChildFunc', testChildFunc);
    taskpoolInteropSuite.addTest('testFooBaseObjInvoke', testFooBaseObjInvoke);
    taskpoolInteropSuite.addTest('testFooBaseObjInvokeMethod', testFooBaseObjInvokeMethod);
    taskpoolInteropSuite.addTest('testFooChildObjInvoke', testFooChildObjInvoke);
    taskpoolInteropSuite.addTest('testFooChildObjInvokeMethod', testFooChildObjInvokeMethod);
    taskpoolInteropSuite.addTest('testArrowFuncInvoke', testArrowFuncInvoke);
    taskpoolInteropSuite.addTest('testOptionalParams', testOptionalParams);
    taskpoolInteropSuite.addTest('testError', testError);
    taskpoolInteropSuite.addTest('testThrowError', testThrowError);
    taskpoolInteropSuite.addTest('testCustomThrowError', testCustomThrowError);
    taskpoolInteropSuite.addTest('testTaskpoolNestedExecute', testTaskpoolNestedExecute);
    taskpoolInteropSuite.addTest('testTaskpoolInteropLoop', testTaskpoolInteropLoop);
    taskpoolInteropSuite.addTest('testTaskpoolSerialize', testTaskpoolSerialize);
    taskpoolInteropSuite.addTest('testTaskpoolSerializeClassInstantiate', testTaskpoolSerializeClassInstantiate);
    taskpoolInteropSuite.addTest('testTaskpoolSerializeUndefined', testTaskpoolSerializeUndefined);
    taskpoolInteropSuite.addTest('testTaskpoolSerializeNull', testTaskpoolSerializeNull);
    const res = taskpoolInteropSuite.run();
    CoroutineExtras.stopTaskpool();
    return res == 0;
}
