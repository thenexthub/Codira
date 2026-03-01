/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

import { A } from "strict_equality"

function TestEqualTrue(): boolean {
    let a = new A(42);
    if (a === a) {
        return true;
    } else {
        return false;
    }
}

function TestEqualFalse(): boolean {
    let a = new A(42);
    let b = new A(42);
    if (a === b) {
        return false;
    } else {
        return true;
    }
}

function TestNotEqualFalse(): boolean {
    let a = new A(42);
    if (a !== a) {
        return false;
    } else {
        return true;
    }
}

function TestNotEqualTrue(): boolean {
    let a = new A(42);
    let b = new A(42);
    if (a !== b) {
        return true;
    } else {
        return false;
    }
}

// Behaviour for equal is the same as strict equal
function TestNotStrictEqualTrue(): boolean {
    let a = new A(42);
    if (a == a) {
        return true;
    } else {
        return false;
    }
}

function TestNotStrictEqualFalse(): boolean {
    let a = new A(42);
    let b = new A(42);
    if (a == b) {
        return false;
    } else {
        return true;
    }
}

function TestNotStrictNotEqualFalse(): boolean {
    let a = new A(42);
    if (a != a) {
        return false;
    } else {
        return true;
    }
}

function TestNotStrictNotEqualTrue(): boolean {
    let a = new A(42);
    let b = new A(42);
    if (a != b) {
        return true;
    } else {
        return false;
    }
}
