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
export const a: number;

export const b: (a: number, b: number) => number;

export let c: string;

export declare const d: boolean;

declare const e: number;

export const f: number, g: string;

export var h: any;

export const i: { x: number; y: string };

export const j: string[];

export const k: string | number | null;

declare namespace MyNamespace {
    export const namespaceVar: string;
    export const anotherVar: number;
    export let mutableVar: boolean;
}

export declare namespace OuterNamespace {
    export const outerVar: number;
    
    namespace InnerNamespace {
        export const innerVar: string;
        export let innerMutable: number;
    }
}

export declare namespace MixedNamespace {
    export const constantValue: number;
    export function someFunction(): void;
    export class SomeClass {
        prop: string;
    }
    export interface SomeInterface {
        id: number;
    }
}

export declare namespace RegularNamespace {
    export const shouldGetDeclare = 100;
}

declare namespace Level1 {
    export namespace Level2 {
        export namespace Level3 {
            export const deepVar: string;
        }
    }
}



export function simpleFunction(): void;

export function functionWithParams(a: number, b?: string): boolean;

export declare function alreadyDeclared(): void;

declare function privateFunction(): number;

declare namespace FunctionNamespace {
    export function namespaceFunction(): void;
    export function anotherFunction(param1: string): number;
}

export function genericFunction<T>(value: T): T;

export function asyncFunction(): Promise<void>;

declare namespace PrivateFunctionNamespace {
    function internalFunction(): void;
    export function publicFunction(): string;
}

export namespace ExportedNamespace {
    export const exportedVar: string;
    export function exportedFunction(): void;
    export class ExportedClass {
        prop: string;
    }
    export interface ExportedInterface {
        id: number;
    }
}

export class ExportedClass {
    prop: string;
}