/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

let path = 'export12'; //1.2
import(path)
	.then((value) => {
		// getProperty
		ASSERT_TRUE(value !== undefined);
		ASSERT_TRUE(typeof value.ClassA === 'function');

		let ClassA = value.ClassA;
		let a = new ClassA();
		ASSERT_TRUE(typeof value.foo === 'function' && value.foo() === 'this is 1.2 foo ets');
		ASSERT_TRUE(value.a === 'this is 1.2 classA ets');

		// deleteProperty
		try {
			delete value.ClassA;
		} catch (error) {
			ASSERT_TRUE(error.toString() == 'TypeError: Cannot delete property');
			let ClassA = value.ClassA;
			let a = new ClassA();
			ASSERT_TRUE(typeof a.foo === 'function' && value.foo() === 'this is 1.2 foo ets');
		}

		// deleteProperty
		try {
			delete value.a;
		} catch (error) {
			ASSERT_TRUE(error.toString() == 'TypeError: Cannot delete property');
			ASSERT_TRUE(value.a === 'this is 1.2 classA ets');
		}

		// deleteProperty
		try {
			delete value.foo;
		} catch (error) {
			ASSERT_TRUE(error.toString() == 'TypeError: Cannot delete property');
			ASSERT_TRUE(typeof value.foo === 'function' && value.foo() === 'this is 1.2 foo ets');
		}

		// ownPropertyKeys
		/**
    ClassA
    length
    prototype
    name
    a
    main
    foo
    _$init$_
   */
		let keys = Reflect.ownKeys(value);
		ASSERT_TRUE(keys.length == 8);

		// SetProperty
		try {
			value.ClassA = {};
		} catch (error) {
			ASSERT_TRUE(error.toString() == 'TypeError: Cannot assign to read only property of Object Module');
			let ClassA = value.ClassA;
			let a = new ClassA();
			ASSERT_TRUE(typeof a.foo === 'function' && a.foo() === 'this is 1.2 classA ets');
		}

		// SetProperty
		try {
			value.a = {};
		} catch (error) {
			ASSERT_TRUE(error.toString() == 'TypeError: Cannot assign to read only property of Object Module');
			ASSERT_TRUE(value.a === 'this is 1.2 classA ets');
		}

		// SetProperty
		try {
			value.foo = {};
		} catch (error) {
			ASSERT_TRUE(error.toString() == 'TypeError: Cannot assign to read only property of Object Module');
			ASSERT_TRUE(typeof value.foo === 'function' && value.foo() === 'this is 1.2 foo ets');
		}

		// DefineOwnProperty true
		{
			ASSERT_TRUE(Reflect.defineProperty(value, 'ClassA', {}));
			let ClassA = value.ClassA;
			let a = new ClassA();
			ASSERT_TRUE(typeof a.foo === 'function' && a.foo() === 'this is 1.2 classA ets');
		}
		{
			ASSERT_TRUE(
				!Reflect.defineProperty(value, 'ClassA', {
					value: 1,
				})
			);
			let ClassA = value.ClassA;
			let a = new ClassA();
			ASSERT_TRUE(typeof a.foo === 'function' && a.foo() === 'this is 1.2 classA ets');
		}
	})
	.catch((error) => {
		print(error.toString());
	});
