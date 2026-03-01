/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

function test(result: boolean, name: String): int {
    if (result) {
        return 0
    }
    return 1
}

interface Serializable {
    serialize(): String;
}

class Point implements Serializable {
    protected x: double
    y: double

    constructor(x: double, y: double) {
        this.x = x
        this.y = y
    }

    constructor() {
        this(0.0, 0.0)
    }

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
    private state: int
    static ZERO: double = 0.0
    constructor(x: double, y: double, z: double) {
        super(x, y)
        this.z = z
        this.state = 0
    }
    constructor(state: int) {
        this(0, 0, 0)
        this.state = 1
    }
}

abstract class Writer {
    abstract write(): int
}

final class HTMLWriter extends Writer {
    private state: int
    constructor(state: int) {
        this.state = state
    }

    public getState(): int {
        return this.state
    }

    override write(): int {
        return 0
    }
}

function findMethodsByNameArray(type: ClassType, names: Array<string>): Array<Method | null> {
    const methods = new Array<Method | null>()
    const namesLength: int = names.length

    const foundMethods = new Map<string, Method>()
    const methodsNum = type.getMethodsNum()
    for (let i = methodsNum - 1; i >= 0; i--) {
        const method = type.getMethod(i)
        for (let nameIdx = 0; nameIdx < namesLength; nameIdx++) {
            if (names[nameIdx] == method.getName()) {
                foundMethods.set(method.getName(), method)
            }
        }
    }

    for (let methodIdx = 0; methodIdx < namesLength; methodIdx++) {
        const methodName = names[methodIdx]
        const method = foundMethods.get(methodName)
        methods.push(method === undefined ? null : method)
    }

    return methods
}

function findMethodsByName(type: ClassType, ...names: Array<string>): Array<Method | null> {
    const methodNames = new Array<string>()
    methodNames.push(...names)
    return findMethodsByNameArray(type, methodNames)
}

function testFields(ct: ClassType): int {
    let failures = 0
    for (let i = 0; i < ct.getFieldsNum(); ++i) {
        let field = ct.getField(i);
        failures += test(ct.getFieldByName(field.getName()).equals(field), "   " + field.getName() + " field")
    }
    return failures
}

function testGetFieldError(ct: ClassType): int {
    let failures = 0
    try {
        ct.getField(1000)
        failures += 1
    } catch (ignored) {
        if (!(ignored instanceof TypeError)) {
            failures += 1
        }
    }
    try {
        ct.getOwnField(1000)
        failures += 1
    } catch (ignored) {
        if (!(ignored instanceof TypeError)) {
            failures += 1
        }
    }
    try {
        ct.getFieldByName("nonExistingField")
        failures += 1
    } catch (ignored) {
        if (!(ignored instanceof TypeError)) {
            failures += 1
        }
    }
    return failures
}

function testGetMethodError(ct: ClassType): int {
    let failures = 0
    try {
        ct.getMethod(1000)
        failures += 1
    } catch (ignored) {
        if (!(ignored instanceof TypeError)) {
            failures += 1
        }
    }
    try {
        ct.getConstructor(1000)
        failures += 1
    } catch (ignored) {
        if (!(ignored instanceof TypeError)) {
            failures += 1
        }
    }
    return failures
}

function testGetInterfaceError(ct: ClassType): int {
    let failures = 0
    try {
        ct.getInterface(1000)
        failures += 1
    } catch (ignored) {
        if (!(ignored instanceof TypeError)) {
            failures += 1
        }
    }
    return failures
}

function testAllFieldsAreOwn(ct: ClassType): int {
    let failures = test(ct.getOwnFieldsNum() == ct.getFieldsNum(), "   OwnFieldsNum == FieldsNum")
    for (let i = 0; i < ct.getOwnFieldsNum(); ++i) {
        let field = ct.getOwnField(i);
        failures += test(ct.getFieldByName(field.getName()).equals(field), "  " + field.getName())
    }
    return failures
}

function testClassExtendsObject(ct: ClassType): int {
    let failures = 0
    let objectMethodsNum = ObjectType.getMethodsNum()
    const objectMethodNames = new Array<string>()
    for (let i = 0; i < objectMethodsNum; ++i) {
        let objectMethodName = ObjectType.getMethod(i).getName()
        objectMethodNames.push(objectMethodName)
    }

    const methods = findMethodsByNameArray(ct, objectMethodNames)
    for (let i = 0; i < objectMethodsNum; ++i) {
        const objectMethodName = objectMethodNames[i]
        failures += test(methods[i] != null,  "       " + objectMethodName + " method")
    }

    return failures
}

function testPoint(): int {
    let pt = Type.of(new Point()) as ClassType

    const pointMethods = findMethodsByName(pt, "add", "serialize")
    const addMethod = pointMethods[0]
    const serializeMethod = pointMethods[1]

    return test(pt.getName() == "TypeClassTypeTest.Point",                  "ClassName: Point") +
            test(pt.hasEmptyConstructor(),                                  "   doesn't have empty constuctor") +
            test(pt.getBaseType().equals(ObjectType),                       "   base type is ObjectType") +
            test(pt.getInterfacesNum() == 1,                                "   1 Interface:") +
            test(pt.getInterface(0).getName() == "TypeClassTypeTest.Serializable",       "        Serializable interface") +
            test(pt.getFieldsNum() == 2,                                    "   2 Fields:") +
            test(pt.hasField("x"),                                          "       x field") +
            test(pt.hasField("y"),                                          "       y field") +
            testAllFieldsAreOwn(pt) +
            test(pt.getMethodsNum() == 2 + ObjectType.getMethodsNum(),      "   2 Methods: ") +
            testClassExtendsObject(pt) +
            test(addMethod != null,                       "       Point.add method") +
            test(serializeMethod != null,                 "       Serializable.serialize method") +
            test(pt.getConstructorsNum() == 2,                              "   2 Constructor:") +
            test(pt.getConstructor(0).getName() == "constructor",           "       default") +
            test(pt.getConstructor(1).getName() == "constructor",           "       default") +
            test(!pt.isFinal(),                                             "   not final") +
            testFields(pt) +
            testGetFieldError(pt) +
            testGetMethodError(pt) +
            testGetInterfaceError(pt)
}

function testPoint3D(): int {
    let pt = Type.of(new Point3D(1.0, 2.0, 3.0)) as ClassType

    const pointMethods = findMethodsByName(pt, "add", "serialize")
    const addMethod = pointMethods[0]
    const serializeMethod = pointMethods[1]

    return test(pt.getName() == "TypeClassTypeTest.Point3D",                "ClassName: Point3D") +
            test(!pt.hasEmptyConstructor(),                                 "   doesn't have empty constructor") +
            test(pt.getBaseType().getName() == "TypeClassTypeTest.Point",                "   base type is Point") +
            test(pt.getInterfacesNum() == 0,                                "   0 Interfaces") +
            test(pt.getFieldsNum() == 5,                                    "   5 Fields:") +
            test(pt.hasField("x"),                                          "       x field") +
            test(pt.hasField("y"),                                          "       y field") +
            test(pt.hasField("z"),                                          "       z field") +
            test(pt.hasField("state"),                                      "       state field") +
            test(pt.hasField("ZERO"),                                       "       ZERO field") +
            test(pt.getOwnFieldsNum() == 3,                                 "   3 Own Fields:") +
            test(pt.getOwnField(0).equals(pt.getFieldByName("ZERO")),       "       ZERO field") +
            test(pt.getOwnField(1).equals(pt.getFieldByName("z")),          "       z field") +
            test(pt.getOwnField(2).equals(pt.getFieldByName("state")),      "       state field") +
            test(pt.getMethodsNum() == 2 + ObjectType.getMethodsNum(),      "   2 Methods:") +
            testClassExtendsObject(pt) +
            test(addMethod != null,                       "       Point.add method") +
            test(serializeMethod != null,                 "       Serializable.serialize method") +
            test(pt.getConstructorsNum() == 2,                              "   2 Constructors") +
            testFields(pt) +
            testGetFieldError(pt) +
            testGetMethodError(pt) +
            testGetInterfaceError(pt)
}

function testHTMLWriter(): int {
    let wt = Type.of(new HTMLWriter(1)) as ClassType

    const writerMethods = findMethodsByName(wt, "write", "getState")
    const writeMethod = writerMethods[0]
    const getStateMethod = writerMethods[1]

    return test(wt.getName() == "TypeClassTypeTest.HTMLWriter",                          "ClassName: HTMLWriter") +
            test(!wt.hasEmptyConstructor(),                                 "   doesn't have empty constructor") +
            test(wt.getBaseType().getName() == "TypeClassTypeTest.Writer",               "   base type is Writer") +
            test(wt.getInterfacesNum() == 0,                                "   0 Interfaces") +
            test(wt.getFieldsNum() == 1,                                    "   1 Field:") +
            test(wt.hasField("state"),                                      "       state field") +
            testAllFieldsAreOwn(wt) +
            test(wt.getMethodsNum() == 2 + ObjectType.getMethodsNum(),      "   5 methods:") +
            testClassExtendsObject(wt) +
            test(writeMethod != null,                     "       HTMLWriter.add method") +
            test(getStateMethod != null,                  "       HTMLWriter.getState method") +
            test(wt.getConstructorsNum() == 1,                              "   1 Constructor:") +
            test(wt.getConstructor(0).getName() == "constructor",           "       default") +
            test(wt.isFinal(),                                              "   final") +
            testFields(wt) +
            testGetFieldError(wt) +
            testGetMethodError(wt) +
            testGetInterfaceError(wt)
}

function testMake(): int {
    let failures = 0
    let pt = Type.of(new Point()) as ClassType
    let wt = Type.of(new HTMLWriter(0)) as ClassType
    let p3t = Type.of(new Point3D(10)) as ClassType
    let point = pt.make([new Double(1.0), new Double(2.0)] as FixedArray<Any>) as Point
    let point3D = p3t.make([new Double(1.0), new Double(2.0), new Double(3.0)] as FixedArray<Any>) as Point3D
    let writer = wt.make([new Int(10)] as FixedArray<Any>) as HTMLWriter
    failures += test(point.serialize() == "{1,2}", "Point")
    failures += test(point3D.serialize() == "{1,2}" && point3D.z == 3, "Point3D")
    failures += test(writer.getState() == 10, "HTMLWriter" )
    return failures
}

function testClassType(): int {
    let failures = 0
    failures += testPoint()
    failures += testPoint3D()
    failures += testHTMLWriter()
    failures += testMake()
    return failures
}

function main(): int {
    let failures = 0
    failures += testClassType()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
