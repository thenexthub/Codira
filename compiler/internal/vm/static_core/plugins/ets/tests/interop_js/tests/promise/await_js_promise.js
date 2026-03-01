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
// resolver: (value) => {}
let resolver = undefined;

let PROMISE_RESOLVE_VALUE = 'JS_PROMISE_RESULT';

// let LOG_LEVEL = 5
let LOG_LEVEL = 1;
let FATAL = 0;
let ERROR = 1;
let INFO = 2;
function msg(s, lvl) {
	if (LOG_LEVEL >= lvl) {
		print(s);
	}
}

const helper = requireNapiPreview('libinterop_test_helper.so', false);

function init() {
	const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
	const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

	let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);
	const etsOpts = {
		'panda-files': gtestAbcPath,
		'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
		'gc-trigger-type': 'heap-trigger',
		'compiler-enable-jit': 'false',
		'run-gc-in-place': 'true',
		'coroutine-impl': 'stackful',
		'xgc-trigger-type': 'never'
		// 'log-debug': 'coroutines'
	};
	const createRes = etsVm.createRuntime(etsOpts);
	if (!createRes) {
		throw Error('Cannot create ETS runtime');
	}
	return etsVm;
}

async function doAwait(pr) {
	msg('Await func: start', INFO);
	let value = await pr;
	msg('Await func: await returned: ' + value, INFO);
	if (value === 'Panda') {
		testSuccess = true;
		msg('Await func: value from await is correct!', INFO);
	}
	msg('Await func: end', INFO);
}

function createPendingPromise() {
	let p = new Promise((res, rej) => {
		resolver = res;
	});
	return p;
}

function createResolvedPromise() {
	let p = new Promise((res, rej) => {
		res(PROMISE_RESOLVE_VALUE);
	});
	return p;
}

function runAwaitTest(name, promiseCreator, promiseResolver) {
	msg('Running test ' + name, INFO);
	let etsVm = init();
	let promise = promiseCreator();
	// similar to async JS function call. Returns a Promise instance
	let packageName = '';
	if (helper.getEnvironmentVar('PACKAGE_NAME')) {
		packageName = helper.getEnvironmentVar('PACKAGE_NAME') + '/';
	} else {
		throw Error('PACKAGE_NAME not set');
	}
	const testAwaitJsPromise = etsVm.getFunction('L' + packageName + 'ETSGLOBAL;', 'testAwaitJsPromise');
	let res = testAwaitJsPromise(promise);
	msg('Called testAwaitJsPromise OK, result:', INFO);
	msg(res, INFO);
	if (typeof res !== 'object') {
		throw Error('Result is not an object');
	}
	if (res.constructor.name !== 'Promise') {
		throw Error("Expect result type 'Promise' but get '" + res.constructor.name + "'");
	}
	let anotherRes = doAwait(res);
	msg('Called doAwait OK, res: ', INFO);
	msg(anotherRes, INFO);
	setTimeout(() => {
		if (testSuccess) {
			throw Error('Promise must not be resolved until JS resolves the passed one');
		}
		// resolve the passed promise if necessary
		promiseResolver(PROMISE_RESOLVE_VALUE);
		// after Q is processed, the test should pass
		setTimeout(async () => {
			msg('Starting await of doAwait() result, its state is:', INFO);
			msg(anotherRes, INFO);
			await anotherRes;
			if (!testSuccess) {
				throw Error('Promise is not resolved or value is wrong');
			} else {
				msg('Test PASSED', INFO);
			}
		}, 0);
	}, 0);
}

function runAwaitPendingTest() {
	runAwaitTest('AwaitPendingTest', createPendingPromise, (v) => {
		resolver(v);
	});
}

function runAwaitResolvedTest() {
	runAwaitTest('AwaitResolvedTest', createResolvedPromise, (v) => {});
}

function runTest(test) {
	if (test === 'pending') {
		runAwaitPendingTest();
	} else if (test === 'resolved') {
		runAwaitResolvedTest();
	} else {
		throw Error('No such test');
	}
}

let args = helper.getArgv();
if (args.length !== 6) {
	throw Error('Expected test name');
}
runTest(args[5]);
