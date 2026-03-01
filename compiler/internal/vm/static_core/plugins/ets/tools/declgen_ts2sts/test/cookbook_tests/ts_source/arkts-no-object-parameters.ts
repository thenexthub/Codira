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

export function foo1(arg: Object) {}

export function foo2(obj: Object) {
    console.log(obj.toString())
}

export let a: Object = 1

export function foo3(): Object { return '123' }

class A {}

export function foo4(arg: Object) {
  return arg instanceof A
}

export function foo5(arg: Object) {
    arg as A
}

export function foo6(arg: Object) {
    return typeof arg
}

export function foo7(arg: Function) {
    return typeof Function
}

export let b: Function = () => {}
