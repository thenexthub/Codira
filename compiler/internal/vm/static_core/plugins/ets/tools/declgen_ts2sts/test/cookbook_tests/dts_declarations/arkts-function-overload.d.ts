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
    add1(a: number, b: number): number;
    add1(a: number, b: number, c: number): number;
    add1(numbers: number[]): string; 
}

export declare class Calculator2 {
  add2(a: number, b: number): number;
  add2(a: number, b: number, c: number): number;
  add2(numbers: number[]): number;
  add2(...args: any[]): any;
}

export declare interface Formatter1 {
    format1(value: number): any;
    format1(value: Date): number;
    format1(value: string, prefix: string): string;
}

export declare interface Formatter2 {
    format2(value: string): string;
    format2(value: number, precision?: number): string;
    format2(value: Date, formatString?: string): string;
}

export declare interface Processor1 {
  process1(input: string): string;
  process1(input: number): number;
  process1(input: boolean): boolean;
}

export declare interface DataProcessor1 {
  processData1(data: string[]): void;
  processData1(data: { key: string; value: number }[]): void;
  processData1(data: number[]): void;
}

export declare interface Logger1 {
  log1(message: string): void;
}

export declare interface Validator1 {
  validate1(input: string): boolean;
  validate1(input: number): boolean;
}

export declare interface Transformer1 {
  transform1<T>(input: T[]): T[];
  transform1<T>(input: T): T;
}

export declare interface Formatter3 {
    format1(value: string): any;
    format1(value: number, precision: number): string;
    format1(value: Date, formatString?: string): string;
}

export declare class Calculator3 {
  add3(a: number, b: number): number;
  subtract3(a: number, b: number): number;
}

