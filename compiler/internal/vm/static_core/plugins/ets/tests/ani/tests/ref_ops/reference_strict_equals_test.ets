/*
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

function GetNull(): null {
    return null
}

function GetUndefined(): undefined {
    return undefined
}

function getObject(): Object {
    return "Hello World!"
}

function GetDoubleNaN(): Object { 
    let x: double = NaN; return x; 
}

function GetFloatNaN(): Object { 
    let x: float = Double.toFloat(NaN); return x; 
}

function GetObjectNaN(): Object { 
    let x: object = NaN; return x; 
}

native function areReferencesStrictEqual(lhs: Object | null | undefined, rhs: Object | null | undefined): boolean

export function checkNullishValuesStrictEquality(): boolean {
    return areReferencesStrictEqual(undefined, undefined) &&
        areReferencesStrictEqual(null, null) &&
        !areReferencesStrictEqual(undefined, null) &&
        !areReferencesStrictEqual(undefined, "") &&
        !areReferencesStrictEqual(null, "") &&
        !areReferencesStrictEqual("abc", "") &&
        areReferencesStrictEqual("", "") &&
        !areReferencesStrictEqual(1, "") &&
        !areReferencesStrictEqual(1, 2) &&
        areReferencesStrictEqual(1, 1) &&
        !areReferencesStrictEqual(GetDoubleNaN(), GetDoubleNaN()) &&
        !areReferencesStrictEqual(GetFloatNaN(), GetFloatNaN()) &&
        !areReferencesStrictEqual(GetObjectNaN(), GetObjectNaN()) &&
        !areReferencesStrictEqual(GetDoubleNaN(), undefined)
}
