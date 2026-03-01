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

export let s = new Set<number>()

export const weakMap: WeakMap<object, string> = new WeakMap<object, string>();

export let w = new WeakSet<object>()

export let map = new Map<string, number>()

export let f32 = new Float32Array()

export let f64 = new Float64Array()

export let i8 = new Int8Array()

export let i16 = new Int16Array()

export let i32 = new Int32Array()

export let ui8 = new Uint8Array()

export let ui16 = new Uint16Array()

export let ui32 = new Uint32Array()

export let ui8c = new Uint8ClampedArray()

const buffer = new ArrayBuffer(16);
export const dataView: DataView = new DataView(buffer);

export let b64 = new BigInt64Array(2)

export let ub64 = new BigUint64Array(2)

const regExp: RegExp = new RegExp('abc');
export const matchResult: RegExpExecArray | null = regExp.exec('abcdef');

export const regExp2: RegExp = new RegExp('abc');

export let foo : (...nums: number[]) => void = (...nums: number[]) : void => {}

export type StringDictionary = Record<string, string>

export type Matrix = number[][]
export type Handler = (s: string, no: number) => string
export type Predicate<T> = (x: T) => Boolean
export type NullableNumber = Number | null
export type Dictionary = Map<string, string>