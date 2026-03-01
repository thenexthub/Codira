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
evaluation:
  expected_imports_base: False
---*/

export abstract class A {
    public abstract return_state(): int;
};

export class B extends A {
    constructor(state_rhs: int)
    {
        super();
        this.b_x = state_rhs;
    }

    public override return_state(): int
    {
        return this.b_x;
    }

    private b_x: int;
};

export class C extends A {
    constructor(state_rhs: int)
    {
        super();
        this.c_x = state_rhs;
    }

    public override return_state(): int
    {
        return this.c_x;
    }

    private c_x: int;
};

function main(): void {
    let variable: A = new B(10);
    undefined // #BP{boolean}
    console.log("After evaluate");
    undefined;  // #BP
}
