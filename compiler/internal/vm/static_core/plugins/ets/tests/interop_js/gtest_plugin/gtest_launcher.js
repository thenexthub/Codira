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

globalThis.ASSERT_TRUE = function assertTrue(v, msg = '') {
	if (v !== true) {
		msg = 'ASSERTION FAILED: ' + msg;
		throw Error(msg);
	}
};

globalThis.ASSERT_EQ = function assertEq(v0, v1) {
	if (!Object.is(v0, v1)) {
		let msg = `ASSERTION FAILED: ${v0}[${typeof v0}] !== ${v1}[${typeof v1}]`;
		throw Error(msg);
	}
};

globalThis.ASSERT_THROWS = function assertThrows(ctor, fn) {
	ASSERT_TRUE(ctor !== undefined && fn !== undefined);
	try {
		fn();
	} catch (exc) {
		if (exc instanceof ctor) {
			return;
		}
		throw Error('ASSERT_THROWS: expected instance of ' + ctor.name);
	}
	throw Error('ASSERT_THROWS: nothing was thrown');
};

globalThis.LOG_PROTO_CHAIN = function logProtoChain(o) {
	print('===== LOG_PROTO_CHAIN of ' + o + ' =====');
	for (let p = o.__proto__; p !== null; p = p.__proto__) {
		print(p.constructor + '\n[' + Object.getOwnPropertyNames(p) + ']');
	}
	print('==========');
};

function main() {
    const helper = requireNapiPreview('lib/libinterop_test_helper.so', false);
    if (helper === undefined) {
		print(`Failed to call requireNapiPreview(lib/libinterop_test_helper.so, false)`);
		return 1;
    }

	// Add 'gtest' object to global space.
	// This object is used by gtests as storage to save and restore variables
	globalThis.gtest = {};

	globalThis.gtest.etsVm = requireNapiPreview('lib/ets_interop_js_napi.so', false);
    globalThis.gtest.helper = helper;

	let stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');
	let gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
	let asmAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ASM_ABC_PATH');


	let argv = helper.getArgv();
	const arkJsNapiCliLastArgIdx = 5;

	let gtestName = argv[arkJsNapiCliLastArgIdx];
	if (gtestName === undefined) {
		print(`Usage: ${argv[0]} ${argv[1]} ${argv[2]} ${argv[3]} ${argv[4]} <test name>`);
		return 1;
	}

	let userPandaFiles = gtestAbcPath;
	if (asmAbcPath !== '') {
		if (userPandaFiles === '') {
			userPandaFiles += asmAbcPath;
		} else {
			userPandaFiles += (':' + asmAbcPath);
		}
	}

	let createRuntimeOptions = {
		'log-level': 'info',
		'log-components': 'ets_interop_js',
		'boot-panda-files': stdlibPath,
		'gc-trigger-type': 'heap-trigger',
		'compiler-enable-jit': 'false',
		'interpreter-type': 'irtoc',
		'taskpool-support-interop': 'true',
		'verification-mode': 'disabled',
	};

	const etsVmRes = globalThis.gtest.etsVm.createRuntime(createRuntimeOptions);
	if (!etsVmRes) {
		print('Failed to create ETS runtime');
		return 1;
	}

	globalThis.require = require;

	let gtestDir = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_DIR');
	if (gtestDir === undefined) {
		print('ARK_ETS_INTEROP_JS_GTEST_DIR is not set');
		return 1;
	}

	// Run gtest
	print(`Run ets_interop_js_gtest module: ${gtestName}`);
	const etsGtest = requireNapiPreview(`lib/${gtestName}.so`, false);
	if (etsGtest === undefined) {
		print(`Failed to call requireNapiPreview(lib/${gtestName}.so, false)`);
		return 1;
	}

	let args = argv.slice(arkJsNapiCliLastArgIdx);
	try {
		return etsGtest.main(args);
	} catch (e) {
		print(`${gtestName}: uncaught exception: ${e}`);
		print('exception.toString():\n', e.toString());
	}
	return 1;
}

let res = main();
if (res !== 0) {
	throw Error('gtest_launcher.js main return 1');
}
