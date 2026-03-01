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

function main(): int {
    let failures: int = 0;

    failures += test(objectKeys(), "Object.keys");

    failures += test(objectValues(), "Object.values");

    failures += test(objectEntries(), "Object.entries");

    failures += test(objectGetOwnPropertyNames(), "Object.getOwnPropertyNames");

    failures += test(objectHasOwnProperty(), "Object.hasOwnProperty");

    failures += test(objectHasOwn(), "Object.hasOwn");

    failures += test(objectFromEntries(), "Object.fromEntries");

    failures += test(objectAssign(), "Object.assign");

    failures += test(objectToString(), "Object.toString");

    failures += test(objectToLocaleString(), "Object.toLocaleString");

    return test(failures, "All tests run");
}

type TupleT = [string, Any]
type NullishTuple = TupleT | null | undefined

class Point2D {
    static axisnum: number = 2
    x: number
    y: number | null
    constructor (x_: number, y_: number | null) {
        this.x = x_
        this.y = y_
    }
}

class Point3D extends Point2D {
    static axisnum: number = 3
    z: number | undefined
    constructor (x_: number, y_: number | null, z_: number | undefined) {
        super(x_, y_)
        this.z = z_
    }
}

function strArraysAreEqual(a: string[], b: string[]): boolean {
    let alen = a.length
    if (alen != b.length) {
        return false
    }
    for (let i = 0; i < alen; i++) {
        if (a[i] != b[i]) {
            return false
        }
    }
    return true
}

function objArraysAreEqual(a: Any[], b: Any[]): boolean {
    let alen = a.length
    if (alen != b.length) {
        return false
    }
    for (let i = 0; i < alen; i++) {
        if (a[i] !== b[i]) {
            return false
        }
    }
    return true
}

function tupleAreEqual(a: NullishTuple[], b: NullishTuple[]): boolean {
    let alen = a.length
    if (alen != b.length) {
        return false
    }
    for (let i = 0; i < alen; i++) {
        if (a[i]![0] != b[i]![0] || a[i]![1] !== b[i]![1]) {
            return false
        }
    }
    return true
}

function mapAreEqual(a: Map<PropertyKey, Any>, b: Map<PropertyKey, Any>): boolean {
    let alen = a.size
    if (alen != b.size) {
        return false
    }
    let areEqual: boolean = true
    a.forEach((av: Any, k: PropertyKey) => {
        const bv = b.get(k)
        if (bv == undefined || av !== bv) {
            areEqual = false
            return
        }
    })
    return areEqual
}

function objectKeys(): int {
    let result: int = 0

    let c: char = c'c'
    let bo: boolean = true
    let bt: byte = 10
    let sh: short = 20
    let i: int = 30
    let lo: long = 40
    let fl: float = 50.0f
    let dou: double = 60.0

    let arr: FixedArray<number> = [10, 20, 30]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, 20, 30)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    let emptyArr: string[] = []

    result += strArraysAreEqual(Object.keys(c), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(bo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(bt), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(sh), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(i), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(lo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(fl), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.keys(dou), emptyArr) ? 0 : 1

    result += strArraysAreEqual(Object.keys(arr), ["0", "1", "2"]) ? 0 : 1

    result += strArraysAreEqual(Object.keys(str), ["0", "1", "2"]) ? 0 : 1

    result += strArraysAreEqual(Object.keys(cl), ["x", "y", "z"]) ? 0 : 1

    result += strArraysAreEqual(Object.keys(lambda), emptyArr) ? 0 : 1

    let arr2 = new Array<number>()
    result += strArraysAreEqual(Object.keys(arr2), emptyArr) ? 0 : 1

    let arr3 = new Array<number>(5)
    result += strArraysAreEqual(Object.keys(arr3), ['0', '1', '2', '3', '4']) ? 0 : 1

    return result
}

function objectValues(): int {
    let result: int = 0

    let c: char = c'c'
    let bo: boolean = true
    let bt: byte = 10
    let sh: short = 20
    let i: int = 30
    let lo: long = 40
    let fl: float = 50.0f
    let dou: double = 60.0

    let arr: FixedArray<number> = [10, 20, 30]
    let arr2: Array<string> = ["a", "b", "c"]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, 20, undefined)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    let emptyArr: Any[] = []

    result += objArraysAreEqual(Object.values(c), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values(bo), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values(bt), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values(sh), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values(i), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values(lo), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values(fl), emptyArr) ? 0 : 1
    result += objArraysAreEqual(Object.values(dou), emptyArr) ? 0 : 1

    let arrVal: Any[] = [new Number(10), new Number(20), new Number(30)]
    result += objArraysAreEqual(Object.values(arr), arrVal) ? 0 : 1

    let arr2Val: Any[] = ["a" as string, "b" as string, "c" as string]
    result += objArraysAreEqual(Object.values(arr2), arr2Val) ? 0 : 1

    let strVal: Any[] = ["a" as string, "b" as string, "c" as string]
    result += objArraysAreEqual(Object.values(str), strVal) ? 0 : 1

    let clVal: Any[] = [new Number(10), new Number(20), undefined]
    result += objArraysAreEqual(Object.values(cl), clVal) ? 0 : 1

    result += objArraysAreEqual(Object.values(lambda), emptyArr) ? 0 : 1

    return result
}

function objectEntries(): int {
    let result: int = 0

    let c: char = c'c'
    let bo: boolean = true
    let bt: byte = 10
    let sh: short = 20
    let i: int = 30
    let lo: long = 40
    let fl: float = 50.0f
    let dou: double = 60.0

    let arr: FixedArray<number> = [10, 20, 30]
    let arr2: Array<string> = ["a", "b", "c"]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, null, 30)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    let emptyTuple: NullishTuple[] = new NullishTuple[0]

    result += tupleAreEqual(Object.entries(c) as Object as NullishTuple[], emptyTuple) ? 0 : 1
    result += tupleAreEqual(Object.entries(bo) as Object as NullishTuple[], emptyTuple) ? 0 : 1
    result += tupleAreEqual(Object.entries(bt) as Object as NullishTuple[], emptyTuple) ? 0 : 1
    result += tupleAreEqual(Object.entries(sh) as Object as NullishTuple[], emptyTuple) ? 0 : 1
    result += tupleAreEqual(Object.entries(i) as Object as NullishTuple[], emptyTuple) ? 0 : 1
    result += tupleAreEqual(Object.entries(lo) as Object as NullishTuple[], emptyTuple) ? 0 : 1
    result += tupleAreEqual(Object.entries(fl) as Object as NullishTuple[], emptyTuple) ? 0 : 1
    result += tupleAreEqual(Object.entries(dou) as Object as NullishTuple[], emptyTuple) ? 0 : 1

    let arrVal: NullishTuple[] = new NullishTuple[3]
    arrVal[0] = ["0", new Number(10)] as TupleT
    arrVal[1] = ["1", new Number(20)] as TupleT
    arrVal[2] = ["2", new Number(30)] as TupleT
    result += tupleAreEqual(Object.entries(arr) as Object as NullishTuple[], arrVal) ? 0 : 1

    let arr2Val: NullishTuple[] = new NullishTuple[3]
    arr2Val[0] = ["0", "a"] as TupleT
    arr2Val[1] = ["1", "b"] as TupleT
    arr2Val[2] = ["2", "c"] as TupleT
    result += tupleAreEqual(Object.entries(arr2) as Object as NullishTuple[], arr2Val) ? 0 : 1

    let strVal: NullishTuple[] = new NullishTuple[3]
    strVal[0] = ["0", "a"] as TupleT
    strVal[1] = ["1", "b"] as TupleT
    strVal[2] = ["2", "c"] as TupleT
    result += tupleAreEqual(Object.entries(str) as Object as NullishTuple[], strVal) ? 0 : 1

    let clVal: NullishTuple[] = new NullishTuple[3]
    clVal[0] = ["x", new Number(10)] as TupleT
    clVal[1] = ["y", null] as TupleT
    clVal[2] = ["z", new Number(30)] as TupleT
    result += tupleAreEqual(Object.entries(cl) as Object as NullishTuple[], clVal) ? 0 : 1

    result += tupleAreEqual(Object.entries(lambda) as Object as NullishTuple[], emptyTuple) ? 0 : 1

    return result
}

function objectGetOwnPropertyNames(): int {
    let result: int = 0

    let c: char = c'c'
    let bo: boolean = true
    let bt: byte = 10
    let sh: short = 20
    let i: int = 30
    let lo: long = 40
    let fl: float = 50.0f
    let dou: double = 60.0

    let arr: FixedArray<number> = [10, 20, 30]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, 20, 30)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    let emptyArr: string[] = []

    result += strArraysAreEqual(Object.getOwnPropertyNames(c), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(bo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(bt), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(sh), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(i), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(lo), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(fl), emptyArr) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(dou), emptyArr) ? 0 : 1

    result += strArraysAreEqual(Object.getOwnPropertyNames(arr), ["0", "1", "2", "length"]) ? 0 : 1
    result += strArraysAreEqual(Object.getOwnPropertyNames(str), ["0", "1", "2", "length"]) ? 0 : 1

    result += strArraysAreEqual(Object.getOwnPropertyNames(cl), ["x", "y", "z"]) ? 0 : 1

    result += strArraysAreEqual(Object.getOwnPropertyNames(lambda), ["length", "name"]) ? 0 : 1

    let arr2 = new Array<number>()
    result += strArraysAreEqual(Object.getOwnPropertyNames(arr2), ['length']) ? 0 : 1

    let arr3 = new Array<number>(5)
    result += strArraysAreEqual(Object.getOwnPropertyNames(arr3), ['0', '1', '2', '3', '4', 'length']) ? 0 : 1

    return result
}

function objectHasOwnProperty(): int {
    let result: int = 0

    let arr: FixedArray<number> = [10, 20, 30]
    let str: string = "abc"

    let cl: Point2D = new Point3D(10, 20, 30)

    let lambda: (a: number) => number = (a: number): number => {
        return a + 1
    }

    result += ((arr as Object).hasOwnProperty(0) == true) ? 0 : 1
    result += ((arr as Object).hasOwnProperty(3) == false) ? 0 : 1
    result += ((arr as Object).hasOwnProperty("length") == true) ? 0 : 1
    result += ((arr as Object).hasOwnProperty("qwerty") == false) ? 0 : 1
    result += ((arr as Object).hasOwnProperty("toString") == false) ? 0 : 1

    result += (str.hasOwnProperty(0) == true) ? 0 : 1
    result += (str.hasOwnProperty(3) == false) ? 0 : 1
    result += (str.hasOwnProperty("length") == true) ? 0 : 1
    result += (str.hasOwnProperty("qwerty") == false) ? 0 : 1

    result += (cl.hasOwnProperty("x") == true) ? 0 : 1
    result += (cl.hasOwnProperty("y") == true) ? 0 : 1
    result += (cl.hasOwnProperty("z") == true) ? 0 : 1
    result += (cl.hasOwnProperty("axisnum") == false) ? 0 : 1
    result += (cl.hasOwnProperty("asdasd") == false) ? 0 : 1
    result += (cl.hasOwnProperty(0) == false) ? 0 : 1
    result += (cl.hasOwnProperty("toString") == false) ? 0 : 1

    result += ((lambda as Object).hasOwnProperty("length") == true) ? 0 : 1
    result += ((lambda as Object).hasOwnProperty("name") == true) ? 0 : 1
    result += ((lambda as Object).hasOwnProperty(0) == false) ? 0 : 1

    return result
}

function objectHasOwn(): int {
    let result: int = 0

    let arr: FixedArray<number> = [10, 20, 30]

    result += (Object.hasOwn(arr as Object, '0') == true) ? 0 : 1
    result += (Object.hasOwn(arr as Object, '3') == false) ? 0 : 1
    result += (Object.hasOwn(arr as Object, "length") == true) ? 0 : 1
    result += (Object.hasOwn(arr as Object, "qwerty") == false) ? 0 : 1
    result += (Object.hasOwn(arr as Object, "toString") == false) ? 0 : 1

    return result
}

type Rec = Record<PropertyKey, Any>

function objectFromEntries(): int {
    let result: int = 0

    const entries: Rec = {
        "x": new Number(10),
        "y": new Number(20),
        "z": new Number(30)
    }

    const expected: Rec = {
        "x": new Number(10),
        "y": new Number(20),
        "z": new Number(30)
    }

    result += mapAreEqual(Object.fromEntries<Any>(entries), expected) ? 0 : 1

    const entriesIterable: Iterable<[PropertyKey, Any]> = entries
    result += mapAreEqual(Object.fromEntries<Any>(entriesIterable), expected) ? 0 : 1

    const emptyObj: Rec = {}

    const emptyEntries = new Array<[PropertyKey, Any]>()
    result += mapAreEqual(Object.fromEntries<Any>(emptyEntries), emptyObj) ? 0 : 1

    return result
}

function objectAssign(): int {
    let result: int = 0

    const target: Rec = {}

    const expected1: Rec = {
        "x": new Number(10),
        "y": new Number(20)
    }

    const expected2: Rec = {
        "x": new Number(100),
        "y": new Number(200),
        "z": new Number(300)
    }

    result += mapAreEqual(Object.assign(target), target) ? 0 : 1
    result += mapAreEqual(Object.assign(target, new Point2D(10, 20)), expected1) ? 0 : 1
    result += mapAreEqual(Object.assign(target, new Point2D(10, 20), new Point3D(100, 200, 300)), expected2) ? 0 : 1

    return result
}

function objectToString(): int  {
    let result: int = 0

    result += (new Object().toString() == "[object Object]") ? 0 : 1

    return result
}

function objectToLocaleString(): int  {
    let result: int = 0

    result += (new Object().toLocaleString() == "[object Object]") ? 0 : 1

    return result
}

function test(result: int, message: String ): int {
    if (result == 0) {
      console.log('PASSED: ' + message);
      return 0;
    }
    console.log('FAILED: ' + message);
    return 1;
}
