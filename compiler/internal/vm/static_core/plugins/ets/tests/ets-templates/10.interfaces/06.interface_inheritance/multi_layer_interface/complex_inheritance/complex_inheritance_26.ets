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
desc: Verify that constants can be accessed normally in the inheritance relationship.
---*/

class Parent {
    static readonly numP = 10;
}

class Child extends Parent {
    static readonly strC = '10';
}

interface G {
    readonly a: string;
}

class GrandChild extends Child implements G {
    static readonly boolC = true;
    readonly a: string = 'a';
}

function main(): void {
    arktest.assertEQ(Parent.numP, 10);
    arktest.assertEQ(Child.strC, '10');
    arktest.assertEQ(GrandChild.boolC, true);

    const g: GrandChild = new GrandChild();
    arktest.assertEQ(g.a, 'a');
}