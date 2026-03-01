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
const CreateEtsSample = etsMod.getFunction('ArrayBuffer_CreateEtsSample');
const TestJSSlice = etsMod.getFunction('ArrayBuffer_TestJSSlice');

{
	// Test JS ArrayBuffer
	const LENGTH = 32;
	TestJSSlice(new ArrayBuffer(LENGTH));
}

{
	// Test ETS ArrayBuffer
	let arr = CreateEtsSample();
	const BEGIN = 8;
	const END = 24;
	let sliced = arr.slice(BEGIN, END);
	const EXPECTED_BYTE_LENGTH = 16;
	ASSERT_EQ(sliced.byteLength, EXPECTED_BYTE_LENGTH);
}

GCODESRuntimeCleanup();
