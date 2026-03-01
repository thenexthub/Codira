/**
 * Copyright (c) 2023-2024-2025 Huawei Device Co., Ltd.
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
const { etsVm, getTestModule } = require('../scenarios.test.abc');

const etsMod = getTestModule('scenarios_test');
const GCODESRuntimeCleanup = etsMod.getFunction('GCODESRuntimeCleanup');
const genericTypeParameterEts = etsMod.getFunction('genericTypeParameterEts');

{
	const STRING_VALUE = '1';
	const INT_VALUE = 1;
	const STRING_TRUE_VALUE = 'true';
	const TRUE_VALUE = true;

	let ret = genericTypeParameterEts(INT_VALUE);
	ASSERT_EQ(typeof ret, 'string');
	ASSERT_EQ(ret, STRING_VALUE);

	ret = genericTypeParameterEts(TRUE_VALUE);
	ASSERT_EQ(typeof ret, 'string');
	ASSERT_EQ(ret, STRING_TRUE_VALUE);
}

GCODESRuntimeCleanup();
