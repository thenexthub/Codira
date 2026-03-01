/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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
class Student {
    constructor(age: int, name: string) {
        this.age = age;
        this.name = name;
    }
    age: int;
    name: string;
}

class Worker {
    private _age: int;
    private _name: string;

    constructor(age: int, name: string) {
        this._age = age;
        this._name = name;
    }

    get age(): int {
        return this._age;
    }

    set age(value: int) {
        this._age = value;
    }

    get name(): string {
        return this._name;
    }

    set name(value: string) {
        this._name = value;
    }
}

function getArray(): string[] {
    let a: string[] = ["Leechy", "Rocket", "Bird", "hello", "world"];
    return a;
}

function GetNumericArray(): int[] {
    let a: int[] = [1, 2, 3, 4, 5];
    return a;
}