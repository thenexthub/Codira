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
const TestJSLength = etsMod.getFunction('ArrayBuffer_TestJSLength');

{
	// Test JS ArrayBuffer
	const LENGTH = 32;
	TestJSLength(new ArrayBuffer(LENGTH));
}

{
	// Test ETS ArrayBuffer
	let arr = CreateEtsSample();
	ASSERT_TRUE(arr instanceof ArrayBuffer);
	const EXPECTED_BYTE_LENGTH = 32;
	ASSERT_EQ(arr.byteLength, EXPECTED_BYTE_LENGTH);
}

GCODESRuntimeCleanup();
