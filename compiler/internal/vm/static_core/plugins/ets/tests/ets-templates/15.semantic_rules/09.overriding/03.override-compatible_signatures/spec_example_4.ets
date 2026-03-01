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
desc: Type Parameter Constraint
---*/

interface Base {
    param<T extends Derived>(p: T): void
    ret<T extends Derived>(): T
}

interface Derived extends Base {
    // Contravariance for constraints of type parameters
    param<T extends Base>(p: T): void
    // Contravariance for constraints of the return type
    ret<T extends Base>(): T
}

class B implements Base {
    param<T extends Derived>(p: T): void {
        res = 1
    }
    ret<T extends Derived>(): T {
        return d as T
    }
}

class D implements Derived {
    param<T extends Base>(p: T): void {
        res = 2
    }
    ret<T extends Base>(): T {
        return b as T
    }
}

let b = new B
let d = new D
let res = 0

function main() {
    b.param<D>(d)
    arktest.assertEQ(res, 1)
    arktest.assertEQ(b.ret<D>(), d)

    d.param<B>(b)
    arktest.assertEQ(res, 2)
    arktest.assertEQ(d.ret<B>(), b)
}
