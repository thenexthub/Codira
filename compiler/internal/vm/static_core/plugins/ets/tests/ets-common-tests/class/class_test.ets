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

const globalClassName = "class_test.ETSGLOBAL"

namespace Outer {
    export namespace Inner {
        export class C {}

        export function getNamespace(): Class {
            return Class.current()
        }
    }

    export function getNamespace(): Class {
        return Class.current()
    }
}

class Test {
    public testMethodsFromClass(): void {
        const currentClass = Class.current()
        arktest.assertNE(currentClass, undefined)
        arktest.assertEQ(currentClass.getName(), "class_test.Test")

        const callerClass = Class.ofCaller()
        arktest.assertNE(callerClass, undefined)
        arktest.assertEQ(callerClass!.getName(), globalClassName)
    }

    public testClassIsNamespace(): void {
        const innerNs = Outer.Inner.getNamespace()
        arktest.assertNE(innerNs, undefined)
        arktest.assertTrue(innerNs!.isNamespace())
        arktest.assertEQ(innerNs!.getName(), "class_test.Outer.Inner")

        const outerNs = Outer.getNamespace()
        arktest.assertNE(outerNs, undefined)
        arktest.assertTrue(outerNs!.isNamespace())
        arktest.assertEQ(outerNs!.getName(), "class_test.Outer")
    }

    private intArrayComponentType(): void {
        const ints: FixedArray<int> = [1, 2]
        const compCls = Class.of(ints).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_INT)
    }

    private longArrayComponentType(): void {
        const longs: FixedArray<long> = [1, 2]
        const compCls = Class.of(longs).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_LONG)
    }

    private doubleArrayComponentType(): void {
        const doubles: FixedArray<double> = [1.0, 2.0]
        const compCls = Class.of(doubles).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_DOUBLE)
    }

    private floatArrayComponentType(): void {
        const floats: FixedArray<float> = [1.0, 2.0]
        const compCls = Class.of(floats).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_FLOAT)
    }

    private byteArrayComponentType(): void {
        const bytes: FixedArray<byte> = [1, 2]
        const compCls = Class.of(bytes).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_BYTE)
    }

    private shortArrayComponentType(): void {
        const shorts: FixedArray<short> = [1, 2]
        const compCls = Class.of(shorts).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_SHORT)
    }

    private charArrayComponentType(): void {
        const chars: FixedArray<char> = [c'A', c'B']
        const compCls = Class.of(chars).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_CHAR)
    }

    private booleanArrayComponentType(): void {
        const bools: FixedArray<boolean> = [true, false]
        const compCls = Class.of(bools).getFixedArrayComponentType()
        arktest.assertEQ(compCls, Class.PRIMITIVE_BOOLEAN)
    }

    public testClassGetFixedArrayComponentType(): void {
        const obj = new Object()
        const objCls = Class.of(obj)
        arktest.assertEQ(objCls.getFixedArrayComponentType(), undefined)

        this.intArrayComponentType()
        this.longArrayComponentType()
        this.doubleArrayComponentType()
        this.floatArrayComponentType()
        this.byteArrayComponentType()
        this.shortArrayComponentType()
        this.charArrayComponentType()

        const arrays: FixedArray<FixedArray<int>> = [[1], [2]]
        const arraysCompCls = Class.of(arrays).getFixedArrayComponentType()
        arktest.assertNE(arraysCompCls, undefined)
        arktest.assertNE(arraysCompCls!.getFixedArrayComponentType(), undefined)
        arktest.assertEQ(arraysCompCls!.getFixedArrayComponentType(), Class.PRIMITIVE_INT)
    }
}

function main(): void {
    // For the test purposes these checks must be located in the entrypoint function
    const currentClass = Class.current()
    arktest.assertNE(currentClass, undefined)
    arktest.assertEQ(currentClass.getName(), globalClassName)

    // `main` is entrypoint, hence it does not have managed callers
    const callerClass = Class.ofCaller()
    arktest.assertEQ(callerClass, undefined)

    const test = new Test()
    test.testMethodsFromClass()
    test.testClassIsNamespace()
    test.testClassGetFixedArrayComponentType()
}
