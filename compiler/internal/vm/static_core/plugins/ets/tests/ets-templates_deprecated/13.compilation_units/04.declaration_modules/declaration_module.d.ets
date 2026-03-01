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


/*---
desc: >-
    Sample from spec, Namespaces with identical namespace names in a single compilation unit
    merge their exported declarations into a single namespace.
tags: [compile-only, not-a-test]
---*/

export declare namespace NS_A {
  export function foo(): int
}

export declare namespace NS_B {
  export function foo(): int
}

export declare class A {
  f: int
}

export declare class B {
  a: A
}

declare class C { }

declare class DD {
  public static fdd(): int
}

declare class EE {
  fee(): string
}

export {DD, EE as ZZ}

export declare class Person {
  name: string
  age: number
  constructor(name: string, age: number)
  getName(): string
  getAge(): number
  static createDefault(): Person
}

export declare function process_field(p: A): int

export declare type Matrix = number[][]
export declare type Handler = (s: string, no: number) => string

export declare type Predicate<T> = (x: T) => Boolean
export declare type NullableNumber = Number | null | undefined
export declare type Dictionary = Map<string, string>
export declare type MapOfString<T> = Map<T, string>
export declare type AAA<T> = Array<AAA<T>>
export declare type AAA2<T> = string | Array<AAA2<T>>
