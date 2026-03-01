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

export type MyTuple = [number, string, boolean];
export declare let tuple: MyTuple;
export declare let tupleObject: [number, number];
export declare function example(args: [string, number]): [boolean];
export declare const value: [string, number];
export interface User {
    info: [string, number];
}
export declare class MyClass {
    data: [number, string];
    constructor();
}
export declare const a: string, b: number;
export type Result = [string, number] | null;
export type Mapped<T> = {
    [K in keyof T]: [T[K], string];
};
export type Test = Mapped<{
    a: number;
    b: boolean;
}>;
export type NestedTuple = [[number, string], [boolean, object]];
export type Mixed = [number, string][] | [string, number];
export declare function getData(): Promise<[string, number]>;
export type WithDefault<T = [string, number]> = T;
export type Example = WithDefault;
export interface Options {
    config?: [string, number];
}
export declare const user: {
    name: string;
    metadata: (string | number)[];
};
export type IfTuple<T> = T extends [any, any] ? true : false;
export type A = IfTuple<[number, string]>;
export type B = IfTuple<string[]>;
export type ReadonlyTuple = readonly [string, number];