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

function test1(): void {
    let num1: double = Number.parseFloat('9.999999999999e1000000000000000000000000000000')
    let num2: double = Number.parseFloat('-9.999999999999e1000000000000000000000000000000')
    arktest.assertEQ(num1, Infinity)
    arktest.assertEQ(num2, -Infinity)
}

function main() {
    const testSuit = new arktest.ArkTestsuite('numberForParseFloatInfinityTest');
    testSuit.addTest('test1', test1);
    return testSuit.run();
}
