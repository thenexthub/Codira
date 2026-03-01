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

export namespace NS_A{
    export function foo(): int {
      return 1
    }
}

export namespace NS_B{
    export function foo(): int {
      return 2
    }
}

export class A {
    f: int = 7
}

export class B {
    public a: A = new A() // the field is exported but its type is not
}

export class C {}

export class DD {
    public static fdd(): int {
      return 3
    }
}

export class EE {
    public fee(): string {
      return "EE str"
    }
}

export class Person {
  name: string
  age: number
  constructor(name: string, age: number) {
    this.name = name
    this.age = age
  }
  getName(): string {
    return this.name
  }
  getAge(): number {
    return this.age
  }
  static createDefault(): Person {
    return new Person("name not set", 999)
  }
}

export function process_field(p: A): int {
  return 4
}

export type Matrix = number[][]
export type Handler = (s: string, no: number) => string

export type Predicate<T> = (x: T) => Boolean
export type NullableNumber = Number | null | undefined
export type Dictionary = Map<string, string>
export type MapOfString<T> = Map<T, string>
export type AAA<T> = Array<AAA<T>> // ok, AAA<T> is used as a type argument
export type AAA2<T> = string | Array<AAA2<T>> // ok
