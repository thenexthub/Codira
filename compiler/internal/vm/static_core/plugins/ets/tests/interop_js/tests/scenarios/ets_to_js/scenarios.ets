/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

package scenarios_test

const STRING_VALUE = '1';
const INT_VALUE = 1;
const INT_VALUE2 = 2;
const INT_VALUE3 = 3;
const FLOAT_VALUE = 1.0;

function standaloneFunctionEts(): int {
    return 1;
}

function functionReturnBoxedString(): String {
    let stsString: String = new String("Hello");
    return stsString;
}

function functionReturnBoxedNumber(): Number {
    let stsNumber: Number = new Number(42 as number);
    return stsNumber;
}

function functionReturnBoxedInt(): Int {
    let stsInt: Int = new Int(42);
    return stsInt;
}

function functionReturnBoxedByte(): Byte {
    let stsByte: Byte = new Byte(126 as byte);
    return stsByte;
}

function functionReturnBoxedShort(): Short {
    let stsShort: Short = new Short(4242 as short);
    return stsShort;
}

function functionReturnBoxedLong(): Long {
    let stsLong: Long = new Long(12 as long);
    return stsLong;
}

function functionReturnBoxedFloat(): Float {
    let stsFloat: Float = new Float(42.42.toFloat());
    return stsFloat;
}

function functionReturnBoxedDouble(): Double {
    let stsDouble: Double = new Double(42.42 as double);
    return stsDouble;
}

function functionReturnBoxedChar(): Char {
    let stsChar: Char = new Char(c'a');
    return stsChar;
}

function functionReturnBoxedBoolean(): Boolean {
    let stsBoolean: Boolean = new Boolean(true);
    return stsBoolean;
}

function functionReturnBoxedBigInt(): BigInt {
    let stsBigInt: BigInt = new BigInt(42);
    return stsBigInt;
}

class ClassWithMethodEts {
    public methodInClass(): int {
        return 1;
    }
};

interface InterfaceWithMethodEts {
    methodInInterface(): int;
}

class ImplInterfaceWithMethodEts implements InterfaceWithMethodEts {
    public override methodInInterface(): int {
        return 1;
    }
}

// NOTE (oignatenko) figure a way (and figure if there is a need) to make a negative test for static methods
// of interface since these do not belong to subset.
function newInterfaceWithMethodEts(): InterfaceWithMethodEts {
    return new ImplInterfaceWithMethodEts();
}

class ClassWithGetterSetterEts {
    private _value: int = 1;

    get value() {
        return this._value; // NOTE figure why this._value fails to run here
    }

    set value(theValue: int) {
        this._value = theValue;
    }
}

let lambdaFunctionEts: () => int;
static {
    lambdaFunctionEts = () => { return 1; }
}

function getLambdaFunctionEts(): () => int {
    return lambdaFunctionEts;
}

function genericFunctionEts<T>(arg: T): T {
    return arg;
}

class ClassToExtendEts {
    public x: number;
    value(): int {
        return 1;
    }
}

function functionArgTypeAnyEts(arg: Any): Any {
    return arg;
}

function functionArgTypeUnknownEts(arg: Any): Any {
    return arg;
}

function functionArgTypeUndefinedEts(arg: Any): Any {
    return arg;
}

function functionArgTypeTupleEts(arg: [number, string]): number {
    return arg[0];
}

function functionArgTypeCallableEts(functionToCall: () => Any): Any {
    return functionToCall();
}

function genericTypeParameterEts<T>(arg: T): String {
    return "" + arg!
}

function genericTypeReturnValueEts<T>(arg: int, value: T): T {
    let arr: FixedArray<T> = [value, value, value];
    return arr[arg];
}

class ClassWithDefaultParameterMethods {
    public intParameterMethod(arg: int = 1): int {
        return arg;
    }

    public static intParameterStaticMethod(arg: int = 1): int {
        return arg;
    }

    // #22991
    // public undefinedParameterMethod(arg: Any | undefined = undefined): Any | undefined {
    //     return arg;
    // }

    // #22991
    // public static undefinedParameterStaticMethod(arg: Any | undefined = undefined): Any | undefined {
    //     return arg;
    // }

    public overloadedMethod(arg: int) {
        // do nothing
    }

    public overloadedMethod(arg: string) {
        // do nothing
    }

    public overloadedStaticMethod(arg: int) {
        // do nothing
    }

    public overloadedStaticMethod(arg: string) {
        // do nothing
    }
}

class DerivedClassWithDefaultParameterMethods extends ClassWithDefaultParameterMethods {
    public intParameterMethod(arg: int): int {
        return -arg;
    }
}

function overloadedFunction(arg: int) {
    // do nothing
}

function overloadedFunction(arg: string) {
    // do nothing
}

function defaultIntParameterFunction(arg: int = 1): int {
    return arg;
}

// #22991
// function defaultUndefinedParameterFunction(arg: Any | undefined = undefined): Any | undefined {
//     return arg;
// }

function functionArgTypeOptionalPrimitive(arg?: Number): Number | undefined {
    return arg;
}

function functionArgTypePrimitive(arg: number): number {
    return arg;
}

function functionReturnTypePrimitive(): number {
    return 1;
}

function etsSumRestParams(...args: FixedArray<number>): number {
    let sum = 0;
    for (let n of args) {
        sum += n;
    }
    return sum;
}

function etsMultiplyLargBySumRestParams(arg0: number, ...args: FixedArray<number>): number {
    let sum = 0;
    for (let n of args) {
        sum += n;
    }
    return arg0 * sum;
}

function etsMultiplySum2argsBySumRestParams(arg0: number, arg1: number, ...args: FixedArray<number>): number {
    let sum = 0;
    for (let n of args) {
        sum += n;
    }
    return sum * (arg0 + arg1);
}

class RestParamsTest {
    x = 2;
    foo() {
        return 1;
    }
    sumInts(...args: FixedArray<Int>) {
        let sum = 0;
        for (let n of args) {
            sum += n;
        }
        return sum;
    }
};

function etsCallFooRestParams(...args: FixedArray<RestParamsTest>): number {
    let sum = 0;
    for (let s of args) {
        sum += s.foo();
        sum += s.x;
    }
    return sum;
}

function etsConcatStringsRestParams(...args: FixedArray<String>): String {
    let str = "";
    for (let s of args) {
        str += s;
    }
    return str;
}

function functionArgTypeConflictArray(): Array<Object> {
    let arr = new Array<Object>();
    arr.push(123);
    arr.push("foo");
    return arr;
}

function functionArgTypeConflictBoolean(): Boolean {
    return Boolean.valueOf(true);
}

function functionArgTypeConflictString(): string {
    return 'foo';
}

function functionArgTypeConflictArraybuffer(): ArrayBuffer {
    let arr = new ArrayBuffer(10);
    //arr.set(0, 123 as byte);
    return arr;
}

function functionArgTypeConflictDataview(): DataView {
    const buffer = new ArrayBuffer(8);
    const view = new DataView(buffer);
    view.setInt32(0, 123);
    return view;
}

function functionArgTypeConflictDate(): Date {
    return new Date(1);
}

function functionArgTypeConflictError(): Error {
    return new Error("foo");
}

function functionArgTypeConflictMap(): Map<string, string> {
    const map = new Map<string, string>();
    map.set('foo', 'bar');
    return map;
}

function functionArgTypeConflictObject(): Object {
    return new Object();
}

function GCODESRuntimeCleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let fullGcId = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(fullGcId);
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
}

interface InterfaceWithPropsAndMethod {
    numVal: number;
    stringVal: string;

    foo(x: int): string;
}

class ClassImplementingInterfaceWithPropsAndMethod implements InterfaceWithPropsAndMethod {
    public numVal: number = 0;
    public stringVal: string = '';

    constructor(numVal: number, stringVal: string) {
        this.numVal = numVal
        this.stringVal = stringVal
    }

    public foo(x: int): string {
        let result = this.stringVal + ' ' + this.numVal + ' ' + x;
        return result;
    }

    public getNumVal(): number {
        return this.numVal;
    }
}

function functionArgIsInterface(arg: InterfaceWithPropsAndMethod): string {
    let stringVal = arg.stringVal;
    let numVal = arg.numVal;
    arg.numVal = numVal * 2;
    return arg.foo(123);
}

interface Unpackable<T> {
    getValue(): T;
}

class Boxx<T> implements Unpackable<T> {
    value_: T;

    constructor(value: T) {
        this.value_ = value;
    }

    public getValue(): T {
        return this.value_;
    }
}

function unpackValue<T>(arg: Unpackable<T>): T {
    let result = arg.getValue();
    return result;
}

interface InterfaceWithUnionEts {
    methodWithUnion(arg: string | double): string | double;
}

class ImplInterfaceWithUnionEts implements InterfaceWithUnionEts {
    public override methodWithUnion(arg: string | double): string | double {
        return arg;
    }
}

function newInterfaceWithUnionEts(): InterfaceWithUnionEts {
    return new ImplInterfaceWithUnionEts();
}

class ClassWithUnionGetterSetterEts {
    private _value: string | int = 1;

    get value() {
        return this._value;
    }

    set value(theValue: string | int) {
        this._value = theValue;
    }
}

function functionArgTypePrimitiveUnion(arg: number | string): number | string {
    return arg;
}

function genericFunctionWithUnionEts<T extends string | number>(arg: T): T {
    return arg;
}

function functionRestParams(...numbers: FixedArray<number>): string {
    return numbers.toString();
}

function functionSpreadParams(arr: FixedArray<number>): string {
    return functionRestParams(...arr)
}

function functionRestParamsUnion(...restParams: FixedArray<(number | string)>): string {
    return restParams.toString();
}

function functionArgTypeOptionalPrimitiveUnion(arg?: int | boolean): int | boolean | undefined {
    return arg;
}

async function asyncFunctionLiteral(): Promise<int> {
    return 1;
}

async function asyncFunctionAny(arg: Any): Promise<Any> {
    return arg;
}

async function asyncFunctionExtraSet(tuple: [number, string]): Promise<number> {
    return tuple[0];
}

async function asyncFunctionSubsetByRef(func: () => Any): Promise<Any> {
    return func;
}

async function asyncFunctionSubsetByValue(num: Number): Promise<Number> {
    return num;
}

async function asyncFunctionUserClass(obj: ClassWithMethodEts): Promise<ClassWithMethodEts> {
    return obj;
}

async function asyncNewInterfaceWithMethodEts(): Promise<InterfaceWithMethodEts> {
    return newInterfaceWithMethodEts();
}

async function asyncFunctionUserInterface(obj: InterfaceWithMethodEts): Promise<int> {
    return obj.methodInInterface();
}

class ClassReturnThis {
  returnThis(): this {
    return this;
  }
  returnThisAsAny(): Any {
    return this;
  }
}

function functionReturnOmittedValue(): void {
    return;
}

function functionRestParameter(...arg: FixedArray<number>): number {
    return arg[0];
}

function functionSpreadParameter(arg1: number, arg2: number): number {
    return arg1 + arg2;
}

function functionOverloaded(): void {
}

function functionOverloaded(arg: number): void {
}

function functionOverload(): number {
    functionOverloaded();
    functionOverloaded(1);
    return 1;
}

function functionCallableReturnValue(): () => int {
    const value = () => 1;
    return value;
}

// NOTE(nikitayegorov) #18198 enable this after literal types are fixed
type TypeString = string; // replace with '1' when #18198 is resolved
type TypeStringU = string; // replace with "1" | "2" when #18198 is resolved
type PrimitiveA = int; // replace with 0 | 1 when #18198 is resolved
type PrimitiveB = int; // replace with 1 | 2 when #18198 is resolved
type PrimitiveAB = PrimitiveA | PrimitiveB;

interface A {
    a: number;
}
interface B {
    b: string;
}

interface AB extends A, B { }


function functionIntersectionTypePrimitive(arg: PrimitiveAB): PrimitiveAB {
  const ret: PrimitiveAB = arg;
  return ret;
}

function functionIntersectionTypeObject(): AB {
    let ab: AB = {
        a: 1,
        b: "1"
    };
    return ab;
}

function functionArgStringLiteralType(arg: TypeString): TypeString {
    return arg as TypeString;
}

function functionArgStringLiteralTypeUnion(arg: TypeStringU): TypeStringU {
    return arg as TypeStringU;
}

function fnWithArgOfTypeSymbol(arg: Any): Any {
    return arg;
}

function fnWithArgOfClassWithSymbolProperty(arg: ClassToExtendEts): ClassToExtendEts {
    arg.x = 1;
    return arg;
}

class RefType1 {
    type: number = 1
    value: string = 'valueone'
}

class RefType2 {
    type: number = 2
    value: string = 'valuetwo'
}

function functionReturnsCompositeType(arg: Any): RefType1 | RefType2 {
    const seed = arg as number;
    return seed > 1 ? new RefType2() : new RefType1();
}
