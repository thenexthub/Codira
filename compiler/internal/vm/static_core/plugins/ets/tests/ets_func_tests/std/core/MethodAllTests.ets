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

class StaticClass {
	static foo(): void {}
	bar(): void {}
}

class PropClass {
	private _x: int
	get x(): int { return this._x }
	set x(v: int) { this._x = v }
}

abstract class Writer {
	abstract write(): void
}

class HTMLWriter extends Writer {
	private state_: int
	override write(): void {}
	get state(): int { return this.state_ }
	set state(v: int) { this.state_ = v }
	constructor(s: int) { super(); this.state_ = s }
}

class CtorClass {
	constructor(a: int) {}
}

function findMethod(ct: ClassType, name: string): Method | null {
	for (let i = 0; i < ct.getMethodsNum(); i++) {
		let m = ct.getMethod(i)
		if (m.getName() === name) {
			return m
		}
	}
	return null
}

function isInherited(f: Method, cls: ClassType): boolean {
	return !f.getOwnerType().equals(cls)
}

function testMethodBasics(): void {
	const ct = Type.of(new StaticClass()) as ClassType

	let mFoo = findMethod(ct, 'foo')!
	arktest.assertEQ('foo', mFoo.getName())
	arktest.assertTrue(mFoo.getType() instanceof MethodType)
	arktest.assertTrue(mFoo.isStatic())
	arktest.assertFalse(isInherited(mFoo, ct))
	arktest.assertFalse(mFoo.isAbstract())
	arktest.assertFalse(mFoo.isGetter())
	arktest.assertFalse(mFoo.isSetter())
	arktest.assertEQ(AccessModifier.PUBLIC, mFoo.getAccessModifier())
	arktest.assertEQ(Attributes.STATIC, mFoo.getAttributes() & Attributes.STATIC)

	let mBar = findMethod(ct, 'bar')!
	arktest.assertFalse(mBar.isStatic())
	arktest.assertFalse(mBar.isAbstract())
	arktest.assertFalse(mBar.isGetter())
	arktest.assertFalse(mBar.isSetter())
	arktest.assertEQ(AccessModifier.PUBLIC, mBar.getAccessModifier())
}

function testAbstractFlag(): void {
	const hwt = Type.of(new HTMLWriter(1)) as ClassType
	const wt = hwt.getBaseType()  // Writer
	let m = findMethod(wt, 'write')!
	arktest.assertTrue(m.isAbstract())
	arktest.assertFalse(m.isStatic())
}

function testGetterSetter(): void {
	const ct = Type.of(new PropClass()) as ClassType
	let gm = findMethod(ct, '%%get-x')!
	let sm = findMethod(ct, '%%set-x')!
	arktest.assertTrue(gm.isGetter())
	arktest.assertFalse(gm.isSetter())
	arktest.assertEQ(AccessModifier.PUBLIC, gm.getAccessModifier())
	arktest.assertTrue(sm.isSetter())
	arktest.assertFalse(sm.isGetter())
	arktest.assertEQ(AccessModifier.PUBLIC, sm.getAccessModifier())
}

function testConstructorMethod(): void {
	const ct = Type.of(new CtorClass(0)) as ClassType
	let ctor = ct.getConstructor(0)
	arktest.assertEQ('constructor', ctor.getName())
	arktest.assertTrue(ctor.isConstructor())
	arktest.assertFalse(ctor.isStatic())
	arktest.assertEQ(AccessModifier.PUBLIC, ctor.getAccessModifier())
}

function testIsFinalAlwaysFalse(): void {
	const ct = Type.of(new StaticClass()) as ClassType
	let m = findMethod(ct, 'bar')!
	arktest.assertFalse(m.isFinal())
}

function main(): int {
	const suite = new arktest.ArkTestsuite('Method mirror API tests for SDK')
	suite.addTest('basics', testMethodBasics)
	suite.addTest('abstract flag', testAbstractFlag)
	suite.addTest('getter/setter', testGetterSetter)
	suite.addTest('constructor', testConstructorMethod)
	suite.addTest('isFinal', testIsFinalAlwaysFalse)
	return suite.run()
}
