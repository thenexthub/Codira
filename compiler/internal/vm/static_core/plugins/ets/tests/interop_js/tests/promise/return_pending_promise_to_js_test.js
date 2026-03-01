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

let testSuccess = false;

const helper = requireNapiPreview('libinterop_test_helper.so', false);

function init() {
	const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
	const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

	let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);

	if (!helper.getEnvironmentVar('PACKAGE_NAME')) {
		throw Error('PACKAGE_NAME is not set');
	}
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

function callEts(etsVm) {
	const packageName = helper.getEnvironmentVar('PACKAGE_NAME');
	const globalName = 'L' + packageName + '/ETSGLOBAL;';
	const testReturnPendingPromise = etsVm.getFunction(globalName, 'testReturnPendingPromise');
	let res = testReturnPendingPromise();
	if (typeof res !== 'object') {
		throw Error('Result is not an object');
	}
	if (res.constructor.name !== 'Promise') {
		throw Error("Expect result type 'Promise' but get '" + res.constructor.name + "'");
	}
	return res;
}

async function doAwait(p) {
	let value = await p;
	if (value === 'Panda') {
		testSuccess = true;
	}
}

function doThen(p) {
	p.then((value) => {
		if (value === 'Panda') {
			testSuccess = true;
		}
	});
}

function queueTasks(etsVm) {
	let tId = 0;
	let queueTasksHelper = () => {
		if (!testSuccess) {
			return null;
		}
		clearInterval(tId);
		return null;
	};

	setTimeout(() => {
		if (testSuccess) {
			throw Error('Promise must not be resolved');
		}
		const packageName = helper.getEnvironmentVar('PACKAGE_NAME');
		if (!packageName) {
			throw Error('PACKAGE_NAME is not set');
		}
		const globalName = 'L' + packageName + '/ETSGLOBAL;';

		const resolvePendingPromise = etsVm.getFunction(globalName, 'resolvePendingPromise');
		if (!resolvePendingPromise()) {
			throw Error("Call of 'resolvePendingPromise' return false");
		}
        tId = setInterval(queueTasksHelper, 0);
	}, 0);
}

function runAwaitTest() {
	let etsVm = init();
	let res = callEts(etsVm);
	doAwait(res);
	queueTasks(etsVm);
}

function runThenTest() {
	let etsVm = init();
	let res = callEts(etsVm);
	doThen(res);
	queueTasks(etsVm);
}

function runTest(test) {
	if (test === 'await') {
		runAwaitTest();
	} else if (test === 'then') {
		runThenTest();
	} else {
		throw Error('No such test');
	}
}

let args = helper.getArgv();
if (args.length !== 6) {
	throw Error('Expected test name');
}
runTest(args[5]);
