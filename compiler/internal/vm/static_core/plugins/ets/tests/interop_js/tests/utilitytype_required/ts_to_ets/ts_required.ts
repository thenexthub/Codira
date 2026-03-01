/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

let etsVm = globalThis.gtest.etsVm;

export class Test {
    public name: string;
    public data1: number;
    public data2: number;

    constructor(name: string, data1: number, data2: number) {
        this.name = name;
        this.data1 = data1;
        this.data2 = data2;
    }
}

type RequiredTypeTest = Required<Test>;
type OptionalTypeTest = Test;

export let requiredTestInTs: RequiredTypeTest = new Test('required_in_ts', 1, 2);
export let optionalTestInTs: OptionalTypeTest = new Test('optional_in_ts', 3, 4);

export function checkRequiredDataInTs(data: RequiredTypeTest): number {
    return data.data1 + data.data2;
}

export function checkOptionalDataInTs(data: OptionalTypeTest): number {
    let val: number = 0;
    if (data.data1 !== undefined) {
        val = val + data.data1;
    }
    if (data.data2 !== undefined) {
        val = val + data.data2;
    }
    return val;
}

let checkRequiredDataInEts = etsVm.getFunction('Ltest_sts_required/ETSGLOBAL;', 'checkRequiredDataInEts');
let checkOptionalDataInEts = etsVm.getFunction('Ltest_sts_required/ETSGLOBAL;', 'checkOptionalDataInEts');
let checkDataEts = etsVm.getFunction('Ltest_sts_required/ETSGLOBAL;', 'checkDataEts');
let checkRequiredDataNewInEts = etsVm.getFunction('Ltest_sts_required/ETSGLOBAL;', 'checkRequiredDataNewInEts');
let checkOptionalDataNewInEts = etsVm.getFunction('Ltest_sts_required/ETSGLOBAL;', 'checkOptionalDataNewInEts');

ASSERT_TRUE(checkRequiredDataInEts() === 3);
ASSERT_TRUE(checkOptionalDataInEts() === 7);
ASSERT_TRUE(checkRequiredDataInTs(requiredTestInTs) === 3);
ASSERT_TRUE(checkRequiredDataInTs(optionalTestInTs) === 7);
ASSERT_TRUE(checkOptionalDataInTs(requiredTestInTs) === 3);
ASSERT_TRUE(checkOptionalDataInTs(optionalTestInTs) === 7);
ASSERT_TRUE(checkRequiredDataNewInEts() === 11);
ASSERT_TRUE(checkOptionalDataNewInEts() === 15);
ASSERT_TRUE(checkDataEts());
