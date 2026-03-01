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
desc: Function Types
---*/

interface Base {
    param(p: (q: Base)=>Derived): void
    ret(): (q: Derived)=> Base
}

interface Derived extends Base {
    param(p: (q: Derived)=>Base): void // Covariant parameter type, contravariant return type
    ret(): (q: Base)=>Derived // Contravariant parameter type, covariant return type
}

class B implements Base {
    param(p: (q: Base)=>Derived): void {
        res = 1
    }
    ret(): (q: Derived)=> Base {
        return (q: Derived): Base => this
    }
}

class D implements Derived {
    param(p: (q: Derived)=>Base): void {
        res = 2
    }
    ret(): (q: Base)=>Derived {
        return (q: Base): Derived => this
    }
}

let b = new B
let d = new D
let res = 0

function main() {
    b.param((q: Base): Derived => d)
    arktest.assertEQ(res, 1)
    arktest.assertEQ(b.ret()(d), b)

    d.param((q: Derived): Base => b)
    arktest.assertEQ(res, 2)
    arktest.assertEQ(d.ret()(b), d)
}
