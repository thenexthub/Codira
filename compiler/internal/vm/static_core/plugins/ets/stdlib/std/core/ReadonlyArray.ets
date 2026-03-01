/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

package std.core;

export interface ReadonlyArray<out T> extends ConcatArray<T> {
    // concat(...items: (T | ConcatArray<T>)[]): T[];
    // concat(...items: (T | ConcatArray<T>)[]): Array<T>;
    concat(...items: FixedArray<ConcatArray<@types.SkipVarianceCheckDontUseItWillBeRemoved T>>): Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T>;
    // Typing and previous overload have to cover it

    entries(): IterableIterator<[number, @types.SkipVarianceCheckDontUseItWillBeRemoved T]>;

    every(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean;

    filter(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T>;

    find(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined;

    findLast(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined;

    findIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int;

    findLastIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int;

    // NOTE(ivan-tyulyandin): TBD, FlatArray is non-subset type
    // flat<A, D extends number = 1>(
    //     this: A,
    //     depth?: D,
    // ): FlatArray<A, D>[];

    // NOTE(ivan-tyulyandin): TBD
    // flatMap<U, This = undefined>(
    //     callback: (this: This, value: T, index: number, array: T[]) => U | ReadonlyArray<U>,
    //     thisArg?: This,
    // ): U[];

    forEach(action: (value: T, index: int, array: ReadonlyArray<T>) => void): void;

    join(separator?: string): string;

    includes(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T, fromIndex?: int): boolean;

    indexOf(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T, fromIndex?: int): int;

    keys(): IterableIterator<number>;

    lastIndexOf(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T): int;

    lastIndexOf(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T, fromIndex?: int): int;

    map<U>(mapper: (value: T, index: int, array: ReadonlyArray<T>) => U): Array<U>;

    reduce(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => @types.SkipVarianceCheckDontUseItWillBeRemoved T): T;

    reduce<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U;

    reduceRight(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => @types.SkipVarianceCheckDontUseItWillBeRemoved T): T;

    reduceRight<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U;

    slice(start?: int, end?: int): Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T>;

    some(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean;

    // toLocaleString(): string;
    // All objects in ArkTS have such method, no need to define it here

    // toString(): string;
    // All objects in ArkTS have such method, no need to define it here

    values(): IterableIterator<T>;
}
