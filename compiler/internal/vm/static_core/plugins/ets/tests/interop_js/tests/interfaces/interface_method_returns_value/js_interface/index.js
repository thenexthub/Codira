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
'use strict';

let __assign = (this && this.__assign) || function (...args) {
    __assign = Object.assign || function(t) {
        for (let s, i = 1, n = args.length; i < n; i++) {
            s = args[i];
            for (let p in s) {
                if (Object.prototype.hasOwnProperty.call(s, p)) {
                    t[p] = s[p];
                }
            }
        }
        return t;
    };
    return __assign.apply(this, args);
};

let _a;
export const STRING_VALUE = 'Panda';
export const INT_VALUE = Number.MAX_SAFE_INTEGER;

export const FLOAT_VALUE = Math.PI;
export const BOOLEAN_VALUE = true;
export const NULL_VALUE = null;
export const UNDEFINED = (_a = {}) === null || _a === void 0 ? void 0 : _a.value;

export const ENUM_VALUE = {
    OPTION_ONE: 0,
    OPTION_TWO: 1,
    OPTION_THREE: 2
};

const TUPLE_VALUE = ['abc', 123];

export class GenericInterface {
    getAny() {
        const randomValues = [STRING_VALUE, INT_VALUE, FLOAT_VALUE, BOOLEAN_VALUE, NULL_VALUE,
            UNDEFINED, new Array(2).fill(' '), {}];
        const index = Math.round(Math.random() * randomValues.length);
        return randomValues[index];
    }

    getInt() {
        return Number(INT_VALUE);
    }

    getNegativeInt() {
        return Number(INT_VALUE) * -1;
    }

    getInfinity() {
        return Number.POSITIVE_INFINITY;
    }

    getNegativeInfinity() {
        return Number.NEGATIVE_INFINITY;
    }

    getNanAsNumber() {
        return Number.NaN;
    }

    getBigInt() {
        return BigInt(INT_VALUE);
    }

    getFloat() {
        return FLOAT_VALUE;
    }

    getString() {
        return STRING_VALUE;
    }

    getBoolean() {
        return BOOLEAN_VALUE;
    }

    getTuple() {
        return ['tuple_item_0', 1];
    }

    getGeneric(arg) {
        return { ...arg, extendingProperty: 0 };
    }

    getFunctionReturningType(arg) {
        return function (arg) { return 0; };
    }

    getEnum() {
        return ENUM_VALUE;
    }

    getNull() {
        return NULL_VALUE;
    }

    getUndefined() {
        return UNDEFINED;
    }
}

export const genericInterfaceImplementation = {
    getAny() {
        const randomValues = [
            STRING_VALUE,
            INT_VALUE,
            FLOAT_VALUE,
            BOOLEAN_VALUE,
            NULL_VALUE,
            UNDEFINED,
            new Array(2).fill(' '),
            {}
        ];
        const index = Math.round(Math.random() * randomValues.length);
        return randomValues[index];
    },

    getInt() {
        return Number(INT_VALUE);
    },

    getBigInt() {
        return BigInt(INT_VALUE);
    },

    getFloat() {
        return FLOAT_VALUE;
    },

    getNegativeInt() {
        return Number(INT_VALUE) * -1;
    },

    getInfinity() {
        return Number.POSITIVE_INFINITY;
    },

    getNegativeInfinity() {
        return Number.NEGATIVE_INFINITY;
    },

    getNanAsNumber() {
        return Number.NaN;
    },

    getString() {
        return STRING_VALUE;
    },

    getBoolean() {
        return BOOLEAN_VALUE;
    },

    getTuple() {
        return TUPLE_VALUE;
    },

    getGeneric: function (arg) {
        return __assign(__assign({}, arg),
            { extendingProperty: Object.keys(arg !== null && arg !== void 0 ? arg : {}).length * -1 });
    },

    getFunctionReturningType: function (arg) {
        return function () {
            print('reached');
            switch (typeof arg) {
                case 'string':
                    return arg.toLowerCase() + arg.toUpperCase();
                case 'bigint':
                    return arg / BigInt(2);
                case 'number':
                    return arg * Math.PI;
                default:
                    return 'undefined';
            }
        };
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
    a: 1
};
