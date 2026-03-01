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

export class ExportClass {
    get memA(): string {
        return 'memAStr';
    }

    memB(): string {
        return 'memBStr';
    }

    memC: string = '';
}

export default class SomeClass {
    foo() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('SomeClass');
    }
}

export function f1(p: number) {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('f1 ' + p);
}
export function f2(p: string) {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('f2 ' + p);
}
export overload foo {f1, f2}

namespace Space1 {
    export function foo() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('Space1');
    }
    export let variable = 1234
    export const constant = 1234
}
export namespace Space2 {
    let tag: string = '';

    export function foo(p: number) {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('Space2', tag);
    }
    export let variable = '1234'
    static {
        tag = 'test';
    }
}

export @interface MyAnno {}

// export type
export type S1 = string
export type S2 = string
export type UNI = string | boolean | int[]

class ClassType1 {
    test(): String {
        return 'Hello World';
    }
}

class ClassType2{
    test(): String {
        return 'Hello World';
    }
}
export type {ClassType1}
export type MyClassType = ClassType2