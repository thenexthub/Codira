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
export declare class Calculator1 {
    add1(param0: number | number[], param1?: number, param2?: number): number | string;
}
export declare class Calculator2 {
    add2(param0: number | number[] | Any[], param1?: number, param2?: number): number | Any;
}
export declare class Calculator3 {
    add3(a: number, b: number): number;
    subtract3(a: number, b: number): number;
}
export declare interface Formatter1 {
    format1(param0: number | Date | string, param1?: string): Any | number | string;
}
export declare interface Formatter2 {
    format2(param0: string | number | Date, param1?: number | string): string;
}
export declare interface Processor1 {
    process1(param0: string | number | boolean): string | number | boolean;
}
export declare interface DataProcessor1 {
    processData1(param0: string[] | Any[] | number[]): void;
}
export declare interface Logger1 {
    log1(message: string): void;
}
export declare interface Validator1 {
    validate1(param0: string | number): boolean;
}
export declare interface Transformer1 {
    transform1<T>(param0: T[] | T): T[] | T;
}
export declare interface Formatter3 {
    format1(param0: string | number | Date, param1?: number | string): Any | string;
}
