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

type Literal = 'start' | 'stop';
type Subset = { a: number };
type UnionType = number | string;
interface IUser {
    tsInt: number;
}
type TupleType = [number, boolean];

export const tsInt = 1;
export const tsString = 'str';
export const tsBool = true;
export const tsArr = [tsInt];
export const tsLiteralStart: Literal = 'start';
export const tsLiteralStop: Literal = 'stop';
export const tsTuple: TupleType = [tsInt, tsBool];
export const BaseObj: Subset = { a: tsInt };
export const user: IUser = { tsInt };

export class GenericStatic {

    static get<T>(value: T): T {
        return value;
    }
}

export function genericStaticMethodCallFromTs<T>(arg: T): T {
    return GenericStatic.get(arg);
}

export class GenericExtends extends GenericStatic { };


export function genericClassExtendsCallFromTs<T>(arg: T): T {
    return GenericExtends.get(arg);
}

export class LiteralStatic {
    static get<T extends Literal>(value: T): T {
        return value;
    }
}

export function genericLiteralCallFromTs(arg: Literal): Literal {
    return LiteralStatic.get(arg);
}


export class ExtraSetStatic {
    static get<T extends TupleType>(value: T): T[0] {
        return value[0];
    }
}

export function genericExtraSetCallFromTs(arg: TupleType): number {
    return ExtraSetStatic.get(arg);
}

export class SubsetByRefStatic {
    static get<T extends Subset>(value: T): T {
        return value;
    }
}

export function genericSubsetByRefStaticCallFromTs(arg: Subset): Subset {
    return SubsetByRefStatic.get(arg);
}


export class SubsetByValueStatic {
    static get<T extends typeof BaseObj.a>(value: T): T {
        return value;
    }
}

export function genericSubsetByValueStaticCallFromTs(arg: typeof BaseObj.a): typeof BaseObj.a {
    return SubsetByValueStatic.get(arg);
}

export class UnionStatic {
    static get<T extends UnionType>(value: T): T {
        return value;
    }
}

export function genericUnionStaticCallFromTs(arg: UnionType): UnionType {
    return UnionStatic.get(arg);
}

export class User {
    constructor(public value: number) { }
}

export class UserClassStatic {
    static get<T extends User>(value: T): T {
        return value;
    }
}

export const checkInstance = <T>(a: T, b: Constructor<T>): boolean => {
    return a instanceof b;
};

export function userClassFromTs(arg: User): User {
    return UserClassStatic.get(arg);
}

export class InterfaceStatic {
    static get<T extends IUser>(value: T): T {
        return value;
    }
}

export function userInterfaceFromTs(arg: IUser): IUser {
    return InterfaceStatic.get(arg);
}

type Constructor<T> = {
    new (...args: never[]): T;
};
