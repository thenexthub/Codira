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

interface BaseIface {
    foo()
}

interface DerivedIface extends BaseIface {
    boo()
    defaultMethod(): int {
        return 0
    }
}

interface DerivedDerivedIface extends DerivedIface {
    foo()
    goo()
}

interface Iface {
    someDefaultMethod(): string {
        return "111"
    }
}

class A implements Iface {
    public static stfooFld1: Number = 1
    public static stfooFld2: Number = 1

    public fooFld1: Number = 1
    public fooFld2: Number = 1

    public foo1(): void {}
    public foo2(): void {}

    public static stfoo1(): void {}
    public static stfoo2(): void {}
    public static stfoo3(): void {}
    public static stfoo3(n1: int, n2: long): void {}

    private prfoo1(): void {}
    private prfoo2(): void {}
}

class B extends A {
    public static stbooFld1: Number = 2
    public static stbooFld2: Number = 2

    public booFld1: Number = 2
    public booFld2: Number = 2

    public boo1(): string {
        return ""
    }

    public boo2(): string {
        return "222"
    }

    public static stboo1(): string {
        return ""
    }

    public static stboo2(n: int): string {
        return "222"
    }
}

class CtorsTest {
    foo: Number
    bla: String
    static numOfCtors: Int = 2

    constructor(x: Number) {
        this.foo = x;
        this.bla = "bla"
    }

    constructor(x: String) {
        this.bla = x;
        this.foo = 2;
    }
}

type testUnion = A | B | Number | String

class C<T> {
    fld: testUnion = new A()
    fld1: String | FixedArray<A | FixedArray<Number | String >> = new String("help") // union
    fld2: A | undefined // not union
    fld3: int | long = 10 // union
    fld4: String | String = 'hello' // not union
    fld5: String | Number = 'world' // union
    fld6: 'lit1' | 'lit2' | 'lit3' = 'lit2' // not union
    fld7: T | undefined // not union
    fld8: T | null // not union
    fld9: FixedArray<String> | FixedArray<Number> = [ 1.0, 2.0, 3.0 ] // union
    fld10: A | null = null // union
    fld11: T | A // not union
    fld12: String | 'table' = 'desk' // not union
    fld13: Array<String> | Array <Int> = [ 'a' , 'b' ] // not union
    fld14: null | undefined // not union
    fld15: A | B = new A() // not union

    foo(a: testUnion): testUnion { return a }
    bar(a: FixedArray<int | string>): FixedArray<int | string> { return a }
}

final class D extends B {
    public static stboo3(): string {
        return "333"
    }

    public static stboo3(b: B): string {
        return "bbb"
    }

    private static stbar1(): string {
        return "priv"
    }

    protected static stbar2(): string {
        return "protec"
    }
}

final class P {
    fldBoolean: boolean = false
    fldByte: byte = 100
    fldShort: short = 1000
    fldChar: char = c'x'
    fldInt: int = 100000
    fldLong: long = 3000000
    fldFloat: float = 5.0
    fldDouble: double = 123456789.99
    foo() {}
}

function testGetName(type: Class, expected: string) {
    arktest.assertEQ(type.getName(), expected)
}

function testGetInstanceMethods(classType: Class, expectedMethodsArrB: string[]) {
    let expectedMtdsSetB: Set<string> = new Set<string>(expectedMethodsArrB)

    let methods: FixedArray<reflect.InstanceMethod> = classType.getInstanceMethods()
    arktest.assertEQ(methods.length, expectedMethodsArrB.length)

    for (let i = 0; i < methods.length; ++i) {
        arktest.assertTrue(expectedMtdsSetB.has(methods[i].getName()), `failed on method: ${methods[i].getName()}`)
    }
}

function testGetInstanceMethodByName(classType: Class, methodsToFind: string[]) {
    for (let i = 0; i < methodsToFind.length; ++i) {
        let obtainedMethod = classType.getInstanceMethod(methodsToFind[i])

        if (obtainedMethod == undefined) {
            throw new Error(`Can't find given method '${methodsToFind[i]}' in testgetInstanceMethod`)
        }

        arktest.assertEQ(obtainedMethod.getName(), methodsToFind[i])
    }
}

function tryGetPrivateInstanceMethodByName(classType: Class, methods: string[]) {
    for (let i = 0; i < methods.length; ++i) {
        let obtainedMethod = classType.getInstanceMethod(methods[i])
        if (obtainedMethod != undefined) {
            throw new Error(`Given method '${methods[i]}' in tryGetPrivateInstanceMethodByName should be undefined`)
        }
    }
}

function testGetInterfaces(intfs: FixedArray<Class>, expectedIntfs: string[]) {
    arktest.assertEQ(intfs.length, expectedIntfs.length)

    let expectedIntfsSet = new Set<string>(expectedIntfs)

    for (let i = 0; i < intfs.length; ++i) {
        arktest.assertTrue(expectedIntfsSet.has(intfs[i].getName()), `failed on interface: ${intfs[i].getName()}`)
    }
}

function testGetConstructors() {
    let ctorTestType = Class.of(new CtorsTest(1))
    let ctors: FixedArray<reflect.Constructor> = ctorTestType.getConstructors()
    arktest.assertEQ(ctors.length, CtorsTest.numOfCtors)

    for (let i = 0; i < ctors.length; ++i) {
        arktest.assertEQ(ctors[i].getName(), "constructor")
    }
}

function testGetStaticMethods() {
    let expectedMethodsArrA = ([ "stfoo1", "stfoo2", "stfoo3", "stfoo3" ])
    let expectedMethodsArrB = ([ "stboo1", "stboo2" ])
    let expectedMethodsArrD = ([ "stboo3", "stboo3" ])

    let expectedMtdsSetA: Set<string> = new Set<string>(expectedMethodsArrA)
    let expectedMtdsSetB: Set<string> = new Set<string>(expectedMethodsArrB)
    let expectedMtdsSetD: Set<string> = new Set<string>(expectedMethodsArrD)

    let infoA = Class.of(new A())
    let methodsA: FixedArray<reflect.StaticMethod> = infoA.getStaticMethods()
    arktest.assertEQ(methodsA.length, expectedMethodsArrA.length)
    for (let i = 0; i < methodsA.length; ++i) {
        arktest.assertTrue(expectedMtdsSetA.has(methodsA[i].getName()))
    }

    let infoB = Class.of(new B())
    let methodsB: FixedArray<reflect.StaticMethod> = infoB.getStaticMethods()
    arktest.assertEQ(methodsB.length, expectedMethodsArrB.length)

    let infoD = Class.of(new D())
    let methodsD: FixedArray<reflect.StaticMethod> = infoD.getStaticMethods()
    arktest.assertEQ(methodsD.length, expectedMethodsArrD.length)
    for (let i = 0; i < methodsD.length; ++i) {
        arktest.assertTrue(expectedMtdsSetD.has(methodsD[i].getName()))
    }
}

function testGetStaticMethod() {
    let infoA = Class.of(new A())
    let methodA1 = infoA.getStaticMethod("getOwnPropertyNames");
    arktest.assertEQ(methodA1, undefined) // look up only direct methods
    let methodA2 = infoA.getStaticMethod("stfoo1");
    arktest.assertNE(methodA2, undefined)
    let methodA3 = infoA.getStaticMethod("stfoo3", []);
    arktest.assertNE(methodA3, undefined)
    let methodA4 = infoA.getStaticMethod("stfoo3", [ Class.PRIMITIVE_INT, Class.PRIMITIVE_LONG ]);
    arktest.assertNE(methodA4, undefined)
    let paramsMethodA4 = methodA4!.getParametersTypes()
    let methodA5 = infoA.getStaticMethod("stfoo3", paramsMethodA4);
    arktest.assertNE(methodA5, undefined)
    let methodA6 = infoA.getStaticMethod("stboo1")
    arktest.assertEQ(methodA6, undefined) // not exist in A

    let infoB = Class.of(new B())
    let methodB1 = infoB.getStaticMethod("stfoo2");
    arktest.assertEQ(methodB1, undefined) // look up only direct methods
    let methodB2 = infoB.getStaticMethod("stboo2", [ Class.PRIMITIVE_INT ]);
    arktest.assertNE(methodB2, undefined)
    let methodB3 = infoB.getStaticMethod("stboo1");
    arktest.assertNE(methodB3, undefined)
    arktest.assertEQ(methodB3!.getOwner(), infoB) // owner is B

    let infoD = Class.of(new D())
    let methodD1 = infoD.getStaticMethod("stbar1");
    arktest.assertEQ(methodD1, undefined) // private in D
    let methodD2 = infoD.getStaticMethod("stbar2");
    arktest.assertEQ(methodD2, undefined) // protected in D

    const expectedErrors = (e: Error): boolean => {
        return e.message == "Found two methods with the same name 'stboo3'. Please specify signature."
    }
    arktest.expectThrow(()=>{infoD.getStaticMethod("stboo3")}, expectedErrors)

    let methodD10 = infoD.getStaticMethod("stboo3", [ infoB ]);
    arktest.assertNE(methodD10, undefined)
    arktest.assertEQ(methodD10!.getOwner(), infoD) // owner is D
    let methodD11 = infoD.getStaticMethod("stboo3", [ infoA ]);
    arktest.assertEQ(methodD11, undefined) // A is not accepted as B
}

function testGetStaticFields() {
    let info = Class.from<B>()
    let expectedFldSet: Set<string> = new Set<string>([ "stbooFld1", "stbooFld2"]);

    let fields: FixedArray<reflect.StaticField> = info.getStaticFields()
    arktest.assertEQ(fields.length, expectedFldSet.size)
    for (let i = 0; i < fields.length; ++i) {
        arktest.assertTrue(expectedFldSet.has(fields[i].getName()))
    }
}

function testGetInstanceFields() {
    let info = Class.from<B>()
    let expectedFldSet: Set<string> = new Set<string>([ "booFld1", "booFld2" ]);

    let fields: FixedArray<reflect.InstanceField> = info.getInstanceFields()
    arktest.assertEQ(fields.length, expectedFldSet.size)
    for (let i = 0; i < fields.length; ++i) {
        arktest.assertTrue(expectedFldSet.has(fields[i].getName()))
    }
}

function testInstanceFieldEquals(): void {
    const aCls = Class.of(new A())

    const fld1 = aCls.getInstanceField("fooFld1")!
    const fld2 = aCls.getInstanceField("fooFld1")!

    arktest.assertFalse(fld1 == fld2)
    arktest.assertTrue(fld1.equals(fld2))

    const fld3 = aCls.getInstanceField("fooFld2")!

    arktest.assertFalse(fld1 == fld3)
    arktest.assertFalse(fld1.equals(fld3))
}

function classGetStaticFieldByName(cls: Class, name: string): reflect.StaticField | undefined {
    const fields = cls.getStaticFields()
    for (let i = 0; i < fields.length; i++) {
        const field = fields[i]
        if (field.getName() == name) {
            return field
        }

    }

    return undefined
}

function testStaticFieldEquals(): void {
    const aCls = Class.of(new A())

    const fld1 = classGetStaticFieldByName(aCls, "stfooFld1")!
    const fld2 = classGetStaticFieldByName(aCls, "stfooFld1")!

    arktest.assertFalse(fld1 == fld2)
    arktest.assertTrue(fld1.equals(fld2))

    const fld3 = classGetStaticFieldByName(aCls, "stfooFld2")!

    arktest.assertFalse(fld1 == fld3)
    arktest.assertFalse(fld1.equals(fld3))
}

function classGetStaticMethod(cls: Class, name: string): reflect.StaticMethod | undefined {
    const methods = cls.getStaticMethods()
    for (let i = 0; i < methods.length; i++) {
        const method = methods[i]
        if (method.getName() == name) {
            return method
        }
    }

    return undefined
}

function testStaticMethodEquals(): void {
    const aCls = Class.of(new A())

    const meth1 = classGetStaticMethod(aCls, 'stfoo1')!
    const meth2 = classGetStaticMethod(aCls, 'stfoo1')!

    arktest.assertFalse(meth1 == meth2)
    arktest.assertTrue(meth1.equals(meth2))

    const meth = classGetStaticMethod(aCls, 'stfoo2')!

    arktest.assertFalse(meth1 == meth)
    arktest.assertFalse(meth1.equals(meth))
}

function testIsUnion() {
    let clsC = new C<B>()
    let classType = Class.of(clsC)

    let fldInfo1 = classType.getInstanceField('fld1')
    let uniInfo1 = fldInfo1!.getType()
    let constituentTypes1 = uniInfo1!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo1!.isUnion())
    arktest.assertNE(constituentTypes1, undefined)

    let fldInfo2 = classType.getInstanceField('fld2')
    let uniInfo2 = fldInfo2!.getType()
    let constituentTypes2 = uniInfo2!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo2!.isUnion())
    arktest.assertEQ(constituentTypes2, undefined)

    let fldInfo3 = classType.getInstanceField('fld3')
    let uniInfo3 = fldInfo3!.getType()
    let constituentTypes3 = uniInfo3!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo3!.isUnion())
    arktest.assertNE(constituentTypes3, undefined)

    let fldInfo4 = classType.getInstanceField('fld4')
    let uniInfo4 = fldInfo4!.getType()
    let constituentTypes4 = uniInfo4!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo4!.isUnion())
    arktest.assertEQ(constituentTypes4, undefined)

    let fldInfo5 = classType.getInstanceField('fld5')
    let uniInfo5 = fldInfo5!.getType()
    let constituentTypes5 = uniInfo5!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo5!.isUnion())
    arktest.assertNE(constituentTypes5, undefined)

    let fldInfo6 = classType.getInstanceField('fld6')
    let uniInfo6 = fldInfo6!.getType()
    let constituentTypes6 = uniInfo6!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo6!.isUnion())
    arktest.assertEQ(constituentTypes6, undefined)

    let fldInfo7 = classType.getInstanceField('fld7')
    let uniInfo7 = fldInfo7!.getType()
    let constituentTypes7 = uniInfo7!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo7!.isUnion())
    arktest.assertEQ(constituentTypes7, undefined)

    let fldInfo8 = classType.getInstanceField('fld8')
    let uniInfo8 = fldInfo8!.getType()
    let constituentTypes8 = uniInfo8!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo8!.isUnion())
    arktest.assertEQ(constituentTypes8, undefined)

    let fldInfo9 = classType.getInstanceField('fld9')
    let uniInfo9 = fldInfo9!.getType()
    let constituentTypes9 = uniInfo9!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo9!.isUnion())
    arktest.assertNE(constituentTypes9, undefined)

    let fldInfo10 = classType.getInstanceField('fld10')
    let uniInfo10 = fldInfo10!.getType()
    let constituentTypes10 = uniInfo10!.getUnionConstituentTypes()
    arktest.assertTrue(uniInfo10!.isUnion())
    arktest.assertNE(constituentTypes10, undefined)
    arktest.assertEQ(constituentTypes10!.length, 2)
    arktest.assertEQ(constituentTypes10![0].getName(), 'ReflectTypeTest.A')
    arktest.assertEQ(constituentTypes10![1].getName(), 'std.core.Null')

    let fldInfo11 = classType.getInstanceField('fld11')
    let uniInfo11 = fldInfo11!.getType()
    let constituentTypes11 = uniInfo11!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo11!.isUnion())
    arktest.assertEQ(constituentTypes11, undefined)

    let fldInfo12 = classType.getInstanceField('fld12')
    let uniInfo12 = fldInfo12!.getType()
    let constituentTypes12 = uniInfo12!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo12!.isUnion())
    arktest.assertEQ(constituentTypes12, undefined)

    let fldInfo13 = classType.getInstanceField('fld13')
    let uniInfo13 = fldInfo13!.getType()
    let constituentTypes13 = uniInfo13!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo13!.isUnion())
    arktest.assertEQ(constituentTypes13, undefined)

    let fldInfo14 = classType.getInstanceField('fld14')
    let uniInfo14 = fldInfo14!.getType()
    let constituentTypes14 = uniInfo14!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo14!.isUnion())
    arktest.assertEQ(constituentTypes14, undefined)

    let fldInfo15 = classType.getInstanceField('fld15')
    let uniInfo15 = fldInfo15!.getType()
    let constituentTypes15 = uniInfo15!.getUnionConstituentTypes()
    arktest.assertFalse(uniInfo15!.isUnion())
    arktest.assertEQ(constituentTypes15, undefined)
}

function testUnionField() {
    let clsC = new C<String>()
    let classType = Class.of(clsC)
    let fldInfo = classType.getInstanceField('fld')
    let uniInfo = fldInfo!.getType()
    arktest.assertTrue(uniInfo!.isUnion())
    let uniTypes = uniInfo!.getUnionConstituentTypes()
    let expectedTypeSet: Set<string> = new Set<string>([ 'ReflectTypeTest.A', 'std.core.Double', 'std.core.String' ]);

    arktest.assertEQ(uniTypes!.length, expectedTypeSet.size)
    for (let i = 0; i < uniTypes!.length; ++i) {
        arktest.assertTrue(expectedTypeSet.has(uniTypes![i].getName()))
    }
}

function testUnionParameter() {
    let clsC = new C<String>()
    let classType = Class.of(clsC)
    let methodInfo1 = classType.getInstanceMethod('foo')
    let paramType1 = methodInfo1!.getParameterType(0)
    arktest.assertTrue(paramType1!.isUnion())
    let uniTypes1 = paramType1!.getUnionConstituentTypes()
      let expectedTypeSet1: Set<string> = new Set<string>([ 'ReflectTypeTest.A', 'std.core.Double', 'std.core.String' ]);

    arktest.assertEQ(uniTypes1!.length, expectedTypeSet1.size)
    for (let i = 0; i < uniTypes1!.length; ++i) {
        arktest.assertTrue(expectedTypeSet1.has(uniTypes1![i].getName()))
    }

    let methodInfo2 = classType.getInstanceMethod('bar')
    let paramType2 = methodInfo2!.getParameterType(0)
    arktest.assertTrue(paramType2!.isFixedArray())
    let componentType = paramType2!.getFixedArrayComponentType()
    arktest.assertTrue(componentType!.isUnion())
    let uniTypes2 = componentType!.getUnionConstituentTypes()
    let expectedTypeSet2: Set<string> = new Set<string>([ 'std.core.Int', 'std.core.String' ]);

    arktest.assertEQ(uniTypes2!.length, expectedTypeSet2.size)
    for (let i = 0; i < uniTypes2!.length; ++i) {
        arktest.assertTrue(expectedTypeSet2.has(uniTypes2![i].getName()))
    }
}

function testUnionRet() {
    let clsC = new C<String>()
    let classType = Class.of(clsC)
    let methodInfo = classType.getInstanceMethod('foo')
    let retClass = methodInfo!.getReturnType()
    arktest.assertTrue(retClass!.isUnion())
    let uniTypes = retClass.getUnionConstituentTypes()
    let expectedTypeSet: Set<string> = new Set<string>([ 'ReflectTypeTest.A', 'std.core.Double', 'std.core.String' ]);

    arktest.assertEQ(uniTypes!.length, expectedTypeSet.size)
    for (let i = 0; i < uniTypes!.length; ++i) {
        arktest.assertTrue(expectedTypeSet.has(uniTypes![i].getName()))
    }
}

function testPrimitive() {
    let arr1: FixedArray<Int> = [ 1, 2 ]
    let arr2: FixedArray<Int | undefined> = [ 1, undefined, 3 ]
    let arr3: FixedArray<boolean> = [ true, false ]
    let arr4: FixedArray<boolean | undefined> = [ undefined, false ]

    let classType1 = Class.of(arr1)
    let classType2 = Class.of(arr2)
    let classType3 = Class.of(arr3)
    let classType4 = Class.of(arr4)

    arktest.assertFalse(classType1.isPrimitive())
    arktest.assertFalse(classType2.isPrimitive())
    arktest.assertFalse(classType3.isPrimitive())
    arktest.assertFalse(classType4.isPrimitive())

    let componentType1 = classType1.getFixedArrayComponentType()!
    let componentType2 = classType2.getFixedArrayComponentType()!
    let componentType3 = classType3.getFixedArrayComponentType()!
    let componentType4 = classType4.getFixedArrayComponentType()!

    arktest.assertTrue(componentType1.isPrimitive())
    arktest.assertFalse(componentType2.isPrimitive())
    arktest.assertTrue(componentType3.isPrimitive())
    arktest.assertFalse(componentType4.isPrimitive())

    arktest.assertEQ(componentType1, Class.PRIMITIVE_INT)
    arktest.assertEQ(componentType3, Class.PRIMITIVE_BOOLEAN)

    arktest.assertEQ(componentType2.getName(), 'std.core.Int')
    arktest.assertEQ(componentType4.getName(), 'std.core.Boolean')

    const pCls = Class.of(new P())
    const fld1 = pCls.getInstanceField('fldBoolean')!
    const fldType1 = fld1.getType()
    arktest.assertTrue(fldType1.isPrimitive())
    arktest.assertEQ(fldType1, Class.PRIMITIVE_BOOLEAN)
    arktest.assertEQ(fldType1.getName(), 'u1')
    const fld2 = pCls.getInstanceField('fldByte')!
    const fldType2 = fld2.getType()
    arktest.assertTrue(fldType2.isPrimitive())
    arktest.assertEQ(fldType2, Class.PRIMITIVE_BYTE)
    arktest.assertEQ(fldType2.getName(), 'i8')
    const fld3 = pCls.getInstanceField('fldShort')!
    const fldType3 = fld3.getType()
    arktest.assertTrue(fldType3.isPrimitive())
    arktest.assertEQ(fldType3, Class.PRIMITIVE_SHORT)
    arktest.assertEQ(fldType3.getName(), 'i16')
    const fld4 = pCls.getInstanceField('fldChar')!
    const fldType4 = fld4.getType()
    arktest.assertTrue(fldType4.isPrimitive())
    arktest.assertEQ(fldType4, Class.PRIMITIVE_CHAR)
    arktest.assertEQ(fldType4.getName(), 'u16')
    const fld5 = pCls.getInstanceField('fldInt')!
    const fldType5 = fld5.getType()
    arktest.assertTrue(fldType5.isPrimitive())
    arktest.assertEQ(fldType5, Class.PRIMITIVE_INT)
    arktest.assertEQ(fldType5.getName(), 'i32')
    const fld6 = pCls.getInstanceField('fldLong')!
    const fldType6 = fld6.getType()
    arktest.assertTrue(fldType6.isPrimitive())
    arktest.assertEQ(fldType6, Class.PRIMITIVE_LONG)
    arktest.assertEQ(fldType6.getName(), 'i64')
    const fld7 = pCls.getInstanceField('fldFloat')!
    const fldType7 = fld7.getType()
    arktest.assertTrue(fldType7.isPrimitive())
    arktest.assertEQ(fldType7, Class.PRIMITIVE_FLOAT)
    arktest.assertEQ(fldType7.getName(), 'f32')
    const fld8 = pCls.getInstanceField('fldDouble')!
    const fldType8 = fld8.getType()
    arktest.assertTrue(fldType8.isPrimitive())
    arktest.assertEQ(fldType8, Class.PRIMITIVE_DOUBLE)
    arktest.assertEQ(fldType8, Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(fldType8.getName(), 'f64')
    const meth1 = pCls.getInstanceMethod('foo')!
    const methRetType1 = meth1.getReturnType()
    arktest.assertTrue(methRetType1.isPrimitive())
    arktest.assertEQ(methRetType1, Class.PRIMITIVE_VOID)
    arktest.assertEQ(methRetType1.getName(), 'void')

    arktest.assertTrue(fldType5.isFinal())
    arktest.assertTrue(fldType5.isAbstract())
    arktest.assertFalse(fldType5.isNamespace())
    arktest.assertFalse(fldType5.isEnum())
    arktest.assertFalse(fldType5.isInterface())
    arktest.assertFalse(fldType5.isUnion())
    arktest.assertFalse(fldType5.isFixedArray())
    arktest.assertEQ(fldType5.getSuper(), undefined)
    arktest.assertEQ(fldType5.getLinker(), getBootRuntimeLinker())
    arktest.assertEQ(fldType5.getFixedArrayComponentType(), undefined)
    arktest.assertEQ(fldType5.getInterfaces().length, 0)
    arktest.assertEQ(fldType5.getInstanceMethods().length, 0)
    arktest.assertEQ(fldType5.getInstanceFields().length, 0)
    arktest.assertEQ(fldType5.getStaticMethods().length, 0)
    arktest.assertEQ(fldType5.getStaticFields().length, 0)
    arktest.assertEQ(fldType5.getConstructors().length, 0)
    arktest.assertEQ(fldType5.getUnionConstituentTypes(), undefined)
}

function main(): int {
    let typeBaseObj = Class.of(new Object())

    testGetName(typeBaseObj, "std.core.Object")

    let typeA = Class.of(new A())

    testGetName(typeA, "ReflectTypeTest.A")

    let typeB = Class.of(new B())

    testGetName(typeB, "ReflectTypeTest.B")

    // Test getSuper()
    arktest.assertEQ(typeBaseObj.getSuper(), undefined)
    arktest.assertEQ(typeA.getSuper(), typeBaseObj)
    arktest.assertEQ(typeB.getSuper(), typeA)

    arktest.assertEQ(Class.from<BaseIface>().getSuper(), undefined)
    arktest.assertEQ(Class.from<DerivedIface>().getSuper(), undefined)
    arktest.assertEQ(Class.from<DerivedDerivedIface>().getSuper(), undefined)

    // GetInterfaces()
    let emptyClass: Class[] = []
    arktest.assertEQ(Class.from<BaseIface>().getInterfaces().length, 0)
    testGetInterfaces(Class.from<DerivedIface>().getInterfaces(), ["ReflectTypeTest.BaseIface"])
    testGetInterfaces(Class.from<DerivedDerivedIface>().getInterfaces(), ["ReflectTypeTest.DerivedIface"])

    // Test methods getters
    let baseObjInstanceMethodsExpected =
        [ "toString", "toLocaleString", "$_hashCode", "hasOwnProperty", "hasOwnProperty" ]

    let baseObjStaticMethodsExpected =
        [ "keys", "keys", "assign", "hasOwn", "hasOwn", "values", "values", "entries",
          "entries", "fromEntries", "getOwnPropertyNames", "getOwnPropertyNames" ]

    testGetInstanceMethods(typeBaseObj, baseObjInstanceMethodsExpected)
    testGetInstanceMethods(typeA, ["someDefaultMethod", "foo1", "foo2"])
    testGetInstanceMethods(typeB, ["boo1", "boo2"])

    // Test instanceMethods for interfaces
    testGetInstanceMethods(Class.from<BaseIface>(), ["foo"])
    testGetInstanceMethods(Class.from<DerivedIface>(), ["boo", "defaultMethod"])
    testGetInstanceMethods(Class.from<DerivedDerivedIface>(), ["foo", "goo"])

    testGetInstanceMethodByName(typeBaseObj, [ "toString", "toLocaleString", "$_hashCode"])
    let hasOwnProperty1 = typeBaseObj.getInstanceMethod("hasOwnProperty", [ Class.from<string>() ])
    let hasOwnProperty2 = typeBaseObj.getInstanceMethod("hasOwnProperty", [ Class.PRIMITIVE_DOUBLE ])
    arktest.assertNE(hasOwnProperty1, undefined)
    arktest.assertNE(hasOwnProperty2, undefined)
    arktest.assertNE(hasOwnProperty1, hasOwnProperty2)

    testGetInstanceMethodByName(typeA, ["someDefaultMethod", "foo1", "foo2"])
    testGetInstanceMethodByName(typeB, ["boo1", "boo2"])

    tryGetPrivateInstanceMethodByName(typeA, [ "prfoo1", "prfoo2" ])

    testGetConstructors()
    testGetStaticMethods()
    testGetStaticMethod()
    testGetStaticFields()
    testGetInstanceFields()

    testInstanceFieldEquals()
    testStaticFieldEquals()
    testStaticMethodEquals()

    testIsUnion()
    testUnionField()
    testUnionParameter()
    testUnionRet()

    testPrimitive()

    return 0
}
