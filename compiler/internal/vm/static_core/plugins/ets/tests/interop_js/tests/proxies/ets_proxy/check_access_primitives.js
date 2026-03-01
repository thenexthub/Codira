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

const PrimitivesAccess = getTestClass('PrimitivesAccess');
const PrimitivesAccessStatic = getTestClass('PrimitivesAccessStatic');


let pa = new PrimitivesAccess();
let pas = PrimitivesAccessStatic;

let typelist = ['byte', 'short', 'int', 'long', 'float', 'double', 'char', 'boolean'];
const cap = (str = '') => str[0].toUpperCase() + str.substring(1);

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
		testAccessorsOf(pa, tname, ...values);
		testAccessorsOf(pas, tname, ...values);
	}

	function intBit(exp) {
		ASSERT_TRUE(exp < 53, 'intBit overflow');
		return (1 << exp % 30) * (1 << (exp - (exp % 30)));
	}
	function testSInt(tname, bits) {
		let msb = bits - 1;
		testAccessors(tname, 0, 1, -1, intBit(msb) - 1, -intBit(msb));
	}
	function testUInt(tname, bits) {
		let msb = bits - 1;
		testAccessors(tname, 0, 1, intBit(msb), intBit(msb + 1) - 1);
	}

	testSInt('byte', 8);
	testSInt('short', 16);
	testSInt('int', 32);
	testSInt('long', 53);
	testAccessors('float', 0, 1, 1.25, 0x1234 / 256, Infinity, NaN);
	testAccessors('double', 0, 1, 1.33333, 0x123456789a / 256, Infinity, NaN);
	testAccessors('char', '1');
	testAccessors('boolean', false, true);
}

{
	// Test typechecks
	typelist.forEach(function (tname) {
		function check(o) {
			ASSERT_THROWS(TypeError, () => (o['f' + cap(tname)] = undefined));
		}
		check(pa);
		check(pas);
	});
}
