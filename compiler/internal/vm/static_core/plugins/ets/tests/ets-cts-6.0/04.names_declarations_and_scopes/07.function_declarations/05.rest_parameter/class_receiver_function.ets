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
desc: Rest function with class receiver
---*/

interface ITest {
}

class Testee implements ITest {
}

function receiver(this: Testee, ...classes: ITest[]): number {
    return classes.length
}

function main() {
    let testee: Testee = new Testee
    let rest: ITest[] = [new Testee, new Testee]
    arktest.assertEQ(receiver(new Testee, new Testee, new Testee), 2)
    arktest.assertEQ(receiver(new Testee, ...rest), 2)

    arktest.assertEQ(testee.receiver(new Testee, new Testee), 2)
    arktest.assertEQ(testee.receiver(...rest), 2)
}
