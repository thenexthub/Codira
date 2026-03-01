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

export let s = new Set<Number>();

export let ws = new WeakSet<Number>();

export let m = new Map<Number, Number>();

export let wp = new WeakMap<Number, String>();

export let tuple: [string, number] = ['hello', 42];

export type User = Record<string, string>;

export const rm: ReadonlyMap<string, number> = new Map([['one', 1], ['two', 2]]);

export const rs: ReadonlySet<number> = new Set([1, 2, 3]);