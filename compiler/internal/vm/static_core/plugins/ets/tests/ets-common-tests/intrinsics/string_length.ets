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

function newString(): String {
    return 'abcde';
}

function testLength() {
    let s = newString();
    arktest.assertEQ(s.length, 5, 's.length != 5');
    let treeStr = s.concat('fghijklmnopqrstuvwxyz')
    arktest.assertEQ(treeStr.length, 26, 'treeStr.length != 26');
    let slicedStr = treeStr.substring(1, 20)
    arktest.assertEQ(slicedStr.length, 19, 'slicedStr.length != 4');
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringLength');
    suite.addTest('testLength', testLength);
    return suite.run();
}
