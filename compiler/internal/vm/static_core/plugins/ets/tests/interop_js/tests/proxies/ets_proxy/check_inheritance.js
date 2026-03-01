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
	getTestFunction,
} = require('ets_proxy.test.abc');

const FooBase = getTestClass('FooBase');
const FooDerived = getTestClass('FooDerived');
const IdentityError = getTestFunction('IdentityError');
const cap = (str = '') => str[0].toUpperCase() + str.substring(1);

{
	// Verify prototype chain and properties
	let base = new FooBase();
	let derived = new FooDerived();

	ASSERT_TRUE(base instanceof FooBase);
	ASSERT_TRUE(derived instanceof FooDerived);
	ASSERT_TRUE(derived instanceof FooBase);

	ASSERT_EQ(Object.getPrototypeOf(FooDerived), FooBase);
	ASSERT_EQ(Object.getPrototypeOf(FooDerived.prototype), FooBase.prototype);

	ASSERT_EQ(base.fBase, 'fBase');
	ASSERT_EQ(FooBase.sfBase, 'sfBase');
	ASSERT_EQ(base.fnBase(), 'fnBase');

	ASSERT_EQ(derived.fDerived, 'fDerived');
	ASSERT_EQ(FooDerived.sfDerived, 'sfDerived');
	ASSERT_EQ(derived.fBase, 'fBase');
	ASSERT_EQ(FooDerived.sfBase, 'sfBase');
	ASSERT_EQ(derived.fnBase(), 'fnBase');
}

{
	// Passing downcasted object preserves proxy prototype
	let derived = new FooDerived();

	ASSERT_EQ(derived.getAsFooBase(), derived);
	ASSERT_TRUE(FooDerived.createAsFooBase() instanceof FooDerived);
	ASSERT_EQ(FooDerived.createAsFooBase().fDerived, 'fDerived');
}

{
	// Passing object as core/Object preserves proxy prototype
	let base = new FooBase();

	ASSERT_EQ(base.getAsObject(), base);
	ASSERT_TRUE(FooBase.createAsObject() instanceof FooBase);
	ASSERT_EQ(FooBase.createAsObject().fBase, 'fBase');

	ASSERT_EQ('' + base, 'FooBase toString');
	ASSERT_EQ('' + FooDerived.createAsFooBase(), 'FooDerived toString');
}

{
	// Simple compat proxy
	let etsErr = new ets.Error('fooo', undefined);
	ASSERT_TRUE(etsErr instanceof Error);
	ASSERT_EQ(etsErr.message, 'fooo');
	ASSERT_EQ(IdentityError(etsErr), etsErr);
}

{
	// Classes derived from core/ appear like js built-in objects
	const FooError1 = getTestClass('FooError1');
	const FooError2 = getTestClass('FooError2');

	let err1 = new FooError1();
	ASSERT_TRUE(err1 instanceof FooError1);
	ASSERT_TRUE(err1 instanceof Error);
	ASSERT_EQ(err1.fError1, 'fError1');
	ASSERT_EQ(err1.message, '');
	err1.toString();

	let err2 = new FooError2();
	ASSERT_TRUE(err2 instanceof FooError2);
	ASSERT_TRUE(err2 instanceof FooError1);
	ASSERT_TRUE(err2 instanceof Error);
	err2.toString();
	ASSERT_EQ(IdentityError(err2), err2);
}
