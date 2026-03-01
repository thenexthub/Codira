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

const helper = requireNapiPreview('libinterop_test_helper.so', false);

async function runTest(test) {
    const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
    const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');
    const packageName = helper.getEnvironmentVar('PACKAGE_NAME');
	if (!packageName) {
		throw Error('PACKAGE_NAME is not set');
	}
	const globalName = 'L' + packageName + '/ETSGLOBAL;';

    let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);

    let runtimeCreated = etsVm.createRuntime({
        'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
        'panda-files': gtestAbcPath,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
        'coroutine-impl': 'stackful',
        'coroutine-workers-count': 2,
        'xgc-trigger-type': 'never',
        // 'log-debug': 'coroutines'
    });
    if (!runtimeCreated) {
        throw Error('Cannot create ETS runtime');
    }
    let valueToResolveWith = 42;
    const runTestImpl = etsVm.getFunction(globalName, test);
    let promise = runTestImpl(valueToResolveWith);
    if (promise == null) {
        throw Error('Function returned null');
    }
    const signalPromiseInJs = etsVm.getFunction(globalName, 'signalPromiseInJs');
    signalPromiseInJs();
    try {
        let result = await promise;
        if (result !== valueToResolveWith) {
            throw Error('Promise was not resolved correctly: result: ', result, ' expected: ', valueToResolveWith);
        }
    } catch (error) {
        throw Error('Promise was rejected with error:', error);
    }
}

let args = helper.getArgv();
if (args.length !== 6) {
    throw Error('Expected test name');
}
runTest(args[4]);
