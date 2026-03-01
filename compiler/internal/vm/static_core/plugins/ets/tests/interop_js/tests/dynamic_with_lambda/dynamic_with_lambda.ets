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

let module = ESValue.load('js_code')
let etsA = module.getProperty('A');

function TestArgs0(): int {
    let a = etsA.instantiate();
    let f: () => int = (): int => { return 25 as int; }
    let res = a.getProperty('bar').invoke(f).toNumber().toInt();
    return res;
}

function TestArgs1(): int {
    let a = etsA.instantiate();
    let f: (x: int) => int = (x: int): int => { return x * x; }
    let res = a.getProperty('bar').invoke(f, 5).toNumber().toInt();
    return res;
}

function TestArgs2(): int {
    let a = etsA.instantiate();
    let f: (x: int, y: int) => int = (x: int, y: int): int => { return x * x + y * y; }
    let res = a.getProperty('bar').invoke(f, 3, 4).toNumber().toInt();
    return res;
}

function TestLambdaJSValue(): int {
    let a = etsA.instantiate();
    let f = (x: int, y: int): int => { return x * x + y * y; }
    let res = a.getProperty('bar').invoke(f, 3, 4).toNumber().toInt();
    return res;
}

function TestLambdaJSValueCast(): int {
    let f = (x: int): int => { return x * x; }
    let f1: (x: int) => int = f as (x: int) => int;
    let res = f1(5) as int;
    return res;
}

function TestLambdaJSValueCache(): int {
    let f = (x: int): int => { return x * x; }
    let f1: (x: int) => int = f as (x: int) => int;
    let f2: (x: int) => int = f as (x: int) => int;
    let res1 = f1(5) as int;
    let res2 = f2(5) as int;
    if (res1 == 25 && res2 == 25) {
        return 25;
    }
    return 0;
}

// TODO(itrubachev) this test can be enabled after fixing TODO in checker::Type *TSAsExpression::Check in es2panda
// function TestLambdaJSValueCastCallAsArgument(): int {
//     let a = new A();
//     let f: JSValue = (x: int): int => { return x * x; }
//     let f1: (x: int) => int = f as (x: int) => int;
//     let res = a.bar(f1, 5) as int
//     return res;
// }
