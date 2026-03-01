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

export type Union = string | number;
export type TupleT = [number, boolean];
export type Literal = 1 | 'string';

export const tsNumber = 1;
export const tsString = 'string';
export const tsBool = true;
export const tsArr = [tsNumber];
export const tsObj = {};
export const tsUnion: Union = tsNumber;
export const tsTuple: TupleT = [tsNumber, tsBool];
export const tsLiteral: Literal = tsNumber;


export function anyTypeParameter<T>(arg: T): T {
    return arg;
}

export function anyTypeParameterExplicitCallFromTsInt(): number {
    return anyTypeParameter<number>(tsNumber);
}

export function anyTypeParameterExplicitCallFromTsString(): string {
    return anyTypeParameter<string>(tsString);
}

export function anyTypeParameterExplicitCallFromTsBool(): boolean {
    return anyTypeParameter<boolean>(tsBool);
}

export function anyTypeParameterExplicitCallFromTsArr(): number[] {
    return anyTypeParameter<number[]>(tsArr);
}

export function anyTypeParameterExplicitCallFromTsObj(): object {
    return anyTypeParameter<object>(tsObj);
}

export function anyTypeParameterExplicitCallFromTsUnion(): Union {
    return anyTypeParameter<Union>(tsUnion);

}

export function anyTypeParameterExplicitCallFromTsTuple(): TupleT {
    return anyTypeParameter<TupleT>(tsTuple);
}

export function anyTypeParameterExplicitCallFromTsLiteral(): Literal {
    return anyTypeParameter<Literal>(tsLiteral);
}

export type gFunType = <T> (arg: T) => T;

export const genericTypeFunctionReturnAny: gFunType = <T>(arg: T): T => {
    return arg;
};

export function genericTypeFunctionExplicitCallFromTsInt(): number {
    return genericTypeFunctionReturnAny<number>(tsNumber);
}

export function genericTypeFunctionExplicitCallFromTsString(): string {
    return genericTypeFunctionReturnAny<string>(tsString);
}

export function genericTypeFunctionExplicitCallFromTsBool(): boolean {
    return genericTypeFunctionReturnAny<boolean>(tsBool);
}

export function genericTypeFunctionExplicitCallFromTsArr(): number[] {
    return genericTypeFunctionReturnAny<number[]>(tsArr);
}

export function genericTypeFunctionExplicitCallFromTsObj(): object {
    return genericTypeFunctionReturnAny<object>(tsObj);
}

export function genericTypeFunctionExplicitCallFromTsUnion(): Union {
    return genericTypeFunctionReturnAny<Union>(tsUnion);

}

export function genericTypeFunctionExplicitCallFromTsTuple(): TupleT {
    return genericTypeFunctionReturnAny<TupleT>(tsTuple);
}

export function genericTypeFunctionExplicitCallFromTsLiteral(): Literal {
    return genericTypeFunctionReturnAny<Literal>(tsLiteral);
}

export type gFunExtendNumber = <T extends number> (arg: T) => T;
export type gFunExtendString = <T extends string> (arg: T) => T;
export type gFunExtendBool = <T extends boolean> (arg: T) => T;
export type gFunExtendArr = <T extends number[]> (arg: T) => T;
export type gFunExtendObj = <T extends object> (arg: T) => T;
export type gFunExtendUnion = <T extends Union> (arg: T) => T;
export type gFunExtendTuple = <T extends TupleT> (arg: T) => T;
export type gFunExtendLiteral = <T extends Literal> (arg: T) => T;

export const extendGenericNumber: gFunExtendNumber = (arg) => arg;

export const extendGenericString: gFunExtendString = (arg) => arg;

export const extendGenericBool: gFunExtendBool = (arg) => arg;

export const extendGenericArr: gFunExtendArr = (arg) => arg;

export const extendGenericObj: gFunExtendObj = (arg) => arg;

export const extendGenericUnion: gFunExtendUnion = (arg) => arg;

export const extendGenericTuple: gFunExtendTuple = (arg) => arg;

export const extendGenericLiteral: gFunExtendLiteral = (arg) => arg;

interface GInterface {
    data: string;
}

export class GInterfaceClass implements GInterface {
    data = 'string';
}

type GType = {
    data: 'string';
};

export class GTypeClass implements GType {
    data: 'string' = 'string';
} 

export function genericExtendInterface<T extends GInterfaceClass>(arg: T): T {
    return arg;
}

export function genericExtendType<T extends GTypeClass>(arg: T): T {
    return arg;
}

export function tupleGeneric<T, U>(arg: [T, U]): [U, T] {
    return [arg[1], arg[0]];
}

export function collectGeneric<T>(...args: T[]): T[] {
    return args;
}

export class UserClass<T, U> {
    private name: T;
    private age: U;

    constructor(name: T, age: U) {
        this.name = name;
        this.age = age;
    }

    public getName(): T {
        return this.name;
    }

    public getAge(): U {
        return this.age;
    }
}

export function createClassFromTs(): UserClass<string, number> {
    return new UserClass<string, number>(tsString, tsNumber);
}

export const userClassInstance = new UserClass<string, number>(tsString, tsNumber);


export function genericDefaultInt<T = number>(arg: T): T {
    return arg;
}

export function genericDefaultString<T = string>(arg: T): T {
    return arg;
}

export function genericDefaultBool<T = boolean>(arg: T): T {
    return arg;
}

export function genericDefaultArr<T = number[]>(arg: T): T {
    return arg;
}

export function genericDefaultObj<T = object>(arg: T): T {
    return arg;
}

export function genericDefaultUnion<T = Union>(arg: T): T {
    return arg;
}

export function genericDefaultTuple<T = TupleT>(arg: T): T {
    return arg;
}

export function genericDefaultLiteral<T = Literal>(arg: T): T {
    return arg;
}

export function genericDefaultIntCallFromTs(): number {
    return genericDefaultInt<number>(tsNumber);
}

export function genericDefaultStringCallFromTs(): string {
    return genericDefaultString<string>(tsString);
}

export function genericDefaultBoolCallFromTs(): boolean {
    return genericDefaultBool<boolean>(tsBool);
}

export function genericDefaultArrCallFromTs(): number[] {
    return genericDefaultArr<number[]>(tsArr);
}

export function genericDefaultObjCallFromTs(): object {
    return genericDefaultObj<object>(tsObj);
}

export function genericDefaultUnionCallFromTs(): Union {
    return genericDefaultUnion<Union>(tsUnion);
}

export function genericDefaultTupleCallFromTs(): TupleT {
    return genericDefaultTuple<TupleT>(tsTuple);
}

export function genericDefaultLiteralCallFromTs(): Literal {
    return genericDefaultLiteral<Literal>(tsLiteral);
}

export class DataClass {
    data = 'string';
}

export function genericExtendClass<T extends DataClass>(arg: T): T {
    return arg;
}
