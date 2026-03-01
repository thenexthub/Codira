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

package std.containers;

export namespace containers {
    /**
    * List interface
    */
    export interface ListInt {
        /**
        * Pushes a value to the begin of the List
        *
        * @param e value to push
        */
        pushFront(e: Int): void;

        /**
        * Pops a value from the begin of the List
        *
        * @returns popped value
        */
        popFront(): Int;

        /**
        * Pushes a value to the end of the List
        *
        * @param e value to push
        */
        pushBack(e: Int): void;

        /**
        * Pops a value from the end of the List
        *
        * @returns popped value
        */
        popBack(): Int;

        /**
        * Returns number of elements in the List
        *
        * @returns number of elements in the List
        */
        size(): int;

        /**
        * Returns an element at the specified index
        *
        * @param index element position
        *
        * @returns an element
        */
        at(index: int): Int;

        /**
        * Sets an element at the specified index
        *
        * @param index element position
        *
        * @param e new value
        */
        set(index: int, e: Int): void;

        /**
        * Checks if an element is in the List
        *
        * @param e value to find
        *
        * @returns true if value present, false otherwise
        */
        has(e: Int): boolean;

    /*  Code below is blocked by internal issue with lambdas #9994
        forEach(fn: (e: Int): Int): ListInt;
        map(fn: (e: Int): ): ;
        fold(combine: (lhs: Int, rhs: Int): Int): Int;
        foldWith(combine: (lhs: Int, rhs: Int): , initVal: ): ;
        filter(filterCond: (e: Int): boolean): ListInt;
        sort(comparator: (lhs: Int, rhs: Int): boolean): ListInt;
    */
    }
}
