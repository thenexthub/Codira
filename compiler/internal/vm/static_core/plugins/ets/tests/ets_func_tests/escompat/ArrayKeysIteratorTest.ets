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

function main(): int {
    const suite = new arktest.ArkTestsuite("Array.keys() iterator behavior")

    suite.addTest("Test Array.keys() is not restored after done", testArrayKeysIteratorFinal)

    return suite.run()
}

function testArrayKeysIteratorFinal() {
    let array: Array<string> = new Array<string>();
    let iterator = array.keys();
    let result: IteratorResult<number>;

    array.push('a');
    result = iterator.next();
    arktest.assertEQ(result.done, false, 'First result `done` flag');
    arktest.assertEQ(result.value, 0, 'First result `value`');

    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value`');

    array.push('b');
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag (after push)');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` (after push)');
}