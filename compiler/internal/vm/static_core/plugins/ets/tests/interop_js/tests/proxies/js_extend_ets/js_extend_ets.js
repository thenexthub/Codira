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

function getTestClass(name) {
	return etsVm.getClass('Ljs_proxy/test/' + name + ';');
}

function getTestFunction(name) {
	return etsVm.getFunction('Ljs_proxy/test/ETSGLOBAL;', name);
}

const FooBase = getTestClass('FooBase');
const FooDerived = getTestClass('FooDerived');

{
	// Verify prototype chain and properties
	class ExtFooBase extends FooBase {}
	class ExtFooDerived extends FooDerived {}
	let ebase = new ExtFooBase();
	let ederived = new ExtFooDerived();

	ASSERT_TRUE(ebase instanceof ExtFooBase);
	ASSERT_TRUE(ebase instanceof FooBase);
	ASSERT_TRUE(ederived instanceof ExtFooDerived);
	ASSERT_TRUE(ederived instanceof FooDerived);
	ASSERT_TRUE(ederived instanceof FooBase);
	ASSERT_TRUE(!(ederived instanceof ExtFooBase));

	ASSERT_EQ(Object.getPrototypeOf(ExtFooDerived), FooDerived);

	ASSERT_EQ(ebase.fn(), 'fn_base');
	ASSERT_EQ(ExtFooBase.sfn(), 'sfn_base');

	ASSERT_EQ(ederived.fn(), 'fn_derived');
	ASSERT_EQ(ExtFooDerived.sfn(), 'sfn_derived');
	ASSERT_EQ(ederived.fnBase(), 'fn_base');
	ASSERT_EQ(ExtFooDerived.sfnBase(), 'sfn_base');
}

{
	// Overriding methods
	class ExtFooDerived extends FooDerived {
		static sfn() {
			return 'sfn_extderived';
		}
		fn() {
			return 'fn_extderived';
		}
	}

	let ederived = new ExtFooDerived();

	ASSERT_EQ(ederived.fn(), 'fn_extderived');
	ASSERT_EQ(ExtFooDerived.sfn(), 'sfn_extderived');

	ASSERT_EQ(ederived.selfFn(), 'fn_extderived');
}

{
	// TODO(vpukhov): overriding accessors
}

{
	// TODO(vpukhov): prevent final methods overriding
	class ExtFooDerived extends FooDerived {
		somethingFinal() {}
	}
}
