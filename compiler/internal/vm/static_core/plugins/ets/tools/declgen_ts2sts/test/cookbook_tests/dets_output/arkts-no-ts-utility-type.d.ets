'use static'
/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
export type LowercaseGreeting = string;
export type Greeting = Any;
export type QuietGreeting = Any;
export type ShoutyGreeting = Any;
export type UncomfortableGreeting = Any;
export declare function createInstance<T extends Any>(Class: T, ...args: Any[]): Any;
export type T0 = Any;
export type T1 = Any;
export type T3 = Any;
export type T4 = Any;
export type T5 = Any[];
export type T6 = Any;
export type NoInfer<T> = Any;
export declare function createStreetLight<C extends string>(colors: C[], defaultColor?: Any): void;
export type UserPreview = Any;
export declare function toHex(_this: number): string;
export declare const fiveToHex: Any;
export declare function numberToString(n: Any): Any;
export type TodoPreview = Any;
export declare function makeObject<T>(obj: Any): T;
export declare class UserIns {
    name: string;
    age: number;
    constructor(name: string, age: number);
}
