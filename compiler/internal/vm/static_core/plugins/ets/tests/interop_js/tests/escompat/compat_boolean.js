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

const { etsVm, getTestModule } = require('escompat.test.abc');

const etsMod = getTestModule('escompat_test');
const CreateEtsSample = etsMod.getFunction('Boolean_CreateEtsSample');
const TestJSSample = etsMod.getFunction('Boolean_TestJSSample');

{
	// Test JS Boolean
	TestJSSample(new Boolean(false));
}

{
	// Test ETS Boolean
	let v = CreateEtsSample();
	ASSERT_TRUE(v instanceof Boolean);

	ASSERT_EQ(v.toString(), 'true');
}
