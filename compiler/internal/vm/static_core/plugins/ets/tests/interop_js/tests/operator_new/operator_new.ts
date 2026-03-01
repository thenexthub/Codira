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

export class NoArgClass {
    message:string;
    constructor() {
        this.message = 'Class without args';
    }
}

export class OneArgClass {
    name:string;
    constructor(name:string) {
        this.name = name;
    }
}

export class SubClass extends OneArgClass {
    name: string;
    surname:string;
    constructor(name:string, surname:string) {
        super(name);
        this.surname = surname;
    }
}

export class TwoArgClass {
    name:string;
    city:string;
    constructor(name:string, city:string) {
        this.name = name;
        this.city = city;
    }
}

export class ManyArgsClass {
    name:string;
    age:number;
    country:string;
    city:string;
    constructor(name:string, age:number, country:string, city:string) {
        this.name = name;
        this.age = age;
        this.country = country;
        this.city = city;
    }
    showInfo(): string {
        return `Name: ${this.name}, age:${this.age}, country: ${this.country}, city: ${this.city}`;
    }
}

export function fnTestUser(name:string, surname:string): void {
    this.name = name;
    this.surname = surname;
}

fnTestUser.prototype.showInfo = function():string {
    return `Name: ${this.name}, surname: ${this.surname}`;
};

function testfn():string {
    return 'test';
};

export const TestObject = {
    name:'Test',
    surname:'Test'
};

export const primitive = 10;