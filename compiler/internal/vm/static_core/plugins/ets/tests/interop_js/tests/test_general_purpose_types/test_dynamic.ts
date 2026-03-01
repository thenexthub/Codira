/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

export let a: number = 42
export let b: string = "Hi"
export let c: bigint = 42n
export let d: boolean = true

export let n: Number = new Number(123)
export let e: number = 123

export type Greeting = "Hello"
export let h: "Hello" = "Hello"
export let g: Greeting = "Hello"

export enum Color {
    Red, Green, Blue
}

export enum UserRole {
    Admin = "admin",
    Editor = "editor",
    Viewer = "viewer",
    Guest = "guest"
}

export type union = number | string
export let u: union = 123

export function foo(n: number): number {
    return n
}

export class C {
    static foo(n: number): number {
        return n
    }
    bar(n?: number): number {
        return n
    }
}

export interface iface {
    foo(n?: number): void
}

export function error(msg: string): never {
    throw new Error(msg)
}

export let numberlist: number[] = [1,2,3,4]
export let stringlist: string[] = ["Hello world"]

export let bar: ()=>number = () => 123

function main() {
    testAll();
}

main();
