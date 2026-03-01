/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

print('[InteropLauncher] start');
const helper = requireNapiPreview('libinterop_test_helper.so', false);
const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');
const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
const etsVmOpts = helper.getEnvironmentVar('ETS_VM_OPTS');
let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);
const etsRT = etsVm.createRuntime(Object.assign(
    {
        'boot-panda-files': stdlibPath + ':' + gtestAbcPath,
        'panda-files': gtestAbcPath
    },
    JSON.parse(etsVmOpts))
);
if (!etsRT) {
    throw new Error('Failed to create ETS Runtime!');
}
const benchName = helper.getEnvironmentVar('VMB_BENCH_NAME');
const launcher = etsVm.getClass('L' + benchName + '/VmbLauncher;');
launcher.main();
print('[InteropLauncher] end');
