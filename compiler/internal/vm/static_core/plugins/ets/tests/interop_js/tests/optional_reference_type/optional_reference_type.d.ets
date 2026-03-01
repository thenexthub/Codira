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
export declare function fnWithAnyParams(arr?: Any[]): object;
type City = 'Moscow' | 'London' | 'Paris';
export declare function fnWithLiteralParam(arr?: City[]): City | string;
export declare function fnWithExtraSetParam(arr?: Any): Any;

export declare interface TestUserType {
    id: number;
    name: string;
    age: number;
    address?: AddressType;
}
export declare interface ObjectTypeWithArr {
    id: number;
    arr?: string[];
}

// export type UserPick = Any;
export declare interface UserPick {
    name: string;
    address?: AddressType;
};
// export type UserOmit = Any;
export declare interface UserOmit {
    name: string;
    address?: AddressType;
};

// export type UserPartial = Partial<TestUserType>;
export declare interface UserPartial {
    id?: number;
    name?: string;
    age?: number;
    address?: AddressType;
}

export declare function fnWithSubsetPick(obj: UserPick): string;
export declare function fnWithSubsetOmit(obj: UserOmit): string;
export declare function fnWithSubsetPartial(obj: UserPartial): string;

export type UnionArrOrObj = string[] | Any;
export declare function fnWithUnionParam(obj?: UnionArrOrObj): string;
export declare function fnWithUserClass(obj?: TestUserClass): string;
export declare function fnWithUserInterface(obj?: TestUserType): string;
export declare function fnWithAnyParamObject(obj: ObjectTypeWithArr): Any;
export declare class TestUserClass {
    id: number;
    name: string;
    constructor(id: number, name: string);
}
export declare interface AddressType {
    street: string;
    city: string;
}
