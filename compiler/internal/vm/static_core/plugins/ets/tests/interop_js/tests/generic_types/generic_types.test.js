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

const etsVm = globalThis.gtest.etsVm;

let EtsGenericValueHandle = etsVm.getClass('Lgeneric_types/test/EtsGenericValueHandle;');
let EtsGenericErrorHandle = etsVm.getClass('Lgeneric_types/test/EtsGenericErrorHandle;');
let etsGetGenericValueIdentity = etsVm.getFunction('Lgeneric_types/test/ETSGLOBAL;', 'etsGetGenericValueIdentity');
let etsGetGenericErrorIdentity = etsVm.getFunction('Lgeneric_types/test/ETSGLOBAL;', 'etsGetGenericErrorIdentity');

function checkGenericValueForFunction(v) {
	// Check the parameter and the return value of the function
	let genValue = etsGetGenericValueIdentity(v);
	ASSERT_EQ(v, genValue);
}

function checkGenericValueForInstance(v) {
	let genHandle = new EtsGenericValueHandle(v);

	// Check the parameter and the return value of the class method
	let value = genHandle.getValue();
	ASSERT_EQ(v, value);

	genHandle.setValue(new ets.Object());

	// Check access to generic field
	genHandle.value = v;
	let fieldValue = genHandle.value;
	ASSERT_EQ(v, fieldValue);
}

function checkGenericValue(v) {
	checkGenericValueForFunction(v);
	checkGenericValueForInstance(v);
}

module.exports = {
	EtsGenericValueHandle,
	EtsGenericErrorHandle,
	checkGenericValue,
	etsGetGenericErrorIdentity,
};
