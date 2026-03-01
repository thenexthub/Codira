/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

const { etsVm, getTestModule } = require('../escompat.test.abc');

const etsMod = getTestModule('escompat_test');
const GCODESRuntimeCleanup = etsMod.getFunction('GCODESRuntimeCleanup');
const FooClass = etsMod.getClass('FooClass');
const CreateEtsSample = etsMod.getFunction('Array_CreateEtsSample');
const TestJSSplice = etsMod.getFunction('Array_TestJSSplice');

// NOTE(oignatenko) enable after interop will be supported for this method signature
const FIXES_IMPLEMENTED = false;

// NOTE(kprokopenko): change to `x.length` when interop support properties
const etsArrLen = (x) => x.length;

{
	// Test JS Array<FooClass>
	TestJSSplice(new Array(new FooClass('zero'), new FooClass('one')));
}

{
	let arr = CreateEtsSample();
	const EXPECT_2 = 2;
	const EXPECT_3 = 3;
	arr.push('spliced');
	ASSERT_EQ(etsArrLen(arr), EXPECT_3);
	arr.splice(1, 1);
	ASSERT_EQ(arr.at(0), 123);
	ASSERT_EQ(arr.at(1), 'spliced');
	ASSERT_EQ(etsArrLen(arr), EXPECT_2);

	let arr1 = CreateEtsSample();
	arr1.push('spliced');
	ASSERT_EQ(etsArrLen(arr1), EXPECT_3);

	if (FIXES_IMPLEMENTED) {
		arr1.splice(1);
		ASSERT_EQ(arr1.at(0), 123);
		ASSERT_EQ(arr1.length, 1);
	}
}

GCODESRuntimeCleanup();
