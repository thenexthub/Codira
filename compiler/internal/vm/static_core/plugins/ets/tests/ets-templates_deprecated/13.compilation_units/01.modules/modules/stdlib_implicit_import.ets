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
desc: Module uses implicit import of exported entities from the stdlib
tags: [compile-only, not-a-test]
---*/

export function decode(str: string): string {
  return decodeURIComponent(str);
}

export function encode(str: string): string {
  return encodeURI(str);
}

export function encodeComp(str: string): string {
  return encodeURIComponent(str);
}

export function iteratorRes(): Object {
  return new IteratorResult<Object>();
}

export function json(): JSON {
  return new JSON();
}

function foo(i: (j: (k:Object)=>void)=>void) {}

export function promise(): Promise<Object> {
  return new Promise<Object>(foo);
}

export function rangeError(): Error {
  return new RangeError();
}

export function referenceError(): Error {
  return new ReferenceError();
}

export function syntaxError(): Error {
  return new SyntaxError();
}

export function uriError(): Error {
  return new URIError();
}

export function regexp(): Object {
  return new RegExp("some str");
}

export function weakMap(): Object {
  return new WeakMap<Object,Object>();
}

export function weakSet(): Object {
  return new WeakSet<Object>();
}
