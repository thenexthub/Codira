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

type ReadonlyTypeTest = Readonly<Test>;
type WritableTypeTest = Test;

export let readonlyTestInTs: ReadonlyTypeTest = new Test('readonly_in_ts', 1, 2);
export let writableTestInTs: WritableTypeTest = new Test('writable_in_ts', 3, 4);

export function checkReadonlyDataInTs(data: ReadonlyTypeTest): number {
	return data.data1 + data.data2;
}

export function checkWritableDataInTs(data: WritableTypeTest): number {
	return data.data1 + data.data2;
}

let checkReadonlyDataFromTs = etsVm.getFunction('Ltest_sts_readonly/ETSGLOBAL;', 'checkReadonlyDataFromTs');
let checkWritableDataFromTs = etsVm.getFunction('Ltest_sts_readonly/ETSGLOBAL;', 'checkWritableDataFromTs');
let checkReadonlyDataInEts = etsVm.getFunction('Ltest_sts_readonly/ETSGLOBAL;', 'checkReadonlyDataInEts');
let checkWritableDataInEts = etsVm.getFunction('Ltest_sts_readonly/ETSGLOBAL;', 'checkWritableDataInEts');
let checkDataEts = etsVm.getFunction('Ltest_sts_readonly/ETSGLOBAL;', 'checkDataEts');

ASSERT_TRUE(checkReadonlyDataFromTs() === 3);
ASSERT_TRUE(checkWritableDataFromTs() === 7);
ASSERT_TRUE(checkReadonlyDataInTs(readonlyTestInTs) === 3);
ASSERT_TRUE(checkReadonlyDataInTs(writableTestInTs) === 7);
ASSERT_TRUE(checkWritableDataInTs(readonlyTestInTs) === 3);
ASSERT_TRUE(checkWritableDataInTs(writableTestInTs) === 7);
ASSERT_TRUE(checkReadonlyDataInEts() === 11);
ASSERT_TRUE(checkWritableDataInEts() === 15);
ASSERT_TRUE(checkDataEts());
