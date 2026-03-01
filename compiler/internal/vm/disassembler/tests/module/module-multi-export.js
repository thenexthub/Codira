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
flags: [module]
---*/


let a = 1;
let b1 = 2;
let b2 = 3;
let d = 5;
let e = 6;

export { a };
export { b1, b2 };

export { c } from './module-export-file.js';
export * from './module-export-file.js';

export { d as dd, e as ee };

export let f = 7;
export let g = 8;
export let h = 9;

let i = 10;
let j = 11;
export class ClassB {
  fun1(i, j) { return i + j; }
}

export function FunctionC() {
  let l = 12;
}