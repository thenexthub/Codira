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

let iteration = 0;
let expectedValue = '';

function equal(actual, expected) {
	// try to convert expected to actual's type
	if (typeof actual === 'boolean') {
		return actual === Boolean(expected);
	} else if (typeof actual === 'number') {
		let eps = 1e-6;
		return Math.abs(actual - Number(expected)) < eps;
	}
	// convert all to string and compare string representation
	return String(actual) === String(expected);
}

const helper = requireNapiPreview('libinterop_test_helper.so', false);

function runTest(test, iter) {
	print("Running test '" + test + "'");
	const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
	const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');
	const packageName = helper.getEnvironmentVar('PACKAGE_NAME');
	if (!packageName) {
		throw Error('PACKAGE_NAME is not set');
	}
	const globalName = 'L' + packageName + '/ETSGLOBAL;';

	let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);
	const etsOpts = {
		'panda-files': gtestAbcPath,
		'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
		'xgc-trigger-type': 'never',
	};
	if (!etsVm.createRuntime(etsOpts)) {
		throw Error('Cannot create ETS runtime');
	}
	const runTestImpl = etsVm.getFunction(globalName, test);
	let res = runTestImpl();
	if (typeof res !== 'object') {
		throw Error('Result is not an object');
	}
	if (res.constructor.name !== 'Promise') {
		throw Error("Expect result type 'Promise' but get '" + res.constructor.name + "'");
	}
	let testSuccess = false;
	res.then((value) => {
		if (equal(value, expectedValue)) {
			testSuccess = true;
		} else {
			print('Wrong value: ' + value + ' (' + typeof value + ') !== ' + expectedValue + ' (' + typeof expectedValue + ')');
		}
	});
	let tId = 0;
	let callback = () => {
		if (!testSuccess) {
			return;
		}
		print('Test passed');
		clearInterval(tId);
	};
	tId = setInterval(callback, 0);
}

let args = helper.getArgv();
expectedValue = args[6];
runTest(args[5]);
