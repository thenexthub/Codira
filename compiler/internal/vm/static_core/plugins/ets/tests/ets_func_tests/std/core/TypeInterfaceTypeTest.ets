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

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name)
        return 0
    }
    console.println("FAILED: " + name)
    return 1
}

interface A {
    doA(): Object;
}

interface B extends A {
    doB(): Object;
}

interface C extends A {
    doC(): Object;
}

interface D extends B, C {
    doD(): Object;
}

class InstanceC implements C {
    override doA(): Object {
        return new Int() as Object
    }
    override doC(): Object {
        return "" as Object
    }
}

class InstanceD implements D {
    override doA(): Object {
        return new Int() as Object
    }
    override doB(): Object {
        return new Double() as Object
    }
    override doC(): Object {
        return "" as Object
    }
    override doD(): Object {
        return "" as Object
    }
}

function findMethodByName(ct: InterfaceType, name: string): Method | null {
    for (let i = 0; i < ct.getMethodsNum(); ++i) {
        let method = ct.getMethod(i)
        if (method.getName() == name) {
            return method
        }
    }
    return null
}

function testGetInterfaceError(it: InterfaceType): boolean {
    try {
        it.getInterface(1000)
    } catch (ignored) {
        if (ignored instanceof TypeError){
            return true
        }
        // Expected error is catched
    }
    return false
}

function testGetMethodError(it: InterfaceType): boolean {
    try {
        it.getMethod(1000)
    } catch (ignored) {
        if (ignored instanceof TypeError){
            return true
        }
        // Expected error is catched
    }
    return false
}

function testInterfaceTypeC(): int {
    let ict = (Type.of(new InstanceC()) as ClassType).getInterface(0)
    let iat = ict.getInterface(0)
    return  test(ict.getName() == "TypeInterfaceTypeTest.C",                     "InterfaceName: C") +
            test(ict.getInterfacesNum() == 1,                   "   1 Interfaces:") +
            test(ict.getInterface(0).equals(iat),               "       A interface") +
            test(ict.getMethodsNum() == 2,                      "   2 Methods:") +
            test(findMethodByName(ict, "doA") != null,          "       doA method") +
            test(findMethodByName(ict, "doC") != null,          "       doC method") +
            test(testGetInterfaceError(ict),                    " non-existing interface") +
            test(testGetMethodError(ict),                       " non-existing method")
}

function testInterfaceTypeD(): int {
    let idt = (Type.of(new InstanceD()) as ClassType).getInterface(0)
    let ibt = idt.getInterface(0)
    let ict = idt.getInterface(1)
    return test(idt.getName() == "TypeInterfaceTypeTest.D",              "InterfaceName: D") +
            test(idt.getInterfacesNum() == 2,           "   2 Interfaces:") +
            test(idt.getInterface(0).equals(ibt),       "       B interface") +
            test(idt.getInterface(1).equals(ict),       "       C interface") +
            test(idt.getMethodsNum() == 4,              "   4 Methods:") +
            test(findMethodByName(idt, "doA") != null,  "       doA method") +
            test(findMethodByName(idt, "doB") != null,  "       doB method") +
            test(findMethodByName(idt, "doC") != null,  "       doC method") +
            test(findMethodByName(idt, "doD") != null,  "       doD method") +
            test(testGetInterfaceError(idt),            " non-existing interface") +
            test(testGetMethodError(idt),               " non-existing method")
}

function main(): int {
    let failures = 0
    failures += testInterfaceTypeC()
    failures += testInterfaceTypeD()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
