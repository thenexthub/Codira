/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
  tags: [not-a-test]
---*/

package package_with_static_block;

// Note: memberExpression could be constant when it is enum property access, this check will be enabled after Issue23082
export enum ME {
    START = 1,
    END = 2
}

function foo1(): ME {
    return ME.START;
}

function foo2(): boolean {
    return true;
}

function foo3(): char {
    return c'c';
}

function foo4(): string {
    return "world";
}

function foo5(): number {
    return 666;
}

export let bb: boolean = true;
export let cc: char = c'c';
export let ss: string = "hello";
export let nn: number = 1;

export let in_bb: boolean;
export let in_cc: char;
export let in_ss: string;
export let in_nn: number;

export const c_bb: boolean = true;
export const c_cc: char = c'c';
export const c_ss: string = "hello";
export const c_nn: number = 1;

export const c_in_bb: boolean;
export const c_in_cc: char;
export const c_in_ss: string;
export const c_in_nn: number;
// first initializer block
static {
    nn = 6
    in_bb = foo2();
    in_cc = foo3();
    in_ss = foo4();
    in_nn = foo5();
}

// second initializer block
static {
    c_in_bb = foo2();
    c_in_cc = foo3();
    c_in_ss = foo4();
    c_in_nn = foo5();
}

export namespace NS {
    export let n = 1
    static {
        n = 66;
    }

    export namespace NS1 {
        export let n_sub = 1;
        static {
            n_sub = 666
        }
    }
}
