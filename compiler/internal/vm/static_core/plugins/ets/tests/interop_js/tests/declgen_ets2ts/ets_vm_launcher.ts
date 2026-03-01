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

// NOTE(kurnevichstanislav):
// This file is neccessary, because of specific execution of imports in ArkJSVM
// Imports executes first of all in the program.
// This file aim is to workaround this execution order.
// Since code from this file must be executed first all in the program,
// main.ts must cotain `import './ets_vm_launcher.ts'` at the beggining.
// Using of './ets_vm_launcher.ts' is not intuitive, but ets_vm_launcher.abc file
// will be located in the same directory as other .abc files, so the `./` relative path will be right.
const launcherName = 'ets_vm_launcher.ts';

// @ts-ignore
export const helper = requireNapiPreview('libinterop_test_helper.so', false);
if (helper === undefined) {
	throw new Error(`${launcherName}: Failed to load libinterop_test_helper.so`);
}

const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

// @ts-ignore
export const etsVm = requireNapiPreview('ets_interop_js_napi.so', false);
if (etsVm === undefined) {
	throw new Error(`${launcherName}: Failed to load ets_interop_js_napi.so`);
}

const etsOpts = {
	'panda-files': gtestAbcPath,
	'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
};

if (!etsVm.createRuntime(etsOpts)) {
	throw new Error('Cannot create ETS runtime');
}

/* CC-OFFNXT(no_explicit_any) std lib */
// Handle comment directive '@ts-nocheck'
(globalThis as any).Panda = etsVm;
