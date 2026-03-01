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
export type MyTuple=[number,string,boolean];
export let tuple:MyTuple=[6,'abc',true];

export let tupleObject: [number, number]=[0x55aa, 0x7c00];

export function example(args: [string, number]): [boolean] {
  return [true];
}

export const value: [string, number] = ['hello', 42];

export interface User {
  info: [string, number];
}

export class MyClass {
  data: [number, string];

  constructor() {
    this.data = [0, ''];
  }
}

export const [a, b]: [string, number] = ['hello', 42];

export type Result = [string, number] | null;

export type Mapped<T> = { [K in keyof T]: [T[K], string] };
export type Test = Mapped<{ a: number; b: boolean }>;

export type NestedTuple = [[number, string], [boolean, object]];

export type Mixed = [number, string][] | [string, number];

export async function getData(): Promise<[string, number]> {
  return ['data', 123];
}

export type WithDefault<T = [string, number]> = T;

export type Example = WithDefault;

export interface Options {
  config?: [string, number];
}

export const user = {
  name: 'Alice',
  metadata: [1, 'info']
};

export type IfTuple<T> = T extends [any, any] ? true : false;

export type A = IfTuple<[number, string]>;
export type B = IfTuple<string[]>;

export type ReadonlyTuple = readonly [string, number];

