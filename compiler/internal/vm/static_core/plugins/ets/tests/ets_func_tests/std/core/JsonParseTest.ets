/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

class Point {
    static s = "s"

    x: number
    y: number

    constructor() {
        this(0, 0)
    }

    constructor(x: number, y: number) {
        this.x = x
        this.y = y
    }

    toString(): string {
        return `Point(x=${this.x},y=${this.y})`
    }
}

class Trash {
    point: Point | null | undefined

    constructor() {
        this.point = undefined
    }

    constructor(point: Point) {
        this.point = point
    }
}

function newUndefiningReviver(...keys: FixedArray<string>): (k: string, v: Any) => Any {
    return (key: string, val: Any) => Array.from<string>(keys).indexOf(key) != -1 ? undefined : val
}

function newNullingReviver(...keys: FixedArray<string>): (k: string, v: Any) => Any {
    return (key: string, val: Any) => Array.from<string>(keys).indexOf(key) != -1 ? null : val
}

function newReplacingReviver(replacements: Map<string, Any>): (k: string, v: Any) => Any {
    return (key: string, val: Any) => {
        if (replacements.has(key)) {
            return replacements.get(key)
        }

        return val
    }
}

function replacingReviver(key: string, val: Any): Any {
    if (key == "x") {
        return 10.0
    } else {
        return val
    }
}

const POINT_TYPE = typeFor([] as FixedArray<Point>)

function jsonParseUsingUndefinedReviver(): void {
    const point = new Point(1, 2)

    const parsedPoint: Point = JSON.parse<Point>(JSON.stringify(point), undefined, POINT_TYPE)!

    arktest.assertEQ(point.x, parsedPoint.x)
    arktest.assertEQ(point.y, parsedPoint.y)
}

function jsonParseUsingRevivierWhichReturnsUndefined(): void {
    const point = new Point(1, 2)

    const parsedPoint = JSON.parse<Point>(JSON.stringify(point), newUndefiningReviver(""), POINT_TYPE)

    arktest.assertEQ(undefined, parsedPoint)
}

const STRING_ARRAY_TYPE = Type.of([] as FixedArray<string>)

function jsonParseArrayUsingReviverWhichReturnsUndefined(): void {
    const parsedArray = JSON.parse<FixedArray<string | null | undefined>>(`["one","two"]`, newUndefiningReviver("0"), STRING_ARRAY_TYPE)
    arktest.assertEQ([null, "two"] as FixedArray<string | null | undefined>, parsedArray)
}

function jsonParseUsingReplacingReviver(): void {
    const NEW_X = 10.0

    const replacements = new Map<string, Any>()
    replacements.set("x", NEW_X)

    const point = new Point(3, 4)
    const parsedPoint: Point = JSON.parse<Point>(JSON.stringify(point), newReplacingReviver(replacements), POINT_TYPE)!

    const replacedPoint = new Point(NEW_X, point.y)
    arktest.assertEQ(replacedPoint.x, parsedPoint.x)
    arktest.assertEQ(replacedPoint.y, parsedPoint.y)

    const trash = new Trash(new Point(10, 20))
    const trashJson = JSON.stringify(trash)
    const trashType = typeFor([] as FixedArray<Trash>)

    let parsedTrash = JSON.parse<Trash>(trashJson, newUndefiningReviver("point"), trashType)

    arktest.assertTrue(parsedTrash != null)
    arktest.assertEQ(undefined, parsedTrash!.point)

    parsedTrash = JSON.parse<Trash>(trashJson, newNullingReviver("point"), trashType)

    arktest.assertTrue(parsedTrash != null)
    arktest.assertEQ(undefined, parsedTrash!.point)
}

function jsonParseArrayWithReplacingReviver(): void {
    const romans: FixedArray<string | null | undefined> = ["I", "II"]

    const replacements = new Map<string, Any>()

    const replacedRomans: FixedArray<string | null | undefined> = ["I"]
    replacements.set("", replacedRomans)

    const parsedRomans = JSON.parse<FixedArray<string | null | undefined>>(JSON.stringify(romans), newReplacingReviver(replacements), STRING_ARRAY_TYPE)
    arktest.assertEQ(parsedRomans, replacedRomans)
}

enum Color {
    Black = 0,
    Red = 1,
    Green = 2
}

function consolePrint(color: Color, text: string): void {
    console.print("\x1b[38;5;" + color.toString() + "m")
    console.print(text)
    console.print("\x1b[0m")
}

class BooleanTestClass {
    flag: boolean = false;
}

function jsonParseWithBoolean(): void {
    let str = '{"flag":true}'
    let type = Type.from<BooleanTestClass>()
    let ret = JSON.parse<BooleanTestClass>(str, type) as BooleanTestClass
    arktest.assertEQ(ret.flag, true)
}

interface IState {
    change()
}

class IStateCLASS implements IState {
    change(){}
}

class classfactory {
    static getIState(): IState {
        return new IStateCLASS()
    }
}

class interfaceTest {
    @JSONParseIgnore() meta: IState = classfactory.getIState()
    name:string = ""
}

function jsonParseWithInterface(): void {
    let str = '{"name":"tester"}'
    let type = Type.from<interfaceTest>()
    let ret = JSON.parse<interfaceTest>(str, type) as interfaceTest
    arktest.assertEQ(ret.name, "tester")
}

class BigIntTest {
    a: bigint = new BigInt(0)
}

function jsonParseWithBigInt(): void {
    const integerStr = "{\"a\": 949491916515615615646465644892374983279473243480923840328049823094}"
    const integerStr2 = "{\"a\": 949491916515615615646465644892374983279473243480923840328049823094.0}"
    const nonIntergerStr = "{\"a\": 949491916515615615646465644892374983279473243480923840328049823094.1}"
    const objI = JSON.parse<BigIntTest>(integerStr, undefined, Type.from<BigIntTest>(),
        { bigIntMode: jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT } as jsonx.ParseOptions) as BigIntTest
    const objI2 = JSON.parse<BigIntTest>(integerStr2, undefined, Type.from<BigIntTest>(),
        { bigIntMode: jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT } as jsonx.ParseOptions) as BigIntTest
    const objNI = JSON.parse<BigIntTest>(nonIntergerStr, undefined, Type.from<BigIntTest>(),
        { bigIntMode: jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT } as jsonx.ParseOptions) as BigIntTest
    arktest.assertEQ(objI.a, 949491916515615615646465644892374983279473243480923840328049823094n)
    arktest.assertEQ(objI2.a, 949491916515615615646465644892374983279473243480923840328049823094n)
    arktest.assertEQ(objNI.a, 0n)
}

class ElementTest {
    ele: string = "abc"
}

class TokenTest {
    property : ElementTest = new ElementTest()
}

function jsonParseWithSpecialToken(): void {
    const jsonStr = '\t\r \n{\t\r \n' +
        '"property"\t\r \n:\t\r \n{\t\r"ele": \t "def"}\t\t \n}\t\t \n'
    const obj = JSON.parse<TokenTest>(jsonStr, Type.from<TokenTest>()) as TokenTest
    arktest.assertEQ(obj.property.ele, "def")
}

function jsonParseUnicodeString(): void {
    let str = '"\\u4F60\\u597D"'
    let type = Class.from<String>()
    arktest.assertEQ(JSON.parse<String>(str, type), '你好')
    str = '"\u4F60\u597D"'
    arktest.assertEQ(JSON.parse<String>(str, type), '你好')
    str = '"\\u4f60\\u597d"'
    arktest.assertEQ(JSON.parse<String>(str, type), '你好')
    str = '"\\u4F601aB\\u597d"'
    arktest.assertEQ(JSON.parse<String>(str, type), '你1aB好')

    str = '"\u{1F60A}"'
    arktest.assertEQ(JSON.parse<String>(str, type), '😊')
    str = '"\U{1F60A}"'
    arktest.assertEQ(JSON.parse<String>(str, type), 'U{1F60A}')
    str = '"\\uD83D\\uDE0A"'
    arktest.assertEQ(JSON.parse<String>(str, type), '😊')
    str = '"\uD83D\uDE0A"'
    arktest.assertEQ(JSON.parse<String>(str, type), '😊')
    str = '"\UD83D\UDE0A"'
    arktest.assertEQ(JSON.parse<String>(str, type), 'UD83DUDE0A')

    arktest.expectThrow(() => {JSON.parse<String>('"\\u{1F60A}"', type)}, (e) => { return e instanceof SyntaxError})
    arktest.expectThrow(() => {JSON.parse<String>('"\\U4f60\\U597d"', type)}, (e) => { return e instanceof SyntaxError})
    arktest.expectThrow(() => {JSON.parse<String>('"\\u4f6"', type)}, (e) => { return e instanceof SyntaxError})
    arktest.expectThrow(() => {JSON.parse<String>('"\\u4f6g"', type)}, (e) => { return e instanceof SyntaxError})
}

function jsonParseEscapedChar(): void {
    let type = Class.from<String>()
    let arr: FixedArray<String> = ['"\\""', '"\\\\"', '"\\/"','"\\b"', '"\\f"', '"\\n"', '"\\r"', '"\\t"']
    let res: FixedArray<String> = ['"', '\\', '/', '\b', '\f', '\n', '\r', '\t']
    for (let i = 0; i < arr.length; i++) {
        arktest.assertEQ(JSON.parse<String>(arr[i], type), res[i]);
    }

    let str = '"\\"\\\\\\/\\b\\f\\n\\r\\t123abc"'
    arktest.assertEQ(JSON.parse<String>(str, type), '"\\/\b\f\n\r\t123abc');
}

class Test {}

class TestObject {
    public a: int = 123
    public b: number = 123.45
    public c: string = 'test'
    public d: boolean = true
    public e: boolean = false
    public g: Test = new Test()
    public h: FixedArray<Test> = [new Test()]
}

function checkSyntaxError(e: Error): string | boolean {
    return e instanceof SyntaxError
}

function jsonParseEnd1(): void {
    arktest.expectThrow(() => {
        JSON.parse<String>('"\\', Class.from<String>())
    }, checkSyntaxError)

    arktest.expectThrow(() => {
        JSON.parse<String>('"\\"', Class.from<String>())
    }, checkSyntaxError)

    arktest.expectThrow(() => {
        JSON.parse<String>('"\\d"', Class.from<String>())
    }, checkSyntaxError)

    arktest.expectThrow(() => {
        JSON.parse<FixedArray<Test>>('[]]', Class.from<FixedArray<Test>>())
    }, checkSyntaxError)

    arktest.expectThrow(() => {
        JSON.parse<Test>('{}}', Class.from<Test>())
    }, checkSyntaxError)

    arktest.expectThrow(() => {
        JSON.parse<String>('"""', Class.from<String>())
    }, checkSyntaxError)
}

function jsonParseEnd2(): void {
    let str = `
        { \t
            "a": 100 \t, \t
            "b": 15.5 \t, \t
            "c": "test123" \t, \t
            "d": false \t, \t
            "e": true \t, \t
            "g": {} \t, \t
            "h": [ \t
                { \t} \t
            ] \t
        } \t
    `
    let obj = JSON.parse<TestObject>(str, Class.from<TestObject>()) as TestObject
    arktest.assertEQ(obj.a, 100)
    arktest.assertEQ(obj.b, 15.5)
    arktest.assertEQ(obj.c, 'test123')
    arktest.assertFalse(obj.d)
    arktest.assertTrue(obj.e)
    arktest.assertEQ(JSON.stringify(obj.g), '{}')
    arktest.assertEQ(JSON.stringify(obj.h), '[{}]')
}

function jsonParseString(): void {
    let type = Class.from<String>()
    let str = '"test\\btest"'
    arktest.assertEQ(JSON.parse<string>(str, type), 'test\btest')

    str = '""'
    arktest.assertEQ(JSON.parse<string>(str, type), '')

    str = '"\\n123"'
    arktest.assertEQ(JSON.parse<string>(str, type), '\n123')

    str = '"test"'
    arktest.assertEQ(JSON.parse<string>(str, type), 'test')

    str = '"\\u4F60test\\u597D"'
    arktest.assertEQ(JSON.parse<string>(str, type), '你test好')
}


function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.parse tests');

    suite.addTest('JSON.parse() using undefined "reviver"', jsonParseUsingUndefinedReviver)
    suite.addTest('JSON.parse() using "reviver" which returns "undefined"', jsonParseUsingRevivierWhichReturnsUndefined)
    // #21831: ArrayStoreError: nullvalue cannot be stored in an array of type [Lstd/core/String;
    // suite.addTest('JSON.parse(array) using "reviver" which returns "undefined"', jsonParseArrayUsingReviverWhichReturnsUndefined)
    // #27136: Nullish field type is UnionType which is not supported yet
    // suite.addTest('JSON.parse() using replacing "reviver"', jsonParseUsingReplacingReviver)
    suite.addTest('JSON.parse(array) using replacing "reviver"', jsonParseArrayWithReplacingReviver)
    suite.addTest('JSON.parse boolean in class', jsonParseWithBoolean)
    suite.addTest('JSON.parse interface(ignore) in class', jsonParseWithInterface)
    suite.addTest('JSON.parse BigInt', jsonParseWithBigInt)
    suite.addTest('JSON.parse specialToken', jsonParseWithSpecialToken)
    suite.addTest('JSON.parse unicode', jsonParseUnicodeString)
    suite.addTest('JSON.parse escaped char', jsonParseEscapedChar)
    suite.addTest('JSON.parse end 1', jsonParseEnd1)
    suite.addTest('JSON.parse end 2', jsonParseEnd2)
    suite.addTest('JSON.parse string', jsonParseString)

    return suite.run();
}

function typeFor<T>(a: FixedArray<T>): Type {
    return (Type.of(a) as ArrayType).getElementType()
}
