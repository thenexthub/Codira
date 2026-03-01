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

function init() {
    const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
    const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');
	if (!helper.getEnvironmentVar('PACKAGE_NAME')) {
		throw Error('PACKAGE_NAME is not set');
    }

    let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);
    const etsOpts = {
        'panda-files': gtestAbcPath,
        'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
        'xgc-trigger-type': 'never',
    };
    if (!etsVm.createRuntime(etsOpts)) {
        throw Error('Cannot create ETS runtime');
    }
    return etsVm;
}

function runTest() {
    let etsVm = init();
    let tId = 0;
    const globalName = 'L' + helper.getEnvironmentVar('PACKAGE_NAME') + '/ETSGLOBAL;';
    let waitForSchedule = () => {
        const isWasScheduled = etsVm.getFunction(globalName, 'wasScheduled');
        let wasSchedulded = isWasScheduled();
        if (wasSchedulded) {
            clearInterval(tId);
        }
    };

    const waitUntillJsIsReady = etsVm.getFunction(globalName, 'waitUntillJsIsReady');
    waitUntillJsIsReady();
    const jsIsReady = etsVm.getFunction(globalName, 'jsIsReady');
    jsIsReady();
    tId = setInterval(waitForSchedule, 0);
}

runTest();
