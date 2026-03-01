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
export type MyTuple = Array<Any>;
export declare let tuple: MyTuple;
export declare let tupleObject: Array<Any>;
export declare function example(args: Array<Any>): Array<Any>;
export declare const value: Array<Any>;
export declare const a: string, b: number;
export type Result = Array<Any> | null;
export type Mapped<T> = Any;
export type Test = Mapped<Any>;
export type NestedTuple = Array<Any>;
export type Mixed = Array<Any>[] | Array<Any>;
export declare function getData(): Promise<Array<Any>>;
export type WithDefault<T = Array<Any>> = T;
export type Example = WithDefault;
export declare const user: Any;
export type IfTuple<T> = Any;
export type A = IfTuple<Array<Any>>;
export type B = IfTuple<string[]>;
export type ReadonlyTuple = Any;
export declare class MyClass {
    data: Array<Any>;
    constructor();
}
export declare interface User {
    info: Array<Any>;
}
export declare interface Options {
    config?: Array<Any>;
}
