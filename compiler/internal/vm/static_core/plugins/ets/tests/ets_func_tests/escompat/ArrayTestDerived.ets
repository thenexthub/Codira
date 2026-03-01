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

const DERIVED_ACCESS_OFFSET = 1000;

class Derived extends Array<Number> {
    public constructor(arrayLen: number) {
        super(arrayLen.toInt());
    }

    public override $_get(index: int): Number {
        return super.$_get(index) + DERIVED_ACCESS_OFFSET;
    }

    public override at(index: int): Number {
        let superRes = super.at(index);
        return superRes + DERIVED_ACCESS_OFFSET;
    }
}

function __noinline__getOutside(a: Array<Number>) {
    return a[0];
}

function __noinline__atOutside(a: Array<Number>) {
    return a.at(0);
}

function main(): int {
    let a = new Derived(10);
    for (let i = 0; i < 10; i++) {
        a[i] = i.toDouble();
    }
    arktest.assertEQ(__noinline__getOutside(a), DERIVED_ACCESS_OFFSET);
    arktest.assertEQ(__noinline__atOutside(a), DERIVED_ACCESS_OFFSET);

    let b = new Array<Number>(10);
    for (let i = 0; i < 10; i++) {
        b[i] = i.toDouble();
    }
    arktest.assertEQ(__noinline__getOutside(b), 0);
    arktest.assertEQ(__noinline__atOutside(b), 0);
    return 0;
}
