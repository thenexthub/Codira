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

const {
    getTestClass,
} = require('ets_proxy.test.abc');

const ReferencesAccess = getTestClass('ReferencesAccess');
const ReferencesAccessStatic = getTestClass('ReferencesAccessStatic');
const UClass1 = getTestClass('UClass1');
const UClass2 = getTestClass('UClass2');
const cap = (str = '') => str[0].toUpperCase() + str.substring(1);

let ra = new ReferencesAccess();
let ras = ReferencesAccessStatic;

// NOTE enable when Promise is fully implemented
const PROMISE_IMPLEMENTED = false;

{
	// Test property accessors
	function testAccessors(tname, ...values) {
		function testAccessorsOf(o, tname, ...values) {
			for (let v of values) {
				o['f' + cap(tname)] = v;
				ASSERT_EQ(o['getf' + cap(tname)](), v);
				o['setf' + cap(tname)](v);
				ASSERT_EQ(o['f' + cap(tname)], v);
			}
		}
		testAccessorsOf(ra, tname, ...values);
		testAccessorsOf(ras, tname, ...values);
	}

	testAccessors('UClass1', new UClass1());
	testAccessors('String', 'fooo', '0123456789abcdef');
	testAccessors('JSValue', 1234, 'fooo', {}, new UClass1());
	PROMISE_IMPLEMENTED &&
		testAccessors(
			'Promise',
			new Promise(
				function () {},
				function () {}
			)
		); // TODO(konstanting): enable

	{
		let arr = ['a', 'bb', 'ccc'];
		ra.fAString = arr;
		ASSERT_EQ(JSON.stringify(ra.fAString), JSON.stringify(arr));
	}
	{
		let arr = [new UClass1(), new UClass1()];
		ra.fAUClass1 = arr;
		ASSERT_EQ(ra.fAUClass1[0], arr[0]);
		ASSERT_EQ(ra.fAUClass1[1], arr[1]);
	}
}

{
	// Test typechecks
	function testTypecheck(tname, ...values) {
		function check(o, v) {
			ASSERT_THROWS(TypeError, () => (o['f' + cap(tname)] = v));
		}
		for (let v of values) {
			check(ra, v);
			check(ras, v);
		}
	}

	testTypecheck('UClass1', NaN, {}, new UClass2());
	testTypecheck('String', NaN, {}, new UClass1());
	PROMISE_IMPLEMENTED && testTypecheck('Promise', NaN, {}, new UClass1()); // TODO(konstanting): enable

	testTypecheck('AString', NaN, {}, [1], ['1', 1], [{}], [new UClass2()]);
	testTypecheck('AUClass1', NaN, {}, [1], ['1', 1], [{}], [new UClass2()]);
}
