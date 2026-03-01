/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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


const helper = requireNapiPreview('libinterop_test_helper.so', false);

const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);

const etsOpts = {
	'panda-files': gtestAbcPath,
	'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
	'gc-trigger-type': 'heap-trigger',
	'compiler-enable-jit': 'false',
	'run-gc-in-place': 'true',
	'coroutine-workers-count': '1',
};

const res = etsVm.createRuntime(etsOpts);
if (!res) {
	throw Error('Cannot create ETS runtime');
}

const Point = etsVm.getClass('Lproxy_reference_storage_2/test/Point;');

// Call SharedReferenceStorage::CreateObjectEtsWrapper() method
for (let i = 0; i < 4 * 4096; ++i) {
	let p = new Point(i, i);
	print(i);
}

// Method SharedReferenceStorage::RemoveSharedReference() will be called before the virtual machine is terminated
