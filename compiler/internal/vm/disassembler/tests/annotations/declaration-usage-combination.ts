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

const enum E {
    A = 1,
    B = -2,
    C = 3
}

const enum E1 {
    A = 'Hello',
    B = 'world'
} 

@interface __$$ETS_ANNOTATION$$__Anno {
    a: number
    b: number[] = [13, -10]
    c: string
    d: boolean
    e: E[] = [1, -2, 3]
    f: number[]
    h: E = new Number(10) as number // no initializer
    i: E[][][] = [[new Array<E>(0)]] // no initializer
    j: E1 // no initializer
    k: E1[][][] = [[new Array<E1>(2)]] // no initializer
}

@__$$ETS_ANNOTATION$$__Anno({a: 20, b: [-13, 10], c: 'ab', d: true, e: [-1, 2, 3], f: new Array<number>(), g: [[[0]]], h: 10, i: [[new Array<E>(1)]], j: 'B', k: [[new Array<E1>(3)]]})
class A {
    @__$$ETS_ANNOTATION$$__Anno({a: -10, b: [1, 2, -3], c: 'cde', d: true, e: [1, -2, 3], f: [1], g: [[[0], [1]]], h: -10, i: [[[10], [20]]], j: 'B', k: [[['A'], ['B']]]})
    foo() {}
}

