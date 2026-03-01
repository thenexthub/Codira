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

package evalPatchFunction;

class A {
    public a: int;
    public b: double;

    constructor(a: int, b: double)
    {
        this.a = a;
        this.b = b;
    }
    
    public foo(val: int): int
    {
        return 100 * val;
    }
    
    public goo(val: double): double
    {
        return 200 * val;
    }
};

export class B extends A {
    constructor(a: int, b: double)
    {
        super(a, b);
    }
};

class evalPatchFunction {
    private static evalPatchFunction() {
        let b = new B(10, 123.456);

        arktest.assertEQ(b.a, 10);
        arktest.assertEQ(b.b, 123.456);

        arktest.assertEQ(b.foo(10), 100 * 10);
        arktest.assertEQ(b.goo(10.11), 200 * 10.11);
    }
}
