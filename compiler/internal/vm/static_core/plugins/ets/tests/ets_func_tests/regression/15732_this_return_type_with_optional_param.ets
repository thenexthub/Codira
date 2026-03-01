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
desc: Optional arguments for method with 'this' return type
tags: []
---*/

class NumberMethodParamClass {
    foo(value: number): this {
        return this;
    }
}


class OptionalNumberMethodParamClass {
    foo(value?: number): this {
        return this;
    }

    bar(value?: number): this {
        return this;
    }

    baz(name: string, value?: number): this {
        return this;
    }

    boo(): OptionalNumberMethodParamClass {
        return this.bar();
    }
}

class OverrideNumberMethodParamClass extends OptionalNumberMethodParamClass {
    override foo(value?: number): this {
        return this;
    }
}

function main(): void {
    let nm = new NumberMethodParamClass();
    arktest.assertEQ(nm, nm.foo(42).foo(1).foo(-5))

    let onm = new OptionalNumberMethodParamClass();
    arktest.assertEQ(onm, onm.foo())
    arktest.assertEQ(onm, onm.foo(42).bar().foo(1).bar(-5).baz("name", 42).boo())

    let oom = new OverrideNumberMethodParamClass();
    arktest.assertEQ(oom, oom.foo(42).bar().foo(1).bar(-5).baz("name", 42).boo())
}
