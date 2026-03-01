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

let ASSERT_EQ = function assertEq(v0, v1) {
    if (!Object.is(v0, v1)) {
        let msg = `ASSERTION FAILED: ${v0}[${typeof v0}] !== ${v1}[${typeof v1}]`;
        throw Error(msg);
    }
};

function runTest() {

    let etsVm = requireNapiPreview('ets_interop_js_napi', true);
    let test = requireNapiPreview('serialize_interop_tests_module', true);

    const etsVmRes = etsVm.createRuntime({
        'load-runtimes': 'ets',
        'boot-panda-files': 'etsstdlib.abc:' + 'serialize_test.abc',
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
        'coroutine-workers-count': '1',
    });
    if (!etsVmRes) {
        throw new Error('Failed to create ETS runtime');
    } else {
        print('ETS runtime created');
    }

    let module = etsVm.getClass('Lserialize_test/ETSGLOBAL;');
    let testClass = etsVm.getClass('Lserialize_test/TestClass;');
    let testFunction = module.testFunction;
    let testArray = module.testArray;
    let testInstance = module.testInstance;
    let testDynamicInstance = {a: 1, b: '2'};

    let o = {
        testClass: testClass,
        testFunction: testFunction,
        testArray: testArray,
        testInstance: testInstance,
        testDynamicInstance: testDynamicInstance
    };

    let ptr = test.napiSerialize(o);
    let o2 = test.napiDeserialize(ptr);
    ASSERT_EQ(o2.testClass.testStaticMethod(), 'testStaticMethod');
    ASSERT_EQ(o2.testClass.staticProp, 'staticProp');
    ASSERT_EQ(new o2.testClass().prop, 'prop');
    ASSERT_EQ(new o2.testClass().testMethod(), 'testMethod');
    ASSERT_EQ(o2.testArray[0], 'testArrayElement');
    ASSERT_EQ(o2.testInstance.prop, 'prop');
    ASSERT_EQ(o2.testFunction(), 'testFunction');
    ASSERT_EQ(o2.testDynamicInstance.a, 1);
    module.checkAsync(ptr);
}

runTest();
