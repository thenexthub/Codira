/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

export let x: {a: number, b: number} = {a: 0, b: 0};

export let a={
    "asnfda":"fadsadsf"
}

export let person: { name: string; age: number } = { name: 'John', age: 30 };

export type Point = { x: number; y: number };
export let myPoint: Point = { x: 10, y: 20 };

export function printPerson(person: { name: string; age: number }): void {
    console.log(`${person.name} is ${person.age} years old.`);
}

export let newPerson = { name: 'Alice', age: 25 };

export type Config = {
    readonly id: number;
    name?: string;
    enabled: boolean;
};

export let config: Config = { id: 1, enabled: true };

export type Calculator = {
    add: (a: number, b: number) => number;
    subtract: (a: number, b: number) => number;
};

export let myCalculator: Calculator = {
    add: (a, b) => a + b,
    subtract: (a, b) => a - b
};

export type User = {
    name: string;
    address: {
        street: string;
        city: string;
        zip: string;
    };
};

export let user: User = {
    name: 'Bob',
    address: {
        street: '123 Main St',
        city: 'Anytown',
        zip: '12345'
    }
};