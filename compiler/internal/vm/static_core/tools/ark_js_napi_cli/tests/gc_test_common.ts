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

// This is a test common file on JS VM side

export namespace interop {

globalThis.test = {};
globalThis.test.etsVm = requireNapiPreview('ets_interop_js_napi', true);
if (!globalThis.test.etsVm.createRuntime({
    'log-level': 'debug',
	'log-components': 'ets_interop_js:gc_trigger',
	'boot-panda-files': 'etsstdlib.abc:gc_test_sts_common.abc',
	'gc-trigger-type': 'heap-trigger',
	'compiler-enable-jit': 'false',
    'coroutine-workers-count': '1'
})) {
    throw new Error('Failed to create ETS runtime');
}

let RunJsGC: () => void = () => {
    globalThis.ArkTools.GC.clearWeakRefForTest();
    print('--- Start JS GC Full ---');
    let gcId = globalThis.ArkTools.GC.startGC('full');
    globalThis.ArkTools.GC.waitForFinishGC(gcId);
    print('--- Finish JS GC Full ---');
};
globalThis.test.RunJsGC = RunJsGC;

export let GetSTSObject: () => Object = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetSTSObject');
export let GetSTSArrayOfObjects: () => Object[] = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetSTSArrayOfObjects');
export let GetSTSObjectAndKeepIt: () => Object = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetSTSObjectAndKeepIt');
export let GetSTSObjectWithWeakRef: () => Object = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetSTSObjectWithWeakRef');
export let GetSTSStringArrayAsObject: () => Object = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetSTSStringArrayAsObject');
export let isSTSObjectCollected: () => boolean = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'isSTSObjectCollected');
export let isSTSPromiseCollected: () => boolean = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'isSTSPromiseCollected');
export let SetJSObject: (obj: Object) => void = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'SetJSObject');
export let SetJSObjectAndKeepIt: (obj: Object) => void = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'SetJSObjectAndKeepIt');
export let SetJSObjectWithWeakRef: (obj: Object) => void = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'SetJSObjectWithWeakRef');
export let SetJSPromiseWithWeakRef: (prms: Promise<Object>) => void = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'SetJSPromiseWithWeakRef');
export let AddPandaArray: (arr: Object[]) => void = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'AddPandaArray');
export let RunPandaGC: () => void = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'RunPandaGC');
let RunInteropGCInternal: () => void = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'RunInteropGC');
export let RunInteropGC: () => void = () => {
    globalThis.ArkTools.GC.clearWeakRefForTest();
    RunInteropGCInternal();
};
export let GetPandaFreeHeapSize: () => number = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetPandaFreeHeapSize');
export let GetPandaUsedHeapSize: () => number = globalThis.test.etsVm.getFunction('LPandaGC/ETSGLOBAL;', 'GetPandaUsedHeapSize');
export const PandaBaseClass = globalThis.test.etsVm.getClass('LPandaGC/PandaBaseClass;');
}
