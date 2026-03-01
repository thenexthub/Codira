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

import { a } from './a';
import * as b from './b';
export let c;
export * from './d';
export { e } from './e';

class HelloWorld {
  str = 'HelloWorld';
}

class Lit {
  *lit(): Generator<never, void, unknown> { }
}

class NestedLiteralArray {
  num: number = 1;
  NestedLiteralArray(): void { }
  constructor() {
    'use sendable';
  }
}

msg: string = '';

function foo(): void {
  try {
    varA = 11;
    x = 22;
    try {
      varA = 1;
    } catch (e) {
      msg = 'inner catch';
      print(msg);
    }
    if (varA === '') {
      throw 'null';
    }
    if (x > 100) {
      throw 'max';
    } else {
      throw 'min';
    }
  }
  catch (err) {
    masg = 'outter catch';
    print(msg);
  }
  finally {
    msg = 'error';
    print(msg);
  }
}

function goo(): void { }

function hoo(): void {
  varA = 1.23;
  let obj = { async * method(): AsyncGenerator<never, void, unknown> { } };
}

let add = (a: number, b: number): number => a + b;
add(1, 2);

function* generateFunc(): Generator<string, void, unknown> {
  yield 'hello';
}

async function* asyncGenerateFunc(): AsyncGenerator<string, void, unknown> {
  yield 'hello';
}

const asyncArrowFunc = async (): Promise<void> => { };

foo();

print(goo.toString());

hoo();

function sendableFunction() {
  "use sendable";
}

async function asyncSendableFunction() {
  "use sendable";
}

