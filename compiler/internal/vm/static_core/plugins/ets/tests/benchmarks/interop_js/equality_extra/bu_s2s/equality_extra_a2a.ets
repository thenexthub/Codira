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
 * @Import { ExtraHolder, foo, fooOther } from '../bu_d2s/libExtraHolder.ets'
 * @Tags interop, s2s
 */
class equalityExtra {

    noEqual: string = '==';
    equal: string = '!=';
    extra: ExtraHolder = new ExtraHolder();

    /**
     * @Benchmark
     */
    compareArrays(): boolean {
        let res: boolean = true;
        res &= this.comparison(this.extra.etsArr, this.extra.etsArr, this.equal);
        res &= this.comparison(this.extra.etsArr, this.extra.etsArrCopy, this.equal);
        res &= this.comparison(this.extra.etsArr, this.extra.etsArrOther, this.noEqual);
        return res;
    }

    /**
     * @Benchmark
     */
    compareArrowFuncs(): boolean {
        let res: boolean = true;
        res &= this.comparison(this.extra.etsArrowFunction, this.extra.etsArrowFunction, this.equal);
        res &= this.comparison(this.extra.etsArrowFunction, this.extra.etsArrowFunctionCopy, this.equal);
        res &= this.comparison(this.extra.etsArrowFunction, this.extra.etsArrowFunctionOther, this.noEqual);
        return res;
    }

    /**
     * @Benchmark
     */
    compareFuncs(): boolean {
        let res: boolean = true;
        res &= this.comparison(foo, fooOther, this.noEqual);
        // [TID 00657a] E/runtime: Unhandled error: escompat.Error
        // [TID 00657a] E/runtime: at escompat.Error.$_invoke (etsstdlib.ets:114:0)
        res &= this.comparison(foo, foo, this.equal);
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
