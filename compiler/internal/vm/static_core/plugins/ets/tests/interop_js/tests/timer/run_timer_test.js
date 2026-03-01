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

function runTest(test, isExternalTimer) {
	print('Running test ' + test);
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
	};
	if (isExternalTimer) {
		etsOpts['coroutine-enable-features:enable-external-timer'] = 'true'
	}
	if (!etsVm.createRuntime(etsOpts)) {
		throw Error('Cannot create ETS runtime');
	}

	const runTestImpl = etsVm.getFunction(globalName, test);
	runTestImpl(isExternalTimer);
	let counter = 0;
	const maxCounter = 5;
	const checkDelay = 1000;
	let tId = 0;
	let checkCallback = () => {
		++counter;
		if (counter > maxCounter) {
			throw new Error('Test failed: timeout.');
		}
		const check = etsVm.getFunction(globalName, 'check');
		let result = check();
		if (result) {
			clearInterval(tId);
		}
	};
	tId = setInterval(checkCallback, checkDelay);
}

let args = helper.getArgv();
if (args.length < 6 || args.length > 7) {
	throw Error('Expected test name');
}
runTest(args[5], args[6] === 'true');
