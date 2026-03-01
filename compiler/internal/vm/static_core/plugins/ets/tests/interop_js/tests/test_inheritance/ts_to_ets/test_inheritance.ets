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

let baseAge: number = 1;
let baseFirstName: string = 'Base';
let baseSecondName: string = 'Base';

class Base {
    public age_: number = baseAge;
    public firstName: string = baseFirstName;
    public secondName: string = baseSecondName;
    get age(): number {
        return this.age_;
    }
    set age(age: number) {
        this.age_ = age;
    }
    setFirstName(name: string) {
        this.firstName = name;
    }
    getFirstName(): string {
        return this.firstName;
    }
    getFirstName(prefix: string): string {
        return prefix + '.' + this.firstName;
    }
    getSecondName(): string {
        return this.secondName;
    }
    getFullName(): string {
        return this.firstName + '.' + this.secondName;
    }
}

function testField(obj: Base): boolean {
    return obj.firstName === baseFirstName;
}

function testCoverField(obj: Base, val: string): boolean {
    return obj.secondName === val;
}

function testGetter(obj: Base): boolean {
    return obj.age === baseAge;
}

function testSetter(obj: Base, val: number): boolean {
     return obj.age === val;
}

function testMethod(obj: Base): boolean {
    return obj.getFirstName() === baseFirstName;
}

function testSetMethod(obj: Base, val: string): boolean {
    return obj.getFirstName() === val;
}

function testSuperMethod(obj: Base, val: string): boolean {
    return obj.getSecondName() === val;
}

function testOverloadMethod(obj: Base, prefix: string): boolean {
    return obj.getFirstName() === baseFirstName && obj.getFirstName(prefix) === prefix + '.' + baseFirstName;
}

function testOverrideMethod(obj: Base, val: string): boolean {
    return obj.getFullName() === obj.firstName + '.' + obj.secondName + '.' + val;
}
