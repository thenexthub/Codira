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
 * @Import { Values } from '../bu_d2s/libValues.ets'
 * @Tags interop, s2s
 */
class equality {

    noEqual: string = '==';
    equal: string = '!=';

    /**
     * @Benchmark
     */
    compareNumber(): boolean {
        return this.comparison(Values.etsNum, Values.etsNum, this.equal);
    }

    /**
     * @Benchmark
     */
    comparePrimitive(): boolean {
        let res = true;
        res &= this.comparison(Values.etsBigint, Values.etsBigint, this.equal);
        res &= this.comparison(Values.etsBoolean, Values.etsBoolean, this.equal);
        res &= this.comparison(Values.etsNull, Values.etsNull, this.equal);
        res &= this.comparison(Values.etsUndefined, Values.etsUndefined, this.equal);
        return res;
    }

    /**
     * @Benchmark
     */
    compareString(): boolean {
        return this.comparison(Values.etsString, Values.etsString, this.equal);
    }

    /**
     * @Benchmark
     */
    compareObject(): boolean {
        let res = true;
        res &= this.comparison(Values.etsObj, Values.etsObj, this.equal);
        res &= this.comparison(Values.etsObj, Values.etsObjCopy, this.equal);
        res &= this.comparison(Values.etsObj, Values.etsObjOther, this.noEqual);
        return res;
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
