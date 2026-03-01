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
desc: The method returns a value of number and the modifier is public to verify that the rewrite works correctly.
---*/

class Parent {
    public getValue(): number {
        return 100;
    }
}

class Child extends Parent {
    public getValue(): number {
        return 200;
    }
}

function main(): void {
    let parent = new Parent();
    let parentValue: number = parent.getValue();
    arktest.assertEQ(parentValue, 100);

    let child = new Child();
    let childValue: number = child.getValue();
    arktest.assertEQ(childValue, 200);
}