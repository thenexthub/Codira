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
 * @Import { jsArr,jsArrCopy,jsArrOther,jsArrowFoo } from '../../shared/js/libFunctions.js'
 * @Import { jsArrowFooCopy,jsArrowFooOther,jsFoo,jsFooOther } from '../../shared/js/libFunctions.js'
 */

/**
 * @State
 * @Tags interop, s2d
 */
class equalityExtra {

    noEqual: string = '==';
    equal: string = '!=';


    /**
     * @Benchmark
     */
    compareArrays(): boolean {
        let res: boolean = true;
        res &= this.comparison(jsArr, jsArr, this.equal);
        res &= this.comparison(jsArr, jsArrCopy, this.equal);
        res &= this.comparison(jsArr, jsArrOther, this.noEqual);
        return res;
    }

    /**
     * @Benchmark
     */
    compareArrowFuncs(): boolean {
        let res: boolean = true;
        res &= this.comparison(jsArrowFoo, jsArrowFoo, this.equal);
        res &= this.comparison(jsArrowFoo, jsArrowFooCopy, this.equal);
        res &= this.comparison(jsArrowFoo, jsArrowFooOther, this.noEqual);
        return res;
    }

    /**
     * @Benchmark
     */
    compareFuncs(): boolean {
        let res: boolean = true;
        res &= this.comparison(jsFoo, jsFooOther, this.noEqual);
        res &= this.comparison(jsFoo, jsFoo, this.equal);
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
