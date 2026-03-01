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

export class A {
    public a_foo(): int
    {
        return 100;
    }
    public a_x: double;
};

export class B extends A {
    public b_foo(): int
    {
        return 200;
    }
    public b_x: double;
};

export class C extends B {
    public c_foo(): int
    {
        return 100;
    }
    public c_x: double;
};

function main(): void {
    undefined;  // #BP{number}
    console.log("After evaluate");
    undefined;  // #BP
}
