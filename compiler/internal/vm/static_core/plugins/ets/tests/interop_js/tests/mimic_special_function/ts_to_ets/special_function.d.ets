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
export declare let arrObj: boolean[];
export declare let ARRNUM1: number;
export declare let ARRNUM2: number;
export declare let ARRNUM3: number;
export declare let testArr1: number[];
export declare let testArr2: number[];
export declare function InsertArray(num: boolean): void;
export declare function GetArrLength(): number;
export declare function SetArrLength(len: number): void;
export declare function IsEqual(): boolean;
export declare class User {
    userName_: string;
    userIdx_: number;
    constructor(userName: string, userIdx: number);
    get userName(): string;
    set userName(newUsername: string);
    get userIdx(): number;
    set userIdx(newIdx: number);
    GetUserName(): string;
    GetUserIdx(): number;
}