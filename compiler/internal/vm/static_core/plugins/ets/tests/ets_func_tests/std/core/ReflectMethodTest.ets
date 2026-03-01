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

interface Serializable {
    serialize(): String
}

class Point implements Serializable {
    protected x: double
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

    testUnion(a: Int | Int[]): String | Int {
        if (a instanceof Int) {
            return "1"
        }
        return 2
    }

    optionalParams(x: int, opt1: int = 1, opt2: int = 2) {}
    restParams(x: int[], ...rest: int[]) {}
    optReadonlyRestParams(x: readonly int[] = [4], y: int = 4, z: readonly [number, string] = [1, "123"], ...rest: int[]) {}
}

abstract class Writer {
    abstract write(): Int
    close() {}
}

class HTMLWriter extends Writer {
    private state_: int

    get state(): int {
        return this.state_
    }

    set state(state: int) {
        this.state_ = state
    }

    static createFileExt(file: String): String {
        return file + ".html"
    }

    override write(): Int {
        return new Int(2)
    }

    constructor(state: int) {
        this.state_ = state
    }

    final finalMethod() {}

    native nativeMethod(): boolean

    async asyncMethod() {}
}

class Base {
    foo() {}
    bar() {}
}

class Derived extends Base {
    foo() {}
}

class ReferenceValueHolder {
    public static readonly objReference = new Object()
    public static readonly booleanReference = new Boolean(true)
    public static readonly byteReference = new Byte(1)
    public static readonly charReference = new Char(c'2')
    public static readonly shortReference = new Short(3)
    public static readonly intReference = new Int(4)
    public static readonly longReference = new Long(5)
    public static readonly floatReference = new Float(3.14)
    public static readonly doubleReference = new Double(-3.14)

    public static readonly objArrReference = [ReferenceValueHolder.intReference, ReferenceValueHolder.longReference, ReferenceValueHolder.doubleReference]
}

interface InvokeTester {
    funcEmpty(): Class
    funcObj(v: Object): Class
    funcBoolean(v: boolean): Class
    funcByte(v: byte): Class
    funcChar(v: char): Class
    funcShort(v: short): Class
    funcInt(v: int): Class
    funcLong(v: long): Class
    funcFloat(v: float): Class
    funcDouble(v: double): Class
    funcOptional(v: Object, v1?: Object, v2?: Object, v3?: Object): Class
    funcRestParams(...v: Object[]): Class
    funcRestParamsObj(v?: Object, ...v1: Object[]): Class
}

class BaseInvokeTester implements InvokeTester {
    public objVal: Object | undefined
    public intVal: int | undefined

    constructor() {
        this.objVal = undefined
        this.intVal = undefined
    }

    constructor(v1: Object) {
        this.objVal = v1
        this.intVal = undefined
    }

    constructor(v1: Object, v2: int) {
        this.objVal = v1
        this.intVal = v2
    }

    constructor(v1: int, v2: int, v3: int) {
        this.objVal = undefined
        this.intVal = v1 + v2 + v2
    }

    public funcEmpty(): Class { return Class.of(this) }

    public funcObj(v: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        return Class.of(this)
    }

    public funcBoolean(v: boolean): Class {
        arktest.assertEQ(v, ReferenceValueHolder.booleanReference)
        return Class.of(this)
    }

    public funcByte(v: byte): Class {
        arktest.assertEQ(v, ReferenceValueHolder.byteReference)
        return Class.of(this)
    }

    public funcChar(v: char): Class {
        arktest.assertEQ(v, ReferenceValueHolder.charReference)
        return Class.of(this)
    }

    public funcShort(v: short): Class {
        arktest.assertEQ(v, ReferenceValueHolder.shortReference)
        return Class.of(this)
    }

    public funcInt(v: int): Class {
        arktest.assertEQ(v, ReferenceValueHolder.intReference)
        return Class.of(this)
    }

    public funcLong(v: long): Class {
        arktest.assertEQ(v, ReferenceValueHolder.longReference)
        return Class.of(this)
    }

    public funcFloat(v: float): Class {
        arktest.assertEQ(v, ReferenceValueHolder.floatReference)
        return Class.of(this)
    }

    public funcDouble(v: double): Class {
        arktest.assertEQ(v, ReferenceValueHolder.doubleReference)
        return Class.of(this)
    }

    public funcOptional(v: Object, v1?: Object, v2?: Object, v3?: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        if (v1 != undefined) {
            arktest.assertEQ(v1, ReferenceValueHolder.objReference)
        }
        if (v2 != undefined) {
            arktest.assertEQ(v2, ReferenceValueHolder.objReference)
        }
        if (v3 != undefined) {
            arktest.assertEQ(v3, ReferenceValueHolder.objReference)
        }
        return Class.of(this)
    }

    public funcRestParams(...v: Object[]): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objArrReference)
        return Class.of(this)
    }

    public funcRestParamsObj(v?: Object, ...v1: Object[]): Class {
        if (v != undefined) {
            arktest.assertEQ(v, ReferenceValueHolder.objReference)
        }
        arktest.assertEQ(v1, ReferenceValueHolder.objArrReference)
        return Class.of(this)
    }

    public get getter(): Class { return Class.of(this) }
    public set setter(v: Object) {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
    }

    private static funcTestStaticPrivate(): Class { return Class.of(new BaseInvokeTester()) }

    public static funcTestStatic(): Class { return BaseInvokeTester.funcTestStaticPrivate() }

    public static funcEmpty(): Class { return Class.of(new BaseInvokeTester()) }

    public static funcObj(v: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcBoolean(v: boolean): Class {
        arktest.assertEQ(v, ReferenceValueHolder.booleanReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcByte(v: byte): Class {
        arktest.assertEQ(v, ReferenceValueHolder.byteReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcChar(v: char): Class {
        arktest.assertEQ(v, ReferenceValueHolder.charReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcShort(v: short): Class {
        arktest.assertEQ(v, ReferenceValueHolder.shortReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcInt(v: int): Class {
        arktest.assertEQ(v, ReferenceValueHolder.intReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcLong(v: long): Class {
        arktest.assertEQ(v, ReferenceValueHolder.longReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcFloat(v: float): Class {
        arktest.assertEQ(v, ReferenceValueHolder.floatReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcDouble(v: double): Class {
        arktest.assertEQ(v, ReferenceValueHolder.doubleReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcOptional(v: Object, v1?: Object, v2?: Object, v3?: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        if (v1 != undefined) {
            arktest.assertEQ(v1, ReferenceValueHolder.objReference)
        }
        if (v2 != undefined) {
            arktest.assertEQ(v2, ReferenceValueHolder.objReference)
        }
        if (v3 != undefined) {
            arktest.assertEQ(v3, ReferenceValueHolder.objReference)
        }
        return Class.of(new BaseInvokeTester())
    }

    public static funcRestParams(...v: Object[]): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objArrReference)
        return Class.of(new BaseInvokeTester())
    }

    public static funcRestParamsObj(v?: Object, ...v1: Object[]): Class {
        if (v != undefined) {
            arktest.assertEQ(v, ReferenceValueHolder.objReference)
        }
        arktest.assertEQ(v1, ReferenceValueHolder.objArrReference)
        return Class.of(new BaseInvokeTester())
    }

    public static get staticGetter(): Class { return Class.of(new BaseInvokeTester()) }
    public static set staticSetter(v: Object) {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
    }
}

class DerivedInvokeTester extends BaseInvokeTester {
    constructor() {}

    public funcEmpty(): Class { return Class.of(this) }

    public funcObj(v: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        return Class.of(this)
    }

    public funcBoolean(v: boolean): Class {
        arktest.assertEQ(v, ReferenceValueHolder.booleanReference)
        return Class.of(this)
    }

    public funcByte(v: byte): Class {
        arktest.assertEQ(v, ReferenceValueHolder.byteReference)
        return Class.of(this)
    }

    public funcChar(v: char): Class {
        arktest.assertEQ(v, ReferenceValueHolder.charReference)
        return Class.of(this)
    }

    public funcShort(v: short): Class {
        arktest.assertEQ(v, ReferenceValueHolder.shortReference)
        return Class.of(this)
    }

    public funcInt(v: int): Class {
        arktest.assertEQ(v, ReferenceValueHolder.intReference)
        return Class.of(this)
    }

    public funcLong(v: long): Class {
        arktest.assertEQ(v, ReferenceValueHolder.longReference)
        return Class.of(this)
    }

    public funcFloat(v: float): Class {
        arktest.assertEQ(v, ReferenceValueHolder.floatReference)
        return Class.of(this)
    }

    public funcDouble(v: double): Class {
        arktest.assertEQ(v, ReferenceValueHolder.doubleReference)
        return Class.of(this)
    }

    public funcOptional(v: Object, v1?: Object, v2?: Object, v3?: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        if (v1 != undefined) {
            arktest.assertEQ(v1, ReferenceValueHolder.objReference)
        }
        if (v2 != undefined) {
            arktest.assertEQ(v2, ReferenceValueHolder.objReference)
        }
        if (v3 != undefined) {
            arktest.assertEQ(v3, ReferenceValueHolder.objReference)
        }
        return Class.of(this)
    }

    public funcRestParams(...v: Object[]): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objArrReference)
        return Class.of(this)
    }

    public funcRestParamsObj(v?: Object, ...v1: Object[]): Class {
        if (v != undefined) {
            arktest.assertEQ(v, ReferenceValueHolder.objReference)
        }
        arktest.assertEQ(v1, ReferenceValueHolder.objArrReference)
        return Class.of(this)
    }

    public get getter(): Class { return Class.of(this) }
    public set setter(v: Object) {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
    }

    public notVirtualDerived(): void {}

    private static funcTestStaticPrivate(): Class { return Class.of(new DerivedInvokeTester()) }

    public static funcTestStatic(): Class { return DerivedInvokeTester.funcTestStaticPrivate() }

    public static funcEmpty(): Class { return Class.of(new DerivedInvokeTester()) }

    public static funcObj(v: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcBoolean(v: boolean): Class {
        arktest.assertEQ(v, ReferenceValueHolder.booleanReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcByte(v: byte): Class {
        arktest.assertEQ(v, ReferenceValueHolder.byteReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcChar(v: char): Class {
        arktest.assertEQ(v, ReferenceValueHolder.charReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcShort(v: short): Class {
        arktest.assertEQ(v, ReferenceValueHolder.shortReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcInt(v: int): Class {
        arktest.assertEQ(v, ReferenceValueHolder.intReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcLong(v: long): Class {
        arktest.assertEQ(v, ReferenceValueHolder.longReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcFloat(v: float): Class {
        arktest.assertEQ(v, ReferenceValueHolder.floatReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcDouble(v: double): Class {
        arktest.assertEQ(v, ReferenceValueHolder.doubleReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcOptional(v: Object, v1?: Object, v2?: Object, v3?: Object): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
        if (v1 != undefined) {
            arktest.assertEQ(v1, ReferenceValueHolder.objReference)
        }
        if (v2 != undefined) {
            arktest.assertEQ(v2, ReferenceValueHolder.objReference)
        }
        if (v3 != undefined) {
            arktest.assertEQ(v3, ReferenceValueHolder.objReference)
        }
        return Class.of(new DerivedInvokeTester())
    }

    public static funcRestParams(...v: Object[]): Class {
        arktest.assertEQ(v, ReferenceValueHolder.objArrReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static funcRestParamsObj(v?: Object, ...v1: Object[]): Class {
        if (v != undefined) {
            arktest.assertEQ(v, ReferenceValueHolder.objReference)
        }
        arktest.assertEQ(v1, ReferenceValueHolder.objArrReference)
        return Class.of(new DerivedInvokeTester())
    }

    public static get staticGetter(): Class { return Class.of(new DerivedInvokeTester()) }
    public static set staticSetter(v: Object) {
        arktest.assertEQ(v, ReferenceValueHolder.objReference)
    }
}

class PrivateCtor {
    private constructor() {}
}

class ProtectedCtor {
    protected constructor() {}
}

function createInvokeExpectedErrorMsg(expectedType: Class, actualType: Class): string {
    return `Object type [${actualType.getName()}] is not compatible with method owner type [${expectedType.getName()}]`
}

function invokeByBaseTester(tester: BaseInvokeTester) {
    let testerType = Class.of(tester)

    arktest.assertEQ(testerType.getInstanceMethod("funcEmpty")!.invoke(tester), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcObj")!.invoke(tester, [ ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcBoolean")!.invoke(tester, [ ReferenceValueHolder.booleanReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcByte")!.invoke(tester, [ ReferenceValueHolder.byteReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcChar")!.invoke(tester, [ ReferenceValueHolder.charReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcShort")!.invoke(tester, [ ReferenceValueHolder.shortReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcInt")!.invoke(tester, [ ReferenceValueHolder.intReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcLong")!.invoke(tester, [ ReferenceValueHolder.longReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcFloat")!.invoke(tester, [ ReferenceValueHolder.floatReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcDouble")!.invoke(tester, [ ReferenceValueHolder.doubleReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, undefined, undefined, undefined ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, undefined, undefined ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, undefined, undefined, ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, undefined, ReferenceValueHolder.objReference, undefined ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcRestParams")!.invoke(tester, [ ReferenceValueHolder.objArrReference ]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcRestParamsObj")!.invoke(tester, [ReferenceValueHolder.objReference, ReferenceValueHolder.objArrReference]), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("funcRestParamsObj")!.invoke(tester, [undefined, ReferenceValueHolder.objArrReference]), testerType)

    arktest.assertEQ(testerType.getInstanceMethod("%%get-getter")!.invoke(tester), testerType)
    arktest.assertEQ(testerType.getInstanceMethod("%%set-setter")!.invoke(tester, [ ReferenceValueHolder.objReference ]), undefined)

    arktest.expectError(() => {
        testerType.getInstanceMethod("funcEmpty")!.invoke(new Object())
    }, new TypeError(createInvokeExpectedErrorMsg(testerType, Class.from<Object>())))

    let notVirtualDerived = testerType.getInstanceMethod("notVirtualDerived")
    if (notVirtualDerived == undefined) {
        arktest.assertEQ(testerType, Class.of(new BaseInvokeTester()))
    } else {
        arktest.assertEQ(notVirtualDerived.invoke(tester), undefined)
        arktest.assertEQ(testerType, Class.of(new DerivedInvokeTester))
    }

    let methods = testerType.getStaticMethods();
    arktest.assertEQ(testerType.getStaticMethod("funcEmpty")!.invoke(), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcObj")!.invoke([ ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcBoolean")!.invoke([ ReferenceValueHolder.booleanReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcByte")!.invoke([ ReferenceValueHolder.byteReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcChar")!.invoke([ ReferenceValueHolder.charReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcShort")!.invoke([ ReferenceValueHolder.shortReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcInt")!.invoke([ ReferenceValueHolder.intReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcLong")!.invoke([ ReferenceValueHolder.longReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcFloat")!.invoke([ ReferenceValueHolder.floatReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcDouble")!.invoke([ ReferenceValueHolder.doubleReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcOptional")!.invoke([ ReferenceValueHolder.objReference, undefined, undefined, undefined ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcOptional")!.invoke([ ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, undefined, undefined ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcOptional")!.invoke([ ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcOptional")!.invoke([ ReferenceValueHolder.objReference, undefined, undefined, ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcOptional")!.invoke([ ReferenceValueHolder.objReference, undefined, ReferenceValueHolder.objReference, undefined ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcRestParams")!.invoke([ ReferenceValueHolder.objArrReference ]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcRestParamsObj")!.invoke([ReferenceValueHolder.objReference, ReferenceValueHolder.objArrReference]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("funcRestParamsObj")!.invoke([undefined, ReferenceValueHolder.objArrReference]), testerType)
    arktest.assertEQ(testerType.getStaticMethod("%%get-staticGetter")!.invoke(), testerType)
    arktest.assertEQ(testerType.getStaticMethod("%%set-staticSetter")!.invoke([ ReferenceValueHolder.objReference ]), undefined)
    arktest.assertEQ(testerType.getStaticMethod("funcTestStatic")!.invoke(), testerType)

    arktest.expectError(() => {
        testerType.getInstanceMethod("funcEmpty")!.invoke(new Object())
    }, new TypeError(createInvokeExpectedErrorMsg(testerType, Class.from<Object>())))

    let constructors = testerType.getConstructors()
    let constructorsNum = constructors.length
    if (tester instanceof DerivedInvokeTester) {
        arktest.assertEQ(constructorsNum, 1)
    } else {
        arktest.assertEQ(constructorsNum, 4)
    }
    let created = 0

    for (let c of constructors) {
        if (c.getParametersNum() == 0) {
            let inst = c.createInstance()!
            arktest.assertNE(inst, undefined)
            inst = inst as BaseInvokeTester
            arktest.assertEQ(Class.ofAny!(inst), testerType)
            arktest.assertEQ(inst.objVal, undefined)
            arktest.assertEQ(inst.intVal, undefined)
            ++created
        } else if (c.getParametersNum() == 1) {
            let inst = c.createInstance([ ReferenceValueHolder.objReference ])!
            arktest.assertNE(inst, undefined)
            inst = inst as BaseInvokeTester
            arktest.assertEQ(Class.ofAny!(inst), testerType)
            arktest.assertEQ(inst.objVal, ReferenceValueHolder.objReference)
            arktest.assertEQ(inst.intVal, undefined)
            ++created
        } else if (c.getParametersNum() == 2) {
            let inst = c.createInstance([ ReferenceValueHolder.objReference, ReferenceValueHolder.intReference ])!
            arktest.assertNE(inst, undefined)
            inst = inst as BaseInvokeTester
            arktest.assertEQ(Class.ofAny!(inst), testerType)
            arktest.assertEQ(inst.objVal, ReferenceValueHolder.objReference)
            arktest.assertEQ(inst.intVal, ReferenceValueHolder.intReference)
            ++created
        } else if (c.getParametersNum() == 3) {
            let inst = c.createInstance([ ReferenceValueHolder.intReference, ReferenceValueHolder.intReference, ReferenceValueHolder.intReference ])!
            arktest.assertNE(inst, undefined)
            inst = inst as BaseInvokeTester
            arktest.assertEQ(Class.ofAny!(inst), testerType)
            arktest.assertEQ(inst.objVal, undefined)
            arktest.assertEQ(inst.intVal, ReferenceValueHolder.intReference + ReferenceValueHolder.intReference + ReferenceValueHolder.intReference)
            ++created
        } else {
            // Unreachable
            arktest.expectError(() => { throw new Error() });
        }
    }

    arktest.assertEQ(constructorsNum, created)
}

function invokeByInterface(tester: InvokeTester, ifaceType: Class) {
    let testerType = Class.of(tester)
    arktest.assertTrue(testerType.isSubtypeOf(ifaceType))

    arktest.assertEQ(ifaceType.getInstanceMethod("funcEmpty")!.invoke(tester), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcObj")!.invoke(tester, [ ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcBoolean")!.invoke(tester, [ ReferenceValueHolder.booleanReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcByte")!.invoke(tester, [ ReferenceValueHolder.byteReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcChar")!.invoke(tester, [ ReferenceValueHolder.charReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcShort")!.invoke(tester, [ ReferenceValueHolder.shortReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcInt")!.invoke(tester, [ ReferenceValueHolder.intReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcLong")!.invoke(tester, [ ReferenceValueHolder.longReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcFloat")!.invoke(tester, [ ReferenceValueHolder.floatReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcDouble")!.invoke(tester, [ ReferenceValueHolder.doubleReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, undefined, undefined, undefined ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, undefined, undefined ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, ReferenceValueHolder.objReference, ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, undefined, undefined, ReferenceValueHolder.objReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcOptional")!.invoke(tester, [ ReferenceValueHolder.objReference, undefined, ReferenceValueHolder.objReference, undefined ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcRestParams")!.invoke(tester, [ ReferenceValueHolder.objArrReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcRestParamsObj")!.invoke(tester, [ ReferenceValueHolder.objReference, ReferenceValueHolder.objArrReference ]), testerType)
    arktest.assertEQ(ifaceType.getInstanceMethod("funcRestParamsObj")!.invoke(tester, [ undefined, ReferenceValueHolder.objArrReference ]), testerType)

    arktest.expectError(() => {
        ifaceType.getInstanceMethod("funcEmpty")!.invoke(new Object())
    }, new TypeError(createInvokeExpectedErrorMsg(ifaceType, Class.from<Object>())))

    arktest.expectError(() => {
        ifaceType.getInstanceMethod("funcBoolean")!.invoke(tester, [ ReferenceValueHolder.floatReference ])
    }, new TypeError("Invalid operand type: Lstd/core/Float;"))

    let notVirtualDerived = testerType.getInstanceMethod("notVirtualDerived")
    if (notVirtualDerived == undefined) {
        arktest.assertEQ(testerType, Class.of(new BaseInvokeTester()))
    } else {
        arktest.assertEQ(notVirtualDerived.invoke(tester), undefined)
        arktest.assertEQ(testerType, Class.of(new DerivedInvokeTester))
    }
}

function testReflectMethodInvoke(): void {
    let baseTester = new BaseInvokeTester()
    let derivedTester = new DerivedInvokeTester()
    let ifaceType = Class.of(baseTester).getInterfaces()[0]

    invokeByBaseTester(baseTester)
    invokeByBaseTester(derivedTester)
    invokeByInterface(baseTester, ifaceType)
    invokeByInterface(derivedTester, ifaceType)
}

function testReflectMethod(): void {
    let typePoint = Class.of(new Point())
    let typeHWriter = Class.of(new HTMLWriter(10))
    let typeWriter = typeHWriter.getSuper()!

    let serializeMethod = typePoint.getInstanceMethod("serialize")!
    let addMethod = typePoint.getInstanceMethod("add")!
    let unionMethod = typePoint.getInstanceMethod("testUnion")!
    let getter = typeHWriter.getInstanceMethod("%%get-state")!
    let setter = typeHWriter.getInstanceMethod("%%set-state")!
    let optionalParamsMethod = typePoint.getInstanceMethod("optionalParams")!

    arktest.assertEQ(serializeMethod.getReturnType(), Class.of(""))
    arktest.assertEQ(serializeMethod.getOwner(), typePoint)
    arktest.assertEQ(serializeMethod.getParametersNum(), 0)
    arktest.assertEQ(addMethod.getParametersNum(), 1)
    arktest.assertEQ(addMethod.getParameterType(0)!, typePoint)
    arktest.assertEQ(addMethod.getReturnType(), typePoint)
    arktest.assertEQ(addMethod.getOwner(), typePoint)
    arktest.assertEQ(getter.getParametersNum(), 0)
    arktest.assertEQ(getter.getReturnType(), Class.PRIMITIVE_INT)
    arktest.assertEQ(getter.getOwner(), typeHWriter)
    arktest.assertEQ(setter.getParametersNum(), 1)
    arktest.assertEQ(setter.getParameterType(0)!, Class.PRIMITIVE_INT)
    arktest.assertEQ(setter.getReturnType().getName(), "void")
    arktest.assertEQ(setter.getOwner(), typeHWriter)
}

function testMethodAttributes(): void {
    let typePoint = Class.of(new Point())
    let typeHWriter = Class.of(new HTMLWriter(10))
    let typeWriter = typeHWriter.getSuper()!

    let getterMethod = typeHWriter.getInstanceMethod("%%get-state")!
    let setterMethod = typeHWriter.getInstanceMethod("%%set-state")!
    let abstractWriteMethod = typeWriter.getInstanceMethod("write")!
    let overrideWriteMethod = typeHWriter.getInstanceMethod("write")!
    let closeMethod = typeWriter.getInstanceMethod("close")!
    let finalMethod = typeHWriter.getInstanceMethod("finalMethod")!
    let nativeMethod = typeHWriter.getInstanceMethod("nativeMethod")!
    let asyncMethod = typeHWriter.getInstanceMethod("asyncMethod")!
    let constructors = typeHWriter.getConstructors()
    let constructorOfClass = constructors[0]

    let isInherited = (m: reflect.InstanceMethod, type: Class): boolean => {
        return m.getOwner().getName() != type.getName()
    }
    let isOverrided = (m: reflect.InstanceMethod, type: Class): boolean => {
        if (m.getOwner().getName() != type.getName()) {
            return false
        }
        let baseType = type.getSuper()!
        return baseType.getInstanceMethod(m.getName()) != undefined
    }

    arktest.assertEQ(getterMethod.getName(), "%%get-state")
    arktest.assertFalse(isInherited(getterMethod, typeHWriter))
    arktest.assertFalse(isOverrided(getterMethod, typeHWriter))
    arktest.assertFalse(getterMethod.isAbstract())
    arktest.assertTrue(getterMethod.isGetter())
    arktest.assertFalse(getterMethod.isSetter())
    arktest.assertFalse(getterMethod.isFinal())
    arktest.assertFalse(getterMethod.isNative())
    arktest.assertFalse(getterMethod.isAsync())
    arktest.assertTrue(getterMethod.isPublic())
    arktest.assertFalse(getterMethod.isProtected())
    arktest.assertFalse(getterMethod.isPrivate())
    arktest.assertEQ(setterMethod.getName(), "%%set-state")
    arktest.assertFalse(isInherited(setterMethod, typeHWriter))
    arktest.assertFalse(isOverrided(setterMethod, typeHWriter))
    arktest.assertFalse(setterMethod.isAbstract())
    arktest.assertFalse(setterMethod.isGetter())
    arktest.assertTrue(setterMethod.isSetter())
    arktest.assertFalse(setterMethod.isFinal())
    arktest.assertFalse(setterMethod.isNative())
    arktest.assertFalse(setterMethod.isAsync())
    arktest.assertTrue(setterMethod.isPublic())
    arktest.assertFalse(setterMethod.isProtected())
    arktest.assertFalse(setterMethod.isPrivate())
    arktest.assertEQ(overrideWriteMethod.getName(), "write")
    arktest.assertFalse(isInherited(overrideWriteMethod, typeHWriter))
    arktest.assertTrue(isOverrided(overrideWriteMethod, typeHWriter))
    arktest.assertFalse(overrideWriteMethod.isAbstract())
    arktest.assertFalse(overrideWriteMethod.isGetter())
    arktest.assertFalse(overrideWriteMethod.isSetter())
    arktest.assertFalse(overrideWriteMethod.isFinal())
    arktest.assertFalse(overrideWriteMethod.isNative())
    arktest.assertFalse(overrideWriteMethod.isAsync())
    arktest.assertTrue(overrideWriteMethod.isPublic())
    arktest.assertFalse(overrideWriteMethod.isProtected())
    arktest.assertFalse(overrideWriteMethod.isPrivate())
    arktest.assertEQ(abstractWriteMethod.getName(), "write")
    arktest.assertFalse(isInherited(abstractWriteMethod, typeWriter))
    arktest.assertFalse(isOverrided(abstractWriteMethod, typeWriter))
    arktest.assertTrue(abstractWriteMethod.isAbstract())
    arktest.assertFalse(abstractWriteMethod.isGetter())
    arktest.assertFalse(abstractWriteMethod.isSetter())
    arktest.assertFalse(abstractWriteMethod.isFinal())
    arktest.assertFalse(abstractWriteMethod.isNative())
    arktest.assertFalse(abstractWriteMethod.isAsync())
    arktest.assertTrue(abstractWriteMethod.isPublic())
    arktest.assertFalse(abstractWriteMethod.isProtected())
    arktest.assertFalse(abstractWriteMethod.isPrivate())
    arktest.assertEQ(closeMethod.getName(), "close")
    arktest.assertTrue(isInherited(closeMethod, typeHWriter))
    arktest.assertFalse(isOverrided(closeMethod, typeHWriter))
    arktest.assertFalse(closeMethod.isAbstract())
    arktest.assertFalse(closeMethod.isGetter())
    arktest.assertFalse(closeMethod.isSetter())
    arktest.assertFalse(closeMethod.isFinal())
    arktest.assertFalse(closeMethod.isNative())
    arktest.assertFalse(closeMethod.isAsync())
    arktest.assertTrue(closeMethod.isPublic())
    arktest.assertFalse(closeMethod.isProtected())
    arktest.assertFalse(closeMethod.isPrivate())
    arktest.assertEQ(finalMethod.getName(), "finalMethod")
    arktest.assertFalse(isInherited(finalMethod, typeHWriter))
    arktest.assertFalse(isOverrided(finalMethod, typeHWriter))
    arktest.assertFalse(finalMethod.isAbstract())
    arktest.assertFalse(finalMethod.isGetter())
    arktest.assertFalse(finalMethod.isSetter())
    arktest.assertTrue(finalMethod.isFinal())
    arktest.assertFalse(finalMethod.isNative())
    arktest.assertFalse(finalMethod.isAsync())
    arktest.assertTrue(finalMethod.isPublic())
    arktest.assertFalse(finalMethod.isProtected())
    arktest.assertFalse(finalMethod.isPrivate())
    arktest.assertEQ(nativeMethod.getName(), "nativeMethod")
    arktest.assertFalse(isInherited(nativeMethod, typeHWriter))
    arktest.assertFalse(isOverrided(nativeMethod, typeHWriter))
    arktest.assertFalse(nativeMethod.isAbstract())
    arktest.assertFalse(nativeMethod.isGetter())
    arktest.assertFalse(nativeMethod.isSetter())
    arktest.assertFalse(nativeMethod.isFinal())
    arktest.assertTrue(nativeMethod.isNative())
    arktest.assertFalse(nativeMethod.isAsync())
    arktest.assertTrue(nativeMethod.isPublic())
    arktest.assertFalse(nativeMethod.isProtected())
    arktest.assertFalse(nativeMethod.isPrivate())
    arktest.assertEQ(asyncMethod.getName(), "asyncMethod")
    arktest.assertFalse(isInherited(asyncMethod, typeHWriter))
    arktest.assertFalse(isOverrided(asyncMethod, typeHWriter))
    arktest.assertFalse(asyncMethod.isAbstract())
    arktest.assertFalse(asyncMethod.isGetter())
    arktest.assertFalse(asyncMethod.isSetter())
    arktest.assertFalse(asyncMethod.isFinal())
    arktest.assertFalse(asyncMethod.isNative())
    arktest.assertTrue(asyncMethod.isAsync())
    arktest.assertTrue(asyncMethod.isPublic())
    arktest.assertFalse(asyncMethod.isProtected())
    arktest.assertFalse(asyncMethod.isPrivate())
    arktest.assertEQ(constructorOfClass.getName(), "constructor")
    arktest.assertFalse(constructorOfClass.isNative())
    arktest.assertTrue(constructorOfClass.isPublic())
    arktest.assertFalse(constructorOfClass.isProtected())
    arktest.assertFalse(constructorOfClass.isPrivate())
}

function testParameters() {
    let typePoint = Class.of(new Point())

    let optionalParamsMethod = typePoint.getInstanceMethod("optionalParams")!
    let restParamsMethod = typePoint.getInstanceMethod("restParams")!
    let optReadonlyRestParamsMethod = typePoint.getInstanceMethod("optReadonlyRestParams")!

    let res = 0
    let paramsOpt = optionalParamsMethod.getParametersTypes()
    let paramsRest = restParamsMethod.getParametersTypes()
    arktest.assertEQ(optionalParamsMethod.getParametersNum(), 3)
    arktest.assertEQ(restParamsMethod.getParametersNum(), 2)
    arktest.assertEQ(optReadonlyRestParamsMethod.getParametersNum(), 4)
}

function testInstanceMethodEquals(): void {
    const pointCls = Class.of(new Point())

    const addMeth1 = pointCls.getInstanceMethod('add')!
    const addMeth2 = pointCls.getInstanceMethod('add')!

    arktest.assertFalse(addMeth1 == addMeth2)
    arktest.assertTrue(addMeth1.equals(addMeth2))

    const meth = pointCls.getInstanceMethod('serialize')!

    arktest.assertFalse(addMeth1 == meth)
    arktest.assertFalse(addMeth1.equals(meth))
}

function testOverriddenInstanceMethodEquals(): void {
    const baseCls = Class.of(new Base())
    const derivedCls = Class.of(new Derived())

    const baseFoo = baseCls.getInstanceMethod('foo')!
    const derivedFoo = derivedCls.getInstanceMethod('foo')!

    arktest.assertTrue(baseFoo.equals(baseFoo))
    arktest.assertTrue(derivedFoo.equals(derivedFoo))
    arktest.assertFalse(baseFoo.equals(derivedFoo))
    arktest.assertFalse(derivedFoo.equals(baseFoo))

    const baseBar = baseCls.getInstanceMethod('bar')!
    const derivedBar = baseCls.getInstanceMethod('bar')!

    arktest.assertTrue(baseBar.equals(baseBar))
    arktest.assertTrue(derivedBar.equals(derivedBar))
    arktest.assertTrue(baseBar.equals(derivedBar))
    arktest.assertTrue(derivedBar.equals(baseBar))
}

function testConstructorEquals(): void {
    const writerCls = Class.of(new HTMLWriter(0))

    const ctor1 = writerCls.getConstructors()[0]
    const ctor2 = writerCls.getConstructors()[0]

    arktest.assertFalse(ctor1 == ctor2)
    arktest.assertTrue(ctor1.equals(ctor2))
}

function testReturn() {
    let typePoint = Class.of(new Point())
    let typeHWriter = Class.of(new HTMLWriter(10))
    let typeWriter = typeHWriter.getSuper()!

    let getterMethod = typeHWriter.getInstanceMethod("%%get-state")!
    let setterMethod = typeHWriter.getInstanceMethod("%%set-state")!
    let abstractWriteMethod = typeWriter.getInstanceMethod("write")!
    let overrideWriteMethod = typeHWriter.getInstanceMethod("write")!
    let closeMethod = typeWriter.getInstanceMethod("close")!
    let finalMethod = typeHWriter.getInstanceMethod("finalMethod")!
    let nativeMethod = typeHWriter.getInstanceMethod("nativeMethod")!
    let constructors = typeHWriter.getConstructors()
    let constructorOfClass = constructors[0]

    arktest.assertEQ(getterMethod.getReturnType(), Class.PRIMITIVE_INT)
    arktest.assertEQ(setterMethod.getReturnType(), Class.PRIMITIVE_VOID)
    arktest.assertEQ(abstractWriteMethod.getReturnType(), Class.PRIMITIVE_INT)
    arktest.assertEQ(overrideWriteMethod.getReturnType(), Class.PRIMITIVE_INT)
    arktest.assertEQ(closeMethod.getReturnType(), Class.PRIMITIVE_VOID)
    arktest.assertEQ(finalMethod.getReturnType(), Class.PRIMITIVE_VOID)
    arktest.assertEQ(nativeMethod.getReturnType(), Class.PRIMITIVE_BOOLEAN)
    arktest.assertEQ(constructorOfClass.getReturnType(), Class.PRIMITIVE_VOID)
}

function testNonPublicCtor(): void {
    arktest.expectError(() => {
        const instance = Class.from<PrivateCtor>().createInstance()
    }, "No default public constructor in LReflectMethodTest/PrivateCtor;")

    arktest.expectError(() => {
        const instance = Class.from<ProtectedCtor>().createInstance()
    }, "No default public constructor in LReflectMethodTest/ProtectedCtor;")
}

function main() {
    const suite = new arktest.ArkTestsuite('Reflect Method tests')
    suite.addTest('Test Reflect Method base APIs', testReflectMethod)
    suite.addTest('Test Reflect Method attributes APIs', testMethodAttributes)
    suite.addTest('Test Reflect Method Parameters APIs', testParameters)
    suite.addTest('Test Reflect Method Return APIs', testReturn)
    suite.addTest('Test Reflect InstanceMethod.equals()', testInstanceMethodEquals)
    suite.addTest('Test Reflect Constructor.equals()', testConstructorEquals)
    suite.addTest('Test Reflect overridden InstanceMethod.equals()', testOverriddenInstanceMethodEquals)
    suite.addTest('Test Reflect Method.invoke() and Constructor.createInstance()', testReflectMethodInvoke)
    suite.addTest('Test Constructor.createInstance() for non-public constructor', testNonPublicCtor)
    return suite.run()
}
