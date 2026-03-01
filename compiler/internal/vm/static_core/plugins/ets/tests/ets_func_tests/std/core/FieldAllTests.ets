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

class SF {
    public static foo: int = 5
    public static readonly bar: int = 10
}

class BaseFld {
    public a: int = 3
    protected c: int = 7
}

class SubFld extends BaseFld {
    public d: int = 9
}

function isInherited(f: Field, cls: ClassType): boolean {
	return !f.getOwnerType().equals(cls)
}

function testStaticFieldBasics(): void {
    const ct = Type.of(new SF()) as ClassType
    const f = ct.getFieldByName(new String('foo')) as Field

    arktest.assertTrue(f.getType().equals(IntType.VAL))
    arktest.assertTrue(f.getOwnerType().equals(ct))
    arktest.assertEQ('foo', f.getName())
    arktest.assertTrue((f.getAttributes() & Attributes.STATIC) != 0)
    arktest.assertEQ(AccessModifier.PUBLIC, f.getAccessModifier())
    const val = f.getStaticValue() as Int
    arktest.assertEQ(5 as int, val as int)
    arktest.assertFalse(isInherited(f, ct))
    arktest.assertFalse(f.isReadonly())
}

function testBarStaticField(): void {
    const ct = Type.of(new SF()) as ClassType
    const f = ct.getFieldByName(new String('bar')) as Field

    arktest.assertTrue((f.getAttributes() & Attributes.STATIC) != 0)
    arktest.assertEQ(AccessModifier.PUBLIC, f.getAccessModifier())
    arktest.assertTrue((f.getAttributes() & Attributes.READONLY) != 0)
    arktest.assertTrue(f.isReadonly())
}

function testInheritedFields(): void {
    const ctSub = Type.of(new SubFld()) as ClassType
    const baseCt = Type.of(new BaseFld()) as ClassType

    const fa = ctSub.getFieldByName(new String('a')) as Field
    arktest.assertTrue(fa.getOwnerType().equals(baseCt))
    arktest.assertTrue(isInherited(fa, ctSub))
    arktest.assertEQ(AccessModifier.PUBLIC, fa.getAccessModifier())

    const fc = ctSub.getFieldByName(new String('c')) as Field
    arktest.assertTrue(fc.getOwnerType().equals(baseCt))
    arktest.assertTrue(isInherited(fc, ctSub))
    arktest.assertEQ(AccessModifier.PROTECTED, fc.getAccessModifier())

    const fd = ctSub.getFieldByName(new String('d')) as Field
    arktest.assertTrue(fd.getOwnerType().equals(ctSub))
    arktest.assertFalse(isInherited(fd, ctSub))
    arktest.assertEQ(AccessModifier.PUBLIC, fd.getAccessModifier())
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Field mirror API tests for SDK')
    suite.addTest('static field basics', testStaticFieldBasics)
    suite.addTest('bar static field flags', testBarStaticField)
    suite.addTest('inherited fields', testInheritedFields)
    return suite.run()
}
