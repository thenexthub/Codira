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
export declare namespace ns {
    export function foo(_this: number): void;
}
export declare function greet(_this: string): number;
export declare const fun: (_this: number, a: number) => number;
export declare const foo: (_short: number, _long: string, _int: number) => number;
export declare const fox: (_byte: number, _char: string) => number;
export declare const corge: (_float: string, _double: string) => string;
export declare function grault(_boolean: number): void;
export declare function waldo(_abstract: string, _async: string): void;
export declare function alex(_Any: string): void;
export declare function bella(_bigint: number, _long: number): void;
export declare function charlie(_interface: string): void;
export declare function ethan(_Byte: number): void;
export declare function fiona(_Char: boolean): void;
export declare function george(_Double: string): void;
export declare function hannah(_Float: number): void;
export declare function isaac(_Int: boolean): void;
export declare function julia(_Long: string): void;
export declare function kevin(_number: number): void;
export declare function mason(_Object: string): void;
export declare function nora(_Short: number): void;
export declare function oliver(_string: boolean): void;
export declare function ruby(_native: boolean): void;
export declare function samuel(_overload: string): void;
export declare function ursula(_constructor: boolean): void;
export declare class A {
    value: number;
    foo(_this: A, a: number): void;
}
export declare interface Itest {
    foo(_this: number): void;
}
