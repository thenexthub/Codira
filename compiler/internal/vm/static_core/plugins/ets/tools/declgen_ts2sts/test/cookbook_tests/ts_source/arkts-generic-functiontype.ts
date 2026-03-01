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

export let cb: (x:number) => number
export type ComplexNormalFunc = (arg1: number, arg2: boolean) => string[];
export let lambdaFunctionGeneric: <T>(a:T)=>T
export type GenericFunc = <T, U>(arg1: T, arg2: U) => [T, U];
export type ConstrainedGenericFunc = <T extends string>(arg: T) => T;
export type NestedGenericFunc = <T>(fn: <U>(u: U) => T) => T;
export type DefaultGenericFunc = <T = string>(arg?: T) => T;
export interface TestInterface {
  genericMethod: <T>(data: T) => T;
  normalMethod: (data: string) => string;
}
export class TestClass {
  genericFunc: <T>(value: T) => T = (value) => value;
  normalFunc: (value: number) => number = (value) => value * 2;
}