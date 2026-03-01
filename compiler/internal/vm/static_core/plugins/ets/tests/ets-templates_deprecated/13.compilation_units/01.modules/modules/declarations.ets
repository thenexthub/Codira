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
desc: Module contains top-level declaration - types, classes, interfaces, enums, variables (including const) and functions
tags: [compile-only, not-a-test]
---*/

// classes, interfaces, enum
export class A {}
export class B extends A {}
export abstract class AC {}
export class C extends AC {}
export interface I {}
export interface II extends I {}
export class D implements II {}i
export enum ENUM { Red, Blue }

// variables
export let a = 1;
export let i: int = 2;
export let n: number = 3;
export let s: String = "knock-knock";
export let c: char = c'A';
export let u: String|Number|undefined|null = undefined;
export let arr: byte[] = new byte[3];

// constants
export const ca = 11;
export const cs = "who`s there";
export const ci: int = 12;

// functions
export let res: int = 0;
export function NoArg(): void { res = 1; };
export function OneArg(i: Number): int { return 2; };
export function TwoArgs(j: char, k: byte): int { return 3; };
export function Gen<T>(i: T): T { return i; };
export function ArgFun(i: () => int): int { return i(); };
export function ArgFun2(i: (j: int) => int, k: int): int { return i(k); };

// types
export type CLASS_A = A;
export type VAR_i = int;
export type TPL = [Number, String];
