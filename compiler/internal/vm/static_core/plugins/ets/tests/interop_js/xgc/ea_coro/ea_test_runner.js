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

exports.runTest = function (cppTestModule, stsFile, testFunc, ...eaWorkerModules) {
    let etsVm = requireNapiPreview('ets_interop_js_napi', true);
    let testModule = requireNapiPreview(cppTestModule, true);

    const etsVmRes = etsVm.createRuntime({
        'boot-panda-files': 'etsstdlib.abc:' + stsFile,
        'log-components': 'ets_interop_js:gc_trigger',
        'log-level': 'debug',
        'gc-trigger-type': 'debug-never',
        'compiler-enable-jit': 'false',
        'coroutine-workers-count': '2',
    });
    if (!etsVmRes) {
        throw new Error('Failed to create ETS runtime');
    }

    testModule.setupAndStartEAWorkers(...eaWorkerModules);
    try {
        testFunc(etsVm, testModule);
    } catch (e) {
        testModule.fireEventToEAWorkers();
        throw e;
    } finally {
        testModule.joinEAWorkers();
    }
};

exports.runXGC = function (etsVm) {
    globalThis.ArkTools.GC.clearWeakRefForTest();
    etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'xgc')();
};

exports.runEtsFullGC = function (etsVm) {
    etsVm.getFunction('Lxgc_ea/ETSGLOBAL;', 'runFullGC')();
};

exports.runJsFullGC = function () {
    globalThis.ArkTools.GC.clearWeakRefForTest();
    let gcId = globalThis.ArkTools.GC.startGC('full');
    globalThis.ArkTools.GC.waitForFinishGC(gcId);
};
