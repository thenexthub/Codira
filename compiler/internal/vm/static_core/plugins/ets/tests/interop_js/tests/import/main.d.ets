'use static'
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
export declare let tsNumber: number;
export declare let tsString: string;
export declare let tsTrue: boolean;
export declare let tsFalse: boolean;
export declare let tsUndefined: Any;
export declare let tsNull: null | number;
export declare let tsSymbol: Any;
export declare let tsNewSymbol: Any;
export declare let tsBigInt: bigint;
export declare function increment(): number;
export declare function concatString(): string;
export declare function changeTrue(): boolean;
export declare function changeFalse(): boolean;
export declare function changeUndefined(): number;
export declare function changeNull(): number;
export declare function changeSymbol(): Any;
export declare function changeBigInt(): bigint;
export declare function changeTsClass(obj: TsClass): void;
declare interface Iobj {
    value: string;
}
export declare const tsObject: Iobj;
export declare function changeObject(): void;
export declare const arr: number[];
export declare function changeArray(): void;
export declare let tsFunction: () => number;
export declare function changeFunction(): void;
export declare let tsDate: Date;
export declare function changeDate(): void;
export declare let tsReg: RegExp;
export declare function changeRegExp(): void;
export default declare function defaultFu(): number;
export declare class TsClass {
    name: string;
}
export declare class User {
    id: number;
}
