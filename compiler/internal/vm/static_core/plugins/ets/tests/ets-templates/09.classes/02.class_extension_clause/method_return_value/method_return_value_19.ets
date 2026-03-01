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
desc: The return value of the method is never, and the modifier is public.
---*/

class Parent {
    public getValue(): never {
        throw new Error('This method always throws an error');
    }
}

class Child extends Parent {}

function main(): void {
    let parent = new Parent();
    try {
        let parentValue: never = parent.getValue();
    } catch (error) {
        if (error instanceof Error) {
            arktest.assertEQ(error.message, 'This method always throws an error');
        } else {
            throw new Error('Unexpected error type');
        }
    }

    let child = new Child();
    try {
        let childValue: never = child.getValue();
    } catch (error) {
        if (error instanceof Error) {
            arktest.assertEQ(error.message, 'This method always throws an error');
        } else {
            throw new Error('Unexpected error type');
        }
    }
}