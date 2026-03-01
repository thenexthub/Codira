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

function setup() {
    let c = 0;
    let table = [];
    for (let n = 0; n < 256; n++) {
        c = n;
        for (let k = 0; k < 8; k++) {
            c = ((c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
        }
        table[n] = c;
    }
    return table;
}

function test(str, table) {
    let crc = 0 ^ (-1);
    for (let i = 0; i < str.length; i++ ) {
        crc = (crc >>> 8) ^ table[(crc ^ str.charCodeAt(i)) & 0xFF];
    }
    return (crc ^ (-1)) >>> 0;
}

function main() {
    console.log('Starting...');
    let penv = process.env;
    let stsVm = require(penv.MODULE_PATH + '/ets_interop_js_napi.node');
    const stsRT = stsVm.createRuntime({ // only to offset time spent on this in other versions of benchmark
        'boot-panda-files': penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
        'panda-files': penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
    });

    if (!stsRT) {
        console.error('Failed to create ETS runtime');
        return 1;
    }

    const State = stsVm.getClass('LCRC32;');

    const MS2NS = 1000000;
    let start = Date.now();
    let table = setup();
    // (almost) everything runs in js
    for (let i = 0; i < 1000; i++) {
        test('Lorem ipsum dolor sit amet, consectetur adipiscing elit', table);
    }
    let timeNs = (Date.now() - start) * MS2NS;
    console.log('Benchmark result: CRC32 ' + timeNs);
    return 0;
}

process.exit(main());
