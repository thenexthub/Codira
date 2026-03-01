/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

package rest_spread.test;

function checkRestOfNumber(f: JSValue): boolean {
    return ESValue.wrap(f).invoke([11, 22, 33], 11, 22, 33).toBoolean();
}

function checkRestOfString(f: JSValue): boolean {
    return ESValue.wrap(f).invoke(['aa', 'bb', 'cc'], 'aa', 'bb', 'cc').toBoolean();
}

interface B{name: string}

class A{}
function checkRestOfObject(f: JSValue): boolean {
    let a = new A();
    let b: B = {name: "bbb"};
    let c = () => {};
    let objArr: FixedArray<object> = [a, b, c];
    return ESValue.wrap(f).invoke(objArr, a, b, c).toBoolean();
}

function checkRestOfTuple(f: JSValue): boolean {
    let tuple1: [int, string] = [1, 'a'];
    let tuple2: [int, string] = [2, 'b'];
    let tuple3: [int, string] = [3, 'c'];
    let tuple4: [int, string][] = [tuple1, tuple2, tuple3];
    return ESValue.wrap(f).invoke(tuple4, tuple1, tuple2, tuple3).toBoolean();
}

function checkRestofUnion(f: JSValue): boolean {
    let unionArr1: FixedArray<int | string | boolean> = [1, false];
    let unionArr2: FixedArray<int | string | boolean> = [2, 3, true, 'a', 'b'];
    return ESValue.wrap(f).invoke([unionArr1, unionArr2], unionArr1, unionArr2).toBoolean();
}

function checkRestOfJSValue(f: JSValue): boolean {
    let a = 1;
    let b = "b";
    let c = true;
    return ESValue.wrap(f).invoke([a, b, c], a, b, c).toBoolean();
}
