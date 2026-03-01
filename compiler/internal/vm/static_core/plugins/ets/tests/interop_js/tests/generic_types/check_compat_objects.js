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

const { EtsGenericErrorHandle, checkGenericValue, etsGetGenericErrorIdentity } = require('generic_types.test.abc');

// NOTE enable when error typing is enabled
const FIXES_IMPLEMENTED = false;

function checkErrorMessage(err, errMsg) {
	checkGenericValue(err);

	let etsErrorHandle = new EtsGenericErrorHandle(err);
	let msg = etsErrorHandle.getErrorMessage();

	ASSERT_EQ(msg, errMsg);

	let genError = etsGetGenericErrorIdentity(err);
	ASSERT_EQ(err, genError);
}

function checkIncorrectJsObject() {
	let errMsg = 'incorrect js object';
	let obj = { message: errMsg };

	ASSERT_THROWS(TypeError, () => checkErrorMessage(obj, errMsg));
	ASSERT_THROWS(TypeError, () => etsGetGenericErrorIdentity(obj));
}

function checkIncorrectEtsObject() {
	let errMsg = 'incorrect ets object';
	let obj = new ets.Object();

	ASSERT_THROWS(TypeError, () => checkErrorMessage(obj, errMsg));
	ASSERT_THROWS(TypeError, () => etsGetGenericErrorIdentity(obj));
}

// TODO(v.cherkashin): Enable when implemented
// Check js/ets errors
checkErrorMessage(new ets.Error('ets Error message', undefined), 'ets Error message');
FIXES_IMPLEMENTED && checkErrorMessage(new Error('js Error message'), 'js Error message');
FIXES_IMPLEMENTED && checkErrorMessage(new ets.TypeError('ets TypeError message'), 'ets TypeError message');
FIXES_IMPLEMENTED && checkErrorMessage(new TypeError('js TypeError message'), 'js TypeError message');

checkIncorrectEtsObject();
checkIncorrectJsObject();
