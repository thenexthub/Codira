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
type TA10 = number;
type TA20 = string;
type TA30 = boolean;
type TA40 = bigint;
type TA50 = object;
type TA60 = symbol;
type TA70 = void;
type TA80 = undefined;
type TA90 = any;
type TA100 = unknown;
type TA110 = never;

type TA200 = () => void;
type TA210<V> = (arg: V) => void;
type TA220<V, W extends string> = (arg: V) => W;
type TA230<V = object, W extends string = '123' | '345'> = (arg: V) => W; // type parameter default values are unsupported
type TA240 = (...a: number[]) => void;
type TA250 = (a?: number) => void;

type TA300 = Promise<string>;
type TA310 = Promise<string>[];

type TA400 = Pick<Promise<void>, 'then'>;
type TA410 = Omit<Promise<void>, ''>;
type TA420 = Omit<Promise<void>, ''>;

type TA500 = 123;
type TA510 = 'abc';
type TA520 = null;

type TA600 = [number, string, number];

type TA700 = Record<string, unknown> | null;
type TA710 = "aaa" | "bbb" | "ccc" | "ddd";
type TA720 = number | string;
type TA730 = Promise<string> | string;
type TA740 = "aaa" | TA710;

type TA810 = { x: number; y: string };
type TA820 = { [p: number]: string };
type TA830 = { (arg: number): string };

type TA900 = readonly number[];
type TA910 = keyof { x: number; y: string };

type TA1000 = typeof setTimeout;
type TA1010 = ReturnType<typeof setTimeout>;

type ARK1 = null | number | string | boolean | Uint8Array | Float32Array | bigint | Int8Array | Int16Array | Uint16Array | Uint32Array | Int32Array | BigInt64Array | BigUint64Array | Float64Array;

// 交叉类型别名
type User = {
    id: number;
    name: string;
} & { isActive: boolean };

// 泛型类型别名
type ApiResponse<T> = {
    data: T;
    status: number;
};

export type AnimationItem = {
    name: string;
    name1: string;
    play11(name?: string,name1?: string): void;
};