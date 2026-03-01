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
desc: Ensuring that method override in subclasses keeps number return types unchanged.
---*/

class Base {
    methodOne(p: Base): number {
        return 1;
    }
}

class Derived extends Base {
    override methodOne(p: Base): number {
        return 1;
    }
}

function main(): void {
    let a = new Base();
    let b = new Derived();

    let resultA = a.methodOne(a);
    let resultB = b.methodOne(a);

    arktest.assertEQ(resultA, resultB);
}