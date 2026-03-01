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

/*---
desc: Issue 25615. Handle NaN values in ets.equals reference comparison fastpath
tags: []
---*/

function GetDoubleNaN(): double { return NaN; }
function GetFloatNaN(): float { return Double.toFloat(NaN); }

function main(): void {
    let objNaN: object = NaN;
    let objResult = ((a: object, b: object) => a == b)(objNaN, objNaN);
    arktest.assertFalse(objResult);

    let dNaN: object = GetDoubleNaN();
    let dResult = ((a: object, b: object) => a == b)(dNaN, dNaN);
    arktest.assertFalse(dResult);

    let fNaN: object = GetFloatNaN();
    let fResult = ((a: object, b: object) => a == b)(fNaN, fNaN);
    arktest.assertFalse(fResult);

    let objStrictResult = ((a: object, b: object) => a === b)(objNaN, objNaN);
    arktest.assertFalse(objStrictResult);

    let dStrictResult = ((a: object, b: object) => a === b)(dNaN, dNaN);
    arktest.assertFalse(dStrictResult);

    let fStrictResult = ((a: object, b: object) => a === b)(fNaN, fNaN);
    arktest.assertFalse(fStrictResult);
}
