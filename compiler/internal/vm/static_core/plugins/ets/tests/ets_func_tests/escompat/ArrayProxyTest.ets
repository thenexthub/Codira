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

class StringArrayProxyHandler extends proxy.DefaultArrayProxyHandler<string> {
    getTrapped: boolean = false
    setTrapped: boolean = false
    setLengthTrapped: boolean = false
    popTrapped: boolean = false
    spliceTrapped: boolean = false
    shiftTrapped: boolean = false
    pushTrapped: boolean = false
    concatTrapped: boolean = false
    unshiftTrapped: boolean = false
    toSplicedTrapped: boolean = false

    override get(target: Array<string>, index: int): string {
        this.getTrapped = true
        return (index == 0) ? "zero" : super.get(target, index)
    }

    override set(target: Array<string>, index: int, value: string): boolean {
        this.setTrapped = true

        if (index == 2) {
            return super.set(target, index, "two")
        }

        return super.set(target, index, value)
    }

    override get(target: Array<string>, name: string): Any {
        return super.get(target, name)
    }

    override set(target: Array<string>, name: string, value: Any): boolean {
        if (name == "length") {
            this.setLengthTrapped = true
            // ignore set new length request
            return true
        }

        return super.set(target, name, value)
    }

    override invoke(target: Array<string>, method: Method, args: FixedArray<Any>): Any {
        const methodName = method.getName()
        if (methodName == "pop") {
            this.popTrapped = true
        } else if (methodName == "splice") {
            this.spliceTrapped = true
        } else if (methodName == "shift") {
            this.shiftTrapped = true
        } else if (methodName == "push") {
            this.pushTrapped = true
        } else if (methodName == "concat") {
            this.concatTrapped = true
        } else if (methodName == "unshift") {
            this.unshiftTrapped = true
        } else if (methodName == "toSpliced") {
            this.toSplicedTrapped = true
        }

        return super.invoke(target, method, args)
    }
}

function verifyArrayProxyOps(array: Array<string>, handler: StringArrayProxyHandler): void {
    let arrayProxy: Array<string> = proxy.Proxy.create(array, handler)

    arktest.assertEQ(arrayProxy.length, 3, "unexpected arrayProxy.length")

    arktest.assertFalse(handler.getTrapped)
    arktest.assertEQ(arrayProxy[0],  "zero")
    arktest.assertTrue(handler.getTrapped)

    arktest.assertFalse(handler.setTrapped)
    arrayProxy[2] = "3"
    arktest.assertTrue(handler.setTrapped)
    arktest.assertEQ(arrayProxy[2],  "two")

    arktest.assertFalse(handler.setLengthTrapped)
    arrayProxy.length = 2
    arktest.assertTrue(handler.setLengthTrapped)
    arktest.assertEQ(arrayProxy.length, 3, "arrayProxy.length updated")

    arktest.assertFalse(handler.popTrapped)
    const popped = arrayProxy.pop()
    arktest.assertEQ(popped, "two", "unexpected Array.pop() result")
    arktest.assertTrue(handler.popTrapped)

    const lengthAfterPush = arrayProxy.push("5")
    arktest.assertTrue(handler.pushTrapped)
    arktest.assertEQ(lengthAfterPush, 3)

    const lastElem = arrayProxy.pop()
    arktest.assertEQ(lastElem, "5", "unexpected Array.pop() after Array.push() result")

    arktest.assertFalse(handler.spliceTrapped)
    let arrayTail = arrayProxy.splice(1, 0, "2")
    arktest.assertEQ(arrayTail.length, 0, "unexpected Array.splice() result length")
    arktest.assertEQ(arrayProxy.length, 3, "unexpected array length after splice")
    arktest.assertTrue(handler.spliceTrapped)

    arktest.assertFalse(handler.shiftTrapped)
    const shifted = arrayProxy.shift()
    arktest.assertEQ(shifted, "0")
    arktest.assertEQ(arrayProxy.length, 2)
    arktest.assertTrue(handler.shiftTrapped)

    arrayTail = arrayProxy.splice(1)
    arktest.assertEQ(arrayTail.length, 1, "unexpected Array.splice() result length")
    arktest.assertEQ(arrayProxy.length, 1, "unexpected array length after splice")

    arrayProxy.push("5", "6")
    arktest.assertEQ(arrayProxy.length, 3, "unexpected arrayProxy.length")
    const lastElem2 = arrayProxy.pop()
    arktest.assertEQ(lastElem2, "6", "unexpected Array.pop() after Array.push() result")
    const lastElem3 = arrayProxy.pop()
    arktest.assertEQ(lastElem3, "5", "unexpected Array.pop() after Array.push() result")

    const lengthAfterUnshift = arrayProxy.unshift("I", "II")
    arktest.assertTrue(handler.unshiftTrapped)
    arktest.assertEQ(arrayProxy.length, 3)

    const tail = ["V"]
    const concatResult = arrayProxy.concat(tail)
    arktest.assertTrue(handler.concatTrapped)
    arktest.assertEQ(concatResult.length, 4)

    const arrayLengthBefore = arrayProxy.length
    const toSplicedResult = arrayProxy.toSpliced(1, 1)
    arktest.assertTrue(handler.toSplicedTrapped)
    arktest.assertEQ(toSplicedResult.length, 2)
    arktest.assertEQ(arrayProxy.length, arrayLengthBefore)
}

function testArrayProxyOps(): void {
    verifyArrayProxyOps(Array.of<string>("0", "1", "2"), new proxy.StringArrayProxyHandler())
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Proxy API")

    suite.addTest("Array proxy operations", testArrayProxyOps)

    return suite.run()
}
