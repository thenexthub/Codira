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

function assertEq(a, b) {
    if (a !== b) {
        throw Error(`assertEq failed: '${a}' == '${b}'`);
    }
}

const helper = requireNapiPreview('libinterop_test_helper.so', false);
const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

globalThis.etsVm = requireNapiPreview('ets_interop_js_napi.so', false);

function runTest() {
    const etsOpts = {
        'log-level': 'error',
        'log-components': 'ets_interop_js',
        'boot-panda-files': stdlibPath + ':' + gtestAbcPath,
        'panda-files': gtestAbcPath,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
    };
    const createRes = etsVm.createRuntime(etsOpts);
    if (!createRes) {
        throw Error('Failed to create ETS runtime');
    }

    const Color = etsVm.getClass('Lstatic_enum_test/Color;');
    let red = Color.RED;
    assertEq(Color.RED.valueOf(), 1);
    assertEq(Color.GREEN.valueOf(), 2);
    assertEq(Color.YELLOW.valueOf(), 3);
    assertEq(Color.BLACK.valueOf(), 4);
    assertEq(Color.BLUE.valueOf(), 5);

    const Level = etsVm.getClass('Lstatic_enum_test/Level;');
    assertEq(Level.DEBUG.valueOf(), 'Debug');
    assertEq(Level.RELEASE.valueOf(), 'Release');
}

runTest();
