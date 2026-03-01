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

let var1: int = 2;
const con1: string = `var2${var1}`;

class Cls1 {
    public field1?: string;
    public field2: int | string;

    constructor(field2: int | string) {
        this.field2 = field2;
    }
}

function foo1(inArr: number[]): string {
    return JSON.stringify(inArr);
}

let var2: Cls1 | string = '';
var2 = new Cls1('abc');
let var3: readonly [number, string] = [123, 'abc'];

// CC-OFFNXT(G.AOD.03) for test purposes
console.log('fun1:' + foo1([var1, var3[0]]));
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(`con2:${con1}|var2.field1:${(var2 as Cls1).field1}|` +
    `var2.field2:${(var2 as Cls1).field2}|var3[1]:${var3[1]}`);
