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
desc: A name must be unique in its block space
---*/

function foo(): int {
    let xyz: int;
    return xyz;
}

class C {
    public xyz: int = 1;
}

class D {
    public xyz(): int {
        return 2;
    }
}

enum Enum { abc, def, ghk, xyz }

function main(): void {
    let xyz: int = 4;
    arktest.assertEQ(foo(), 0);
    arktest.assertEQ(new C().xyz, 1);
    arktest.assertEQ(new D().xyz(), 2);
    arktest.assertEQ(Enum.xyz.valueOf(), 3);
    arktest.assertEQ(xyz, 4);
    
    xyz = 0;
    arktest.assertEQ(xyz, 0);
}
