'use static'
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
export type AnyAlias<T> = Any;
export type UnknownAlias<T> = Any;
export type SymbolAlias<T> = Any;
export type FunctionAlias<T> = (arg: T) => void;
export type ObjectAlias<T> = Any;
export type CallSignatureAlias<T, R> = (arg: T) => R;
export type ConstructorSignatureAlias<T> = Any;
export type IndexSignatureAlias<T> = Any;
export type IntersectionAlias<T, U> = Any;
export type TypeA = Any;
export type TypeB = Any;
export type Result = IntersectionAlias<TypeA, TypeB>;
export type Person = Any;
export type ConditionalAlias<T, U, V> = Any;
export type Result2 = ConditionalAlias<number, string, boolean>;
export type MappedAlias<T> = Any;
export type Person2 = Any;
export type NewPerson = MappedAlias<Person>;
export type TemplateLiteralAlias<T extends string> = Any;
export type Result3 = TemplateLiteralAlias<'A' | 'B'>;
export type PickAlias<T, K extends Any> = Any;
export type Person3 = Any;
export type NameOnly = PickAlias<Person, 'name'>;
export type MappedArray<T, U> = Array<(item: T) => U>;
export type ArrayFilter<T> = (item: T) => boolean;
export type FunctionTransformer<Input, Output, NewInput, NewOutput> = (fn: (input: Input) => Output) => (newInput: NewInput) => NewOutput;
export type FunctionComposer<Input, Middle, Output> = (f: (input: Input) => Middle, g: (middle: Middle) => Output) => (input: Input) => Output;
export type ObjectPropertyMapper<T, U> = Any;
export type Fox<T> = Any;
declare class MyClass<T> {
    value: T;
    constructor(value: T);
}
