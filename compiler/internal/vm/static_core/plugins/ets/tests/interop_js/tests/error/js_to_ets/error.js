'use strict';
/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export function throwNum() {
    throw 0;
}

export function throwStr() {
    throw 'hello';
}

export function throwObj() {
    throw {a: 1, b: 2};
}

class SubError extends Error {
    extraField = 123;
    foo() { return 456; }
}

export function throwErrorSubClass() {
    throw new SubError();
}

export function throwError1() {
    let err = new Error();
    err.message = {data: 123};
    throw err;
}

export function throwError2() {
    throw new Error('null pointer error');
}

export function throwError3() {
    try {
        throw new Error('original error');
    } catch (originalError) {
        throw new Error('current error', { cause: originalError });
    }
}

export function throwEtsError(fn) {
    return fn();
}