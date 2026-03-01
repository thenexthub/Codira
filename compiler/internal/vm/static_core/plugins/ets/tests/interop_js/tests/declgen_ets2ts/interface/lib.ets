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

export interface Iface {
  n: number
  s() : string
}

export interface Iface2 {
  foo1() : void
}

export interface Iface3 {
  foo2(...nums: number[]) : void
}

export @interface Position {}

export function foo3(a : Array<string>) {
  console.log(a[0])
}
export class C {
  static foo3(a : Array<string>) { console.log(a[0]) }
  bar(a : Array<string>) { console.log(a[0]) }
}
export interface Iface4 {
  foo3(a : Array<string>) : void
}

export interface I {
  s: string
}

export interface Iface5 {
  foo5() : void
}
export function foo6(iface5 : Iface5) {
  console.log(iface5 instanceof Iface5) // true
}

export interface I01 {
  str: string;
  readonly s_read: string;
  s_optional?: string;
  get num1(): int;
  set num2(p1: int);
  clsMethod:(args: string)=> string;
  clsMethod1:(args: string)=> string;
}

namespace A {
  export type B = number
}

export interface I02 {
  a: A.B
  foo(): A.B
}

export interface Literal_Function_handler_Object_error {
  handler: Function0<void>;
  error: Type;
  toJSON(): jsonx.JsonElement;
}

interface I03 {
}
enum E01  {
    a = 1,
    b = 2
}
enum E02 {
    a = 1,
    b = 2
}
export interface I04 extends I03 {
    e: E01;
    f: E02;
    g: E01 | E02;
}

export namespace N01 {
  export interface I {}
}
export class C01 implements N01.I {}
