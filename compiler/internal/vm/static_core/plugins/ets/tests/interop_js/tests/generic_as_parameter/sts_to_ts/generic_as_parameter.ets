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

package generic_as_parameter.test;

type Union = string | int;
type TupleT = [int, boolean];

const etsInt = 1;
const etsString = 'string';
const etsBool = true;
const etsArr: FixedArray<int>;
const etsObj: object;
const etsUnion: Union;
const etsTuple: TupleT;
static {
    etsArr = [etsInt];
    etsObj = {};
    etsUnion = etsInt;
    etsTuple = [etsInt, etsBool];
}


// NOTE: issue (1835) enable this after fix Literal type
// type Literal = 1 | 'string';

// const etsLiteral: Literal = etsInt;

// function genericDefaultLiteralCallFromEts(): Literal {
//     return genericDefaultLiteral<Literal>(etsLiteral);
// }

// function genericDefaultLiteral<T = Literal>(arg: T): T {
//     return arg;
// }

function anyTypeParameter<T>(arg: T): T {
    return arg
}

function anyTypeParameterExplicitCallFromEtsInt(): int {
    return anyTypeParameter<int>(etsInt);
}

function anyTypeParameterExplicitCallFromEtsString(): string {
    return anyTypeParameter<string>(etsString);
}

function anyTypeParameterExplicitCallFromEtsBool(): boolean {
    return anyTypeParameter<boolean>(etsBool);
}

function anyTypeParameterExplicitCallFromEtsArr(): FixedArray<int> {
    return anyTypeParameter<FixedArray<int>>(etsArr);
}

function anyTypeParameterExplicitCallFromEtsObj(): object {
    return anyTypeParameter<object>(etsObj);
}

function anyTypeParameterExplicitCallFromEtsUnion(): Union {
    return anyTypeParameter<Union>(etsUnion);

}

function anyTypeParameterExplicitCallFromEtsTuple(): TupleT {
    return anyTypeParameter<TupleT>(etsTuple);
}

function genericDefaultInt<T = int>(arg: T): T {
    return arg;
}

function genericDefaultString<T = string>(arg: T): T {
    return arg;
}

function genericDefaultBool<T = boolean>(arg: T): T {
    return arg;
}

function genericDefaultArr<T = FixedArray<int>>(arg: T): T {
    return arg;
}

function genericDefaultObj<T = object>(arg: T): T {
    return arg;
}

function genericDefaultUnion<T = Union>(arg: T): T {
    return arg;
}

function genericDefaultTuple<T = TupleT>(arg: T): T {
    return arg;
}

function genericDefaultIntCallFromEts(): int {
    return genericDefaultInt<int>(etsInt);
}

function genericDefaultStringCallFromEts(): string {
    return genericDefaultString<string>(etsString);
}

function genericDefaultBoolCallFromEts(): boolean {
    return genericDefaultBool<boolean>(etsBool);
}

function genericDefaultArrCallFromEts(): FixedArray<int> {
    return genericDefaultArr<FixedArray<int>>(etsArr);
}

function genericDefaultObjCallFromEts(): object {
    return genericDefaultObj<object>(etsObj);
}

function genericDefaultUnionCallFromEts(): Union {
    return genericDefaultUnion<Union>(etsUnion);
}

function genericDefaultTupleCallFromEts(): TupleT {
    return genericDefaultTuple<TupleT>(etsTuple);
}


interface GInterface {
    data: string;
}

class GInterfaceClass implements GInterface {
    data = 'string';
}

function genericExtendInterface<T extends GInterfaceClass>(arg: T): T {
    return arg;
}

function genericExtendInterfaceFromEts(): GInterfaceClass {
    const obj: GInterfaceClass = { data: 'string' }

    return genericExtendInterface(obj as GInterfaceClass);
}

// NOTE: issue (17924) enable this after fix class implements interface property
// class TypeClass {
//     data = 'string';
// }

// type GType = TypeClass;

// class GTypeClass implements GType {
//     data = 'string';
// }

// function genericExtendType<T extends GTypeClass>(arg: T): T {
//     return arg;
// }

// function genericExtendTypeFromEts(): GTypeClass {
//     const obj: GTypeClass = { data: 'string' }
//     return genericExtendInterface(obj as GTypeClass);
// }

class User {
    data = 'string';
}

function genericExtendClass<T extends User>(arg: T): T {
    return arg;
}

function genericExtendClassFromEts(): User {
    const obj: User = { data: 'string' }
    return genericExtendClass(obj as User);
}

function tupleGeneric<T, U>(arg: FixedArray<T|U>): [U, T] {
    return [arg[1] as U, arg[0] as T]
}

function collectGeneric<T>(...args: FixedArray<T>): FixedArray<T> {
    return args;
}

class UserClass<T, U> {
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

function createClassFromEts(): UserClass<string, int> {
    return new UserClass<string, int>(etsString, etsInt);
}
