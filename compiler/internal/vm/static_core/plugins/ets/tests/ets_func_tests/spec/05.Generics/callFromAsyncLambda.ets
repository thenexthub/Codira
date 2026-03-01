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
desc: 05.Generics
name: 05.generics/callFromAsyncLambda
tags: []
---*/

type fooAsyncLambda = () => Promise<void>

class Getter {
    get<T>(args: T): T {
        return args;
    }

    get<T>(args: T[]): T[] {
        return args;
    }
}

function main(): void {
    let getter = new Getter();
    let arr: Int[] = [0, 1, 2];
    let foo: fooAsyncLambda = async (): Promise<void> => {
        let str = getter.get<string>("F");
        arktest.assertTrue( str instanceof string)

        let arr2: Int[] = getter.get<Int>(arr);
        arktest.assertTrue( arr2 instanceof Object)
        arktest.assertTrue( arr2[0] instanceof Int)
    }
    waitForCompletion(foo);
}
