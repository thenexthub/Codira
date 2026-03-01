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

class AHAH {
    constructor() {}
    constructor(a: int) {}

    meth(): int {
        return this.hoho
    }

    // TODO(kprokopenko): add test with reading this field jsut after resolve
    static fld: int = 30
    hoho: int = 3
}

function main() {
    const typ = Type.resolve("TypeResolve.AHAH")
    arktest.assertNE(typ, undefined)
    arktest.assertTrue(typ instanceof ClassType)
    const cType = typ as ClassType
    const ctorsN = cType.getConstructorsNum()

    arktest.assertEQ(cType.getConstructorsNum(), 2)
    cType.getConstructor(1).getName()

    let ok = false
    for (let i = 0; i < cType.getMethodsNum(); i++) {
        const m = cType.getMethod(i)
        if (m.getName() == "meth") {
            arktest.assertTrue( m.getType().getResultType() instanceof IntType)
            ok = true
            break
        }
    }
    arktest.assertTrue(ok)
    arktest.assertEQ((cType.make() as AHAH).meth(), 3)
}
