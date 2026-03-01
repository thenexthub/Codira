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
export function fnWithAnyParams(name:string, surname?:{}):string {
    return surname ? `${name}; ${surname}` : name;
}

export interface ObjectTypeWithAny {
    name:string;
    surname?:{};
}

export function fnWithAnyParamsObject(obj:ObjectTypeWithAny):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}


export function fnWithLiteralParam(name:string, surname?:'Smith' | 'Dou'):string {
     return surname ? `${name}; ${surname}` : name;
}

export interface ObjectTypeWithLiteral {
    name:string;
    surname?:'Smith' | 'Dou';
}

export function fnWithLiteralObjectParam(obj:ObjectTypeWithLiteral):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}

export function fnWithExtraSetParam(name:string, surname?:unknown):string {
    return surname ? `${name}; ${surname}` : name;
}

export interface ObjectTypeWithExtraSet {
    name:string;
    surname?:unknown;
}

export function fnWithExtraSetObjectParam(obj:ObjectTypeWithExtraSet):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}

export interface TestUserType {
    name:string;
    id?:number | string;
    city?:string;
}

export interface BasicTestUser {
    name: string;
    city?: number | string;
}

export function fnWithSubSetParam(obj:BasicTestUser):string {
    return obj.city ? `${obj.name}; ${obj.city}` : obj.name;
}

export function fnWithUnionParam(id?:string | number):string | number {
    return id ? id : 'id not found';
}

export function fnWithUnionObjectParam(obj:TestUserType):string | number {
    return obj.id ? obj.id : 'id not found';
}

export interface TestSecondUserType {
    name:string;
    surname?: string;
    id:number;
    city:string;
}

export interface TestUserTypeReduseProperty {
    name:string;
    surname?: string;
}

export function fnWithSubSetReduseParam(obj:TestUserTypeReduseProperty):string {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}

export interface PartialTestUserType {
    name?:string;
    surname?: string;
    id?:number;
    city?:string;
}

export function fnWithSubSetPartialParam(obj:PartialTestUserType):string | undefined {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}