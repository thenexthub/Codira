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

export interface I0 {
    I0Method(a: string): string { return a; };
}

export interface I1 {
    I1Method(a: double): double { return a; };
}

export class Base {
    a: double = 1;
    constructor(a: double) {this.a = a;}
}

export class Derived extends Base implements I0, I1 {
    I0Method(a: string): string { return a; };
    I1Method(a: double): double { return a; };
    b: double = 2;
    constructor(a: double, b: double) {super(a); this.b = b;}
}

export class X {
    a: number = 0
    protected b: number = 0
    private c: number = 0
}

export class X1 {
  foo() {}
}
export let cb: (this: X1) => void = (this: X1) => { this.foo() }

export class X2 {
 foo2(): this { return this }
}

export class C {
  foo() { console.log("this.s") }
}

export class C1 {
  foo1(n : number = 1) { console.log(n) }
}

export class C2 {
  foo2(...nums: number[]) {}
}

export class C3 {
  native foo3() : void
}

export class C4 {
    async foo4() {}
}

export class C5 {
  static foo5() : void {}
}

export class C6 {
  static foo6(n : number = 1) { console.log(n) }
}

export class C7 {
  static foo7(...nums: number[]) {}
}

export class C8 {
  native foo8() : void
}

export class C9 {
  static async foo9() {}
}

export class C10 {
  s : string = 'Hi'
}

export class C11 {
  static foo11 : number = 0
}

export class Person {
    private name_: string = ""
    constructor(name: string) {
        this.name_ = name
    }
    get name(): string {
        console.log(this.name_)
        return this.name_
    }
    set name(name: string) {
        this.name_ = name
        console.log(this.name_)
    }
}

export class A {}
get fullName(this: A): string {return '123'}
set fullName(this: A, val: string) {}

export abstract class Animal {
  abstract move(): void
}

class B {
  foo() : void {}
}
export class D extends B {
  override foo() : void { console.log("D.foo") }
}

export class C12 {
  foo(i: "some str") {
    return 2;
  }
}

export class DeviceInfo {
  DeviceInfo() {}
  static instance?: DeviceInfo;
}

export class ReadOnlyClass {
  foo1(nums: readonly number[]) {}
  foo2(nums: readonly [number, string]) {}
  foo3(nums: readonly Array<number>) {}
  static foo4(nums: readonly Array<number>) {}
  foo5(...nums: readonly Array<number>) {}
  constructor(nums: readonly [number, string]) {}
}

export class Dog {
  s: string = ''
  n: number = 0;
  foo1(s?: string, n?: number) {}
  constructor(s?: string, n?: number) {
    if (s !== undefined) {
      this.s = s;
    }
    if (n !== undefined) {
      this.n = n;
    }
  }
}

export interface I2 {
  test(): void;
}

export declare interface I3 extends I2 {
  default foo(): void;
}
