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
exports.runTest = function (module, stsFile, testFn) {
    let etsVm = requireNapiPreview('ets_interop_js_napi', true);
    let test = requireNapiPreview(module, true);

    const etsVmRes = etsVm.createRuntime({
        'log-components': 'ets_interop_js',
        'boot-panda-files': 'etsstdlib.abc:' + stsFile,
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
    test.setup();
    testFn(etsVm);
    const xgc = etsVm.getFunction('Lxgc_test/ETSGLOBAL;', 'xgc');
    xgc();
    test.check();
};
