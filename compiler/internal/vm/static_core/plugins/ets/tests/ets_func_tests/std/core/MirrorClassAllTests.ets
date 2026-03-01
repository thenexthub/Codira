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

class StaticFieldHolder {
	public static a: int = 5
	public b: int = 10
}

interface Serializable {
	serialize(): String
}

class Point implements Serializable {
	protected x: double
	y: double
	override serialize(): String {
		return '{' + this.x + ',' + this.y + '}'
	}
	add(oth: Point): Point {
		let r = new Point()
		r.x = this.x + oth.x
		r.y = this.y + oth.y
		return r
	}
}

function paramDummy(x: int, y: string): void {}

class MethodHolder {
	public static sm(): void {}
	public f(a: int): string { return '' }
	get g(): int { return 1 }
	set g(v: int) {}
	private p(): void {}
}

enum E { X, Y }

type U = int|string

function getMethodByName(t: ClassType, name: string): Method {
    for (let i = 0; i < t.getMethodsNum(); i++) {
        let m = t.getMethod(i);
        if (m.getName() === name) {
            return m;
        }
    }
    throw new TypeError(`no method "${name}"`);
}

function testFieldBasics(): void {
	const ct = Type.of(new StaticFieldHolder()) as ClassType

	const fa = ct.getFieldByName('a')
	arktest.assertEQ('a', fa.getName())
	arktest.assertTrue(fa.getType().equals(IntType.VAL))
	arktest.assertEQ(AccessModifier.PUBLIC, fa.getAccessModifier())
	arktest.assertTrue((fa.getAttributes() & Attributes.STATIC) !== 0)
	arktest.assertEQ(5 as int, fa.getStaticValue() as int)

	const fb = ct.getFieldByName('b')
	arktest.assertEQ('b', fb.getName())
	arktest.assertTrue(fb.getType().equals(IntType.VAL))
	arktest.assertEQ(AccessModifier.PUBLIC, fb.getAccessModifier())
	arktest.assertFalse((fb.getAttributes() & Attributes.STATIC) !== 0)
}

class Parent { public x: int = 1 }
class Child extends Parent { public y: int = 2 }

function testFieldIsInherited(): void {
	const ct = Type.of(new Child()) as ClassType
	const names = new Array<string>()
	for (let i = 0; i < ct.getFieldsNum(); ++i) {
		names.push(ct.getField(i).getName())
	}
	arktest.assertTrue(names.indexOf('x') !== -1)
	arktest.assertTrue(names.indexOf('y') !== -1)
}

function testParameterGetType(): void {
	const ft = Type.of(paramDummy) as FunctionType
	const p0 = ft.getParameter(0)
	arktest.assertTrue(p0.getType().equals(Type.of(0 as int)))
	const p1 = ft.getParameter(1)
	arktest.assertTrue(p1.getType().equals(Type.of(new String(''))))
}

function isInherited(f: Method, cls: ClassType): boolean {
	return !f.getOwnerType().equals(cls)
}

function testMethodBasics(): void {
	const ct = Type.of(new MethodHolder()) as ClassType
	const got = new Set<string>()
	for (let i = 0; i < ct.getMethodsNum(); ++i) {
		got.add(ct.getMethod(i).getName())
	}
	for (const name of ['sm','f','%%get-g','%%set-g','p']) {
		arktest.assertTrue(got.has(name), `should have method ${name}`)
	}
	const mSm = getMethodByName(ct, 'sm')
	arktest.assertEQ(0 as long, mSm.getType().getParametersNum())
	arktest.assertEQ(AccessModifier.PUBLIC, mSm.getAccessModifier())
	arktest.assertTrue(mSm.isStatic())
	arktest.assertFalse(isInherited(mSm, ct))
	arktest.assertFalse(mSm.isGetter())
	arktest.assertFalse(mSm.isSetter())

	const mF = getMethodByName(ct, 'f')
	arktest.assertEQ(1 as long, mF.getType().getParametersNum())
	arktest.assertTrue(mF.getType().getResultType().equals(StringType.REF))
	arktest.assertEQ(AccessModifier.PUBLIC, mF.getAccessModifier())

	const mGet = getMethodByName(ct, '%%get-g')
	arktest.assertTrue(mGet.isGetter())
	arktest.assertEQ(0 as long, mGet.getType().getParametersNum())
	arktest.assertTrue(mGet.getType().getResultType().equals(IntType.VAL))

	const mSet = getMethodByName(ct, '%%set-g')
	arktest.assertTrue(mSet.isSetter())
	arktest.assertEQ(1 as long, mSet.getType().getParametersNum())
}

function main(): int {
	const suite = new arktest.ArkTestsuite('Mirror classes API tests for SDK')
	suite.addTest('Field.getName/Type/Owner/Attrs/Access/Static', testFieldBasics)
	suite.addTest('Field.isInherited', testFieldIsInherited)
	suite.addTest('Parameter.getType', testParameterGetType)
	suite.addTest('Method.getName/Type/Attrs/Access/Static/Getter/Setter', testMethodBasics)
	return suite.run()
}
