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
    Verify that a child interface inherits the default method implementation when one parent has a default method and
    another has a regular method.
---*/

interface Parent1 {
    greet(): string {
        return "Hello from Parent1 (default)";
    }
}

interface Parent2 {
    greet(): string;
}

class Parent1Impl implements Parent1 {}

class Parent2Impl implements Parent2 {
    greet(): string {
        return "Hello from Parent2 (regular)";
    }
}

interface Child extends Parent1, Parent2 {
}

class ChildImpl implements Child {
    greet(): string {
        return new Parent1Impl().greet();
    }
}

function main(): void {
    const child = new ChildImpl();
    arktest.assertEQ(child.greet(), "Hello from Parent1 (default)");
}
