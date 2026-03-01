/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;
let testAll = etsVm.getFunction('Ltest_static/ETSGLOBAL;', 'testAll');


export function foo1(x: string): string {
    return x
}

export class X {
    s: string = ""
    n: number = 0
    constructor(...nums: number[]) {
        for(let num of nums) {
            this.n += num
        }
    }
}

export class Y {
    static fooArgs(...nums: number[]): number {
        return nums[0]
    }
}

export function foo2(x: X): string {
    return x.s
}

export function foo3(...args: number[]): number[] {
    return args
}

export function foo4(arg1: number = 1): number {
    return arg1 + 1
}

export function bar<T>(x: T): T {
    return x
}

function main() {
    testAll();
}

main();
