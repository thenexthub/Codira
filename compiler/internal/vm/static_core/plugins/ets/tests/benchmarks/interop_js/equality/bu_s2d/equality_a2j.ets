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

/**
 * @State
 * @Import { jsNumber,jsBigint,jsBoolean,jsNull,jsUndefined } from '../../shared/js/libValues.js'
 * @Import { jsString,jsObj,jsObjCopy,jsObjOther } from '../../shared/js/libValues.js'
 * @Tags interop, s2d
 */
class equality {

    noEqual: string = '==';
    equal: string = '!=';

    /**
     * @Benchmark
     */
    compareNumber(): boolean {
        return this.comparison<number>(jsNumber, jsNumber, this.equal);
    }

    /**
     * @Benchmark
     */
    comparePrimitive(): boolean {
        let res = true;
        res &= this.comparison(jsBigint, jsBigint, this.equal);
        res &= this.comparison(jsBoolean, jsBoolean, this.equal);
        res &= this.comparison(jsNull, jsNull, this.equal);
        res &= this.comparison(jsUndefined, jsUndefined, this.equal);
        return res;
    };

    /**
     * @Benchmark
     */
    compareString(): boolean {
        return this.comparison(jsString, jsString, this.equal);
    }

    /**
     * @Benchmark
     */
    compareObject(): boolean {
        let res = true;
        res &= this.comparison(jsObj, jsObj, this.equal);
        res &= this.comparison(jsObj, jsObjCopy, this.equal);
        res &= this.comparison(jsObj, jsObjOther, this.noEqual);
        return res;
    }


    comparisonObject(valueA: JSValue, valueB: JSValue, compare: string): boolean {
        if (compare == this.equal && valueA !== valueB) {
            throw Error();
        }
        if (compare == this.noEqual && valueA === valueB) {
            throw Error();
        }
        return true;
    }

    comparison<T>(valueA: T, valueB: T, compare: string): boolean {
        if (compare == this.equal && valueA != valueB) {
            throw Error();
        }
        if (compare == this.noEqual && valueA == valueB) {
            throw Error();
        }
        return true;
    }

}
