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

let initBenchmarks = require('./caller.js').initBenchmarks;
let JSCallee = require('./callee.js');

function initEtsVM() {
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
    return etsVm;
}

function runTest() {
    print('Running test ' + bench + ' (' + callerSite + '->' + calleeSite + ')');

    let benchmarks;
    if (callerSite === 'JS') {
        benchmarks = initBenchmarks(etsVM);
    } else if (callerSite === 'STS') {
        let initializer = etsVM.getFunction('Lcaller/ETSGLOBAL;', 'initBenchmarks');
        benchmarks = initializer(callerSite !== calleeSite);
    }

    let caller = benchmarks[bench];
    if (!caller) {
        throw 'Benchmark not found: \'' + bench + '\' (' + callerSite + '->' + calleeSite + ')';
    }
    const MS2NS = 1000000;

    // Warmup:
    let start = Date.now();
    caller(warmup);
    let timeNs = (Date.now() - start) * MS2NS;
    print('warmup ' + bench + ': ' + timeNs / warmup + ' ns/iter, iters: ' + warmup);

    start = Date.now();
    caller(iters);
    timeNs = (Date.now() - start) * MS2NS;
    print('iters  ' + bench + ': ' + timeNs / iters + ' ns/iter, iters: ' + iters);

    return null;
}

globalThis.require = require;
globalThis.jsCallee = JSCallee;

let args = helper.getArgv();
if (args.length !== 10) {
    throw Error('Expected <test name> <caller-site(JS/STS)> <callee-site(JS/STS)> <warmup iters> <bench iters>');
}
let bench = args[5];
let callerSite = args[6];
let calleeSite = args[7];

if (callerSite !== 'STS' && callerSite !== 'JS') {
    throw 'Caller site should be \'STS\' or \'JS\', got \'' + callerSite + '\'.';
}
if (calleeSite !== 'STS' && calleeSite !== 'JS') {
    throw 'Callee site should be \'STS\' or \'JS\', got \'' + calleeSite + '\'.';
}
let etsVM = (callerSite === 'STS' || calleeSite === 'STS') ? initEtsVM() : undefined;
let warmup = parseInt(args[8], 10);
let iters = parseInt(args[9], 10);

runTest();
