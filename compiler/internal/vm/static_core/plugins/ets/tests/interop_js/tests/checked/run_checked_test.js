/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const helper = requireNapiPreview('../../module/libinterop_test_helper.so', false);

function parseArgs(input) {
    const args = input.split(',');
    const opts = {};

    for (let i = 0; i < args.length; i++) {
		if (args[i].startsWith('--')) {
			args[i] = args[i].slice(2);

			if (args[i].includes('=')) {
				const [key, value] = args[i].split('=', 2);
				opts[key.trim()] = value.trim();
			} else if (i + 1 < args.length && !args[i + 1].startsWith('--')) {
				// this case realized when "--ark-aot", "path" comes
				opts[args[i]] = args[i + 1];
				++i;
			} else {
				opts[args[i]] = 'true';
			}
		}
    }

    return opts;
}

function runTest(test, pandaOptions) {
	print('Running test ' + test);

	const testAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_TEST_ABC_PATH');
	const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

	const etsVm = requireNapiPreview('../../module/ets_interop_js_napi.so', false);

	const options = {
		'xgc-trigger-type': 'never',
		'panda-files': testAbcPath,
		'boot-panda-files': `${stdlibPath}:${testAbcPath}`,
	};
	const transformedPandaOptions = parseArgs(pandaOptions.toString());
	const etsOpts = { ...options, ...transformedPandaOptions };

	const etsVmRes = etsVm.createRuntimeLegacy(etsOpts);
	if (!etsVmRes) {
		let message = '\n';
		for (const key in etsOpts) {
			message += `  '${key}': '${etsOpts[key]}'\n`;
		}
		throw Error(`Failed to create ETS runtime with options: ${message}`);
	}

	if (!helper.getEnvironmentVar('PACKAGE_NAME')) {
		throw Error('PACKAGE_NAME is not set');
	}
	const globalName = 'L' + helper.getEnvironmentVar('PACKAGE_NAME') + '/ETSGLOBAL;';
	try {
		const runTestImpl = etsVm.getFunction(globalName, test);
		let res = runTestImpl();
		if (res !== 0) {
			throw 'test failed: ' + res;
		}
	} catch {
		return 1;
	}
	return 0;
}

let args = helper.getArgv();
if (args.length < 5) {
	throw Error('Expected ark options, test package and test function name');
}
// checker.rb passes file and entry point as two last arguments
let test = args[args.length - 1];
runTest(test, args.slice(4, -2));
