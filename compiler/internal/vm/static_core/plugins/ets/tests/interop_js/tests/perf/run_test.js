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

function runTest(test, warmup, iters) {
	print('Running test ' + test);

	const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
	const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

	let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);
	const etsOpts = { // NB: Consider setting compiler-enable-jit=false for local run
		'log-level': 'info',
		'log-components': 'ets_interop_js',
		'panda-files': gtestAbcPath,
		'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
		'gc-trigger-type': 'heap-trigger',
		'compiler-enable-jit': 'false',
		'run-gc-in-place': 'true',
	};

	if (!etsVm.createRuntime(etsOpts)) {
		throw Error('Cannot create ETS runtime');
	}

	const runTestImpl = etsVm.getFunction('Ltest_frontend/ETSGLOBAL;', test);
	if (warmup > 0) {
		runTestImpl(warmup);
	}

	const MS2NS = 1000000;
	let start = Date.now();
	runTestImpl(iters);
	let timeNs = (Date.now() - start) * MS2NS;
	print('js interop test ' + test + ': ' + timeNs / iters + ' ns/iter, iters: ' + iters);

	return null;
}

let args = helper.getArgv();
if (args.length !== 8) {
	throw Error('Expected <test name> <number of warmup iterations> <number of iterations>');
}
let test = args[5];
let warmup = parseInt(args[6], 10);
let iters = parseInt(args[7], 10);
runTest(test, warmup, iters);
