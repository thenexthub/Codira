/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

interface I1 { foo(): void; }
interface I2 { bar(): void; }

class BaseTypeA {}
class BaseTypeB extends BaseTypeA {}
class BaseTypeC extends BaseTypeB {}

class E implements I1, I2 { foo(): void {} bar(): void {} }
class F extends E {}

class FieldBase {
    public x: int;
    private y: int;
    constructor() { this.x = 1; this.y = 2; }
}
class FieldSub extends FieldBase {
    public z: int;
    private w: int;
    constructor() { super(); this.z = 3; this.w = 4; }
}

class MethodBase { public m1(): void {} }
class MethodSub extends MethodBase { public m2(): void {} }

class NamedMethods { public foo(): void {} private bar(): void {} }
class ExtendNamedMethods extends NamedMethods { public baz(): void {} }

class ConstrA {}
class ConstrB { public x: int; constructor(x: int) { this.x = x; } }
class ConstrC { public x: int; constructor(x?: int) { this.x = x === undefined ? 0 : x; } }

class NonFinalClass {}
final class FinalClass {}

class MakeA { public value: int; constructor() { this.value = 42; } }
class MakeB { public x: int; public y: string; constructor(x: int, y: string) { this.x = x; this.y = y; } }

class C1 {}

class MethodsA {}
class MethodsB { public m1(): void {} }
class MethodsC extends MethodsB { public m2(): void {} }
class MethodsD extends MethodsC {}
class AMethod { public foo(): void {} private bar(): void {} }

class Foo {}


class A {
    public foo(): void {}
    private bar(): void {}
}
class B extends A {
    public baz(): void {}
}

class Base {
    a: int;
    b: int;
    constructor() { this.a = 1; this.b = 2; }
}

class Sub extends Base {
    c: int;
    d: int;
    constructor() { super(); this.c = 3; this.d = 4; }
}

function getMethodByName(t: ClassType, name: string): Method {
    for (let i = 0; i < t.getMethodsNum(); i++) {
        let m = t.getMethod(i);
        if (m.getName() === name) {
            return m;
        }
    }
    throw new TypeError(`no method "${name}"`);
}

function hasMethod(t: ClassType, name: string): boolean {
    for (let i = 0; i < t.getMethodsNum(); i++) {
        if (t.getMethod(i).getName() === name) {
            return true;
        }
    }
    return false;
}

function testGetInterfaceOutOfRange() {
    const tE = Type.of(new E()) as ClassType;
    const n = tE.getInterfacesNum();
    arktest.expectThrow(
    () => { tE.getInterface(n); },
    (e: Error): boolean => {
        return e instanceof TypeError && `${e}` === `TypeError: no interface at ${n}`;
    }
);
}

function testGetFieldOutOfRange() {
    const tSub = Type.of(new FieldSub()) as ClassType;
    const n = tSub.getFieldsNum();
    arktest.expectThrow(
        () => { tSub.getField(n); },
        (e: Error): boolean => {
            return e instanceof TypeError && `${e}` === `TypeError: no field at ${n}`;
        }
    );
}

function testGetOwnFieldOutOfRange() {
    const tSub = Type.of(new FieldSub()) as ClassType;
    const n = tSub.getOwnFieldsNum();
    arktest.expectThrow(
        () => { tSub.getOwnField(n); },
        (e: Error): boolean => {
            return e instanceof TypeError && `${e}` === `TypeError: no own field at ${n}`;
        }
    );
}

function testGetConstructorOutOfRange() {
    const tA = Type.of(new ConstrA()) as ClassType;
    const n = tA.getConstructorsNum();
    arktest.expectThrow(
        () => { tA.getConstructor(n); },
        (e: Error): boolean => {
            return e instanceof TypeError && `${e}` === `TypeError: no constructor at ${n}`;
        }
    );
}

function testGetMethodOutOfRange() {
    const tBase = Type.of(new MethodBase()) as ClassType;
    const n = tBase.getMethodsNum();
    arktest.expectThrow(
        () => { tBase.getMethod(n as long); },
        (e: Error): boolean => {
            return e instanceof TypeError && `${e}` === `TypeError: no method at ${n}`;
        }
    );
}

function testNameAndHasName() {
    const objType = Type.of({} as Object) as ClassType;
    arktest.assertEQ('std.core.Object', objType.getName());
    arktest.assertEQ(true, objType.hasName());
    const aType = Type.of(new BaseTypeA()) as ClassType;
    arktest.assertEQ('ClassTypeAllTests.BaseTypeA', aType.getName());
    arktest.assertEQ(true, aType.hasName());
}

function testGetBaseType() {
    const tA = Type.of(new BaseTypeA()) as ClassType;
    arktest.assertEQ('std.core.Object', tA.getBaseType().getName());
    const tB = Type.of(new BaseTypeB()) as ClassType;
    arktest.assertEQ('ClassTypeAllTests.BaseTypeA', tB.getBaseType().getName());
    const tC = Type.of(new BaseTypeC()) as ClassType;
    arktest.assertEQ('ClassTypeAllTests.BaseTypeB', tC.getBaseType().getName());
}

function testObjectBaseType() {
    const o = {} as Object;
    const tO = Type.of(o) as ClassType;
    arktest.assertEQ(
        tO.getName(),
        tO.getBaseType().getName(),
        `Expected ${tO.getName()}, got ${tO.getBaseType().getName()}`
    );
}

function testInterfaces() {
    const tE = Type.of(new E()) as ClassType;
    arktest.assertEQ(2 as long, tE.getInterfacesNum());
    arktest.assertEQ('ClassTypeAllTests.I1', tE.getInterface(0).getName());
    arktest.assertEQ('ClassTypeAllTests.I2', tE.getInterface(1).getName());
    const tF = Type.of(new F()) as ClassType;
    arktest.assertEQ(0 as long, tF.getInterfacesNum());
}

function testFieldsNum() {
    const tBase = Type.of(new FieldBase()) as ClassType;
    arktest.assertEQ(2 as long, tBase.getFieldsNum());
    arktest.assertEQ(2 as long, tBase.getOwnFieldsNum());
    const tSub = Type.of(new FieldSub()) as ClassType;
    arktest.assertEQ(4 as long, tSub.getFieldsNum());
    arktest.assertEQ(2 as long, tSub.getOwnFieldsNum());
}

function testFieldAccess() {
    const tSub = Type.of(new FieldSub()) as ClassType;
    arktest.assertEQ('x', tSub.getField(0).getName());
    arktest.assertEQ('w', tSub.getField(3).getName());
    arktest.assertEQ('w', tSub.getOwnField(1).getName());
    arktest.assertEQ('z', tSub.getFieldByName('z').getName());
}

function testHasField() {
    const tSub = Type.of(new FieldSub()) as ClassType;
    arktest.assertEQ(true, tSub.hasField('x'));
    arktest.assertEQ(false, tSub.hasField('non'));
}

function testMethodsNumAndGetMethod() {
    const tBase = Type.of(new MethodBase()) as ClassType;
    const numBase = tBase.getMethodsNum();
    let foundM1 = false;
    for (let i = 0; i < numBase; i++) {
        if (tBase.getMethod(i).getName() == 'm1') {
            foundM1 = true;
            break;
        }
    }
    arktest.assertEQ(true, foundM1, 'MethodBase should contain method m1');

    const tSub = Type.of(new MethodSub()) as ClassType;
    const numSub = tSub.getMethodsNum();
    arktest.assertEQ(numBase + 1 as long, numSub, 'MethodSub should have one additional method');
    let foundM2 = false;
    for (let i = 0; i < numSub; i++) {
        if (tSub.getMethod(i).getName() == 'm2') {
            foundM2 = true;
            break;
        }
    }
    arktest.assertEQ(true, foundM2, 'MethodSub should contain method m2');
}

function testMethodByNameAndHasMethod() {
    const t = Type.of(new ExtendNamedMethods()) as ClassType;
    arktest.assertEQ(true, hasMethod(t, 'foo'));
    arktest.assertEQ(true, hasMethod(t, 'baz'));
    arktest.assertEQ('bar', getMethodByName(t, 'bar').getName());
    arktest.assertEQ(false, hasMethod(t, 'none'));
}

function testConstructors() {
    const tA = Type.of(new ConstrA()) as ClassType;
    arktest.assertEQ(1 as long, tA.getConstructorsNum());
    arktest.assertEQ(true, tA.hasEmptyConstructor());
    const tB = Type.of(new ConstrB(5)) as ClassType;
    arktest.assertEQ(1 as long, tB.getConstructorsNum());
    arktest.assertEQ(false, tB.hasEmptyConstructor());
    const tC = Type.of(new ConstrC()) as ClassType;
    arktest.assertEQ(2 as long, tC.getConstructorsNum());
    arktest.assertEQ(true, tC.hasEmptyConstructor());
    arktest.assertEQ(1 as long, tC.getConstructor(0).getType().getParametersNum());
}

function testIsFinal() {
    const tNF = Type.of(new NonFinalClass()) as ClassType;
    arktest.assertEQ(false, tNF.isFinal());
    const tF = Type.of(new FinalClass()) as ClassType;
    arktest.assertEQ(true, tF.isFinal());
}

function testMake() {
    const tA = Type.of(new MakeA()) as ClassType;
    arktest.assertEQ(42 as int, (tA.make() as MakeA).value);
    const tB = Type.of(new MakeB(0, "")) as ClassType;
    const args = new FixedArray<Object>(2) as FixedArray<Any>;
    args[0] = new Int(7);
    args[1] = new String("hi");
    const b = tB.make(args) as MakeB;
    arktest.assertEQ(7 as int, b.x);
    arktest.assertEQ("hi", b.y);
}

function testGetConstructorParamCounts() {
    const tA = Type.of(new ConstrA()) as ClassType;
    arktest.assertEQ(
        0 as long,
        tA.getConstructor(0).getType().getParametersNum(),
        'Constructor of A should have 0 parameters'
    );
    const tB = Type.of(new ConstrB(5)) as ClassType;
    arktest.assertEQ(
        1 as long,
        tB.getConstructor(0).getType().getParametersNum(),
        'Constructor of B should have 1 parameter'
    );
}

function testGetFieldByNameValid() {
    const tB = Type.of(new FieldSub()) as ClassType;
    const fx = tB.getFieldByName('x');
    const fy = tB.getFieldByName('y');
    arktest.assertEQ('x', fx.getName());
    arktest.assertEQ('y', fy.getName());
}

function testGetFieldByNameOwn() {
    const tB = Type.of(new FieldSub()) as ClassType;
    const fz = tB.getFieldByName('z');
    arktest.assertEQ('z', fz.getName());
    const ownFieldNames = [
        tB.getOwnField(0).getName(),
        tB.getOwnField(1).getName(),
    ];

    arktest.assertEQ(
        true,
        ownFieldNames.includes(fz.getName()),
        'Field "z" should be among own fields of FieldSub'
    );
}

function testGetFieldByNameInvalid() {
    const tA = Type.of(new FieldBase()) as ClassType;
    let msg = '';
    try { tA.getFieldByName('nonexistent'); } catch (e) { msg = `${e}`; }
    arktest.assertEQ('TypeError: no field \"nonexistent\"', msg);
}

function testNoFieldsCount() {
    const t = Type.of(new C1()) as ClassType;
    arktest.assertEQ(0 as long, t.getFieldsNum(), 'Expected 0 total fields for C1');
    arktest.assertEQ(0 as long, t.getOwnFieldsNum(), 'Expected 0 own fields for C1');
}

function testGetFieldValidIndex() {
    const tFB = Type.of(new FieldBase()) as ClassType;
    const f0 = tFB.getField(0);
    const f1 = tFB.getField(1);
    arktest.assertEQ('x', f0.getName());
    arktest.assertEQ('y', f1.getName());
}

function testGetOwnFieldOnB() {
    const tSub = Type.of(new FieldSub()) as ClassType;
    arktest.assertEQ(2 as long, tSub.getOwnFieldsNum());
    const own0 = tSub.getOwnField(0);
    arktest.assertEQ('z', own0.getName());
}

function testNoInterfaces() {
    const tA = Type.of(new BaseTypeA()) as ClassType;
    arktest.assertEQ(
        0 as long,
        tA.getInterfacesNum(),
        'Class with no implements should have 0 interfaces'
    );
}

function testGetExistingMethodByName() {
    const t = Type.of(new NamedMethods()) as ClassType;
    const mFoo = getMethodByName(t, 'foo');
    const mBar = getMethodByName(t, 'bar');
    arktest.assertEQ('foo', mFoo.getName());
    arktest.assertEQ('bar', mBar.getName());
}

function testGetMissingMethodByName() {
    const t = Type.of(new NamedMethods()) as ClassType;
    let msg = '';
    try { getMethodByName(t, 'baz'); } catch (e) { msg = `${e}`; }
    arktest.assertEQ('TypeError: no method "baz"', msg);
}

function testNoOwnMethods() {
    const tA = Type.of(new MethodsA()) as ClassType;
    const numA = tA.getMethodsNum();
    const numObj = Type.of({} as Object) as ClassType;
    arktest.assertEQ(
        numObj.getMethodsNum(),
        numA,
        'MethodsA should have same methodsNum as Object'
    );
}

function testSingleMethodCount() {
    const tB = Type.of(new MethodsB()) as ClassType;
    const numObj = Type.of({} as Object) as ClassType;
    arktest.assertEQ(
        numObj.getMethodsNum() + 1 as long,
        tB.getMethodsNum(),
        'MethodsB should have Object methods count + 1'
    );
}

function testInheritanceMethodsCount() {
    const tC = Type.of(new MethodsC()) as ClassType;
    const tB = Type.of(new MethodsB()) as ClassType;
    arktest.assertEQ(
        tB.getMethodsNum() + 1 as long,
        tC.getMethodsNum(),
        'MethodsC should have MethodsB methods count + 1'
    );
}

function testSubclassNoNewMethods() {
    const tD = Type.of(new MethodsD()) as ClassType;
    const tC = Type.of(new MethodsC()) as ClassType;
    arktest.assertEQ(
        tC.getMethodsNum(),
        tD.getMethodsNum(),
        'MethodsD extends MethodsC without new methods, should have same methodsNum as MethodsC'
    );
}

function testGetMethodValidIndex() {
    const tA = Type.of(new AMethod()) as ClassType;
    const total = tA.getMethodsNum();
    const m = tA.getMethod(0);
    arktest.assertEQ(
        true,
        m.getName().length > 0,
        `Expected non-empty method name, got "${m.getName()}"`
    );
    const mLast = tA.getMethod(total - 1 as long);
    arktest.assertEQ(
        true,
        mLast.getName().length > 0,
        `Expected non-empty method name at last index, got "${mLast.getName()}"`
    );
}

function testCustomClassGetName() {
    const foo = new Foo();
    const t = Type.of(foo) as ClassType;
    arktest.assertEQ('ClassTypeAllTests.Foo', t.getName());
}

function testCustomClassHasName() {
    const foo = new Foo();
    const t = Type.of(foo) as ClassType;
    arktest.assertEQ(true, t.hasName());
}

function testNoOwnFieldsOnObject() {
    const tO = Type.of({} as Object) as ClassType;
    arktest.assertEQ(
        0 as long,
        tO.getOwnFieldsNum(),
        'Object should have 0 own fields'
    );
}

function testGetOwnFieldValidBase() {
    const tBase = Type.of(new FieldBase()) as ClassType;
    const f0 = tBase.getOwnField(0);
    const f1 = tBase.getOwnField(1);
    arktest.assertEQ('x', f0.getName());
    arktest.assertEQ('y', f1.getName());
}

function testGetOwnFieldValidSub() {
    const tSub = Type.of(new FieldSub()) as ClassType;
    const f0 = tSub.getOwnField(0);
    const f1 = tSub.getOwnField(1);
    arktest.assertEQ('z', f0.getName());
    arktest.assertEQ('w', f1.getName());
}

function testGetOwnFieldValidBaseClass() {
    const tBase = Type.of(new Base()) as ClassType;
    arktest.assertEQ('a', tBase.getOwnField(0).getName());
    arktest.assertEQ('b', tBase.getOwnField(1).getName());
}

function testGetOwnFieldValidSubClass() {
    const tSub = Type.of(new Sub()) as ClassType;
    arktest.assertEQ('c', tSub.getOwnField(0).getName());
    arktest.assertEQ('d', tSub.getOwnField(1).getName());
}

function testGetOwnFieldOutOfRangeClass() {
    const tBase = Type.of(new Base()) as ClassType;
    let msg = '';
    try {
        tBase.getOwnField(2);
    } catch (e) {
        msg = `${e}`;
    }
    arktest.assertEQ('TypeError: no own field at 2', msg);
}

function testHasOwnMethods() {
    const t = Type.of(new A() as Object) as ClassType;
    arktest.assertEQ(true, hasMethod(t, 'foo'), 'A should have method foo');
    arktest.assertEQ(true, hasMethod(t, 'bar'), 'A should have method bar');
}

function testMissingMethod() {
    const t = Type.of(new A() as Object) as ClassType;
    arktest.assertEQ(false, hasMethod(t, 'noSuch'), 'A should not have method noSuch');
}

function testInheritedMethods() {
    const t = Type.of(new B() as Object) as ClassType;
    arktest.assertEQ(true, hasMethod(t, 'foo'), 'B should inherit method foo');
    arktest.assertEQ(true, hasMethod(t, 'bar'), 'B should inherit private bar');
    arktest.assertEQ(true, hasMethod(t, 'baz'), 'B should have own method baz');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('ClassType API tests for SDK');
    suite.addTest('getName and hasName', testNameAndHasName);
    suite.addTest('getBaseType', testGetBaseType);
    suite.addTest('getInterfacesNum and getInterface', testInterfaces);
    suite.addTest('getFieldsNum and getOwnFieldsNum', testFieldsNum);
    suite.addTest('getField, getFieldByName, getOwnField', testFieldAccess);
    suite.addTest('hasField', testHasField);
    suite.addTest('getMethodsNum and getMethod', testMethodsNumAndGetMethod);
    suite.addTest('getMethodByName and hasMethod', testMethodByNameAndHasMethod);
    suite.addTest('getConstructorsNum, getConstructor and hasEmptyConstructor', testConstructors);
    suite.addTest('isFinal', testIsFinal);
    suite.addTest('make', testMake);
    suite.addTest('getInterface out-of-range', testGetInterfaceOutOfRange);
    suite.addTest('getField out-of-range', testGetFieldOutOfRange);
    suite.addTest('getOwnField out-of-range', testGetOwnFieldOutOfRange);
    suite.addTest('getConstructor out-of-range', testGetConstructorOutOfRange);
    suite.addTest('getMethod out-of-range', testGetMethodOutOfRange);
    suite.addTest('getBaseType for Object class', testObjectBaseType);
    suite.addTest('Constructor param count', testGetConstructorParamCounts);
    suite.addTest('getFieldByName on base and inherited fields', testGetFieldByNameValid);
    suite.addTest('getFieldByName on own field', testGetFieldByNameOwn);
    suite.addTest('getFieldByName invalid name throws', testGetFieldByNameInvalid);
    suite.addTest('No fields', testNoFieldsCount);
    suite.addTest('Valid getField indices', testGetFieldValidIndex);
    suite.addTest('Own fields on FieldSub', testGetOwnFieldOnB);
    suite.addTest('No interfaces', testNoInterfaces);
    suite.addTest('Get existing method by name', testGetExistingMethodByName);
    suite.addTest('Throws on missing method', testGetMissingMethodByName);
    suite.addTest('No own methods', testNoOwnMethods);
    suite.addTest('One method in B', testSingleMethodCount);
    suite.addTest('Inheritance adds methods', testInheritanceMethodsCount);
    suite.addTest('Subclass without new methods same count', testSubclassNoNewMethods);
    suite.addTest('getMethod valid index', testGetMethodValidIndex);
    suite.addTest('getMethod out-of-range', testGetMethodOutOfRange);
    suite.addTest('Custom class getName', testCustomClassGetName);
    suite.addTest('Custom class hasName', testCustomClassHasName);
    suite.addTest('No own fields on Object', testNoOwnFieldsOnObject);
    suite.addTest('getOwnField valid indices on Base', testGetOwnFieldValidBase);
    suite.addTest('getOwnField valid indices on Subclass', testGetOwnFieldValidSub);
    suite.addTest('getOwnField valid indices on Base class', testGetOwnFieldValidBaseClass);
    suite.addTest('getOwnField valid indices on Subclass class', testGetOwnFieldValidSubClass);
    suite.addTest('getOwnField out-of-range for Base class', testGetOwnFieldOutOfRangeClass);
    suite.addTest('hasMethod: own methods', testHasOwnMethods);
    suite.addTest('hasMethod: missing method', testMissingMethod);
    suite.addTest('hasMethod: inherited and own methods', testInheritedMethods);

    return suite.run();
}
