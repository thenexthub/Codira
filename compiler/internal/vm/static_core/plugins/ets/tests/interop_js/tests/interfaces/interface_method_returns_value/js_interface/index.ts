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

export const STRING_VALUE = 'Panda';
export const INT_VALUE = Number.MAX_SAFE_INTEGER;

export const FLOAT_VALUE = Math.PI;
export const BOOLEAN_VALUE = true;
export const NULL_VALUE = null;
export const UNDEFINED = (<Record<string, undefined>>{})?.value;

export enum ENUM_VALUE {
    OPTION_ONE,
    OPTION_TWO,
    OPTION_THREE
};

type AnyType = {};

type TEST_TUPLE = [string, number];
const TUPLE_VALUE: TEST_TUPLE = ['abc', 123];

export interface AbstractInterface {
    value: string;
};

export interface ExtensionInterface {
    extendingProperty: number;
};

export type TFunctionReturnsType<T> = () => T;
export type TFunctionReturnsAny = TFunctionReturnsType<AnyType>;
export type TFunctionReturnsString = TFunctionReturnsType<string>;
export type TFunctionReturnsNumber = TFunctionReturnsType<number>;
export type TFunctionReturnsBigInt = TFunctionReturnsType<bigint>;
export type TFunctionReturnsBoolean = TFunctionReturnsType<boolean>;
export type TFunctionReturnsStringLiteral = TFunctionReturnsType<'Panda'>;
export type TFunctionReturnsNumberLiteral = TFunctionReturnsType<1234>
export type TFunctionReturnsEnum = TFunctionReturnsType<typeof ENUM_VALUE>;
export type TFunctionReturnsNull = TFunctionReturnsType<null>;
export type TFunctionReturnsUndefined = TFunctionReturnsType<undefined>;
export type TFunctionReturnsFunctionOfType<T> = (arg?: T) => TFunctionReturnsType<T>;

export class GenericInterface {
    getAny(): AnyType {
        const randomValues = [STRING_VALUE, INT_VALUE, FLOAT_VALUE, BOOLEAN_VALUE, NULL_VALUE,
            UNDEFINED, new Array(2).fill(' '), <Record<string, AnyType>>{}];
        const index = Math.round(Math.random() * randomValues.length);
        return randomValues[index];
    }

    public getInt: TFunctionReturnsNumber = function(): Object {
        return Number(INT_VALUE);
    }

    public getNegativeInt: TFunctionReturnsNumber = function(): Object {
        return Number(INT_VALUE) * -1;
    }

    public getInfinity: TFunctionReturnsNumber = function(): Object {
        return Number.POSITIVE_INFINITY;
    }

    public getNegativeInfinity: TFunctionReturnsNumber = function(): Object {
        return Number.NEGATIVE_INFINITY;
    }

    public getNanAsNumber: TFunctionReturnsNumber = function(): Object {
        return Number.NaN;
    }

    public getBigInt: TFunctionReturnsBigInt = function (): Object {
        return BigInt(INT_VALUE);
    }

    public getFloat: TFunctionReturnsNumber = function (): Object {
        return FLOAT_VALUE;
    }

    public getString: TFunctionReturnsString = function (): Object {
        return STRING_VALUE;
    }

    public getBoolean: TFunctionReturnsBoolean = function(): Object {
        return BOOLEAN_VALUE;
    }

    public getTuple(): TEST_TUPLE {
        return ['tuple_item_0', 1];
    }

    public getGeneric<T extends {}>(arg: T): T & ExtensionInterface {
        return { ...arg, extendingProperty: 0 };
    }

    public getFunctionReturningType<T>(arg?: T): () => T {
        return (arg?: T) => 0 as T;
    }

    public getEnum: TFunctionReturnsEnum = function(): Object {
        return ENUM_VALUE;
    }

    public getNull: TFunctionReturnsNull = function(): Object {
        return NULL_VALUE;
    }

    public getUndefined: TFunctionReturnsUndefined = function(): Object {
        return UNDEFINED;
    }
}

export const genericInterfaceImplementation: GenericInterface = {
    getAny() {
        const randomValues = [
            STRING_VALUE,
            INT_VALUE,
            FLOAT_VALUE,
            BOOLEAN_VALUE,
            NULL_VALUE,
            UNDEFINED,
            new Array(2).fill(' '),
            <Record<string, AnyType>>{}
        ];
        const index = Math.round(Math.random() * randomValues.length);
        return randomValues[index];
    },

    getInt(){
        return Number(INT_VALUE);
    },

    getBigInt(){
        return BigInt(INT_VALUE);
    },

    getFloat(){
        return FLOAT_VALUE;
    },

    getNegativeInt(){
        return Number(INT_VALUE)*-1;
    },

    getInfinity(){
        return Number.POSITIVE_INFINITY;
    },

    getNegativeInfinity(){
        return Number.NEGATIVE_INFINITY;
    },

    getNanAsNumber(){
        return Number.NaN;
    },

    getString(){
        return STRING_VALUE;
    },

    getBoolean(){
        return BOOLEAN_VALUE;
    },

    getTuple() {
        return TUPLE_VALUE;
    },

    getGeneric<T extends {}>(arg: T): T & ExtensionInterface {
        return { ...arg, extendingProperty: Object.keys(arg ?? {}).length * -1 }
    },

    getFunctionReturningType<T>(arg?: T) {
        return () => {
            console.log('reached')
            switch (typeof arg) {
                case 'string':
                    return (<string>arg).toLowerCase() + (<string>arg).toUpperCase() as T;
                case 'bigint':
                    return (<bigint>arg) / BigInt(2) as T;
                case 'number':
                    return (<number>arg) * Math.PI as T;
                default:
                    return 'undefined' as T;
            }
        }
    },

    getEnum() {
        return ENUM_VALUE;
    },

    getNull() {
        return NULL_VALUE;
    },

    getUndefined() {
        return UNDEFINED;
    }
};

export const testObject = {
    a: 1;
};
