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

export declare function evaluateNumber(): void;

export declare function evaluateNumber(v0: int, v1: int): int;

export declare function evaluateNumber(v0: string, v1: string): string;

export declare function emptyFunction(): void;

export declare function evaluateObject(obj: ExampleClass): ExampleClass;

export declare function evaluateArray(arr: FixedArray<int>, size: int): FixedArray<int>;

export declare class ExampleClass {
    v0: JSValue;
    v1: JSValue;

    constructor(v0: JSValue, v1: JSValue);

    static evaluateNumber(v2: int, v3: int): int;

    static evaluateNumber(v2: string, v3: string): string;

    static evaluateArray(arr: FixedArray<int>, size: int): FixedArray<int>;

    instanceEvaluateNumber(): JSValue;

    evaluateObject(obj: ExampleClass): ExampleClass;

    getV0(): FixedArray<int>;

    getV1(): FixedArray<int>;

    static emptyMethod(): void;
};

export declare class ClassWithEmptyConstructor {
    v0: JSValue;
    v1: JSValue;

    constructor();

    verifyProperties(v0Expected: JSValue, v1Expected: JSValue): JSValue;

    getV0(): FixedArray<int>;

    getV1(): FixedArray<int>;
}

/**
 * TS Namespace for reference
 * 
 * export namespace MyNamespace {
 *     class Kitten {
 *         constructor(id: number, name: string) {
 *             this.id = id;
 *             this.name = name;
 *         }
 *
 *         id: number;
 *         name: string;
 *     }
 *
 *     export function createKitten(id: number, name: string) {
 *         return new Kitten(id, name);
 *     }
 * }
 */


// TODO: support namespaces in front-end

export declare class Kitten {
    constructor(id: int, name: string);
    id: int;
    name: string;
}

export declare class MyNamespace {
    static createKitten(id: int, name: string): Kitten;
}