/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;
let testAll = etsVm.getFunction('Ltest_any_static/ETSGLOBAL;', 'testAll');

export class Foo {
    bar: number = 0xcafe;
    baz: string = 'hello world';
}

export let fooInstance = new Foo();

export let arrayInstance = [0x55aa, 0x7c00];

export class NumberMap {
    [key: number]: number;
};

export let numberMapInstance = new NumberMap();
numberMapInstance[0] = 0xcafe;
numberMapInstance[1] = 0xbabe;

export function functionArg0(): number {
    return 0xcafe;
}

export function functionArg1(arg1: number): number {
    return arg1 + 1;
}

export function functionArg2(arg1: number, arg2: number): number {
    return arg1 + arg2;
}

export class Qux {
    methodArg0(): number {
        return 0xcafe;
    }

    methodArg1(arg1: number): number {
        return arg1 + 1;
    }

    methodArg2(arg1: number, arg2: number): number {
        return arg1 + arg2;
    }
}

export let quxInstance = new Qux();

export class TestNew0 {
    bar: number = 0xcafe;

    constructor() {
    }
}

export class TestNew1 {
    bar: number;

    constructor(_bar: number) {
        this.bar = _bar;
    }
}

export class TestNew2 {
    bar: number;

    constructor(lhs: number, rhs: number) {
        this.bar = lhs + rhs;
    }
}

export class Foobar {
    receivedUndefined: boolean;
    constructor(arg: Object | undefined, arg2: Object) {
        if (arg === undefined) {
            this.receivedUndefined = true;
        } else {
            this.receivedUndefined = false;
        }
    }
}

export let arrNum = new Array<Number>(1, 2, 3, 4);
export let arrFill = new Array<Number>(1, 2, 3, 4);
export let big_val = 1234567890n;

export class UserAccount {
    private _username: string;
    private _age: number;

    constructor(username: string, age: number) {
        this._username = username;
        this._age = age;
    }

    get username(): string {
        return this._username;
    }

    set username(newUsername: string) {
        if (newUsername.length < 3) {
            throw new Error('Username must be at least 3 characters long');
        }
        if (newUsername.length > 20) {
            throw new Error('Username cannot be longer than 20 characters');
        }
        this._username = newUsername;
    }

    get age(): number {
        return this._age;
    }

    set age(newAge: number) {
        if (newAge < 0) {
            throw new Error('Age cannot be negative');
        }
        if (newAge > 150) {
            throw new Error('Age cannot be greater than 150');
        }
        if (!Number.isInteger(newAge)) {
            throw new Error('Age must be an integer');
        }
        this._age = newAge;
    }
}

function main() {
    testAll();
}

main();
