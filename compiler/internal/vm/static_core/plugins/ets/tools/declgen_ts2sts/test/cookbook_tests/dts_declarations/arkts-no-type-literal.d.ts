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

export declare let x: {
    a: number;
    b: number;
};
export declare let a: {
    asnfda: string;
};
export declare let person: {
    name: string;
    age: number;
};
export type Point = {
    x: number;
    y: number;
};
export declare let myPoint: Point;
export declare function printPerson(person: {
    name: string;
    age: number;
}): void;
export declare let newPerson: {
    name: string;
    age: number;
};
export type Config = {
    readonly id: number;
    name?: string;
    enabled: boolean;
};
export declare let config: Config;
export type Calculator = {
    add: (a: number, b: number) => number;
    subtract: (a: number, b: number) => number;
};
export declare let myCalculator: Calculator;
export type User = {
    name: string;
    address: {
        street: string;
        city: string;
        zip: string;
    };
};
export declare let user: User;
