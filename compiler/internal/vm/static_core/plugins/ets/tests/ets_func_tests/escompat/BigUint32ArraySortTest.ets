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

function sortTest() {
    let arr = [18446744073709551615n, 9223372036854775808n, 9223372036854775807n, 9223372036854775806n, 20n, 10n, 1n]
    let arr1 = new BigUint64Array(arr);
    arr1.sort();
    let arr2 = new BigUint64Array(arr);
    arr2.sort((a, b) => a - b);

    let len = arr.length;
    for (let i = 0; i < len; i++) {
        arktest.assertEQ(arr[len - i - 1], arr1[i]);
        arktest.assertEQ(arr[len - i - 1], arr2[i]);
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('BigUint64Array sort');
    suite.addTest('sortTest', sortTest);
    return suite.run();
}
