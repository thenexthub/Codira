/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

function GetDouble42(): Object {
    let x: double = 42.0; return x;
}

class Pack {
    boolean_value: boolean = true
    string_value: string = ""
    byte_value: byte = 0
}

function newPackObject(): Pack {
    return new Pack()
}

class A {
    public static new_A(): A {
        return new A()
    }

    concat(s0: string, s1: string): string {
        return s0 + s1
    }
}

class B extends A {
    public static new_B(): B {
        return new B()
    }

    concat(s0: string, s1: string): string {
        return s0 + "_" + s1
    }
}

native function areReferencesEqual(lhs: Object | null | undefined, rhs: Object | null | undefined): boolean

export function checkNullishValuesEquality(): boolean {
    return areReferencesEqual(undefined, undefined) &&
        areReferencesEqual(null, null) &&
        areReferencesEqual(undefined, null) &&
        !areReferencesEqual(undefined, "") &&
        !areReferencesEqual(null, "") &&
        !areReferencesEqual("abc", "") &&
        areReferencesEqual("", "") &&
        !areReferencesEqual(1, "") &&
        !areReferencesEqual(1, 2) &&
        areReferencesEqual(1, 1)
}
