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
let PROMISE_RESOLVE_VALUE = 'JS_PROMISE_RESULT';

function sleep(ms) {
    return new Promise((resolve) => {
        setTimeout(() => { resolve(PROMISE_RESOLVE_VALUE) }, ms);
    });
}

async function main() {
    const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
    const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');
    let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);
    const etsVmRes = etsVm.createRuntime({
        'panda-files': gtestAbcPath,
        'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
        'load-runtimes': 'ets',
        'compiler-enable-jit': 'false',
        'coroutine-workers-count': '8',
        'xgc-trigger-type': 'never'
    });

    if (!etsVmRes) {
        throw new Error('Failed to create ETS runtime');
    }
    let packageName = '';
    if (helper.getEnvironmentVar('PACKAGE_NAME')) {
        packageName = helper.getEnvironmentVar('PACKAGE_NAME') + '/';
    } else {
        throw Error('PACKAGE_NAME not set');
    }

    const testAwaitJsPromiseSimple = etsVm.getFunction('L' + packageName + 'ETSGLOBAL;', 'testAwaitJsPromiseSimple');
    await testAwaitJsPromiseSimple(sleep(1000));
    await testAwaitJsPromiseSimple(sleep(1000));
}

main();
