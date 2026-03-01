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
desc: Verify the accuracy of the union type returned by the method for complex inheritance relationships.
---*/

interface G {
    greet(a: number): string | number {
        return "Hello from interface";
    }
}

class GrandParent implements G {
    greet(a: number): string | number {
        return "Hello from GrandParent";
    }
}

class Parent extends GrandParent implements G {
    greet(a: number): string | number {
        return a > 5 ? "Hello from Parent" : 123.0;
    }
}

class Child extends Parent {}

function main(): void {
    const child = new Child();
    arktest.assertEQ(child.greet(10), "Hello from Parent");
    arktest.assertEQ(child.greet(2), 123.0);
}