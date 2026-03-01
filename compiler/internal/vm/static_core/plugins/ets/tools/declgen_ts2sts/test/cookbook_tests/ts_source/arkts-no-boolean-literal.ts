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

export let trueLiteral: true = true;
export let falseLiteral: false = false;

export function getTrue(): true {
    return true;
}

export function getFalse(): false {
    return false;
}

// type narrowing to boolean literal
export function bar() {
    let flag = false;
    function sideEffect() {
        flag = true;
    }
    const result = Math.random() > 2;
    return result && flag;
}

export function foo() {
    try {
        let calledGetter = false;
        const obj = {};
        Object.defineProperty(obj, 'fn', {
            get() {
                calledGetter = true;
                return function () {
                    return 'from-getter';
                };
            }
        });
        const result = Reflect.apply(obj.fn, null, []);
        return result === 'from-getter' && calledGetter;
    } catch (e) {
        return false
    }
}
export class Test {
    a;
    constructor() {
        this.a = (b) => {
            let c = false;
        }
        return b === 'aaa' && c;
    }
}
export const aaa = (b) => {
    let c = false;
    return b === 'aaa' && c;
}
export interface TestA {
    aaa(): false;
}
