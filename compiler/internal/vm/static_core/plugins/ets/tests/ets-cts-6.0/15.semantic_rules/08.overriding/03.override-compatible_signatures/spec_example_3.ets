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
desc: Union Types
---*/

interface BaseSuperType {}

interface Base extends BaseSuperType {
    // Overriding for parameters
    param<T extends Derived, U extends Base>(p: T | U): void
    // Overriding for return type
    ret<T extends Derived, U extends Base>(): T | U
}

interface Derived extends Base {
    // Overriding kinds for parameters, Derived <: Base
    // contravariant parameter type: Derived | Base <: Base | BaseSuperType
    param<T extends Base, U extends Object>(p: Base | BaseSuperType): void
    // Overriding kinds for return type
    ret<T extends Base, U extends BaseSuperType>(): T | U
}

class B implements Base {
    param<T extends Derived, U extends Base>(p: T | U): void {
        res = 1
    }
    ret<T extends Derived, U extends Base>(): T | U {
        return b as U
    }
}

class D implements Derived {
    param<T extends Base, U extends Object>(p: Base | BaseSuperType): void {
        res = 2
    }
    ret<T extends Base, U extends BaseSuperType>(): T | U {
        return b as T
    }
}

let b = new B
let d = new D
let res = 0

function main() {
    b.param<D, B>(d)
    arktest.assertEQ(res, 1)
    arktest.assertEQ(b.ret<D, B>(), b)

    b.param<D, B>(b)
    arktest.assertEQ(res, 1)

    d.param<B, Object>(b)
    arktest.assertEQ(res, 2)
    arktest.assertEQ(d.ret<B, BaseSuperType>(), b)
}
