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
    export interface ListString {
        /**
        * Pushes a value to the begin of the List
        *
        * @param e value to push
        */
        pushFront(e: String): void;

        /**
        * Pops a value from the begin of the List
        *
        * @returns popped value
        */
        popFront(): String;

        /**
        * Pushes a value to the end of the List
        *
        * @param e value to push
        */
        pushBack(e: String): void;

        /**
        * Pops a value from the end of the List
        *
        * @returns popped value
        */
        popBack(): String;

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
        at(index: int): String;

        /**
        * Sets an element at the specified index
        *
        * @param index element position
        *
        * @param e new value
        */
        set(index: int, e: String): void;

        /**
        * Checks if an element is in the List
        *
        * @param e value to find
        *
        * @returns true if value present, false otherwise
        */
        has(e: String): boolean;

    /*  Code below is blocked by internal issue with lambdas #9994
        forEach(fn: (e: String): String): ListString;
        map(fn: (e: String): ): ;
        fold(combine: (lhs: String, rhs: String): String): String;
        foldWith(combine: (lhs: String, rhs: String): , initVal: ): ;
        filter(filterCond: (e: String): boolean): ListString;
        sort(comparator: (lhs: String, rhs: String): boolean): ListString;
    */
    }
}
