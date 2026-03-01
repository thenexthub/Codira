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

class A {
    public param: string = '123';
    foo(a: string): void {
      console.log(a);
    }
}

class MyClass {
    public prop: string;
    constructor() {
        this.prop = 'class prop';
    }
}

export function initFuncName(a: string, inst: MyClass) {
  console.log(a)
  console.log(inst.prop)
}
let f1 = initFuncName

function add1(a: int, b: int): int {
    return a;
}
function add2(a: int, b: int): number {
    return a;
}
function add3(a: int, b: int): string {
    return '123';
}
async function foo2(){}
let f2 = add1
function main() {
    let classA = new A();
    let f3 = classA.foo;
    initFuncName('123', new MyClass());
    console.log(add1(1, 2));
    console.log(add3(1, 2));
    classA.foo("foo");
}

