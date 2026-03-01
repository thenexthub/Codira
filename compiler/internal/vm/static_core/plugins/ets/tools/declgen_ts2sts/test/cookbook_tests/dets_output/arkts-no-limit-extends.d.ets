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
import { C1 } from './include/lib';
export type CustomPromise<T> = Any;
export declare class Animal {
    move(): void;
}
export declare class Dog extends Animal {
    bark(): void;
}
export declare class test extends C1 {
    penWidth: number;
}
export type testPromise = Any;
export declare interface Shape {
    color: string;
}
export declare interface PenStroke {
    penWidth: number;
}
export declare interface Square extends Shape, PenStroke {
    sideLength: number;
}
export type SquareTest = Any;
