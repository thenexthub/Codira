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
 desc: >-
    The first form contains an expression that specifies a default value.
    It is called a parameter with default value. The value of the parameter
    is set to the default value if the argument corresponding to that parameter
    is omitted in a function or method call
 ---*/

class A {
    meth(a: string[], b: double[]): string {
        return '' + a[0] + b[0]
    }
}

class B extends A {
    meth(a: string[] = ['a'], b: double[] = [1.0]): string {
        return super.meth(a, b)
    }
}

function main() {
    let b = new B
    arktest.assertEQ(b.meth(['x'], [42.0]), 'x42')
    arktest.assertEQ(b.meth(['x']), 'x1')
    arktest.assertEQ(b.meth(), 'a1')
    arktest.assertEQ(b.meth(['x'], undefined), 'x1')
    arktest.assertEQ(b.meth(undefined, [42.0]), 'a42')
    arktest.assertEQ(b.meth(undefined, undefined), 'a1')
}
