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

export const tsNumber = 1;
export const tsString = 'string';

interface AgeInterface {
    age: number;
}

interface NameInterface {
    name: string;
}

export type AgeNameInterface = AgeInterface & NameInterface;

export class AgeNameInterfaceClass implements AgeNameInterface {
    public name: string;
    public age: number;

    constructor(name: string, age: number) {
        throwExceptionInterface({ name, age });

        this.name = name;
        this.age = age;
    }

    public createUser(name: string, age: number): AgeNameInterfaceClass {
        throwExceptionInterface({ name, age });

        return new AgeNameInterfaceClass(name, age);
    }
}

export function createAgeNameClassInterfaceFromTs(): boolean {
    new AgeNameInterfaceClass(tsString, tsNumber);
    return true;
}

export function callMethodAgeNameClassInterfaceFromTs(): boolean {
    const user = new AgeNameInterfaceClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceAgeNameInterfaceClass = new AgeNameInterfaceClass(tsString, tsNumber);

export class ChildAgeNameInterfaceClass extends AgeNameInterfaceClass { };

export function createChildAgeNameClassInterfaceFromTs(): boolean {
    new ChildAgeNameInterfaceClass(tsString, tsNumber);
    return true;
}

export function callMethodChildAgeNameClassInterfaceFromTs(): boolean {
    const user = new ChildAgeNameInterfaceClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceChildAgeNameInterfaceClass = new ChildAgeNameInterfaceClass(tsString, tsNumber);

export function checkIntersectionInterface(arg: AgeNameInterface): boolean {
    if (checkInterfaceAge(arg) && checkInterfaceName(arg)) { return true };

    return false;
}

function checkInterfaceAge(object: AgeInterface): object is AgeInterface {
    const key = 'age';
    return key in object && typeof object[key] === 'number';
}

function checkInterfaceName(object: NameInterface): object is NameInterface {
    const key = 'name';
    return key in object && typeof object[key] === 'string';
}

function throwExceptionInterface(arg: AgeNameInterface): void {
    const check = checkIntersectionInterface(arg as AgeNameInterfaceClass);
    if (!check) {
        throw new Error('Invalid input');
    }
}
//<< generic type
export function checkAgeNameGeneric<T extends AgeInterface & NameInterface>(arg: T): boolean {
    if (checkTypeGenericAge(arg) && checkTypeGenericName(arg)) { return true };

    return false;
}

function checkTypeGenericAge<T extends AgeInterface>(object: T): boolean {
    const key = 'age';
    return key in object && typeof object[key] === 'number';
}

function checkTypeGenericName<T extends NameInterface>(object: T): boolean {
    const key = 'name';
    return key in object && typeof object[key] === 'string';
}

export class AgeNameGenericClass<T extends AgeNameInterface> implements AgeNameInterface {
    public name: string;
    public age: number;

    constructor(name: string, age: number) {
        throwExceptionGeneric({ name, age });

        this.name = name;
        this.age = age;
    }

    public createUser<U extends T>(name: string, age: number): AgeNameGenericClass<U> {
        throwExceptionGeneric({ name, age });

        return new AgeNameInterfaceClass(name, age);
    }
}

export function createAgeNameClassGenericFromTs(): boolean {
    new AgeNameGenericClass(tsString, tsNumber);
    return true;
}

export function callMethodAgeNameClassGenericFromTs(): boolean {
    const user = new AgeNameGenericClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceAgeNameGenericClass = new AgeNameGenericClass<AgeNameInterface>(tsString, tsNumber);

export class ChildAgeNameGenericClass extends AgeNameGenericClass<AgeNameInterface> { };

export function createChildAgeNameClassGenericFromTs(): boolean {
    new ChildAgeNameGenericClass(tsString, tsNumber);
    return true;
}

export function callMethodChildAgeNameClassGenericFromTs(): boolean {
    const user = new ChildAgeNameGenericClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceChildAgeNameGenericClass = new ChildAgeNameGenericClass(tsString, tsNumber);

function throwExceptionGeneric<T extends AgeNameInterface>(arg: T): void {
    const check = checkIntersectionInterface(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
// type union
type AgeUnion = {
    age: number | boolean;
};

type NameUnion = {
    name: string | number;
};

export type AgeNameUnionType = AgeUnion & NameUnion;

export function checkUnionType(arg: AgeNameUnionType): boolean {
    if (checkTypeAgeUnion(arg) && checkTypeNameUnion(arg)) { return true };

    return false;
}

function checkTypeAgeUnion(object: AgeUnion): object is AgeUnion {
    const key = 'age';
    return key in object && (typeof object[key] === 'number' || typeof object[key] === 'boolean');
}

function checkTypeNameUnion(object: NameUnion): object is NameUnion {
    const key = 'name';
    return key in object && (typeof object[key] === 'string' || typeof object[key] === 'number');
}

export class AgeNameUnionClass implements AgeNameUnionType {
    public name: string | number;
    public age: number | boolean;

    constructor(name: string | number, age: number | boolean) {
        throwExceptionUnion({ name, age });

        this.name = name;
        this.age = age;
    }

    public createUser(name: string | number, age: number | boolean): AgeNameUnionClass {
        throwExceptionUnion({ name, age });

        return new AgeNameUnionClass(name, age);
    }
}

export function createAgeNameUnionClassFromTs(): boolean {
    new AgeNameUnionClass(tsString, tsNumber);
    return true;
}

export function callMethodAgeNameUnionClassFromTs(): boolean {
    const user = new AgeNameUnionClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceAgeNameUnionClass = new AgeNameUnionClass(tsString, tsNumber);

export class ChildAgeNameUnionClass extends AgeNameUnionClass { };

export function createChildAgeNameUnionClassFromTs(): boolean {
    new ChildAgeNameUnionClass(tsString, tsNumber);
    return true;
}

export function callMethodChildAgeNameUnionClassFromTs(): boolean {
    const user = new ChildAgeNameUnionClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceChildAgeNameUnionClass = new ChildAgeNameUnionClass(tsString, tsNumber);

function throwExceptionUnion(arg: AgeNameUnionType): void {
    const check = checkUnionType(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
// literal type
type AgeLiteral = {
    age: 1 | true;
};

type NameLiteral = {
    name: 'string' | 1;
};

export type AgeNameLiteralType = AgeLiteral & NameLiteral;

export function checkLiteralType(arg: AgeNameLiteralType): boolean {
    if (checkTypeAgeLiteral(arg) && checkTypeNameLiteral(arg)) { return true };

    return false;
}

function checkTypeAgeLiteral(object: AgeLiteral): object is AgeLiteral {
    const key = 'age';
    return key in object && (object[key] === 1 || object[key] === true);
}

function checkTypeNameLiteral(object: NameLiteral): object is NameLiteral {
    const key = 'name';
    return key in object && (object[key] === 'string' || object[key] === 1);
}

export class AgeNameLiteralClass implements AgeNameLiteralType {
    public name: 'string' | 1;
    public age: 1 | true;

    constructor(name: 'string' | 1, age: 1 | true) {
        throwExceptionLiteral({ name, age });

        this.name = name;
        this.age = age;
    }

    public createUser(name: 'string' | 1, age: 1 | true): AgeNameLiteralClass {
        throwExceptionLiteral({ name, age });

        return new AgeNameLiteralClass(name, age);
    }
}

export function createAgeNameLiteralClassFromTs(): boolean {
    new AgeNameLiteralClass(tsString, tsNumber);
    return true;
}

export function callMethodAgeNameLiteralClassFromTs(): boolean {
    const user = new AgeNameLiteralClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceAgeNameLiteralClass = new AgeNameLiteralClass(tsString, tsNumber);

export class ChildAgeNameLiteralClass extends AgeNameLiteralClass { };

export function createChildAgeNameLiteralClassFromTs(): boolean {
    new ChildAgeNameLiteralClass(tsString, tsNumber);
    return true;
}

export function callMethodChildAgeNameLiteralClassFromTs(): boolean {
    const user = new ChildAgeNameLiteralClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}

export const instanceChildAgeNameLiteralClass = new ChildAgeNameLiteralClass(tsString, tsNumber);

function throwExceptionLiteral(arg: AgeNameLiteralType): void {
    const check = checkLiteralType(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
