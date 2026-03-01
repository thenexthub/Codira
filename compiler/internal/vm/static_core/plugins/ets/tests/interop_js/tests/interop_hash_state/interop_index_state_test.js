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
const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

globalThis.etsVm = requireNapiPreview('ets_interop_js_napi.so', false);

function runTest(test) {
    print('Running test ' + test);
	const etsVmRes = etsVm.createRuntime({
        'log-level': 'error',
        'log-components': 'ets_interop_js',
        'boot-panda-files': stdlibPath + ':' + gtestAbcPath,
        'panda-files': gtestAbcPath,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
    });
    if (!etsVmRes) {
        throw Error(`Failed to create ETS runtime`);
    }

    if (test === 'hash_to_info') {
        hashToInfo(etsVm);
    } else {
	    throw Error('Expected test name');
    }
}

function hashToInfo(etsVm) {
    // This test should show correct usage of interop hash for object that already have hash
    const createHashedObject = etsVm.getFunction('Lhashed_object/ETSGLOBAL;', 'createHashedObject');
    let obj = createHashedObject();
    const checkIfInSet = etsVm.getFunction('Lhashed_object/ETSGLOBAL;', 'checkIfInSet');
    if (!checkIfInSet(obj)) {
        throw Error('Problem with hash');
    }
}

const testNameNum = 6;
let args = helper.getArgv();
if (args.length !== testNameNum) {
	throw Error('Expected test name');
}

runTest(args[testNameNum - 1]);
