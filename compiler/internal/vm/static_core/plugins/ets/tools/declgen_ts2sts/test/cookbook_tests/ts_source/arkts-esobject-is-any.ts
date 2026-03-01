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

export type TA<T> = { data: T; obj: ESObject }

export class CX { data: ESObject = 0 }
export class CY<T> { data: T; obj: ESObject }

export interface IA { foo(arg: ESObject): ESObject }

export let va: ESObject = 1
export let vb: CY<ESObject> = { data: '123', obj: '123' }
export let vc: CY<CY<ESObject>> = { data: { data: '123', obj: '123' }, obj: '123' }
export let vd = 1 as ESObject
export let ve: CY<ESObject> = {} as CY<ESObject>

export function foo1(): ESObject {
    return '123'
}
export function foo2(arg: ESObject): ESObject {
    return '123'
}
