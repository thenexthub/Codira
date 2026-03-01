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

const CROSS_REFS_NUMBER: number = 10;

function loadStaticVM(): Object {
    let etsVm = requireNapiPreview('ets_interop_js_napi', true);
    if (!etsVm.createRuntime({
        'log-level': 'debug',
        'log-components': 'ets_interop_js:gc_trigger',
        'boot-panda-files': 'etsstdlib.abc:gc_test_sts_common.abc',
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'coroutine-workers-count': '1',
        'run-gc-in-place': 'true', // Required option to check XGC call by trigger
        'xgc-min-trigger-threshold': CROSS_REFS_NUMBER.toString()
    })) {
        throw new Error('Failed to create ETS runtime');
    }
    return etsVm;
}

function getSTSWeakRef(etsVm): WeakRef<Object> {
    return new WeakRef(etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetSTSObjectWithWeakRef')());
}

function main(): void {
    let etsVm = loadStaticVM();
    let wr = new WeakRef<Object>(new Object);
    for (let i = 0; i < CROSS_REFS_NUMBER; ++i) {
        wr = getSTSWeakRef(etsVm);
    }
    globalThis.ArkTools.GC.clearWeakRefForTest();
    // It should trigger XGC trigger
    let stsObject = etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetSTSObject')();
    print('--- Start JS GC Full ---');
    globalThis.ArkTools.GC.clearWeakRefForTest();
    let gcId = globalThis.ArkTools.GC.startGC('full');
    globalThis.ArkTools.GC.waitForFinishGC(gcId);
    print('--- Finish JS GC Full ---');
    etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'RunPandaGC')();
    //
    if (wr.deref() !== undefined) {
        throw new Error('XGC trigger was not called');
    }
    if (stsObject === undefined) {
        throw new Error('nullish stsObject');
    }
    return;
}

main();
