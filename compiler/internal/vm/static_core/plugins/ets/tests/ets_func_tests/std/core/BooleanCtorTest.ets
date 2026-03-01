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


/*---
desc: Boolean ctor tests
---*/

function main(): int {
    let failure = 0
    failure += ((new Boolean()).valueOf() == false) ? 0 : 1
    failure += ((new Boolean(undefined)).valueOf() == false) ? 0 : 1
    failure += ((new Boolean(false)).valueOf() == false) ? 0 : 1
    failure += ((new Boolean(true)).valueOf() == true) ? 0 : 1

    const suite = new arktest.ArkTestsuite("Boolean API test for SDK");
    suite.addTest("Boolean constructor", testBooleanConstructor);
    failure += suite.run();

    return failure;
}

class TestData {
    static readonly a: boolean = new Boolean(new Object());
    static readonly expected: boolean = true;
}

function testBooleanConstructor() {
    let a = TestData.a;
    arktest.assertEQ(TestData.expected, a);
}
