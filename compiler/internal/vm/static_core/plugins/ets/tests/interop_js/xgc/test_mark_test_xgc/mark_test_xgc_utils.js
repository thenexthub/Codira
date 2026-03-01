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
let etsVm;
let testModule;

class JsTestClass {
    constructor() {
        this.obj = null;
    }
}

function init(module, stsFile) {
    etsVm = requireNapiPreview('ets_interop_js_napi', true);
    testModule = requireNapiPreview(module, true);

    const etsVmRes = etsVm.createRuntime({
        'load-runtimes': 'ets',
        'log-components': 'ets_interop_js',
        'boot-panda-files': 'etsstdlib.abc:' + stsFile,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
        'coroutine-workers-count': '1',
        'enable-xgc-verifier': 'true',
    });
    if (!etsVmRes) {
        throw new Error('Failed to create ETS runtime');
    } else {
        print('ETS runtime created');
    }
    testModule.setup();
    return etsVm;
}

function triggerXGC() {
    const xgc = etsVm.getFunction('Lxgc_test_ets/ETSGLOBAL;', 'xgc');
    xgc();
}

function checkXRefsNumber(jsNum, stsNum) {
    testModule.checkXRefsNumber(jsNum, stsNum);
}

function validationXGCResult(beforeJsNum, beforeStsNum, afterJsNum, afterStsNum) {
    checkXRefsNumber(beforeJsNum, beforeStsNum);
    triggerXGC();
    checkXRefsNumber(afterJsNum, afterStsNum);
}

module.exports = {
    init,
    triggerXGC,
    checkXRefsNumber,
    validationXGCResult,
    JsTestClass,
};
