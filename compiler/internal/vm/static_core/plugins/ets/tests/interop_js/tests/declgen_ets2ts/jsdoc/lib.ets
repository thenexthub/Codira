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

/**
 * default
 */
export let a1: string = '1';

/**
 * @noninterop
 */
export let a2: string = '1';

/**
 * default
 */
export function b1(num: number): void {};

/**
 * @noninterop
 */
export function b2(num: number): void {};

/**
 * default
 */
export class c1 {
  a: string = '1';
  b(): string { return this.a };
}

/**
 * @noninterop
 */
export class c2 {
  a: string = '1';
  b(): string { return this.a };
}

/**
 * default
 */
export class c3 extends c1 {
  a: string = '1';
  b(): string { return this.a };
}

/**
 * @noninterop
 */
export class c4 extends c2 {
  a: string = '1';
  b(): string { return this.a };
}

/**
 * default
 */
export type d1 = string;

/**
 * @noninterop
 */
export type d2 = string;

/**
 * default
 */
export enum e1 {
  a,
  b,
  c
}

/**
 * @noninterop
 */
export enum e2 {
  a,
  b,
  c
}

/**
 * default
 */
export interface f1 {
  a:string;
  b:number;
  c:()=>void;
}

/**
 * @noninterop
 */
export interface f2 {
  a:string;
  b:number;
  c:()=>void;
}

/**
 * default
 */
export class c5 implements f1 {
  a: string = '1';
  b:number = 1;
  c:()=>void = () => {};
}

/**
 * @noninterop
 */
export class c6 implements f2 {
  a: string = '1';
  b:number = 1;
  c:()=>void = () => {};
}