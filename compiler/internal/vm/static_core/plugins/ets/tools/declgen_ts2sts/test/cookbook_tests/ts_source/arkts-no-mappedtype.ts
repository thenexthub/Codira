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


export type Class1<T> = {
    new(...args: Object[]): T;
}

export type Nullable<T> = {
    [K in keyof T]: T[K] | null;
};

export type Person = {
    name: string;
    age: number;
    address: string;
};

export type ReadonlyPerson = {
    readonly [K in keyof Person]: Person[K];
};

export type ArrayElementMap<T extends any[], U> = {
    [K in keyof T]: U;
};
export type NumberArray = [1, 2, 3];
export type StringArray = ArrayElementMap<NumberArray, string>; 
export type Book = {
    title: string;
    author: string;
    year: number;
    price: number;
};
export type SelectedKeys = 'title' | 'author';
export type PartialBook = {
    [K in SelectedKeys]: Book[K];
};

export class Class2<T> {
    a : Person;
}

export type a = Person;