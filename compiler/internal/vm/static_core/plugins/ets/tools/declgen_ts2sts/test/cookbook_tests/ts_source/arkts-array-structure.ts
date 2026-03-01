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

export let ub64 = new BigUint64Array(4000);

export let b64 = new BigInt64Array(1000);

export let ab = new ArrayBuffer(128);

export let arr = [1, 2, 3];
export function fooArr(a: number[]) {
    return a;
}
export let res = fooArr(arr);
export function fooArr2(a : Array<string>) {
    console.log(a[0])
}
export class C {
    static foo(a : Array<string>) { console.log(a[0]) }
    bar(a : Array<string>) { console.log(a[0]) }
}
export interface Iface {
    foo(a : Array<string>) : void
}