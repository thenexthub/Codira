/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

package std.interop;

import {IterableIterator, IteratorResult} from "std/core";

// NOTE(from InteropTransferHelper.ets): #IC8LO3 remove local declaration after Any type is supported
type Any = Object | null | undefined

export type StaticOrESValue = ESValue | Object | null | undefined
export type ESValueCallbackData = object
export type ESValueCallback = (argThis: ESValue, argNewTgt: ESValue, args: FixedArray<ESValue>, data?: ESValueCallbackData) => ESValue
export class ESValuePropertyDescriptor {
    key: string | ESValue = ''
    call?: ESValueCallback
    getter?: ESValueCallback
    setter?: ESValueCallback
    value?: ESValue
    data?: ESValueCallbackData
}

export final class ESValue {
    // #IC51G4: this.ev should be able to storage anything
    private ev: Any
    private isStatic: boolean

    private static OBJECT_CLASS: Class = Class.of(new Object());
    private static NUMBER_CLASS: Class = Class.of(new Number());
    private static BOOLEAN_CLASS: Class = Class.of(new Boolean());
    private static STRING_CLASS: Class = Class.of(new String());
    private static BIGINT_CLASS: Class = Class.of(new BigInt());

    constructor(ev: Any) {
        this.ev = ev
        this.isStatic = false;
    }

    constructor(ev: Any, isStatic: boolean) {
        this.ev = ev
        this.isStatic = isStatic;
    }

    public static get Undefined(): ESValue {
        return new ESValue(undefined, true);
    }

    public static get Null(): ESValue {
        return new ESValue(null, true);
    }

    public static wrapBoolean(b: boolean): ESValue {
        return new ESValue(b, true);
    }

    public static wrapString(s: string): ESValue {
        return new ESValue(s, true);
    }

    public static wrapNumber(n: number): ESValue {
        return new ESValue(n, true);
    }

    public static wrapBigInt(bi: bigint): ESValue {
        return new ESValue(bi, true);
    }

    public static wrapByte(b: byte): ESValue {
        return new ESValue(b, true);
    }

    public static wrapShort(s: short): ESValue {
        return new ESValue(s, true);
    }

    public static wrapInt(i: int): ESValue {
        return new ESValue(i, true);
    }

    public static wrapLong(l: long): ESValue {
        return new ESValue(l, true);
    }

    public static wrapLongLossy(l: long): ESValue {
        return new ESValue(l, true);
    }

    public static wrapFloat(f: float): ESValue {
        return new ESValue(f, true);
    }

    public static wrapDouble(d: double): ESValue {
        return new ESValue(d, true);
    }

    public static wrap(o: Any): ESValue {
        let wrappedObject = new ESValue(o);
        if (o == undefined) {
            wrappedObject.isStatic = true;
        } else {
            wrappedObject.isStatic = JSRuntime.isStaticValue(o);
        }
        return wrappedObject;
    }

    public unwrap(): Any {
        return this.ev;
    }

    public isBoolean(): boolean {
        return this.typeOf() == "boolean";
    }

    public isString(): boolean {
        return this.typeOf() == "string";
    }

    public isNumber(): boolean {
        switch (this.typeOf()) {
            case "byte":
            case "short":
            case "int":
            case "long":
            case "float":
            case "number":
                return true;
        }
        return false;
    }

    public isBigInt(): boolean {
        return this.typeOf() == "bigint";
    }

    public isUndefined(): boolean {
        return this.typeOf() == "undefined";
    }

    public isNull(): boolean {
        return this.isStrictlyEqualTo(new ESValue(null));
    }

    public isStaticObject(): boolean {
        return this.isStatic;
    }

    public isECMAObject(): boolean {
        return !this.isStaticObject() &&
            !this.isBoolean() &&
            !this.isString() &&
            !this.isNumber() &&
            !this.isBigInt() &&
            !this.isUndefined() &&
            !this.isNull() &&
            !this.isFunction();
    }

    public isObject(): boolean {
        return this.isECMAObject() || this.isStaticObject();
    }

    public isFunction(): boolean {
        return this.typeOf() == "function";
    }

    public toBoolean(): boolean {
        if (!this.isBoolean()) {
            throw new TypeError("Boolean expected");
        }
        return this.ev as boolean;
    }

    public toString(): string {
        if (!this.isString()) {
            throw new TypeError("String expected");
        }
        return this.ev as string;
    }

    public toNumber(): number {
        if (!this.isNumber()) {
            throw new TypeError("Number expected");
        }

        const thisVal = this.ev
        if (thisVal instanceof Number) {
            return thisVal
        } else if (thisVal instanceof Int) {
            return thisVal.toDouble()
        }

        throw new TypeError("Invalid number: " + Class.ofAny(thisVal)?.getName())
    }

    public toBigInt(): bigint {
        if (!this.isBigInt()) {
            throw new TypeError("bigint expected");
        }
        return this.ev as bigint;
    }

    public toUndefined(): undefined {
        if (this.isUndefined()) {
            return undefined;
        }
        throw Error("Can not cast to undefined")
    }

    public toNull(): null {
        if (this.isNull()) {
            return null;
        }
        throw Error("Can not cast to null")
    }

    public toStaticObject(): object {
        if (this.isStatic) {
            return this.ev as Object;
        }
        throw Error("Can not cast to a static object")
    }

    public static areEqual(ev1: ESValue, ev2: ESValue): boolean {
        return ev1.ev == ev2.ev;
    }

    public static areStrictlyEqual(ev1: ESValue, ev2: ESValue): boolean {
        return ev1.ev === ev2.ev;
    }

    public static areEqualSafe(ev1: ESValue, ev2: ESValue): boolean {
        if (ev1.isUndefined() || ev1.isNull() &&
            (ev2.isUndefined() || ev2.isNull())) {
            return true;
        }
        return ESValue.areStrictlyEqual(ev1, ev2);
    }

    public isEqualTo(other: ESValue): boolean {
        return ESValue.areEqual(this, other);
    }

    public isStrictlyEqualTo(other: ESValue): boolean {
        return ESValue.areStrictlyEqual(this, other);
    }

    public isEqualToSafe(other: ESValue): boolean {
        return ESValue.areEqualSafe(this, other);
    }

    public instantiate(...args: FixedArray<StaticOrESValue>): ESValue {
        let argsAny: FixedArray<Any> = new FixedArray<Any>(args.length);
        // NOTE(www): ICMTI1, it should be handled in JSRuntime.
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argsAny[i] = arg.ev;
            } else {
                argsAny[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.instantiate(this.ev, ...argsAny))
    }

    public static instantiateEmptyObject(): ESValue {
        return new ESValue(JSRuntime.createObject());
    }

    public static instantiateEmptyArray(): ESValue {
        return new ESValue(JSRuntime.createArray());
    }

    public getProperty(name: string): ESValue {
        return new ESValue(JSRuntime.getPropertyObject(this.ev, name));
    }

    public getProperty(index: number): ESValue {
        if (Number.isInteger(index) && index >= 0) {
            return new ESValue(JSRuntime.getElementObject(this.ev, index.toInt()));
        }
        let numObj = ESValue.wrapNumber(index);
        return this.getProperty(numObj);
    }

    public getProperty(property: ESValue): ESValue {
        return new ESValue(JSRuntime.getProperty(this.ev, property.ev));
    }

    public getPropertySafe(name: string): ESValue {
        if (this.hasProperty(name)) {
            return this.getProperty(name);
        }
        return ESValue.Undefined;
    }

    public getPropertySafe(index: number): ESValue {
        if (JSRuntime.hasElement(this.ev, index.toInt())) {
            return this.getProperty(index);
        }
        return ESValue.Undefined;
    }

    public getPropertySafe(property: ESValue): ESValue {
        if (this.hasProperty(property)) {
            return this.getProperty(property);
        }
        return ESValue.Undefined;
    }

    public setProperty(name: string, value: StaticOrESValue): void {
        if (value instanceof ESValue) {
            JSRuntime.setPropertyObject(this.ev, name, (value as ESValue).ev);
            return;
        }
        JSRuntime.setPropertyObject(this.ev, name, ESValue.wrap(value).ev);
    }

    public setProperty(index: number, value: StaticOrESValue): void {
        let valueObj: Any;
        if (value instanceof ESValue) {
            valueObj = (value as ESValue).ev
        } else {
            valueObj = ESValue.wrap(value).ev;
        }
        if (Number.isInteger(index) && index >= 0) {
            JSRuntime.setElementObject(this.ev, index.toInt(), valueObj);
            return;
        }
        let numObj = ESValue.wrapNumber(index);
        this.setProperty(numObj, value);
    }

    public setProperty(property: ESValue, value: StaticOrESValue): void {
        if (value instanceof ESValue) {
            JSRuntime.setProperty(this.ev, property.ev, (value as ESValue).ev);
        } else {
            JSRuntime.setProperty(this.ev, property.ev, value);
        }
    }

    public hasProperty(property: ESValue): boolean {
        return JSRuntime.hasPropertyObject(this.ev, property.ev);
    }

    public hasProperty(name: string): boolean {
        return JSRuntime.hasProperty(this.ev, name);
    }

    public hasOwnProperty(property: ESValue): boolean {
        return JSRuntime.hasOwnPropertyJSValue(this.ev, property.ev)
    }

    public hasOwnProperty(name: string): boolean {
        return JSRuntime.hasOwnProperty(this.ev, name);
    }

    public invoke(...args: FixedArray<StaticOrESValue>): ESValue {
        let argsAny: FixedArray<Any> = new FixedArray<Any>(args.length);
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argsAny[i] = arg.ev;
            } else {
                argsAny[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.invoke(ESValue.Undefined.ev, this.ev, ...argsAny));
    }

    public invokeWithRecv(recv: ESValue, ...args: FixedArray<StaticOrESValue>): ESValue {
        let argsAny: FixedArray<Any> = new FixedArray<Any>(args.length);
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argsAny[i] = arg.ev;
            } else {
                argsAny[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.invoke(recv.ev, this.ev, ...argsAny));
    }

    public invokeMethod(method: string, ...args: FixedArray<StaticOrESValue>): ESValue {
        let callable = this.getPropertySafe(method);
        if (callable.isUndefined()) {
            throw Error("failed to get property");
        }
        let argsAny: FixedArray<Any> = new FixedArray<Any>(args.length);
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof ESValue) {
                let arg = args[i] as ESValue;
                argsAny[i] = arg.ev;
            } else {
                argsAny[i] = ESValue.wrap(args[i]).ev;
            }
        }
        return new ESValue(JSRuntime.invoke(this.ev, callable.ev, ...argsAny));
    }

    public keys(): IterableIterator<ESValue> {
        const global = ESValue.getGlobal();
        const globalObject = global.getProperty("Object");
        const keysFunc = globalObject.getProperty("keys");

        let keysIter = keysFunc.invoke(this);
        return new ESValueIterator(keysIter);
    }

    public values(): IterableIterator<ESValue> {
        const global = ESValue.getGlobal();
        const globalObject = global.getProperty("Object");
        const valuesFunc = globalObject.getProperty("values");

        let valuesIter = valuesFunc.invoke(this);
        return new ESValueIterator(valuesIter);
    }

    public entries(): IterableIterator<[ESValue, ESValue]> {
        const global = ESValue.getGlobal();
        const globalObject = global.getProperty("Object");
        const entriesFunc = globalObject.getProperty("entries");

        let entries = entriesFunc.invoke(this);
        return new ESValueEntriesIterator(entries);
    }

    public instanceOf(type: ESValue | Type): boolean {
        if (type instanceof ESValue) {
            return JSRuntime.instanceOfDynamic(this.ev, (type as ESValue).ev);
        }
        if ((type) instanceof ByteType
            || (type) instanceof ShortType
            || (type) instanceof IntType
            || (type) instanceof LongType
            || (type) instanceof FloatType
            || (type) instanceof DoubleType
        ) {
            return this.typeOf() == "number";
        } else if (type.isPrimitive()) {
            return this.typeOf() == type.getLiteral().toLowerCase();
        } else if (type instanceof ClassType) {
            return type.assignableFrom(Type.of(this.ev));
        }
        return false;
    }

    public typeOf(): String {
        return typeof this.ev;
    }

    public static load(module: string): ESValue {
        return new ESValue(JSRuntime.loadModule(module));
    }

    public static getGlobal(): ESValue {
        return new ESValue(JSRuntime.getGlobal());
    }

    public isPromise(): boolean {
        return this.ev instanceof Promise;
    }

    public toPromise(): Promise<ESValue> {
        if (!this.isPromise()) {
            throw new TypeError("Promise expected");
        }
        return new Promise<ESValue>((resolve, reject) => {
            let jsPromise = this.ev as Promise<Any>;
            jsPromise.then((value: Any) => {
                resolve(ESValue.wrap(value));
            }).catch((error) => {
                reject(error);
            });
        });
    }

    static defineClass(
        name: String,
        ctor: ESValueCallback,
        data?: ESValueCallbackData,
        properties?: FixedArray<ESValuePropertyDescriptor>,
        superC?: ESValue
    ): ESValue {
        throw Error("not implemented yet");
    }

    public $_iterator(): IterableIterator<ESValue> {
        return new ESValueIterator(this);
    }

    public iterator(): IterableIterator<ESValue> {
        return this.$_iterator();
    }
}

final class ESValueIterator implements IterableIterator<ESValue> {
    private iterator: ESValue;

    constructor(iterableObj: ESValue) {
        const global = ESValue.getGlobal();
        const symbol = global.getProperty("Symbol");
        const symbolIterator = symbol.getProperty("iterator");

        const symbolIteratorMethod = iterableObj.getPropertySafe(symbolIterator);
        if (symbolIteratorMethod.isUndefined()) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }

        this.iterator = symbolIteratorMethod.invokeWithRecv(iterableObj);
    }

    override next(): IteratorResult<ESValue> {
        let nextObj = this.iterator.invokeMethod("next");
        if (nextObj.getProperty("done").toBoolean()) {
            return new IteratorResult<ESValue>();
        }

        return new IteratorResult<ESValue>(nextObj.getProperty("value"));
    }

    override $_iterator(): IterableIterator<ESValue> {
        return this;
    }
}

final class ESValueEntriesIterator implements IterableIterator<[ESValue, ESValue]> {
    private iterator: ESValue;

    constructor(iterableObj: ESValue) {
        const global = ESValue.getGlobal();
        const symbol = global.getProperty("Symbol");
        const symbolIterator = symbol.getProperty("iterator");

        const symbolIteratorMethod = iterableObj.getPropertySafe(symbolIterator);
        if (symbolIteratorMethod.isUndefined()) {
            throw new Error('Can not retrieve [Symbol.iterator] from given object, object is not iterable.');
        }

        this.iterator = symbolIteratorMethod.invokeWithRecv(iterableObj);
    }

    override next(): IteratorResult<[ESValue, ESValue]> {
        let nextObj = this.iterator.invokeMethod("next");
        if (nextObj.getProperty("done").toBoolean()) {
            return new IteratorResult<[ESValue, ESValue]>();
        }
        let iterValue = nextObj.getProperty("value");
        let key = iterValue.getProperty(0);
        let value = iterValue.getProperty(1);

        return new IteratorResult<[ESValue, ESValue]>([key, value]);
    }

    override $_iterator(): IterableIterator<[ESValue, ESValue]> {
        return this;
    }
}
