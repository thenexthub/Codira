/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {C1} from './include/lib';
export class CustomPromise<T> extends Promise<T> {
  resolve: (value: T | PromiseLike<T>) => void;
  reject: (reason?: Error) => void;
  constructor() {
    let tempResolve: (value: T | PromiseLike<T>) => void;
    let tempReject: (reason?: Error) => void;
    super((resolve, reject) => {
      tempResolve = resolve;
      tempReject = reject;
    });
  }
  static get [Symbol.species](): typeof Promise {
    return Promise;
  }
}

export class Animal {
  move() {
    console.log('Moving!');
  }
}

export class Dog extends Animal {
  bark() {
    console.log('Woof!');
  }
}

export interface Shape {
  color: string;
}

export interface PenStroke {
  penWidth: number;
}

export interface Square extends Shape, PenStroke {
  sideLength: number;
}
export class test extends C1 {
  penWidth: number;
}

export class testPromise extends Promise<void> {
}

export interface SquareTest extends Shape, Promise<void>{
  sideLength: number;
}