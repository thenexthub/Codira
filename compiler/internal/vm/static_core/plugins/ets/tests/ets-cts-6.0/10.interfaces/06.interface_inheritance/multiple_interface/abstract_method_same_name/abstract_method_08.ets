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
desc: The method with the same name has different parameter types and return types, redefine the method.
---*/

interface Parent1 {
    processString(data: string): string;
}

interface Parent2 {
    processNumber(data: number): number;
}

interface Child extends Parent1, Parent2 {
    processString(data: string): string;
    processNumber(data: number): number;
}

class ChildClass implements Child {
    processString(data: string): string {
        return `Processed String: ${data}`;
    }

    processNumber(data: number): number {
        return data * 2;
    }
}

function main(): void {
    const child = new ChildClass();
    arktest.assertEQ(child.processString("Hello"), "Processed String: Hello");
    arktest.assertEQ(child.processNumber(21), 42);
}
