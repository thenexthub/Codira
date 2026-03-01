/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

interface Serializable {
    serialize(): String
}

class Point implements Serializable {
    protected xP: double
    x: double
    y: double

    override serialize(): String {
        return "{" + this.x + "," + this.y + "}"
    }

    add(oth: Point): Point {
        let res = new Point()
        res.x = this.x + oth.x
        res.y = this.y + oth.y
        return res
    }
}

class Point3D extends Point {
    z: double
    y: double = 3.0
    private stateP: int
    state: int
    static ZERO: double = 0.0
    readonly r: int = 1
    ability: String | Number = new Number(1)
}

function testField(): void {
    let typePoint = Class.from<Point>()
    let xf = typePoint.getInstanceField("x")!
    let yf = typePoint.getInstanceField("y")!
    arktest.assertEQ(typePoint.getInstanceField("xP"), undefined) // protected

    arktest.assertEQ(xf.getName(), "x")
    arktest.assertEQ(xf.getOwner(), typePoint)
    arktest.assertEQ(xf.getType(), Class.PRIMITIVE_DOUBLE)
    arktest.assertFalse(xf.isReadonly())
    arktest.assertTrue(xf.isPublic())
    arktest.assertFalse(xf.isProtected())
    arktest.assertFalse(xf.isPrivate())

    arktest.assertEQ(yf.getName(), "y")
    arktest.assertEQ(yf.getOwner(), typePoint)
    arktest.assertEQ(yf.getType(), Class.PRIMITIVE_DOUBLE)
    arktest.assertFalse(yf.isReadonly())
    arktest.assertTrue(yf.isPublic())
    arktest.assertFalse(yf.isProtected())
    arktest.assertFalse(yf.isPrivate())

    let typePoint3D = Class.from<Point3D>()
    let sf = typePoint3D.getInstanceField("state")!
    let rf = typePoint3D.getInstanceField("r")!
    let abilityf = typePoint3D.getInstanceField("ability")!
    arktest.assertEQ(typePoint3D.getInstanceField("yf"), undefined) // parent field
    arktest.assertEQ(typePoint3D.getInstanceField("stateP"), undefined) // private
    arktest.assertEQ(typePoint3D.getInstanceField("xP"), undefined) // parent field
    arktest.assertEQ(typePoint3D.getInstanceField("x"), undefined) // parent field

    arktest.assertEQ(sf.getName(), "state")
    arktest.assertEQ(sf.getOwner(), typePoint3D)
    arktest.assertEQ(sf.getType(), Class.PRIMITIVE_INT)
    arktest.assertFalse(sf.isReadonly())
    arktest.assertTrue(sf.isPublic())
    arktest.assertFalse(sf.isProtected())
    arktest.assertFalse(sf.isPrivate())

    arktest.assertEQ(rf.getName(), "r")
    arktest.assertEQ(rf.getOwner(), typePoint3D)
    arktest.assertEQ(rf.getType(), Class.PRIMITIVE_INT)
    arktest.assertTrue(rf.isReadonly())
    arktest.assertTrue(rf.isPublic())
    arktest.assertFalse(rf.isProtected())
    arktest.assertFalse(rf.isPrivate())
    arktest.assertEQ(abilityf.getName(), "ability")
    arktest.assertEQ(abilityf.getOwner(), typePoint3D)
}

class UClassA {}
class UClassB {}

enum TestFieldEnum {
    ONE,
    TWO,
    THREE
}

enum WrongEnum {
    ONE,
    TWO,
    THREE
}

class Parent {
    private static PRIV: boolean = true
    protected static PROT: string = 'prot'

    static ID: number = 1.0
    static STR: string = ''
    static UN: UClassA | UClassB | number = 42
    static readonly RDONLYPRIM: int = 8
    static readonly RDONLYSTR: string = ''
    static TPLE: [int, string] = [42, "str"]
    static ENM = TestFieldEnum.ONE

    static get PROP(): number {
        return 1
    }

    get prop(): number {
        return 0
    }

    id: number = 0
    str: string = ''
    readonly rdonlyPrim: int = 8
    readonly rdonlyStr: string = ''
    un: UClassA | UClassB | number = 42
    tple: [int, string] = [42, "str"]
    enm: TestFieldEnum = TestFieldEnum.ONE
    private priv = true
    protected prot = true
}

class Child extends Parent {
    static ID: number = 2.0
    static EOL: char = c'\n'
    public EOL: string = '\n'

    id: number = 1
    str: string = ''
}

function testInstanceGetSetPrimitive(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentIdFld = parentCls.getInstanceField('id')!
    const localStr = 'I\'m a string'
    const localNum = 3.14

    arktest.assertNE(localNum, parentInst.id)
    arktest.assertNE(localNum, parentIdFld.getValue(parentInst))
    parentIdFld.setValue(parentInst, localNum)
    arktest.assertEQ(localNum, parentInst.id)
    arktest.assertEQ(localNum, parentIdFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childIdFld = childCls.getSuper()!.getInstanceField('id')!

    arktest.assertNE(localNum, childInst.id)
    arktest.assertNE(localNum, childIdFld.getValue(childInst))
    childIdFld.setValue(childInst, localNum)
    arktest.assertEQ(localNum, childInst.id)
    arktest.assertEQ(localNum, childIdFld.getValue(childInst))
}

function testInstanceGetSetReference(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentStrFld = parentCls.getInstanceField('str')!
    const parentIdFld = parentCls.getInstanceField('id')!
    const localStr = 'I\'m a string'
    const localNum = parentInst.id

    arktest.assertNE(localStr, parentInst.str)
    arktest.assertNE(localStr, parentStrFld.getValue(parentInst))
    parentStrFld.setValue(parentInst, localStr)
    arktest.assertEQ(localStr, parentInst.str)
    arktest.assertEQ(localNum, parentIdFld.getValue(parentInst))

    parentStrFld.setValue(parentInst, undefined)
    arktest.assertEQ(undefined, parentInst.str)
    arktest.assertEQ(undefined, parentStrFld.getValue(parentInst))
    parentInst.str = localStr

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childStrFld = childCls.getSuper()!.getInstanceField('str')!

    arktest.assertNE(localStr, childInst.str)
    arktest.assertNE(localStr, childStrFld.getValue(childInst))
    childStrFld.setValue(childInst, localStr)
    arktest.assertEQ(localStr, childInst.str)
    arktest.assertEQ(childInst.id, parentIdFld.getValue(childInst))

    childStrFld.setValue(childInst, undefined)
    arktest.assertEQ(undefined, childInst.str)
    arktest.assertEQ(undefined, childStrFld.getValue(childInst))
    childInst.str = localStr
}

function testInstanceGetSetUnion(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentUnFld = parentCls.getInstanceField('un')!
    const localUA = new UClassA()
    const localUB = new UClassB()
    const localNum = 3.14

    arktest.assertNE(localNum, parentInst.un)
    arktest.assertNE(localUA, parentInst.un)
    arktest.assertNE(localUB, parentInst.un)
    arktest.assertNE(localNum, parentUnFld.getValue(parentInst))
    parentUnFld.setValue(parentInst, localNum)
    arktest.assertEQ(localNum, parentInst.un)
    arktest.assertEQ(localNum, parentUnFld.getValue(parentInst))

    arktest.assertNE(localUA, parentInst.un)
    arktest.assertNE(localUB, parentInst.un)
    arktest.assertNE(localUA, parentUnFld.getValue(parentInst))
    parentUnFld.setValue(parentInst, localUA)
    arktest.assertEQ(localUA, parentInst.un)
    arktest.assertEQ(localUA, parentUnFld.getValue(parentInst))

    arktest.assertNE(localUB, parentInst.un)
    arktest.assertNE(localNum, parentInst.un)
    arktest.assertNE(localUB, parentUnFld.getValue(parentInst))
    parentUnFld.setValue(parentInst, localUB)
    arktest.assertEQ(localUB, parentInst.un)
    arktest.assertEQ(localUB, parentUnFld.getValue(parentInst))
}

function testInstanceGetSetTuple(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentTpleFld = parentCls.getInstanceField('tple')!
    const localTuple: [int, string] = [0, "local"]

    arktest.assertNE(localTuple, parentInst.tple)
    arktest.assertNE(localTuple, parentTpleFld.getValue(parentInst))
    parentTpleFld.setValue(parentInst, localTuple)
    arktest.assertEQ(localTuple, parentInst.tple)
    arktest.assertEQ(localTuple, parentTpleFld.getValue(parentInst))

    const undefTuple: [int | undefined, string | undefined] = [undefined, undefined]
    parentTpleFld.setValue(parentInst, undefTuple)
    arktest.assertEQ(undefTuple, parentInst.tple)
    arktest.assertEQ(undefTuple, parentTpleFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    arktest.assertEQ(childCls.getInstanceField('tple'), undefined) // parent field
}

function testInstanceGetSetEnum(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentEnmFld = parentCls.getInstanceField('enm')!
    const localEnum = TestFieldEnum.TWO

    arktest.assertNE(localEnum, parentInst.enm)
    arktest.assertNE(localEnum, parentEnmFld.getValue(parentInst))
    parentEnmFld.setValue(parentInst, localEnum)
    arktest.assertEQ(localEnum, parentInst.enm)
    arktest.assertEQ(localEnum, parentEnmFld.getValue(parentInst))
}

function testInstanceGetSetPrimitiveNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentStrFld = parentCls.getInstanceField('str')!
    const parentIdFld = parentCls.getInstanceField('id')!
    const parentRdonlyPrimFld = parentCls.getInstanceField('rdonlyPrim')!
    const localStr = 'I\'m a string'
    const localNumParent = parentInst.id

    arktest.expectError(() => {
        parentRdonlyPrimFld.setValue(parentInst, 42)
    }, new TypeError("setValue to a readonly field"))
    arktest.assertEQ(8 , parentInst.rdonlyPrim)
    arktest.assertEQ(parentRdonlyPrimFld.getValue(parentInst) , parentInst.rdonlyPrim)

    arktest.expectError(() => {
        parentIdFld.setValue(parentInst, undefined)
    }, new NullPointerError("undefined argument is not allowed for primitive reciever"))
    arktest.assertNE(undefined, parentIdFld.getValue(parentInst))
    arktest.assertNE(undefined, parentInst.id)
    arktest.assertEQ(localNumParent, parentInst.id)
    arktest.assertEQ(localNumParent, parentIdFld.getValue(parentInst))

    arktest.expectError(() => {
        parentIdFld.setValue(parentInst, localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, parentIdFld.getValue(parentInst))
    arktest.assertNE(localStr, parentInst.id)
    arktest.assertEQ(localNumParent, parentInst.id)
    arktest.assertEQ(localNumParent, parentIdFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childIdFld = childCls.getSuper()!.getInstanceField('id')!
    const localNumChild = childInst.id

    arktest.expectError(() => {
        childIdFld.setValue(childInst, undefined)
    }, new NullPointerError("undefined argument is not allowed for primitive reciever"))
    arktest.assertNE(undefined, childIdFld.getValue(childInst))
    arktest.assertNE(undefined, childInst.id)
    arktest.assertEQ(localNumChild, childInst.id)
    arktest.assertEQ(localNumChild, childIdFld.getValue(childInst))

    arktest.expectError(() => {
        childIdFld.setValue(childInst, localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, childInst.id)
    arktest.assertEQ(localNumChild, childInst.id)
}

function testInstanceGetSetReferenceNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentStrFld = parentCls.getInstanceField('str')!
    const parentRdonlyStrFld = parentCls.getInstanceField('rdonlyStr')!
    const localStrParent = parentInst.str
    const localNum = 3.14

    arktest.expectError(() => {
        parentRdonlyStrFld.setValue(parentInst, 'str')
    }, new TypeError("setValue to a readonly field"))
    arktest.assertEQ('' , parentInst.rdonlyStr)
    arktest.assertEQ(parentRdonlyStrFld.getValue(parentInst) , parentInst.rdonlyStr)

    arktest.expectError(() => {
        parentStrFld.setValue(parentInst, localNum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localNum, parentInst.str)
    arktest.assertNE(localNum, parentStrFld.getValue(parentInst))
    arktest.assertEQ(localStrParent, parentInst.str)
    arktest.assertEQ(localStrParent, parentStrFld.getValue(parentInst))

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childStrFld = childCls.getSuper()!.getInstanceField('str')!
    const localStrChild = childInst.str

    arktest.expectError(() => {
        childStrFld.setValue(childInst, localNum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localNum, childInst.str)
    arktest.assertEQ(localStrChild, childInst.str)
}

function testInstanceGetSetUnionNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentUnFld = parentCls.getInstanceField('un')!
    const localStr = 'I\'m a string'

    arktest.expectError(() => {
        parentUnFld.setValue(parentInst, localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, parentInst.un)
    arktest.assertNE(localStr, parentUnFld.getValue(parentInst))
    arktest.assertEQ(42, parentInst.un)
    arktest.assertEQ(42, parentUnFld.getValue(parentInst))
}

function testInstanceGetSetTupleNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentTpleFld = parentCls.getInstanceField('tple')!
    const localTuple: [number, Object, string] = [3.14, new Object(), '']
    const originalTuple = parentInst.tple

    arktest.expectError(() => {
        parentTpleFld.setValue(parentInst, localTuple)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalTuple, parentInst.tple)
    arktest.assertEQ(originalTuple, parentTpleFld.getValue(parentInst))
}

function testInstanceGetSetEnumNegative(): void {
    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentEnmFld = parentCls.getInstanceField('enm')!
    const localEnum = WrongEnum.ONE
    const originalEnum = parentInst.enm

    arktest.expectError(() => {
        parentEnmFld.setValue(parentInst, localEnum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalEnum, parentInst.enm)
    arktest.assertEQ(originalEnum, parentEnmFld.getValue(parentInst))
}

function testStaticGetSetPrimitive(): void {
    const parentCls = Class.from<Parent>()
    const parentIdFld = parentCls.getStaticField('ID')!
    const localNum = 3.14

    arktest.assertNE(localNum, Parent.ID)
    arktest.assertNE(localNum, parentIdFld.getValue())
    parentIdFld.setValue(localNum)
    arktest.assertEQ(localNum, Parent.ID)
    arktest.assertEQ(localNum, parentIdFld.getValue())

    const childCls = Class.from<Child>()
    const childIdFld = childCls.getStaticField('ID')!

    arktest.assertNE(localNum, Child.ID)
    arktest.assertNE(localNum, childIdFld.getValue())
    childIdFld.setValue(localNum)
    arktest.assertEQ(localNum, Child.ID)
    arktest.assertEQ(localNum, childIdFld.getValue())
}

function testStaticGetSetReference(): void {
    const parentCls = Class.from<Parent>()
    const parentStrFld = parentCls.getStaticField('STR')!
    const localStr = 'I\'m a string'

    arktest.assertNE(localStr, Parent.STR)
    arktest.assertNE(localStr, parentStrFld.getValue())
    parentStrFld.setValue(localStr)
    arktest.assertEQ(localStr, Parent.STR)
    arktest.assertEQ(localStr, parentStrFld.getValue())
}

function testStaticGetSetUnion(): void {
    const parentCls = Class.from<Parent>()
    const parentUnFld = parentCls.getStaticField('UN')!
    const localUA = new UClassA()
    const localUB = new UClassB()
    const localNum = 3.14
    Parent.UN = 42

    arktest.assertNE(localNum, Parent.UN)
    arktest.assertNE(localUA, Parent.UN)
    arktest.assertNE(localUB, Parent.UN)
    arktest.assertNE(localNum, parentUnFld.getValue())
    parentUnFld.setValue(localNum)
    arktest.assertEQ(localNum, Parent.UN)
    arktest.assertEQ(localNum, parentUnFld.getValue())

    arktest.assertNE(localUA, Parent.UN)
    arktest.assertNE(localUB, Parent.UN)
    arktest.assertNE(localUA, parentUnFld.getValue())
    parentUnFld.setValue(localUA)
    arktest.assertEQ(localUA, Parent.UN)
    arktest.assertEQ(localUA, parentUnFld.getValue())

    arktest.assertNE(localUB, Parent.UN)
    arktest.assertNE(localNum, Parent.UN)
    arktest.assertNE(localUB, parentUnFld.getValue())
    parentUnFld.setValue(localUB)
    arktest.assertEQ(localUB, Parent.UN)
    arktest.assertEQ(localUB, parentUnFld.getValue())
}

function testStaticGetSetTuple(): void {
    const parentCls = Class.from<Parent>()
    const parentTpleFld = parentCls.getStaticField('TPLE')!
    const localTuple: [int, string] = [0, "local"]
    const originalTuple = Parent.TPLE

    arktest.assertNE(localTuple, Parent.TPLE)
    arktest.assertNE(localTuple, parentTpleFld.getValue())
    parentTpleFld.setValue(localTuple)
    arktest.assertEQ(localTuple, Parent.TPLE)
    arktest.assertEQ(localTuple, parentTpleFld.getValue())

    Parent.TPLE = originalTuple
}

function testStaticGetSetEnum(): void {
    const parentCls = Class.from<Parent>()
    const parentEnmFld = parentCls.getStaticField('ENM')!
    const localEnum = TestFieldEnum.TWO

    arktest.assertNE(localEnum, Parent.ENM)
    arktest.assertNE(localEnum, parentEnmFld.getValue())
    parentEnmFld.setValue(localEnum)
    arktest.assertEQ(localEnum, Parent.ENM)
    arktest.assertEQ(localEnum, parentEnmFld.getValue())
}

function testStaticGetSetPrimitiveNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentIdFld = parentCls.getStaticField('ID')!
    const parentRdonlyPrimFld = parentCls.getStaticField('RDONLYPRIM')!
    const localNum = 3.14
    const localStr = 'I\'m a string'

    arktest.expectError(() => {
        parentRdonlyPrimFld.setValue(42)
    }, new TypeError("setValue to a readonly field"))
    arktest.assertEQ(8 , Parent.RDONLYPRIM)
    arktest.assertEQ(parentRdonlyPrimFld.getValue() , Parent.RDONLYPRIM)

    arktest.expectError(() => {
        parentIdFld.setValue(localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, Parent.ID)
    arktest.assertNE(localStr, parentIdFld.getValue())
    arktest.assertEQ(localNum, Parent.ID)
    arktest.assertEQ(localNum, parentIdFld.getValue())
}

function testStaticGetSetReferenceNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentStrFld = parentCls.getStaticField('STR')!
    const localNum = 3.14
    const localStr = 'I\'m a string'

    arktest.expectError(() => {
        parentStrFld.setValue(localNum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localNum, Parent.STR)
    arktest.assertNE(localNum, parentStrFld.getValue())
    arktest.assertEQ(localStr, Parent.STR)
    arktest.assertEQ(localStr, parentStrFld.getValue())
}

function testStaticGetSetUnionNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentUnFld = parentCls.getStaticField('UN')!
    const localStr = 'I\'m a string'
    Parent.UN = 42

    arktest.expectError(() => {
        parentUnFld.setValue(localStr)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertNE(localStr, Parent.UN)
    arktest.assertNE(localStr, parentUnFld.getValue())
    arktest.assertEQ(42, Parent.UN)
    arktest.assertEQ(42, parentUnFld.getValue())
}

function testStaticGetSetTupleNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentTpleFld = parentCls.getStaticField('TPLE')!
    const localTuple: [number, Object, string] = [3.14, new Object(), '']
    const originalTuple = Parent.TPLE

    arktest.expectError(() => {
        parentTpleFld.setValue(localTuple)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalTuple, Parent.TPLE)
    arktest.assertEQ(originalTuple, parentTpleFld.getValue())
}

function testStaticGetSetEnumNegative(): void {
    const parentCls = Class.from<Parent>()
    const parentEnmFld = parentCls.getStaticField('ENM')!
    const localEnum = WrongEnum.TWO
    const originalEnum = Parent.ENM

    arktest.expectError(() => {
        parentEnmFld.setValue(localEnum)
    }, new TypeError("Value is not assignable to the provided field"))
    arktest.assertEQ(originalEnum, Parent.ENM)
    arktest.assertEQ(originalEnum, parentEnmFld.getValue())
}

function testInstanceField(): void {
    testInstanceGetSetPrimitive()
    testInstanceGetSetReference()
    testInstanceGetSetUnion()
    testInstanceGetSetTuple()
    testInstanceGetSetEnum()
    testInstanceGetSetPrimitiveNegative()
    testInstanceGetSetReferenceNegative()
    testInstanceGetSetUnionNegative()
    testInstanceGetSetTupleNegative()
    testInstanceGetSetEnumNegative()

    const parentInst = new Parent()
    const parentCls = Class.of(parentInst)
    const parentIdFld = parentCls.getInstanceField('id')!
    const parentStrFld = parentCls.getInstanceField('str')!

    arktest.assertEQ(parentIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(parentIdFld.getOwner(), parentCls)
    arktest.assertEQ(parentStrFld.getType(), Class.of(new String()))
    arktest.assertEQ(parentStrFld.getOwner(), parentCls)

    const childInst = new Child()
    const childCls = Class.of(childInst)
    const childIdFld = childCls.getSuper()!.getInstanceField('id')!
    const childStrFld = childCls.getSuper()!.getInstanceField('str')!

    arktest.assertEQ(childIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(childIdFld.getOwner(), parentCls)
    arktest.assertEQ(childStrFld.getType(), Class.of(new String()))
    arktest.assertEQ(childStrFld.getOwner(), parentCls)

    arktest.assertEQ(parentCls.getInstanceField('prop'), undefined)
    arktest.assertEQ(parentCls.getInstanceField('priv'), undefined)
    arktest.assertEQ(parentCls.getInstanceField('prot'), undefined)
    arktest.assertEQ(childCls.getSuper()!.getInstanceField('prot'), undefined)

    const parentFlds = parentCls.getInstanceFields()
    const parentFldNames = new Set<string>(Array.from<reflect.InstanceField, string>(parentFlds, (f) => f.getName()))
    arktest.assertFalse(parentFldNames.has('priv'))
    arktest.assertFalse(parentFldNames.has('prot'))
    arktest.assertTrue(parentFldNames.has('id'))

    const childFlds = childCls.getInstanceFields()
    const childFldNames = new Set<string>(Array.from<reflect.InstanceField, string>(childFlds, (f) => f.getName()))
    arktest.assertFalse(childFldNames.has('prot'))
    arktest.assertTrue(parentFldNames.has('id'))

    const bools: FixedArray<boolean> = [true]
    const boolsCls = Class.of(bools)
    arktest.assertEQ(boolsCls.getInstanceField('length'), undefined)
}

function testStaticField(): void {
    testStaticGetSetPrimitive()
    testStaticGetSetReference()
    testStaticGetSetUnion()
    testStaticGetSetTuple()
    testStaticGetSetEnum()
    testStaticGetSetPrimitiveNegative()
    testStaticGetSetReferenceNegative()
    testStaticGetSetUnionNegative()
    testStaticGetSetTupleNegative()
    testStaticGetSetEnumNegative()

    const parentCls = Class.of(new Parent)
    const parentIdFld = parentCls.getStaticField('ID')!
    const parentStrFld = parentCls.getStaticField('STR')!

    arktest.assertEQ(parentIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(parentIdFld.getOwner(), parentCls)
    arktest.assertEQ(parentStrFld.getType(), Class.of(new String()))
    arktest.assertEQ(parentStrFld.getOwner(), parentCls)
    arktest.assertEQ(parentCls.getStaticField('EOL'), undefined)

    const childCls = Class.of(new Child)
    const childIdFld = childCls.getStaticField('ID')!

    arktest.assertEQ(childIdFld.getType(), Class.PRIMITIVE_NUMBER)
    arktest.assertEQ(childIdFld.getOwner(), childCls)
    arktest.assertEQ(childCls.getStaticField('STR'), undefined) // parent field

    arktest.assertEQ(childCls.getStaticField('EOL')!.getType(), Class.PRIMITIVE_CHAR)

    arktest.assertEQ(parentCls.getStaticField('PROP'), undefined)
    arktest.assertEQ(childCls.getStaticField('PROP'), undefined)

    arktest.assertEQ(parentCls.getStaticField('PRIV'), undefined)
    arktest.assertEQ(childCls.getStaticField('PROT'), undefined)

    const parentFlds = parentCls.getStaticFields()
    const parentFldNames = new Set<string>(Array.from<reflect.StaticField, string>(parentFlds, (f) => f.getName()))
    arktest.assertFalse(parentFldNames.has('PRIV'))
    arktest.assertFalse(parentFldNames.has('PROT'))
    arktest.assertTrue(parentFldNames.has('ID'))

    const childFlds = childCls.getStaticFields()
    const childFldNames = new Set<string>(Array.from<reflect.StaticField, string>(childFlds, (f) => f.getName()))
    arktest.assertFalse(childFldNames.has('PROT'))
    arktest.assertTrue(childFldNames.has('ID'))
}

interface Iface {
    id: string
}

class IfaceImpl implements Iface {
    get id():string {return ''}
    set id(v: string) {}
}

function testInterfaceField(): void {
    const ifaceCls = Class.from<FixedArray<Iface>>().getFixedArrayComponentType()!

    const idFld = ifaceCls.getInstanceField('id')
    arktest.assertEQ(idFld, undefined)

    const impl = new IfaceImpl()
    const idFldImpl = Class.of(impl).getInstanceField('id')
    arktest.assertEQ(idFldImpl, undefined)

    arktest.assertEQ(ifaceCls.getInstanceFields().length, 0)
    arktest.assertEQ(ifaceCls.getStaticFields().length, 0)
}

function main() {
    const suite = new arktest.ArkTestsuite('Reflect Field tests')
    suite.addTest('Test Reflect Field APIs', testField)
    suite.addTest('Test Reflect instance Field APIs', testInstanceField)
    suite.addTest('Test Reflect static Field APIs', testStaticField)
    suite.addTest('Test Reflect interface Field APIs', testInterfaceField)
    return suite.run()
}
