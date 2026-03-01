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
desc: Ambient Namespace declarations
name: namespace_declarations.d_10
tags:
- compile-only
- not-a-test
---*/

export declare namespace Ambient {
  namespace NestedAmbient {
    let x: int
    const y: float
    function foo(): void
      class A {}
      interface I {}
      enum Color {White, Blue, Red}
      const enum Season {Winter, Spring, Summer, Autumn}
      type Matrix = number[][]
  } 
  export namespace NestedAmbientExp {
    export let x: int
    export const y: float
    export function foo(): int
    export class A { a: string }
    export interface I { a: string}
    export enum Color {White, Blue, Red}
    export const enum Season {Winter = 'w', Spring = 'sp', Summer = 'su', Autumn = 'a'}
    export type Matrix = number[][]
  }
}
