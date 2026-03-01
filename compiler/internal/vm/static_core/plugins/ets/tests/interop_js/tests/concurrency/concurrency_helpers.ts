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

// Basic type
export function processInput(input: string): string;
export function processInput(input: number): number;
export function processInput(input: boolean): boolean;
export function processInput(input: string | number | boolean): string | number | boolean {
  if (typeof input === 'string') {
    return input.toUpperCase();
  } else if (typeof input === 'number') {
    return input * 2;
  } else {
    return !input;
  }
}

// Generic
export function createArray<T>(value: T, length: number): T[];
export function createArray<T, U>(value1: T, value2: U, length: number): (T | U)[];
export function createArray<T, U>(
  value: T,
  value2OrLength: number | U,
  length?: number
): T[] | (T | U)[] {
  if (typeof length === 'number') {
    const value2 = value2OrLength as U;
    return Array.from({ length }, (_, i) => i % 2 === 0 ? value : value2);
  } else {
    const len = value2OrLength as number;
    return Array(len).fill(value);
  }
}

// Object
export interface User {
  id: number;
  name: string;
}

export interface Admin {
  id: number;
  role: string;
}

export function getUser(id: number): User;
export function getUser(role: string): Admin;
export function getUser(idOrRole: number | string): User | Admin;
export function getUser(idOrRole: number | string): User | Admin {
  if (typeof idOrRole === 'number') {
    return { id: idOrRole, name: 'John Doe' };
  } else {
    return { id: 1, role: idOrRole };
  }
}

export class TestClass {
  name: string = 'aaa';
  age: number = 24;
}

export class UnionPropClass {
  unionProp: number | string | object | TestClass | boolean = 1;
}

export let dynamicObj: UnionPropClass = new UnionPropClass();


export class Base {
  baseVal: number = 1;
  getCurrent(): Base {
      return this;
  }
}

export class Child extends Base {
  childVal: number = 2;
  getSuper(): Base {
      return super.getCurrent();
  }
}

export let baseFunc = new Base().getCurrent;
export let childFunc = new Child().getSuper;

export let fooBaseObj = {
  foo: baseFunc,
  fooFunc: function(): Base {
      return this.foo();
  }
};
export let fooChildObj = {
  foo: childFunc,
  fooFunc: function(): Base {
      return this.foo();
  }
};


export let arrowFunc: () => undefined = ():undefined => {
  return this;
};

export function foo1(a: number, b: string, c?: boolean, d: number = 1): boolean {
  return a === 1 && b === 'str' && c === undefined && d === 1;
}

export function foo2(a: number, b: string, c?: boolean, d: number = 1): boolean {
  return a === 1 && b === 'str' && c === true && d === 1;
}

export function foo3(a: number, b: string, c?: boolean, d: number = 1): boolean {
  return a === 1 && b === 'str' && c === true && d === 2;
}

export function fun1(a: number, b: string, c: boolean = true, d?: number): boolean {
  return a === 1 && b === 'str' && c === true && d === undefined;
}

export function fun2(a: number, b: string, c: boolean = true, d?: number): boolean {
  return a === 1 && b === 'str' && c === false && d === undefined;
}

export function fun3(a: number, b: string, c: boolean = true, d?: number): boolean {
  return a === 1 && b === 'str' && c === false && d === 2;
}

// errors
export let err: Error = new Error('This is a Error');

export function ErrorFunc(message: string): void {
  throw new Error(message);
}

export class CustomError extends Error {
  code: number;
  constructor(message: string, code: number) {
    super(message);
    this.name = 'CustomError';
    this.code = code;
  }
}

export function CustomErrorFunc(message: string, code: number): void {
  throw new CustomError(message, code);
}

export class SerializeTest {
  value1: string;
  value2: number;
  valueArray: string[];
  constructor(v1: string, v2: number, v3: string[]) {
    this.value1 = v1;
    this.value2 = v2;
    this.valueArray = v3;
  }
}

export let serializeTestObj = new SerializeTest('SerializeTest', 1, ['1', '2', '3']);
export let serializeUndefined = undefined;
export let serializeNull = null;
