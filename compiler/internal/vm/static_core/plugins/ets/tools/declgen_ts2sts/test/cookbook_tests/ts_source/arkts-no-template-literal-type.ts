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

export type A = `hello ${string}`;
export let v1: A = 'hello world';
export function foo(v1: `hello ${string}`): void { };
export function bar<T extends `hello ${string}`>(arg: `hello ${string}`): A { return arg; };

export type v2 = 'world';
export type B = `hello ${v2}`;
