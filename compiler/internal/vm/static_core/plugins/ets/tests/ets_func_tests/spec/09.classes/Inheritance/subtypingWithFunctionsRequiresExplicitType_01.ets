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
desc: 9.6.9 Inheritance
name: 09.classes/09.06.method_declarations/09.06.9.inheritance/subtypingWithFunctionsRequiresExplicitType_01
tags: []
---*/

class A {}

type regularLambdaA = () => A

let global_a: int = 0

function foo(f: () => A): void {
    let a = f();
}

function main(): void {
    let fooLambdaA: regularLambdaA = (): A => {
        global_a = 1
        return new A()
    };
    global_a = 0
    foo(fooLambdaA);
    arktest.assertEQ( global_a, 1)
}
