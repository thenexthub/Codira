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

export type AnyAlias<T> = any;
export type UnknownAlias<T> = unknown;
export type SymbolAlias<T> = symbol;
export type FunctionAlias<T> = (arg: T) => void;
export type ObjectAlias<T> = { value: T };
export type CallSignatureAlias<T, R> = (arg: T) => R;
class MyClass<T> {
    constructor(public value: T) {}
}
export type ConstructorSignatureAlias<T> = new (value: T) => MyClass<T>;
export type IndexSignatureAlias<T> = { [index: number]: T };
export type IntersectionAlias<T, U> = T & U;
export type TypeA = { x: number };
export type TypeB = { y: string };
export type Result = IntersectionAlias<TypeA, TypeB>;
export type Person = { name: string, age: number };
export type ConditionalAlias<T, U, V> = T extends U ? V : U;
export type Result2 = ConditionalAlias<number, string, boolean>;
export type MappedAlias<T> = { [K in keyof T]: T[K] };
export type Person2 = { name: string, age: number };
export type NewPerson = MappedAlias<Person>;
export type TemplateLiteralAlias<T extends string> = `${T}_id`;
export type Result3 = TemplateLiteralAlias<'A' | 'B'>;
export type PickAlias<T, K extends keyof T> = Pick<T, K>;
export type Person3 = { name: string, age: number };
export type NameOnly = PickAlias<Person, 'name'>;
export type MappedArray<T, U> = Array<(item: T) => U>;
export type ArrayFilter<T> = (item: T) => boolean;
export type FunctionTransformer<Input, Output, NewInput, NewOutput> = (
    fn: (input: Input) => Output
) => (newInput: NewInput) => NewOutput;
export type FunctionComposer<Input, Middle, Output> = (
    f: (input: Input) => Middle,
    g: (middle: Middle) => Output
) => (input: Input) => Output;
export type ObjectPropertyMapper<T, U> = {
    [K in keyof T]: (value: T[K]) => U;
};
export type Fox<T> = {
    value: T;
};