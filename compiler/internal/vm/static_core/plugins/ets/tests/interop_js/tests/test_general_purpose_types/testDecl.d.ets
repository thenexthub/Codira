/**
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

export declare let a: number
export declare let b: string
export declare let c: bigint
export declare let d: boolean

export declare let n: Number
export declare let e: number

export declare type Greeting = "Hello"
export declare let h: "Hello"
export declare let g: Greeting

export declare enum Color {
    Red, Green, Blue
}

export declare type union = number | string
export declare let u: union

export declare enum UserRole {
    Admin = "admin",
    Editor = "editor",
    Viewer = "viewer",
    Guest = "guest"
}

export declare function foo(n: number): number
export declare class C {
    static foo(n: number): number
    bar(n: number): number
}

export declare interface iface {
    foo(n?: number): void
}

export declare function error(msg: string): never

export declare let numberlist: number[]
export declare let stringlist: string[]

export declare let bar: ()=>number