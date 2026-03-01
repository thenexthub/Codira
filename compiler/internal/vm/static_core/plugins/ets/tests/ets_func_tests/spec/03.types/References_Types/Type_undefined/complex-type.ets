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
desc: 3.8.9 The only value of type undefined is represented by the keyword undefined
name: complex-type.ets
tags:

---*/

class ComplexClass {
    n : number | undefined | null = null;
    s : string | undefined | null = null;
}

type T_ = ComplexClass | undefined | null;
type T = Array<ComplexClass> | Array<ComplexClass | undefined> | Array<ComplexClass | null> | Array<T_> | T_;

function check(b : boolean): int {
    return b ? 0 : 1;
}

function callMe<T>(p : T): T {
    if (p === undefined)
        return null;

    if (p === null)
        return undefined;

    if (p instanceof ComplexClass)
        return { n: p.s?.length, s: p.n?.toString() };

    if (p instanceof Array) {
        let tmp = p[0]
        if (tmp === null)
            return null;
        if (tmp === undefined)
            return undefined;
        if (tmp instanceof ComplexClass)
            return [null, undefined, new ComplexClass(), {n: null, s: undefined}]
    }

    return new Array<ComplexClass | undefined>();
}

function main(): int {
    let result: int = 0;

    result += check(callMe(undefined) === null);
    result += check(callMe(null) === undefined);
    result += check(callMe(callMe(undefined)) === callMe(null));

    let v14 = new C11()
    v14.n = 10101;
    v14.s = "42";
    result += check((callMe(v14) as ComplexClass).n === 2);
    result += check((callMe(v14) as ComplexClass).s === "10101");

    return result;
}
