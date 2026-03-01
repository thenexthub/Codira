/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

package class_proxy.test;

class jsvars {
    static global = JSRuntime.getGlobal();
};

class ListNode {
    constructor(tag: double, next?: ListNode) {
        this.next = next;
        this.tag = tag;
    }
    public getNext(): ListNode | undefined {
        return this.next;
    }
    public setNext(v: ListNode): void {
        this.next = v;
    }

    public next?: ListNode;
    public tag: double;
};

class ProxyEscapeCtor {
    constructor(cb: JSValue) {
        ESValue.wrap(cb).invoke(this);
    }
};

class ProxyClassCalls {
    constructor(cb: JSValue) {
        ESValue.wrap(cb).invoke();
    }
    static call(cb: JSValue) {
        ESValue.wrap(cb).invoke();
    }
};

class PrimitivesAccess {
    public getfByte(): byte { return this.fByte; }
    public getfShort(): short { return this.fShort; }
    public getfInt(): int { return this.fInt; }
    public getfLong(): long { return this.fLong; }
    public getfFloat(): float { return this.fFloat; }
    public getfDouble(): double { return this.fDouble; }
    public getfChar(): char { return this.fChar; }
    public getfBoolean(): boolean { return this.fBoolean; }

    public setfByte(v: byte): void { this.fByte = v; }
    public setfShort(v: short): void { this.fShort = v; }
    public setfInt(v: int): void { this.fInt = v; }
    public setfLong(v: long): void { this.fLong = v; }
    public setfFloat(v: float): void { this.fFloat = v; }
    public setfDouble(v: double): void { this.fDouble = v; }
    public setfChar(v: char): void { this.fChar = v; }
    public setfBoolean(v: boolean): void { this.fBoolean = v; }

    public fByte: byte;
    public fShort: short;
    public fInt: int;
    public fLong: long;
    public fFloat: float;
    public fDouble: double;
    public fChar: char;
    public fBoolean: boolean;
};

class PrimitivesAccessStatic {
    public static getfByte(): byte { return PrimitivesAccessStatic.fByte; }
    public static getfShort(): short { return PrimitivesAccessStatic.fShort; }
    public static getfInt(): int { return PrimitivesAccessStatic.fInt; }
    public static getfLong(): long { return PrimitivesAccessStatic.fLong; }
    public static getfFloat(): float { return PrimitivesAccessStatic.fFloat; }
    public static getfDouble(): double { return PrimitivesAccessStatic.fDouble; }
    public static getfChar(): char { return PrimitivesAccessStatic.fChar; }
    public static getfBoolean(): boolean { return PrimitivesAccessStatic.fBoolean; }

    public static setfByte(v: byte): void { PrimitivesAccessStatic.fByte = v; }
    public static setfShort(v: short): void { PrimitivesAccessStatic.fShort = v; }
    public static setfInt(v: int): void { PrimitivesAccessStatic.fInt = v; }
    public static setfLong(v: long): void { PrimitivesAccessStatic.fLong = v; }
    public static setfFloat(v: float): void { PrimitivesAccessStatic.fFloat = v; }
    public static setfDouble(v: double): void { PrimitivesAccessStatic.fDouble = v; }
    public static setfChar(v: char): void { PrimitivesAccessStatic.fChar = v; }
    public static setfBoolean(v: boolean): void { PrimitivesAccessStatic.fBoolean = v; }

    public static fByte: byte;
    public static fShort: short;
    public static fInt: int;
    public static fLong: long;
    public static fFloat: float;
    public static fDouble: double;
    public static fChar: char;
    public static fBoolean: boolean;
}

class UClass1 { };
class UClass2 { };
function foo(resolve: (value: Object) => void): void { resolve(new Object()); }

class ReferencesAccess {
    public getfUClass1() { return this.fUClass1; }
    public getfUClass2() { return this.fUClass2; }
    public getfString() { return this.fString; }
    public getfJSValue() { return this.fJSValue; }
    public getfPromise() { return this.fPromise; }
    public getfAString() { return this.fAString; }
    public getfAUClass1() { return this.fAUClass1; }

    public setfUClass1(v?: UClass1): void { this.fUClass1 = v; }
    public setfUClass2(v?: UClass2): void { this.fUClass2 = v; }
    public setfString(v?: String): void { this.fString = v; }
    public setfJSValue(v?: Any): void { this.fJSValue = v; }
    public setfPromise(v?: Promise<Object>): void { this.fPromise = v; }
    public setfAString(v?: FixedArray<String>): void { this.fAString = v; }
    public setfAUClass1(v?: FixedArray<UClass1>): void { this.fAUClass1 = v; }

    public fUClass1?: UClass1 = new UClass1();
    public fUClass2?: UClass2 = new UClass2();
    public fString?: String = new String();
    public fJSValue?: Any = '0' as Any;
    public fPromise?: Promise<Object> = new Promise<Object>(foo);
    public fAString?: FixedArray<String> = [];
    public fAUClass1?: FixedArray<UClass1> = [];
};

class ReferencesAccessStatic {
    public static getfUClass1(): UClass1 { return ReferencesAccessStatic.fUClass1; }
    public static getfUClass2(): UClass2 { return ReferencesAccessStatic.fUClass2; }
    public static getfString(): String { return ReferencesAccessStatic.fString; }
    public static getfJSValue(): Any { return ReferencesAccessStatic.fJSValue; }
    public static getfPromise(): Promise<Object> { return ReferencesAccessStatic.fPromise; }

    public static setfUClass1(v: UClass1): void { ReferencesAccessStatic.fUClass1 = v; }
    public static setfUClass2(v: UClass2): void { ReferencesAccessStatic.fUClass2 = v; }
    public static setfString(v: String): void { ReferencesAccessStatic.fString = v; }
    public static setfJSValue(v: Any): void { ReferencesAccessStatic.fJSValue = v; }
    public static setfPromise(v: Promise<Object>): void { ReferencesAccessStatic.fPromise = v; }

    public static fUClass1: UClass1 = new UClass1();
    public static fUClass2: UClass2 = new UClass2();
    public static fString: String = new String();
    public static fJSValue: Any = '0' as Any;
    public static fPromise: Promise<Object> = new Promise<Object>(foo);
};

class FooBase {
    public fBase: String = "fBase";
    public static sfBase: String = "sfBase";

    public getAsObject(): Object { return this; }
    public static createAsObject(): Object { return new FooDerived(); }

    public override toString(): String {
        return "FooBase toString";
    }
    public fnBase(): String { return "fnBase"; }
};

class FooDerived extends FooBase {
    public fDerived: String = "fDerived";
    public static sfDerived: String = "sfDerived";

    public getAsFooBase(): FooBase { return this; }
    public static createAsFooBase(): FooBase { return new FooDerived(); }

    public override toString(): String {
        return "FooDerived toString";
    }
};

class FooError1 extends Error {
    public fError1: String = "fError1";
};

class FooError2 extends FooError1 {
    public fError2: String = "fError2";

    public override toString(): String {
        return "FooError2 toString";
    }
}

function IdentityObject(a: Object): Object {
    return a;
}
function IdentityError(a: Error): Error {
    return a;
}
