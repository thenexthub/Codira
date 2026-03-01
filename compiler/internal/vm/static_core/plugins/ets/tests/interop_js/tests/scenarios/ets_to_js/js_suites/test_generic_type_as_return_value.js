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
const genericTypeReturnValueEts = etsMod.getFunction('genericTypeReturnValueEts');

{
	const VALUE0 = 0;
	const VALUE1 = 1;
	const VALUE2 = 2;
	const STRING_VALUE = '2';
	const TRUE_VALUE = true;

	let ret = genericTypeReturnValueEts(VALUE0, VALUE1);
	ASSERT_EQ(ret, VALUE1);

	ret = genericTypeReturnValueEts(VALUE1, STRING_VALUE);
	ASSERT_EQ(ret, STRING_VALUE);

	ret = genericTypeReturnValueEts(VALUE2, TRUE_VALUE);
	ASSERT_EQ(ret, TRUE_VALUE);
}

GCODESRuntimeCleanup();
