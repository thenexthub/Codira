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
export declare function fnWithAnyParams(name: string, surname?: Any): string;
export declare function fnWithAnyParamsObject(obj: ObjectTypeWithAny): string;
export declare function fnWithLiteralParam(name: string, surname?: 'Smith' | 'Dou'): string;
export declare function fnWithLiteralObjectParam(obj: ObjectTypeWithLiteral): string;
export declare function fnWithExtraSetParam(name: string, surname?: Any): string;
export declare function fnWithExtraSetObjectParam(obj: ObjectTypeWithExtraSet): string;
export declare function fnWithUnionParam(id?: string | number): string | number;
export declare function fnWithUnionObjectParam(obj: TestUserType): string | number;
export declare interface BasicTestUser {
    name: string;
    city?: number | string;
}
export declare interface TestUserTypeReduseProperty {
    name: string;
    surname?: string;
}
export declare interface PartialTestUserType {
    name?: string;
    surname?: string;
    id?: number;
    city?: string;
}

export declare function fnWithSubSetParam(obj: BasicTestUser): string;
export declare function fnWithSubSetReduseParam(obj: TestUserTypeReduseProperty): string;
export declare function fnWithSubSetPartialParam(obj: PartialTestUserType): string | undefined;
export declare interface ObjectTypeWithAny {
    name: string;
    surname?: Any;
}
export declare interface ObjectTypeWithLiteral {
    name: string;
    surname?: string;
}
export declare interface ObjectTypeWithExtraSet {
    name: string;
    surname?: Any;
}
export declare interface TestUserType {
    name: string;
    id?: number | string;
    city?: string;
}
export declare interface TestSecondUserType {
    name: string;
    surname?: string;
    id: number;
    city: string;
}
