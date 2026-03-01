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

function main() {
    let suite = new arktest.ArkTestsuite('string get');
    suite.addTest('test', test);
    return suite.run();
}

function test() {
    let basestring: String = '12345678901234';
    let basestring2: String = 'abcdefghijklmn';
    let treestring: String = basestring.concat(basestring2); // "12345678901234abcdefghijklmn"
    let slicedstring: String = basestring.substring(3, 19); // "4567890123"

    arktest.assertEQ(treestring.charAt(5), c'6');
    arktest.assertEQ(slicedstring.charAt(5), c'9');
    arktest.assertEQ(treestring.substring(5, 6), '6');
}
