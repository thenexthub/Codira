
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

package generic_static.test;

const stsInt = 1;
type Union = int | string | number;
interface IUser {
    num: int;
}
class Base {
    a = stsInt;
}
type ArrayInt = Array<Int>;
const BaseObj: Base;
static {
    BaseObj = { a: stsInt };
}

// NOTE: issue (1835) fix Literal type
// type Literal = 'start' | 'stop';

// class LiteralStatic {
//     static get<T extends Literal>(value: T): T {
//         return value;
//     }
// }

// function genericLiteralCallFromSts(arg: Literal): Literal {
//     return LiteralStatic.get(arg);
// }

// NOTE: issue (18007) fix constrain
// type TupleT = [int, boolean];

// class ExtraSetStatic {
//     static get<T extends TupleT>(value: T): int {
//         return value[0];
//     }
// }

// function genericExtraSetCallFromSts(arg: TupleT): int {
//     return ExtraSetStatic.get(arg);
// }

class GenericStatic {

    static get<T>(value: T): T {
        return value;
    }
}

function genericStaticMethodCallFromSts<T>(arg: T): T {
    return GenericStatic.get(arg);
}

class GenericExtends extends GenericStatic { };


function genericClassExtendsCallFromSts<T>(arg: T): T {
    return GenericExtends.get(arg);
}

class SubsetByRefStatic {
    static get<T extends ArrayInt>(value: T): T {
        return value;
    }
}

function genericSubsetByRefStaticCallFromSts(arg: ArrayInt): ArrayInt {
    return SubsetByRefStatic.get(arg);
}


// NOTE: issue (18007) fix constrain
// class SubsetByValueStatic {
//     static get<T extends typeof BaseObj.a>(value: T): T {
//         return value;
//     }
// }

// function genericSubsetByValueStaticCallFromSts(arg: typeof BaseObj.a): typeof BaseObj.a {
//     return SubsetByValueStatic.get(arg);
// }

class UnionStatic {
    static get<T extends Union>(value: T): T {
        return value;
    }
}

function genericUnionStaticCallFromSts(arg: Union): Union {
    return UnionStatic.get(arg);
}

class User {
    constructor(value: int) { }
}

class UserClassStatic {
    static get<T extends User>(value: T): T {
        return value;
    }
}

function userClassFromSts(arg: User): User {
    return UserClassStatic.get(arg);
}


class InterfaceStatic {
    static get<T extends IUser>(value: T): T {
        return value;
    }
}

function userInterfaceFromSts(arg: IUser): IUser {
    return InterfaceStatic.get(arg);
}
